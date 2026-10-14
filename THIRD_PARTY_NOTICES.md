# Third-party notices

HyperFrames C++ is an independent project. Third-party files and design references retain their own terms.

## stb headers

Files: `vendor/stb/stb_truetype.h` and `vendor/stb/stb_image_write.h`.

Copyright Sean Barrett and contributors. The upstream [stb](https://github.com/nothings/stb) project offers its components under public-domain and MIT dual-license alternatives. Both files preserve their upstream license information in their original headers.

## DejaVu Sans

The font family used at **build time** is DejaVu Sans and DejaVu Sans Bold. Its complete copyright and licensing notice is reproduced in `assets/fonts/LICENSE.txt`. Local system fonts are embedded in the compiled executable by `scripts/embed_fonts.py`. Raw TTF files and the generated `font_embedded.hpp` are not distributed through this repository.

The public example screenshots may include glyphs rendered from DejaVu Sans.

## HyperFrames-inspired example

The HTML sample uses a scene layout developed with the HyperFrames composition conventions. It is included for demonstrating the subset of HTML scene semantics supported by this experimental engine; it does not include an upstream HyperFrames implementation or ship a browser or JavaScript animation runtime.

The upstream HyperFrames project is a separate project and is not affiliated with this repository. Users are responsible for respecting licenses of their own input HTML, audio, videos, fonts and images.
