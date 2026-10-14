# Native compositor architecture (experimental branch)

`HTML (supported subset) -> scene model -> animation sampling -> native rasterizer -> optional resize -> MJPEG/RLE8/RGB24 -> AVI+PCM`

The design favors bounded, deterministic rendering over executing arbitrary web content.

## Scene import and basic CSS

`html_composition.hpp` extracts six scene classes and metadata from the bundled sample. `css_properties.hpp` currently accepts **only positive integer `px` declarations** in exact CSS selectors inside one `<style>` block. The classic theme applies `.upper` top/left, `.kicker` height/font-size, `.headline` margin/font-size, `.sub` margin/font-size and `.scene-N .headline` font size. It does not implement selectors, cascade, box model, flexbox, CSS variables or computed styles generally. The news theme intentionally overrides these layout measurements.

## Animation

`animation_timeline.hpp` contains pure easing functions and sequential `Clip` tracks for `x`, `y`, `scale` and `opacity`. A sample at time `t` produces the same visual values regardless of frame rate or render order. Built-in themes currently use time-derived entrance transitions; arbitrary JS/GSAP evaluation is **not implemented**. Existing supported curves: `none`, `power1.in`, `power2.out`, `power3.out`, `expo.out`, and `back.out(1.2)` and `back.out(1.6)`.

## Rendering and output size

Design positions are defined in a logical 720x1280 coordinate system. `output_scale.hpp` applies bilinear scaling before encoding, restricted to even-dimension 9:16 output sizes from 180x320 to 1080x1920. This is **resampling, not responsive CSS layout**.

## AVI and codecs

MJPEG embeds a JPEG encoder from stb. RLE8 is implemented in C++ and sacrifices color resolution. RGB24 is uncompressed. All use PCM 24kHz mono audio. The AVI RIFF container has a 32-bit size limit; the implementation does not provide MP4 or H.264.

## Security boundaries

The importer does not execute scripts or fetch remote resources. It bounds input HTML size and scene count. The parser is intentionally limited: arbitrary hostile HTML/WAV has not undergone an independent security audit. Avoid running untrusted inputs with root privileges; do not embed credentials in samples or generated artifacts.
