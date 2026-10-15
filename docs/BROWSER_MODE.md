# Browser-backed rendering (experimental)

The `browser` theme uses **an existing Chromium-family installation** to render trusted local HTML, CSS and JavaScript. HyperFrames C++ controls the browser using the **Chrome DevTools Protocol (CDP)** and captures each frame at an explicit timeline timestamp. Its built-in video and audio codecs then create the final AVI or MP4.

This mode fixes a fundamental fidelity gap in the prototype native HTML renderer: the browser calculates flex/grid layouts, masks, transforms, gradients, filters and the real GSAP animation state rather than approximating them.

**Important:** The renderer is still a native C++17 application, but browser mode deliberately **depends on an installed browser at runtime**. It is optional: `classic`, `news` and `html` can work without a browser. Browser mode **does not bundle or distribute Chromium**.

## Supported browsers

The executable searches `HFC_BROWSER` (explicit path), `PATH` for `chromium`, `chromium-browser`, `google-chrome(-stable)`, `microsoft-edge(-stable)`, and `brave-browser` on Linux. On Windows it looks for Chrome, Edge and Chromium in the usual Program Files and Local AppData directories and also searches `PATH`. Detection can be overridden using an **absolute executable path** in `HFC_BROWSER`.

These browsers must support CDP, Headless Chrome and screenshot capture. Firefox and Safari are **not yet supported**.

```bash
build/native-linux/hyperframes-cpp-native --browser-check
# e.g. Detected browser: /usr/bin/chromium
```

If no suitable browser is installed, browser mode returns an actionable error. The user must install a compatible browser; the executable does not silently download a browser binary.

## Building native MP4 + browser mode

On Debian/Ubuntu (build-time requirements only):

```bash
sudo apt install g++ gcc make cmake python3 git fonts-dejavu-core
# Only for testing browser-backed mode:
sudo apt install chromium
./scripts/build-native-mp4.sh
```

To cross-compile the statically linked C++/C codec libraries for Windows:

```bash
sudo apt install g++-mingw-w64-x86-64-win32 gcc-mingw-w64-x86-64-win32
./scripts/build-native-windows.sh
```

Runtime dependencies of the resulting Windows `.exe`: Windows system libraries and a supported installed browser for `browser` mode. `libstdc++-6.dll` and `libgcc_s_seh-1.dll` are linked statically. The Windows build was cross-compiled; it **has not been executed on a real Windows machine**.

## Rendering

Use a trusted local HTML composition that includes its local GSAP JavaScript assets and defines a timeline at `window.__timelines.main`. Make sure the HTML has contiguous `section.scene` elements with `data-start` and `data-duration`, and local matching WAV files or `-` for silence.

```bash
# Inspect detected browser
build/native-linux/hyperframes-cpp-native --browser-check

# Capture a single true browser frame (PNG) at t=4.0s
build/native-linux/hyperframes-cpp-native --browser-frame /path/to/index.html 4 preview.png

# Entire MP4, H.264 + AAC, real browser frames
build/native-linux/hyperframes-cpp-native \
  /path/to/index.html /path/to/wav-segments movie.mp4 24 h264 browser 720x1280

# Five-second preview
build/native-linux/hyperframes-cpp-native \
  /path/to/index.html /path/to/wav-segments preview.mp4 24 h264 browser 720x1280 5
```

Set `HFC_BROWSER=/absolute/path/to/chrome` if auto-detection does not find the browser.

The `clip-seconds` final argument is optional, must be positive and no longer than the source composition. Input frames are sampled at `time=frame_index/fps`, independent of wall clock time. WAV narration and MP4 audio are cut to the preview duration.

**No finished video compositions or GSAP assets are bundled with this repository.** Point the renderer at your own complete local HTML composition and supply any JavaScript libraries under their own licenses. GSAP is subject to its [own license](https://gsap.com/standard-license), including terms for competitive visual animation builders.

## Architecture

1. C++ detects a browser executable (Linux and Windows) and starts it headless with a **temporary profile** and a DevTools endpoint bound to localhost.
2. C++ connects via a masked WebSocket and the Chrome DevTools Protocol. It navigates to a trusted local `file://` HTML.
3. It waits for the GSAP timeline to be ready, then seeks to each frame's exact time, synchronizes scene visibility using the scene data attributes, and requests a screenshot.
4. It decodes the captured image in process and hands BGR pixels to the existing H.264/AAC/MP4 codec or AVI writer, with optional vertical scaling.
5. On exit, the browser process and temporary profile are cleaned up.

The browser can execute JavaScript and make network requests specified by the HTML. **Do not render untrusted HTML with privileged permissions.** It is not a hardened sandbox for adversarial documents. Only a local CDP endpoint is exposed; remote browsing is not supported.

## Validation

```bash
bash tests/smoke-linux.sh
bash tests/browser-integration.sh build/native-linux/hyperframes-cpp-native
```

The second script uses a minimal, synthetic technical fixture rather than third-party GSAP or user projects. It verifies browser detection, time-dependent JavaScript animation and a short H.264/AAC MP4, and skips when no compatible browser is present.

## Known limitations

- Chrome screenshot capture has substantial overhead compared to an integrated 2D compositor. A 60 FPS output is supported by the frame sampler but **60 FPS export performance is not guaranteed**; benchmark your machine first.
- GSAP scenes require the correct original scripts; browser mode does not provide GSAP automatically.
- The HTML parser for timing is still limited to our known scene format. A general webpage without scene metadata cannot yet be exported as a timed video.
- Browser mode supports Chromium-family CDP only. Firefox/Safari require a separate backend.
- H.264/AAC patent and redistribution obligations may apply despite permissive source-code licenses.

### Running-process fallback

If no standard install path is found, the C++ detector looks at running Chromium-family browser executable paths: `/proc/<pid>/exe` on Linux, Windows Toolhelp process enumeration and `QueryFullProcessImageNameW` on Windows. This is path discovery **only**; existing browser profiles/tabs are never reused. A dedicated headless browser process is started for each render. If running processes cannot be inspected due to permissions, use the `HFC_BROWSER` override.

### Comprobar la ruta de un navegador ya abierto

También puedes consultar **solo los procesos** (sin iniciar el navegador) con:

```bash
build/native-linux/hyperframes-cpp-native --browser-process-check
```

Este comando devuelve la ruta absoluta de un ejecutable Chromium compatible si hay uno en ejecución. Si no lo hay, devuelve un error. `--browser-check` mantiene el mecanismo automático completo: variable de entorno, PATH, ubicaciones conocidas y procesos como último recurso. En Windows la detección y el lanzamiento utilizan rutas UTF-16 del sistema, incluida una ruta con espacios o caracteres Unicode. No reutiliza ni inspecciona pestañas, sesiones o cookies del navegador que ya está abierto.
