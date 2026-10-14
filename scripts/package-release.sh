#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
version="${1:-v1.0.0}"
if [[ "$version" != v[0-9]* ]]; then echo "Version must start with v" >&2;exit 2;fi
linux="build/native-linux/hyperframes-cpp-native"
windows="build/native-windows/hyperframes-cpp-native.exe"
if [[ ! -x "$linux" || ! -s "$windows" ]]; then
  echo "Build both native targets before packaging" >&2; exit 2
fi
mkdir -p dist
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
skill=".claude/skills/hyperframes-cpp"
source_copy() {
 local root="$1"
 mkdir -p "$root/skills/hyperframes-cpp" "$root/bin"
 cp -R "$skill/." "$root/skills/hyperframes-cpp/"
 cp LICENSE THIRD_PARTY_NOTICES.md README.md "$root/"
 mkdir -p "$root/assets/fonts"
 cp assets/fonts/LICENSE.txt "$root/assets/fonts/LICENSE.txt"
 cp scripts/install-skill.sh "$root/install-skill.sh"
 chmod +x "$root/install-skill.sh"
}
label="hyperframes-cpp-$version"
linux_root="$work/$label-linux-x64"
source_copy "$linux_root"
cp "$linux" "$linux_root/bin/hyperframes-cpp-native"
chmod +x "$linux_root/bin/hyperframes-cpp-native"
tar -C "$work" -czf "dist/$label-linux-x64.tar.gz" "$label-linux-x64"
win_root="$work/$label-windows-x64"
source_copy "$win_root"
cp "$windows" "$win_root/bin/hyperframes-cpp-native.exe"
(cd "$work" && zip -qr "$OLDPWD/dist/$label-windows-x64.zip" "$label-windows-x64")
skill_root="$work/$label-agent-skill"
mkdir -p "$skill_root/hyperframes-cpp"
cp -R "$skill/." "$skill_root/hyperframes-cpp/"
(cd "$work" && zip -qr "$OLDPWD/dist/$label-agent-skill.zip" "$label-agent-skill")
(cd dist && sha256sum "$label-linux-x64.tar.gz" "$label-windows-x64.zip" "$label-agent-skill.zip" > SHA256SUMS.txt)
ls -lh dist/
echo "RELEASE_PACKAGES_READY $version"
