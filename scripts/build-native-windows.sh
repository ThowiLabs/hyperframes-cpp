#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/.."
if [ ! -f font_embedded.hpp ]; then python3 scripts/embed_fonts.py; fi
sh scripts/fetch-html-deps.sh
sh scripts/fetch-codecs.sh
cmake -S . -B build/native-windows \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
  -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
  -DCMAKE_BUILD_TYPE=Release \
  -DHFC_HTML_LAYOUT=ON -DHFC_NATIVE_MP4=ON -DHFC_BROWSER_CAPTURE=ON
cmake --build build/native-windows -j 3
echo "Windows browser-aware MP4 executable: build/native-windows/hyperframes-cpp-native.exe"
