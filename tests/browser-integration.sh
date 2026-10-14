#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/.."
binary="build/native-linux/hyperframes-cpp-native"
if [ "$#" -gt 0 ]; then binary="$1"; fi
if [ ! -x "$binary" ]; then
  echo "Missing executable: $binary" >&2
  exit 1
fi
if ! "$binary" --browser-check >/dev/null 2>&1; then
  echo "SKIP: no compatible Chrome/Chromium/Edge installed"
  exit 0
fi
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
echo "[browser] two GSAP-compatible timeline frames"
"$binary" --browser-frame tests/fixtures/browser-scene.html 1 "$tmp/one.png" >/dev/null
"$binary" --browser-frame tests/fixtures/browser-scene.html 3 "$tmp/three.png" >/dev/null
test -s "$tmp/one.png"
test -s "$tmp/three.png"
if cmp -s "$tmp/one.png" "$tmp/three.png"; then
  echo "Browser did not advance the JavaScript timeline" >&2
  exit 1
fi
echo "[browser] native MP4 + AAC from browser frames"
"$binary" tests/fixtures/browser-scene.html - "$tmp/clip.mp4" 2 h264 browser 288x512 2 >/dev/null
test -s "$tmp/clip.mp4"
if command -v ffprobe >/dev/null 2>&1; then
  ffprobe -v error -select_streams v -show_entries stream=codec_name,nb_frames -of csv=p=0 "$tmp/clip.mp4" | grep -q 'h264'
  ffprobe -v error -select_streams a -show_entries stream=codec_name -of csv=p=0 "$tmp/clip.mp4" | grep -q 'aac'
  ffprobe -v error -show_entries format=duration -of csv=p=0 "$tmp/clip.mp4" | grep -q '^2.000'
fi
echo "BROWSER_RENDER_TESTS_PASSED"
