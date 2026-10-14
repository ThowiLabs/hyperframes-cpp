# HyperFrames C++

**An experimental C++17 video renderer with native AVI/MP4 encoding and optional browser-assisted HTML/CSS/JavaScript capture.**

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C)
![License](https://img.shields.io/badge/license-Apache--2.0-blue)
![Linux & Windows](https://img.shields.io/badge/platform-Linux%20%7C%20Windows-444)
![Experimental](https://img.shields.io/badge/status-experimental-orange)

> **Independent project.** This is not the official HyperFrames implementation or a general-purpose HTML/browser engine. Input HTML must follow the currently supported scene timing conventions.

HyperFrames C++ combines a bounded HTML scene importer, a native software renderer, a browser-driven renderer (Chrome DevTools Protocol), and statically linked video/audio encoders. It exports AVI or MP4 **without invoking FFmpeg**.

**This repository contains engine source, build scripts, documentation and technical regression tests only.** It deliberately does not bundle video projects, finished videos, narration, demonstration photography or GSAP assets.

## Capabilities

| Feature | Status |
|---|---|
| Native AVI RGB24 / RLE8 / MJPEG with PCM | Implemented |
| Native MP4 H.264 + AAC | Experimental, implemented |
| Up to 60 FPS, time-indexed animation sampling | Implemented; throughput varies |
| Portrait output from 180×320 to 1080×1920 | Implemented, fixed 9:16 aspect |
| HTML scene metadata importer | Supported subset only |
| Native TrueType/UTF-8 text | Implemented |
| CSS/HTML layout through litehtml | Experimental; limited CSS features |
| Chrome/Chromium/Edge detection + CDP capture | Implemented; needs a browser installed |
| CSS/JS/GSAP fidelity in browser mode | Uses original scripts supplied by the user |
| Native MP3/M4A decoding | Not implemented; currently expects PCM WAV |
| General websites, arbitrary JS DOM and aspect ratios | Not supported |
| Windows x64 | Cross-compiles; real Windows runtime testing pending |

The browser backend renders frames using the locally installed Chromium/Chrome/Edge and lets the C++ encoder write H.264/AAC MP4. The other renderers work without a browser. Neither path executes FFmpeg.

## Build

Requirements: C++17 GCC, Python 3 for build-time font embedding, DejaVu Sans and DejaVu Sans Bold installed locally, and Git/CMake to build optional third-party backends. MinGW-w64 is needed for Windows cross-compilation.

On Debian/Ubuntu:

```bash
sudo apt update
sudo apt install g++ gcc python3 fonts-dejavu-core git cmake make
# Optional browser mode:
sudo apt install chromium
# Windows cross-compilation:
sudo apt install gcc-mingw-w64-x86-64-win32 g++-mingw-w64-x86-64-win32
```

Clone and build the complete experimental Linux engine:

```bash
git clone https://github.com/ThowiLabs/hyperframes-cpp.git
cd hyperframes-cpp
./scripts/build-native-mp4.sh
```

The main executable is `build/native-linux/hyperframes-cpp-native`. The build script fetches *pinned* litehtml, minih264, minimp4 and vo-aacenc source revisions and statically links them. For a lightweight AVI-only build without additional downloaded codec libraries:

```bash
./scripts/build-linux.sh
# Output: ./hyperframes-linux
```

For Windows x64 cross-compilation:

```bash
./scripts/build-native-windows.sh
# Output: ./build/native-windows/hyperframes-cpp-native.exe
```

Raw TTF files and the generated `font_embedded.hpp` are not tracked. The binary embeds necessary glyph data at build time. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for font and codec notices.

## Render your own composition

The importer expects a local HTML file with consecutive `section.scene` blocks containing `data-start` and `data-duration`. Supported classic themes can also read `kicker`, `wordline`, `sub`, `barlabel` and selected CSS pixel properties.

Minimal scene structure (illustration of **syntax**, not a bundled video project):

```html
<section class="scene" data-start="0" data-duration="5" style="--a:#47def5">
  <div class="kicker">SECTION</div>
  <div class="wordline">Example headline</div>
  <div class="sub">Scene description</div>
  <div class="barlabel">CHAPTER</div>
</section>
```

Each scene normally has a corresponding WAV input (`segment-01.wav`, `segment-02.wav`, etc.): mono, signed 16-bit PCM, 24 kHz. Pass `-` for silence. Input HTML, audio and images remain **outside this repository**, under your own project directory.

```bash
# Inspect scene times and titles
./build/native-linux/hyperframes-cpp-native --inspect /path/to/project/index.html

# Check for a compatible browser
./build/native-linux/hyperframes-cpp-native --browser-check

# Render a browser frame at exactly t = 4 seconds (PNG)
./build/native-linux/hyperframes-cpp-native --browser-frame \
  /path/to/project/index.html 4 /tmp/frame.png

# Export native MP4: 720x1280, 24 FPS, original CSS/JS from installed browser
./build/native-linux/hyperframes-cpp-native \
  /path/to/project/index.html /path/to/project/audio \
  output.mp4 24 h264 browser 720x1280

# Or export AVI using the native software theme
./build/native-linux/hyperframes-cpp-native \
  /path/to/project/index.html - \
  output.avi 24 mjpeg classic 360x640
```

The optional final positional argument is a preview duration in seconds, e.g. `... browser 720x1280 5`.

`browser` mode requires a complete **trusted local HTML project** with all its CSS, scripts and images present. Its JavaScript must expose `window.__timelines.main` with a `seek(t)` method to support deterministic animation capture. GSAP is not distributed. The browser backend currently supports Chromium-family browsers only; override detection with `HFC_BROWSER=/absolute/path/to/browser`.

For a complete reference of browser behavior, security limitations and Windows build requirements, see [docs/BROWSER_MODE.md](docs/BROWSER_MODE.md). For codec, timeline and layout details, see [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## Repository structure

```text
main.cpp                  CLI, audio input and video muxing
browser_detection.hpp     Browser discovery
browser_capture.hpp       CDP/WebSocket frame capture
html_composition.hpp      Bounded HTML scene/timing importer
render_scene.hpp          Native classic renderer
render_news.hpp           Native news renderer
html_scene_engine.hpp     Experimental litehtml renderer
native_mp4.hpp            H.264/AAC MP4 writer
native_codecs/            Native codec C compilation units
animation_timeline.hpp    Deterministic easing and tracks
output_scale.hpp          Vertical output resampler
vendor/                   Licenses and source headers
scripts/                  Build and dependency fetch scripts
tests/                    Unit tests and temporary synthetic fixtures
contexto/, tareas/        Engineering history and outstanding work
docs/                     Architecture, browser guide and roadmap
```

## Tests

```bash
bash tests/smoke-linux.sh
bash tests/browser-integration.sh build/native-linux/hyperframes-cpp-native
```

The smoke suite **generates its own synthetic HTML and PCM audio in a temporary directory**. The browser test uses a small, self-contained technical HTML fixture; no third-party animation library, photographs or finished video projects are committed. The browser test skips when no compatible installed browser is available. If installed, `ffprobe` is used **only to verify output**, never for rendering or encoding.

## Design and security

- The browser is a deliberate *runtime dependency* **only** for browser-assisted mode. The C++ codecs do not require FFmpeg or Chromium.
- Running arbitrary untrusted HTML is **not safe**; Chromium will execute JavaScript and may access the network. Use trusted local compositions only, preferably in a low-privilege environment.
- The engine is currently experimental: AVI has a 32-bit container-size limit; native H.264/AAC codecs have performance and patent/distribution considerations; Windows has not been runtime-tested on a physical Windows installation.
- Scene importing does not constitute complete DOM/CSS/GSAP compatibility. For exact browser visuals, use the browser mode with original JS/assets.
- Audio accepts WAV PCM as described above; MP3/AAC input decoding and audio mixing are planned, not presently supported.

See [docs/ROADMAP.md](docs/ROADMAP.md) and [SECURITY.md](SECURITY.md).

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) and [SECURITY.md](SECURITY.md). Technical regression tests and reproducible bug reports are welcome. **Do not submit video projects, copyrighted narrations, third-party GSAP distributions, compiled binaries or generated media to this repository.**

## License

Apache License 2.0, except third-party components that retain their independent licenses. See [LICENSE](LICENSE), [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) and [assets/fonts/LICENSE.txt](assets/fonts/LICENSE.txt). H.264/AAC patent obligations and external GSAP/browser licenses must be reviewed separately.
