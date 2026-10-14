#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/.."
if [ ! -f font_embedded.hpp ]; then python3 scripts/embed_fonts.py; fi
sh scripts/fetch-html-deps.sh
cmake -S . -B build/html-linux -DCMAKE_BUILD_TYPE=Release -DHFC_HTML_LAYOUT=ON
cmake --build build/html-linux -j 3
echo "Native HTML/CSS renderer: build/html-linux/hyperframes-cpp-native"
