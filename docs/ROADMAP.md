# Roadmap and compatibility policy

Work is delivered in incremental, testable branches. Passing an integration test for the bundled sample is not equivalent to full browser compatibility.

| Phase | Scope | State |
|---|---|---|
| A | Deterministic native timeline with frame-rate-independent easing | Implemented on develop/native-compositor |
| B | Configurable vertical output resolutions and bilinear resampling | Implemented on develop/native-compositor |
| C | Basic HTML-driven CSS px properties | Implemented on develop/native-compositor (limited) |
| D | General HTML/CSS layout via evaluated litehtml + Blend2D integration | Pending |
| E | Internal H.264/AAC + MP4 packaging with legal/runtime review | Pending |
| F | QuickJS with controlled DOM-compatible APIs and security limits | Pending |
| G | Tests in Windows OS, fuzzing, API documentation and CI | Pending |

The reference implementation must continue to compile without system FFmpeg or Chromium at runtime. New dependencies must have redistributable licenses and be buildable statically on Linux and Windows. Prefer feature-detection and explicit unsupported errors over silently mis-rendering complex input.
