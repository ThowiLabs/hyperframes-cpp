#pragma once
#include <cstdlib>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <cctype>
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <tlhelp32.h>
#include <cwctype>
#else
#include <unistd.h>
#endif

// Locate a Chrome DevTools Protocol capable executable.
// We only identify an executable from a running process: HyperFrames starts
// its own fresh browser profile and never attaches to somebody else's session.
namespace browser {
namespace fs = std::filesystem;

inline std::string verified_path(const fs::path& candidate) {
    std::error_code ec;
    if (!fs::is_regular_file(candidate, ec)) return {};
#ifndef _WIN32
    if (access(candidate.c_str(), X_OK) != 0) return {};
#endif
    auto path = fs::weakly_canonical(candidate, ec);
    // Windows native filesystem paths are UTF-16. Expose an UTF-8 string
    // consistently and decode it back to UTF-16 before CreateProcessW.
    if (ec) return candidate.u8string();
    return path.u8string();
}

inline std::string find_in_path(const std::string& name) {
    if (name.empty()) return {};
#ifdef _WIN32
    const auto namePath=fs::u8path(name);
#else
    const auto namePath=fs::path(name);
#endif
    if (namePath.has_parent_path() || namePath.is_absolute())
        return verified_path(namePath);
#ifdef _WIN32
    const auto extension = namePath.extension().empty() ? L".exe" : L"";
    const DWORD length = GetEnvironmentVariableW(L"PATH", nullptr, 0);
    if (length == 0 || length > 32768) return {};
    std::wstring paths(length, L'\0');
    DWORD copied=GetEnvironmentVariableW(L"PATH", paths.data(),length);
    if (!copied || copied>=length) return {};
    paths.resize(copied);
    size_t start=0;
    while (start<=paths.size()) {
        auto end=paths.find(L';',start);
        if (end==std::wstring::npos) end=paths.size();
        if (end>start) {
            auto found=verified_path(fs::path(paths.substr(start,end-start)) /
                                     (namePath.wstring()+extension));
            if (!found.empty()) return found;
        }
        if (end==paths.size()) break;
        start=end+1;
    }
#else
    const char* env=std::getenv("PATH");
    const std::string paths=env?env:"";
    size_t start=0;
    while (start<=paths.size()) {
        auto end=paths.find(':',start);
        if (end==std::string::npos) end=paths.size();
        if (end>start) {
            auto found=verified_path(fs::path(paths.substr(start,end-start))/namePath);
            if (!found.empty()) return found;
        }
        if (end==paths.size()) break;
        start=end+1;
    }
#endif
    return {};
}

#if defined(_WIN32)
inline bool recognized_browser(const std::wstring& name) {
    std::wstring lower = name;
    std::transform(lower.begin(),lower.end(),lower.begin(),
                   [](wchar_t c){return std::towlower(c);});
    return lower==L"chrome.exe" || lower==L"msedge.exe" ||
           lower==L"chromium.exe" || lower==L"brave.exe";
}
inline std::string discover_running_process() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return {};
    PROCESSENTRY32W entry{};entry.dwSize=sizeof(entry);
    std::string found;
    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (!recognized_browser(entry.szExeFile)) continue;
            HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,entry.th32ProcessID);
            if (!process) continue; // Other-user/system processes may be inaccessible.
            std::vector<wchar_t> pathname(32768);
            DWORD len=static_cast<DWORD>(pathname.size());
            if (QueryFullProcessImageNameW(process,0,pathname.data(),&len)) {
                found=verified_path(fs::path(std::wstring(pathname.data(),len)));
            }
            CloseHandle(process);
            if (!found.empty()) break;
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return found;
}
#else
inline bool recognized_browser(std::string name) {
    std::transform(name.begin(),name.end(),name.begin(),
                   [](unsigned char c){return static_cast<char>(std::tolower(c));});
    return name=="chrome" || name=="chromium" || name=="chromium-browser" ||
           name=="google-chrome" || name=="msedge" || name=="microsoft-edge" ||
           name=="brave" || name=="brave-browser";
}
inline std::string discover_running_process() {
    std::error_code ec;
    for (auto it=fs::directory_iterator("/proc",fs::directory_options::skip_permission_denied,ec);
         !ec && it!=fs::directory_iterator(); it.increment(ec)) {
        const auto pid=it->path().filename().string();
        if (pid.empty() || !std::all_of(pid.begin(),pid.end(),
                  [](unsigned char c){return std::isdigit(c)!=0;})) continue;
        auto executable=fs::read_symlink(it->path()/"exe",ec);
        if (ec) {ec.clear();continue;}
        if (!recognized_browser(executable.filename().string())) continue;
        const auto found=verified_path(executable);
        if (!found.empty()) return found;
    }
    return {};
}
#endif

inline std::string discover() {
    // An explicit override always takes priority. If invalid, the caller
    // receives a clear error instead of silently switching to another browser.
#if defined(_WIN32)
    wchar_t overridePath[32768]{};
    const DWORD overrideLength=GetEnvironmentVariableW(L"HFC_BROWSER",overridePath,32768);
    if (overrideLength) {
        if (overrideLength>=32768) return {};
        return find_in_path(fs::path(std::wstring(overridePath,overrideLength)).u8string());
    }
#else
    if (const char* value=std::getenv("HFC_BROWSER");value && *value)
        return find_in_path(value);
#endif
#ifdef _WIN32
    const std::vector<std::string> names={"chrome","msedge","chromium","brave"};
#else
    const std::vector<std::string> names={
        "chromium","chromium-browser","google-chrome-stable",
        "google-chrome","microsoft-edge","microsoft-edge-stable","brave-browser"
    };
#endif
    for (const auto& name:names) {
        auto result=find_in_path(name);
        if (!result.empty()) return result;
    }
#ifdef _WIN32
    const wchar_t* variables[]={L"PROGRAMFILES",L"PROGRAMFILES(X86)",
                                L"LOCALAPPDATA",L"ProgramW6432"};
    const std::vector<fs::path> suffixes={
        "Google/Chrome/Application/chrome.exe",
        "Microsoft/Edge/Application/msedge.exe",
        "Chromium/Application/chrome.exe",
        "BraveSoftware/Brave-Browser/Application/brave.exe"
    };
    for (const wchar_t* variable:variables) {
        wchar_t root[32768]{};
        const DWORD len=GetEnvironmentVariableW(variable,root,32768);
        if (!len || len>=32768) continue;
        for (const auto& suffix:suffixes) {
            auto result=verified_path(fs::path(root)/suffix);
            if (!result.empty()) return result;
        }
    }
#else
    for (const char* candidate:{
        "/usr/bin/chromium","/usr/bin/google-chrome",
        "/usr/bin/microsoft-edge","/usr/bin/brave-browser",
        "/usr/local/bin/chromium","/usr/local/bin/google-chrome",
        "/opt/google/chrome/chrome","/opt/microsoft/msedge/msedge",
        "/opt/brave.com/brave/brave","/snap/bin/chromium"
    }) {
        auto result=verified_path(candidate);
        if (!result.empty()) return result;
    }
#endif
    // Last resort: resolve a running browser's process executable, not its
    // open user profile or CDP port. No process is hijacked or modified.
    return discover_running_process();
}
}
