#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
if [[ ! -f font_embedded.hpp ]]; then
 python3 scripts/embed_fonts.py
fi
x86_64-w64-mingw32-g++ -std=c++17 -O2 -static -static-libgcc -static-libstdc++ -s main.cpp -o hyperframes-windows-x64.exe
printf 'Compilado Windows: %s\n' "$PWD/hyperframes-windows-x64.exe"
