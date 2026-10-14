# HyperFrames C++ v1.0.0

First documented engine release with **Linux x64** and **Windows x64** binaries, plus a portable **Agent Skill for Claude Code and compatible skill-based tools**.

## Contents

- Native H.264/AAC MP4 and AVI output from C++17, without invoking FFmpeg at render/export time.
- Four compositor themes: classic, news, limited litehtml and browser-assisted HTML/CSS/JavaScript.
- Automatic browser discovery on Linux/Windows from `HFC_BROWSER`, `PATH`, common installation directories and running process executable paths. Only Chrome/Chromium/Edge/Brave-based Chromium engines are supported.
- Browser mode uses a temporary headless browser profile, not an existing interactive session.
- Agent Skill covering executable detection, HTML/GSAP scene metadata, narration/TTS spelling, pronunciation of numbers and abbreviations, audio preparation, debugging and quality checks.
- Synthetically generated tests; **no video projects, photos, third-party voice assets or finished MP4s in the engine repository**.

## Download artifacts

- `hyperframes-cpp-v1.0.0-linux-x64.tar.gz`: Linux static binary, README, notices, skill and installer.
- `hyperframes-cpp-v1.0.0-windows-x64.zip`: Windows x64 cross-compiled binary, README, notices, skill and installer.
- `hyperframes-cpp-v1.0.0-agent-skill.zip`: Standalone Agent Skill (SKILL.md + references).
- `SHA256SUMS.txt`: Integrity hashes for the archive files.

The GitHub integration currently allows creating releases and pushing branches, but does not expose direct binary release-attachment upload. Until that permission is enabled, verified binaries and SHA256 checksums are hosted in the separate `release-assets-v1.0.0` branch and linked from the release notes. They are **not GitHub Release attached assets**; this limitation is explicitly disclosed rather than hidden. Once authenticated `gh release upload` is available, the verified files can be attached without changing the `v1.0.0` tag.

## Quick start

Linux:

```bash
tar -xzf hyperframes-cpp-v1.0.0-linux-x64.tar.gz
cd hyperframes-cpp-v1.0.0-linux-x64
./bin/hyperframes-cpp-native --browser-check
bash ./install-skill.sh --claude --agents
```

Windows: extract `hyperframes-cpp-v1.0.0-windows-x64.zip`, and run `bin\hyperframes-cpp-native.exe --browser-check` using PowerShell, or install the Agent Skill by copying `skills\hyperframes-cpp\` to your agent's skill directory.

The renderer is a CLI; it is **not** a video editor UI. Projects require their own local HTML scene definitions and media, with narration supplied as PCM mono 16-bit WAV at 24 kHz (or `-` for silence). See the Skill for templates and practical commands.

## Compatibility and important limitations

- Linux binary was compiled and tested in a Debian environment. Windows x64 binary was cross-compiled; **Windows runtime has not been directly tested on a Windows computer**.
- Browser-assisted rendering **requires an installed Chromium-family browser**. Firefox and Safari are not supported. Auto-detection does not hijack an active browser session: it only resolves the executable and starts a separate headless instance.
- Native input audio currently accepts WAV PCM; MP3/M4A decoding, TTS synthesis and arbitrary JSON scene manifests are **not** features of this release.
- Native minih264/VisualOn AAC/minimp4 codec stack is experimental. Review codecs' patent/distribution obligations before commercial redistribution.
- GSAP and font libraries are subject to their own licensing; GSAP is **not bundled**, and video compositions must supply their own appropriate licensed assets.
- Do not render untrusted HTML with high privileges: the browser executes JS and can access network resources.

Detailed API and build instructions are in README, docs/BROWSER_MODE.md and the packaged SKILL.md.
