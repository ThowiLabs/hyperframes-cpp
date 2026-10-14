#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/.."
destination="samples/ley-antimemes/assets/vendor/gsap.min.js"
if [ -s "$destination" ]; then
  echo "GSAP already present in demo assets"
  exit 0
fi
echo "Fetching GSAP 3.14.2 for LOCAL DEMO USE ONLY."
echo "Read its independent license: https://gsap.com/standard-license"
mkdir -p "$(dirname "$destination")"
curl --fail --silent --show-error --location --max-time 35 \
  "https://cdnjs.cloudflare.com/ajax/libs/gsap/3.14.2/gsap.min.js" \
  --output "$destination"
test -s "$destination"
echo "Local GSAP asset: $destination"
