#!/usr/bin/env bash
# 从 logo.svg 生成 macOS .icns 图标文件

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
SVG_FILE="${PROJECT_ROOT}/resources/icons/logo.svg"
ICONSET_DIR="${PROJECT_ROOT}/resources/icons/app.iconset"
ICNS_FILE="${PROJECT_ROOT}/resources/icons/app.icns"

echo "==> Generating macOS .icns from logo.svg"

if [ ! -f "${SVG_FILE}" ]; then
    echo "ERROR: ${SVG_FILE} not found"
    exit 1
fi

if ! command -v rsvg-convert &>/dev/null; then
    echo "ERROR: rsvg-convert not found"
    echo "Please install: brew install librsvg"
    exit 1
fi

rm -rf "${ICONSET_DIR}"
mkdir -p "${ICONSET_DIR}"

SIZES=(16 32 64 128 256 512 1024)

for SIZE in "${SIZES[@]}"; do
    echo "Generating ${SIZE}x${SIZE}..."
    rsvg-convert -w ${SIZE} -h ${SIZE} "${SVG_FILE}" -o "${ICONSET_DIR}/icon_${SIZE}x${SIZE}.png"
    
    if [ ${SIZE} -ne 1024 ]; then
        SIZE_2X=$((SIZE * 2))
        echo "Generating ${SIZE}x${SIZE}@2x..."
        rsvg-convert -w ${SIZE_2X} -h ${SIZE_2X} "${SVG_FILE}" -o "${ICONSET_DIR}/icon_${SIZE}x${SIZE}@2x.png"
    fi
done

echo "Creating .icns file..."
iconutil -c icns "${ICONSET_DIR}" -o "${ICNS_FILE}"

rm -rf "${ICONSET_DIR}"

echo "==> Generated: ${ICNS_FILE}"
ls -lh "${ICNS_FILE}"
