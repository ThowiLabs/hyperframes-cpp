#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/.."
if [ ! -f font_embedded.hpp ]; then python3 scripts/embed_fonts.py; fi
sh scripts/fetch-html-deps.sh
sh scripts/fetch-codecs.sh
cmake -S . -B build/native-linux -DCMAKE_BUILD_TYPE=Release -DHFC_HTML_LAYOUT=ON -DHFC_NATIVE_MP4=ON
cmake --build build/native-linux -j 3
echo "Native HTML/CSS and MP4 app: build/native-linux/hyperframes-cpp-native"
