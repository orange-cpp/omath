#!/usr/bin/env bash
set -euo pipefail

# Format all CMakeLists.txt and *.cmake files in the repo (excluding common build dirs)
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cd "$REPO_ROOT"

find . \
  -path "./build" -prune -o \
  -path "./cmake-build-*" -prune -o \
  -path "./out" -prune -o \
  -path "./.git" -prune -o \
  \( -name "CMakeLists.txt" -o -name "*.cmake" \) -print0 \
| xargs -0 cmake-format -i