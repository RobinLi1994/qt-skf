#!/usr/bin/env bash
# 从 logo.svg 生成所有平台的图标文件
# - macOS: app.icns
# - Windows: app.ico
# - Linux: app.png (多种尺寸)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
SVG_FILE="${PROJECT_ROOT}/resources/icons/logo.svg"
ICONS_DIR="${PROJECT_ROOT}/resources/icons"

echo "==> Generating application icons from logo.svg"

if [ ! -f "${SVG_FILE}" ]; then
    echo "ERROR: ${SVG_FILE} not found"
    exit 1
fi

if ! command -v rsvg-convert &>/dev/null; then
    echo "ERROR: rsvg-convert not found"
    echo "Please install: brew install librsvg"
    exit 1
fi

# ==============================================================================
# 1. 生成 macOS .icns 文件
# ==============================================================================
echo ""
echo "--- Generating macOS .icns ---"
ICONSET_DIR="${ICONS_DIR}/app.iconset"
ICNS_FILE="${ICONS_DIR}/app.icns"

rm -rf "${ICONSET_DIR}"
mkdir -p "${ICONSET_DIR}"

SIZES=(16 32 64 128 256 512 1024)

for SIZE in "${SIZES[@]}"; do
    echo "  ${SIZE}x${SIZE}..."
    rsvg-convert -w ${SIZE} -h ${SIZE} "${SVG_FILE}" -o "${ICONSET_DIR}/icon_${SIZE}x${SIZE}.png"
    
    if [ ${SIZE} -ne 1024 ]; then
        SIZE_2X=$((SIZE * 2))
        echo "  ${SIZE}x${SIZE}@2x..."
        rsvg-convert -w ${SIZE_2X} -h ${SIZE_2X} "${SVG_FILE}" -o "${ICONSET_DIR}/icon_${SIZE}x${SIZE}@2x.png"
    fi
done

iconutil -c icns "${ICONSET_DIR}" -o "${ICNS_FILE}"
rm -rf "${ICONSET_DIR}"
echo "✓ Generated: ${ICNS_FILE}"
ls -lh "${ICNS_FILE}"

# ==============================================================================
# 2. 生成 Windows .ico 文件
# ==============================================================================
echo ""
echo "--- Generating Windows .ico ---"
ICO_FILE="${ICONS_DIR}/app.ico"
TEMP_DIR="${ICONS_DIR}/temp_ico"

rm -rf "${TEMP_DIR}"
mkdir -p "${TEMP_DIR}"

WIN_SIZES=(16 32 48 64 128 256)

for SIZE in "${WIN_SIZES[@]}"; do
    echo "  ${SIZE}x${SIZE}..."
    rsvg-convert -w ${SIZE} -h ${SIZE} "${SVG_FILE}" -o "${TEMP_DIR}/icon_${SIZE}.png"
done

if command -v magick &>/dev/null; then
    # 使用 ImageMagick 合并为 .ico
    magick convert "${TEMP_DIR}"/icon_*.png "${ICO_FILE}"
    echo "✓ Generated: ${ICO_FILE}"
    ls -lh "${ICO_FILE}"
elif command -v convert &>/dev/null; then
    # 旧版 ImageMagick
    convert "${TEMP_DIR}"/icon_*.png "${ICO_FILE}"
    echo "✓ Generated: ${ICO_FILE}"
    ls -lh "${ICO_FILE}"
else
    echo "WARN: ImageMagick not found, cannot generate .ico file"
    echo "Please install: brew install imagemagick"
    echo "PNG files saved in: ${TEMP_DIR}"
    exit 1
fi

rm -rf "${TEMP_DIR}"

# ==============================================================================
# 3. 生成标准 PNG 图标 (用于 Linux 和其他用途)
# ==============================================================================
echo ""
echo "--- Generating standard PNG icons ---"
PNG_SIZES=(16 24 32 48 64 128 256 512)

for SIZE in "${PNG_SIZES[@]}"; do
    PNG_FILE="${ICONS_DIR}/app_${SIZE}.png"
    rsvg-convert -w ${SIZE} -h ${SIZE} "${SVG_FILE}" -o "${PNG_FILE}"
    echo "  ${SIZE}x${SIZE} -> app_${SIZE}.png"
done

# 主图标（512x512）
MAIN_PNG="${ICONS_DIR}/app.png"
rsvg-convert -w 512 -h 512 "${SVG_FILE}" -o "${MAIN_PNG}"
echo "✓ Generated: ${MAIN_PNG}"

echo ""
echo "==> All icons generated successfully!"
echo "  macOS: ${ICNS_FILE}"
echo "  Windows: ${ICO_FILE}"
echo "  PNG: ${ICONS_DIR}/app*.png"
