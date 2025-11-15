#!/bin/bash
# Usage: ./split_skybox.sh skybox.png
# Splits a horizontal-cross skybox into 6 faces: right, left, top, bottom, front, back

if [ $# -ne 1 ]; then
  echo "Usage: $0 <input_image>"
  exit 1
fi

input="$1"
basename=$(basename "$input" .${input##*.})

# Get full image width and height
width=$(identify -format "%w" "$input")
height=$(identify -format "%h" "$input")

# Each face size
face=$((width / 4))

echo "Input: $input"
echo "Image size: ${width}x${height}"
echo "Each face: ${face}x${face}"

# Crop and save
# Offsets are based on the cross layout (4x3 grid)
# +x  → right
# -x  → left
# +y  → top
# -y  → bottom
# +z  → front
# -z  → back

# Middle row Y offset
midY=$face

# Extract faces
convert "$input" -crop "${face}x${face}+$((2*face))+$midY" "${basename}_right.png"
convert "$input" -crop "${face}x${face}+0+$midY"           "${basename}_left.png"
convert "$input" -crop "${face}x${face}+$face+0"           "${basename}_top.png"
convert "$input" -crop "${face}x${face}+$face+$((2*face))" "${basename}_bottom.png"
convert "$input" -crop "${face}x${face}+$((3*face))+$midY" "${basename}_front.png"
convert "$input" -crop "${face}x${face}+$face+$midY"       "${basename}_back.png"

echo "✅ Skybox faces saved as:"
ls "${basename}"_*.png
