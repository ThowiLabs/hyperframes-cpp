---
name: hyperframes-cpp
description: Build, narrate and render 9:16 HTML/CSS/GSAP videos using HyperFrames C++, browser discovery and native H.264/AAC MP4. Use for vertical videos, animations, narration, TTS pronunciation, media projects, and renderer troubleshooting.
---

# HyperFrames C++ — video production skill

HyperFrames C++ is a C++17 command-line video renderer with a native compositor and a browser-assisted HTML/CSS/JavaScript compositor. It can encode MP4/H.264/AAC and AVI without running FFmpeg during export. It does **not** synthesize speech, read generic JSON projects, or decode arbitrary audio formats by itself. The `browser` theme requires an installed Chromium, Chrome or Edge.

**Never put a user's video project, photographs, narration, generated video or third-party GSAP inside the engine's `main` branch.** Use a separate project directory.

## Find the real executable first

Do not hardcode the engine's repository path. Check the terminal command before searching installation folders.

**Linux shell:**
```bash
command -v hyperframes-cpp-native || command -v hyperframes-cpp || command -v hyperframes-linux
```

If none: inspect `~/.local/bin/`, `~/bin/`, an explicit user-supplied path, `./build/native-linux/hyperframes-cpp-native` and `./hyperframes-linux`. For binaries obtained from [GitHub Releases](https://github.com/ThowiLabs/hyperframes-cpp/releases/latest), validate checksums and obtain approval before installing. A binary named `hyperframes-linux` may be an older **AVI-only** build.

**Windows PowerShell:**
```powershell
Get-Command hyperframes-cpp-native.exe -ErrorAction SilentlyContinue
Get-Command hyperframes-cpp.exe -ErrorAction SilentlyContinue
```

If not found: look for `build\native-windows\hyperframes-cpp-native.exe`, `%LOCALAPPDATA%\HyperFramesCPP`, other explicitly supplied locations or the release. Properly quote executable paths with spaces. **Windows x64 has been cross-compiled, not verified at runtime on Windows.** A macOS build is not published.

Use the *verified* path in all subsequent commands:
```bash
HFC="$(command -v hyperframes-cpp-native)"
"$HFC" --browser-check
```
If `command -v` fails, stop and locate the installed binary; do not execute the above blindly.

## Browser auto-detection

The renderer locates Chromium-family executables in this priority:
1. `HFC_BROWSER` override, a full path or executable name on `PATH`.
2. Standard commands from `PATH`.
3. Known Chrome, Chromium, Brave or Edge installations on Linux/Windows.
4. A **running browser process**, using `/proc/<pid>/exe` on Linux or Windows process-enumeration APIs.

This **discovers the executable path only**, and does not attach to or hijack browser tabs or profiles. The renderer launches its own headless browser profile.

```bash
"$HFC" --browser-check
HFC_BROWSER=/usr/bin/chromium "$HFC" --browser-check
```

On Windows: `$env:HFC_BROWSER = 'C:\Program Files\Google\Chrome\Application\chrome.exe'`. Do not assume Firefox/Safari are supported. Browser mode runs local HTML JavaScript: only render trusted projects, preferably without elevated privileges.

## Recommended project structure

```text
my-video/
  index.html                  # HTML, CSS, local JS, scenes and animation timeline
  timeline.json               # optional metadata used by scripts; NOT parsed by CLI
  storyboard.md               # optional voice + scene plan
  assets/
    img/                      # licensed photos or graphics
    audio/
      segment-01.wav
      segment-02.wav
    vendor/
      gsap.min.js             # user-provided under separate license
  CREDITS.md                  # author, image source URL, license
  output.mp4                  # generated result
```

**Important:** the current CLI reads the **HTML** and audio directory. A `timeline.json` may support your own preparation script but there is **no** `hyperframes-cpp-native render project.json` command today.

Use HTML sections with `class="scene"`, `data-start`, `data-duration` (seconds), sequential start times, and optional text hooks `.kicker`, `.wordline`, `.sub`, `.barlabel` for the native themes. Browser compositions should define a paused GSAP timeline at `window.__timelines.main` with `pause()` and `seek(seconds)`. Keep all scripts/photos local. Prefer 720×1280 portrait for mobile videos. **The GSAP library is not part of the engine**, has a separate license and must be supplied correctly by the video project.

Read [production workflows](references/WORKFLOWS.md) when making a new composition.

## TTS and phonetic narration — critical

A TTS model may pronounce technical strings, numbers, acronyms, units and dates badly. **Maintain two versions** of every scene's important words:
- *On screen*: correct code and exact spellings (`ssh -p 2222`, `C++`, `sshd -t`, `H.264`).
- *Spoken text*: write **how it should sound**, not necessarily how it is spelled (`puerto dos mil doscientos veintidós`, `ce más más`, `ese ese hache de, guion te`).

Examples to verify by **listening to actual samples**:

| Display | Narration suggestion in Spanish |
|---|---|
| SSH | «ese ese hache» (or tested alternative) |
| C++ | «ce más más» |
| Python | «páiton», only if voice mispronounces Python |
| 22 | «veintidós» |
| 2222 | «dos mil doscientos veintidós» |
| 2026 | «dos mil veintiséis» |
| 720 × 1280 | «setecientos veinte por mil doscientos ochenta» |
| sshd -t | «ese ese hache de, guion te» |
| 127.0.0.1 | say each number group and dot clearly |
| 2.5 GB | «dos punto cinco gigabytes» |
| 01/06/2026 | say full day/month/year to avoid ambiguity |

Pronunciation varies by voice/provider. Test acronyms/English product names, write phonetically only where necessary, and do **not** alter on-screen terminal commands to imitate speech. For accents, choose deliberate phrasing, spelling and punctuation. Write numbers **as words** when needed to control TTS; distinguish decimals, versions, ports, prices and dates.

For each scene, synthesize the voice, measure its **actual** length, add a modest pause (~0.6–1s), then fill `data-start` and `data-duration`. Never invent durations. Check first, middle and final sections. TTS may output MP3/M4A; the current native audio input expects `segment-01.wav` etc., **mono PCM signed 16-bit, 24 kHz**. Audio conversion or TTS may use external tools (FFmpeg/SoX) in **preproduction**. Only the C++ video export is FFmpeg-free. The engine is not a multitrack audio editor; pre-mix music/effects if necessary.

## CLI reference

Check available features (full MP4 build versus older AVI-only build):

```bash
"$HFC" --inspect "/absolute/project/index.html"
"$HFC" --browser-check
"$HFC" --browser-frame "/absolute/project/index.html" 4 "/absolute/project/preview.png"
```

Render HTML/GSAP + WAV voice to 720×1280 MP4 24fps:

```bash
"$HFC" "/absolute/project/index.html" "/absolute/project/assets/audio" \
  "/absolute/project/output.mp4" 24 h264 browser 720x1280
```

Quick 5-second preview:

```bash
"$HFC" "/absolute/project/index.html" "/absolute/project/assets/audio" \
  "/absolute/project/preview.mp4" 24 h264 browser 720x1280 5
```

Native AVI without a browser:

```bash
"$HFC" "/absolute/project/index.html" - \
  "/absolute/project/output.avi" 24 mjpeg classic 360x640
```

Positional syntax:
```text
hyperframes-cpp-native COMPOSITION.html AUDIO_DIRECTORY|- OUTPUT.avi|mp4
  [fps:1..60] [mjpeg|rgb24|rle8|h264] [classic|news|html|browser]
  [WIDTHxHEIGHT] [clip-seconds]
```

Do not confuse `html` (limited litehtml compositor) with `browser` (real CSS/JS). Browser screenshots run at the video's local time, not wall clock time. High-fps outputs may encode slowly.

## Quality controls and delivery

1. Validate `--inspect` timing and supported scene count.
2. Capture screenshots at an early, middle and late timestamp; check image licenses, layout, contrast and clipped text.
3. Listen to every narrated WAV; verify pronunciation, codec, length, clipping and quiet gaps.
4. Export a short preview before doing a full video.
5. Wait for renderer process exit **zero**; do not call a partially written MP4 complete.
6. Inspect real output codecs/frame count/duration and try decoding/playback. External tools (`ffprobe`/player) may be used for QA; they are not invoked by the C++ encoder.
7. Include `CREDITS.md` and licensed media provenance (Wikimedia, Unsplash, Pexels etc.). Respect restrictions for third-party images, GSAP, music and TTS providers.
8. Return a working link to the rendered video and optionally an editable ZIP. Keep media out of the core engine repo.

## Further reading and limitations

[Releases](https://github.com/ThowiLabs/hyperframes-cpp/releases/latest) · [Browser guide](https://github.com/ThowiLabs/hyperframes-cpp/blob/main/docs/BROWSER_MODE.md) · [Third-party notices](https://github.com/ThowiLabs/hyperframes-cpp/blob/main/THIRD_PARTY_NOTICES.md).

The native MP4 encoder is experimental. The browser is required for browser mode, and generic sites without supported timed scenes are not accepted. The CLI cannot currently decode MP3/M4A input, mix arbitrary audio tracks, synthesize TTS or render from a generic JSON manifest directly. H.264/AAC patent obligations and GSAP licensing require independent review.
