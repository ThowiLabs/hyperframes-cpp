#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
original="samples/ley-antimemes/index.html"
if [[ ! -f font_embedded.hpp ]]; then python3 scripts/embed_fonts.py; fi
python3 scripts/generate_demo_audio.py "$tmp/audio-fixtures" >/dev/null
echo "[1/7] Compilar C++17 estatico"
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic -static -s main.cpp -o "$tmp/hyperframes"
echo "[2/7] Importar seis escenas del HTML real"
"$tmp/hyperframes" --inspect "$original" > "$tmp/escenas.txt"
test "$(grep -cE '^[1-6] \|' "$tmp/escenas.txt")" = 6
grep -q 'EL FOCO' "$tmp/escenas.txt"
grep -q 'INFÓRMATE' "$tmp/escenas.txt"
echo "[3/7] Verificar HTML inválido"
printf '<html>Sin escenas</html>' > "$tmp/mal.html"
if "$tmp/hyperframes" --inspect "$tmp/mal.html" >/dev/null 2>&1; then
  echo "ERROR: HTML sin escenas aceptado" >&2; exit 1
fi
echo "[4/7] Verificar limite de 60 FPS"
for fps in 0 61 abc 12fps; do
  if "$tmp/hyperframes" "$original" - "$tmp/invalid.avi" "$fps" > /dev/null 2>&1; then
   echo "ERROR: se acepto FPS $fps" >&2;exit 1
  fi
done
echo "[5/7] Renderizar AVI sin audio y comprobar que HTML controla fotogramas"
cp "$original" "$tmp/cambiado.html"
python3 - "$tmp/cambiado.html" <<'PY'
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
assert 'PROHIBIÓ LOS' in s
s=s.replace('PROHIBIÓ LOS','VIDEO DISTINTO')
p.write_text(s)
PY
"$tmp/hyperframes" --inspect "$tmp/cambiado.html" > "$tmp/escenas-cambiadas.txt"
grep -q 'VIDEO DISTINTO' "$tmp/escenas-cambiadas.txt"
"$tmp/hyperframes" "$original" - "$tmp/original.avi" 1 > /dev/null 2>&1
"$tmp/hyperframes" "$tmp/cambiado.html" - "$tmp/cambiado.avi" 1 > /dev/null 2>&1
if cmp -s "$tmp/original.avi" "$tmp/cambiado.avi"; then
 echo "ERROR: modificacion del HTML no afecta el video" >&2; exit 1
fi
echo "[6/7] Probar salida con WAV real a 1 FPS"
"$tmp/hyperframes" "$original" "$tmp/audio-fixtures" "$tmp/audio.avi" 1 >/dev/null 2>&1
test -s "$tmp/audio.avi"
echo "[7/7] Verificar streams con ffprobe (si esta instalado)"
if command -v ffprobe >/dev/null 2>&1; then
 rate="$(ffprobe -v error -select_streams v:0 -show_entries stream=r_frame_rate -of default=noprint_wrappers=1:nokey=1 "$tmp/audio.avi")"
 duration="$(ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 "$tmp/audio.avi")"
 audio="$(ffprobe -v error -select_streams a:0 -show_entries stream=codec_name -of default=noprint_wrappers=1:nokey=1 "$tmp/audio.avi")"
 test "$rate" = "1/1"
 test "$duration" = "67.000000"
 test "$audio" = "pcm_s16le"
fi
echo "[8/8] Video con compresion AVI RLE8 incorporada"
"$tmp/hyperframes" "$original" "$tmp/audio-fixtures" "$tmp/rle8.avi" 1 rle8 >/dev/null 2>&1
test -s "$tmp/rle8.avi"
if command -v ffprobe >/dev/null 2>&1; then
 codec="$(ffprobe -v error -select_streams v:0 -show_entries stream=codec_name -of default=noprint_wrappers=1:nokey=1 "$tmp/rle8.avi")"
 test "$codec" = "msrle"
fi
if "$tmp/hyperframes" "$original" "$tmp/audio-fixtures" "$tmp/invalid.avi" 1 h264 >/dev/null 2>&1; then
 echo "ERROR: codec no admitido" >&2;exit 1
fi
echo "[9/9] Video MJPEG de alta calidad con tipografia Unicode"
"$tmp/hyperframes" "$original" "$tmp/audio-fixtures" "$tmp/mjpeg.avi" 1 mjpeg >/dev/null 2>&1
test -s "$tmp/mjpeg.avi"
if command -v ffprobe >/dev/null 2>&1; then
 codec="$(ffprobe -v error -select_streams v:0 -show_entries stream=codec_name -of default=noprint_wrappers=1:nokey=1 "$tmp/mjpeg.avi")"
 test "$codec" = "mjpeg"
fi
echo "[10/10] Noticiero C++ y comprobacion de tema"
"$tmp/hyperframes" "$original" "$tmp/audio-fixtures" "$tmp/news.avi" 1 mjpeg news >/dev/null 2>&1
test -s "$tmp/news.avi"
if cmp -s "$tmp/news.avi" "$tmp/mjpeg.avi"; then
 echo "ERROR: el tema news es identico a classic" >&2; exit 1
fi
if "$tmp/hyperframes" "$original" - "$tmp/invalid-theme.avi" 1 mjpeg desconocido >/dev/null 2>&1; then
 echo "ERROR: el tema desconocido fue aceptado" >&2; exit 1
fi
"$tmp/hyperframes" "$tmp/cambiado.html" - "$tmp/news-modificado.avi" 1 mjpeg news >/dev/null 2>&1
if cmp -s "$tmp/news.avi" "$tmp/news-modificado.avi"; then
 echo "ERROR: cambiar el HTML no cambia el noticiero" >&2; exit 1
fi
if command -v ffprobe >/dev/null 2>&1; then
 frames="$(ffprobe -v error -select_streams v:0 -show_entries stream=nb_frames -of default=noprint_wrappers=1:nokey=1 "$tmp/news.avi")"
 test "$frames" = "67"
fi
echo "PRUEBAS HTML, FUENTES, CODECS Y NOTICIERO SUPERADAS"
