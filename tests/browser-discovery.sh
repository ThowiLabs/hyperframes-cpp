#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
temp="$(mktemp -d)"
pid=""
cleanup() {
 if [[ -n "$pid" ]]; then kill "$pid" 2>/dev/null || true; wait "$pid" 2>/dev/null || true; fi
 rm -rf "$temp"
}
trap cleanup EXIT
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic tests/browser-discovery-test.cpp -o "$temp/browser-discovery"
mkdir -p "$temp/mock"
cp /bin/sleep "$temp/mock/chromium"
chmod +x "$temp/mock/chromium"
printf '[1] HFC_BROWSER absolute override\n'
HFC_BROWSER="$temp/mock/chromium" "$temp/browser-discovery" |grep -Fx "$temp/mock/chromium"
printf '[2] Browser lookup in PATH\n'
PATH="$temp/mock:/usr/bin:/bin" "$temp/browser-discovery" path chromium |grep -Fx "$temp/mock/chromium"
printf '[3] Running process fallback via /proc\n'
"$temp/mock/chromium" 30 &
pid=$!
sleep .25
"$temp/browser-discovery" running |grep -Fx "$temp/mock/chromium"
printf '[4] Invalid override is not silently ignored\n'
if HFC_BROWSER="$temp/does-not-exist" "$temp/browser-discovery"; then
 echo 'Invalid HFC_BROWSER should not succeed' >&2
 exit 1
fi
printf 'BROWSER_DISCOVERY_TESTS_PASSED\n'
