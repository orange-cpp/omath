#!/bin/bash

# =============================================================================
# Gource Timelapse — renders the repository history as a video
# Requires: gource, ffmpeg
# =============================================================================

set -euo pipefail

# --- Config (override via env vars) ---
OUTPUT="${OUTPUT:-gource-timelapse.mp4}"
SECONDS_PER_DAY="${SECONDS_PER_DAY:-0.1}"
RESOLUTION="${RESOLUTION:-1920x1080}"
FPS="${FPS:-60}"
TITLE="${TITLE:-omath}"

# --- Dependency checks ---
for cmd in gource ffmpeg; do
    if ! command -v "$cmd" &>/dev/null; then
        echo "Error: '$cmd' is not installed."
        echo "  macOS:  brew install $cmd"
        echo "  Linux:  sudo apt install $cmd"
        exit 1
    fi
done

echo "----------------------------------------------------"
echo "Rendering gource timelapse → $OUTPUT"
echo "  Resolution : $RESOLUTION"
echo "  FPS        : $FPS"
echo "  Speed      : ${SECONDS_PER_DAY}s per day"
echo "----------------------------------------------------"

gource \
    --title "$TITLE" \
    --seconds-per-day "$SECONDS_PER_DAY" \
    --auto-skip-seconds 0.1 \
    --time-scale 3 \
    --max-files 0 \
    --hide filenames \
    --date-format "%Y-%m-%d" \
    --multi-sampling \
    --bloom-multiplier 0.5 \
    --elasticity 0.05 \
    --${RESOLUTION%x*}x${RESOLUTION#*x} \
    --output-framerate "$FPS" \
    --output-ppm-stream - \
    | ffmpeg -y \
        -r "$FPS" \
        -f image2pipe \
        -vcodec ppm \
        -i - \
        -vcodec libx264 \
        -preset fast \
        -pix_fmt yuv420p \
        -crf 18 \
        "$OUTPUT"

echo "----------------------------------------------------"
echo "Done: $OUTPUT"
echo "----------------------------------------------------"
