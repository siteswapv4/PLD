#!/usr/bin/env bash
set -euo pipefail

# Go through all images recursively and resize them to 50%
find . -type f \( \
  -iname "*.jpg" -o -iname "*.jpeg" -o -iname "*.png" -o \
  -iname "*.webp" -o -iname "*.tif" -o -iname "*.tiff" \
\) -print0 | while IFS= read -r -d '' file; do
  echo "Resizing: $file"
  magick mogrify -resize 50% "$file"
done
