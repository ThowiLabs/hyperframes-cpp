# Roadmap and compatibility policy

Work is delivered in incremental, testable branches. Passing an integration test for the bundled sample is not equivalent to full browser compatibility.

| Phase | Scope | State |
|---|---|---|
| A | Deterministic native timeline with frame-rate-independent easing | Implemented on develop/native-compositor |
| B | Configurable vertical output resolutions and bilinear resampling | Implemented on develop/native-compositor |
| C | Basic HTML-driven CSS px properties | Implemented on develop/native-compositor (limited) |
| D | litehtml HTML/CSS layout adapter | Experimental backend implemented; Blend2D evaluation built statically but not integrated |
| E | Native H.264/AAC MP4 integration | Linux validated; Windows cross-compiled; patent review pending |
| F | Optional QuickJS with controlled DOM-compatible APIs | Pending; browser mode already executes existing JavaScript |
| G | Tests in Windows OS, fuzzing, API documentation and CI | Pending |

The reference implementation must continue to compile without system FFmpeg or Chromium at runtime. New dependencies must have redistributable licenses and be buildable statically on Linux and Windows. Prefer feature-detection and explicit unsupported errors over silently mis-rendering complex input.

| H | Installed browser detection, CDP screenshots and original GSAP timelines | Implemented and verified Linux; Windows cross-compiled |
