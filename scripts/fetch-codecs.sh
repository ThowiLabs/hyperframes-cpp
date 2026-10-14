#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p third_party/_sources
for entry in "minih264:lieff/minih264:b0baea7a80ef9d12da97301dd1099b8791b5ba43" "minimp4:lieff/minimp4:2a3014a7753cfcba4442adeb469431967238fe6a" "vo-aacenc:mstorsjo/vo-aacenc:a277487e051e92e99a532294eed3c673f4d879f2"; do
  name=$(printf '%s' "$entry" | cut -d: -f1)
  repo=$(printf '%s' "$entry" | cut -d: -f2)
  rev=$(printf '%s' "$entry" | cut -d: -f3)
  if [ ! -d "third_party/_sources/$name/.git" ]; then
    git clone -q --depth 1 "https://github.com/$repo.git" "third_party/_sources/$name"
  fi
  git -C "third_party/_sources/$name" fetch -q --depth 1 origin "$rev"
  git -C "third_party/_sources/$name" checkout -q "$rev"
  echo "$name $(git -C "third_party/_sources/$name" rev-parse --short HEAD)"
done
