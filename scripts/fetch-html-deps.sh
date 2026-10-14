#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p third_party/_sources
if [ ! -d third_party/_sources/litehtml/.git ]; then
 git clone -q --depth 1 https://github.com/litehtml/litehtml.git third_party/_sources/litehtml
fi
git -C third_party/_sources/litehtml fetch -q --depth 1 origin 8848b99b1fae17026117c45c3ec9cd3e38b2e511
git -C third_party/_sources/litehtml checkout -q 8848b99b1fae17026117c45c3ec9cd3e38b2e511
echo "litehtml revision: $(git -C third_party/_sources/litehtml rev-parse --short HEAD)"
