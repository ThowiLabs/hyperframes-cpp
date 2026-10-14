#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
original="$tmp/generic-scenes.html"
python3 tests/generate_scene_fixture.py "$original"
if [[ ! -f font_embedded.hpp ]]; then python3 scripts/embed_fonts.py; fi
python3 tests/generate_audio_fixture.py "$tmp/audio-fixtures" >/dev/null
echo "[0/10] Unit tests: deterministic animation and resolution scaling"
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic tests/timeline_test.cpp -o "$tmp/animation-tests"
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic tests/output_scale_test.cpp -o "$tmp/output-tests"
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic tests/css_properties_test.cpp -o "$tmp/css-tests"
"$tmp/animation-tests"
"$tmp/output-tests"
"$tmp/css-tests"
echo "[1/10] Compilar C++17 estatico"
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic -static -s main.cpp -o "$tmp/hyperframes"
echo "[2/7] Importar seis escenas sintéticas"
"$tmp/hyperframes" --inspect "$original" > "$tmp/escenas.txt"
test "$(grep -cE '^[1-6] \|' "$tmp/escenas.txt")" = 6
grep -q 'MOTOR DE VIDEO' "$tmp/escenas.txt"
grep -q 'SALIDA NATIVA' "$tmp/escenas.txt"
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
assert 'VIDEO NATIVO' in s
s=s.replace('VIDEO NATIVO','VIDEO DISTINTO')
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
 test "$duration" = "12.000000"
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
 test "$frames" = "12"
fi
echo "[10/10] AVI resized output / invalid aspect ratio"
"$tmp/hyperframes" "$original" - "$tmp/360p.avi" 1 mjpeg news 360x640 >/dev/null 2>&1
if command -v ffprobe >/dev/null 2>&1; then
  dimensions="$(ffprobe -v error -select_streams v:0 -show_entries stream=width,height -of csv=p=0:s=x "$tmp/360p.avi")"
  test "$dimensions" = "360x640"
fi
if "$tmp/hyperframes" "$original" - "$tmp/invalid-size.avi" 1 mjpeg news 720x720 >/dev/null 2>&1; then
  echo "ERROR: invalid output aspect ratio accepted" >&2; exit 1
fi
echo "[11/11] CSS edit changes classic output"
python3 - "$original" "$tmp/edited-css.html" <<'PYCSS'
from pathlib import Path
import sys
source = Path(sys.argv[1]).read_text()
assert "font-size:68px;line-height" in source
source = source.replace("font-size:68px;line-height",
                        "font-size:40px;line-height", 1)
Path(sys.argv[2]).write_text(source)
PYCSS
"$tmp/hyperframes" "$tmp/edited-css.html" - "$tmp/css-change.avi" 1 mjpeg classic >/dev/null 2>&1
"$tmp/hyperframes" "$original" - "$tmp/css-original.avi" 1 mjpeg classic >/dev/null 2>&1
if cmp -s "$tmp/css-change.avi" "$tmp/css-original.avi"; then
 echo "ERROR: edited headline CSS did not affect the rendered video" >&2; exit 1
fi
echo "PRUEBAS DE ANIMACIONES, CSS, DIMENSIONES Y CODECS SUPERADAS"
