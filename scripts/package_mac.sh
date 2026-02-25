#!/usr/bin/env bash
# ==============================================================================
# macOS 打包脚本
#
# 构建 Release 版本并生成 .dmg 安装包
#
# 用法: bash scripts/package_mac.sh
# 依赖: cmake, Qt 6 (macdeployqt), hdiutil
# ==============================================================================

set -euo pipefail

APP_NAME="wekey-skf"
BUILD_DIR="build-release"
DIST_DIR="dist"
# 优先使用 CI 传入的 PKG_ARCH，本地运行时自动检测当前架构
ARCH="${PKG_ARCH:-$(uname -m)}"
# 统一为 amd64/arm64 命名
[ "${ARCH}" = "x86_64" ] && ARCH="amd64"
[ "${ARCH}" = "aarch64" ] && ARCH="arm64"
DMG_NAME="${APP_NAME}-macos-${ARCH}.dmg"

echo "==> macOS Packaging: ${APP_NAME} [${ARCH}]"

# ==============================================================================
# 1. Release 构建
# ==============================================================================
echo "--- Step 1: Release Build ---"
cmake -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=OFF

NPROC=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
cmake --build "${BUILD_DIR}" -j"${NPROC}"

# ==============================================================================
# 2. 准备分发目录
# ==============================================================================
echo "--- Step 2: Prepare Distribution ---"
rm -rf "${DIST_DIR}"
mkdir -p "${DIST_DIR}"

APP_BUNDLE="${BUILD_DIR}/src/app/${APP_NAME}.app"
BINARY="${BUILD_DIR}/src/app/${APP_NAME}"

# 检查是否是 .app bundle
if [ -d "${APP_BUNDLE}" ]; then
    echo "Found .app bundle: ${APP_BUNDLE}"
    cp -R "${APP_BUNDLE}" "${DIST_DIR}/"

    # ==============================================================================
    # 3. macdeployqt (打包 Qt 依赖)
    # ==============================================================================
    echo "--- Step 3: Deploy Qt Dependencies ---"
    if command -v macdeployqt &>/dev/null; then
        macdeployqt "${DIST_DIR}/${APP_NAME}.app" -verbose=2
        echo "Qt dependencies bundled successfully"
    else
        echo "ERROR: macdeployqt not found, please install Qt"
        exit 1
    fi
elif [ -f "${BINARY}" ]; then
    echo "Found standalone binary: ${BINARY}"
    cp "${BINARY}" "${DIST_DIR}/"
    echo "WARN: No .app bundle, Qt dependencies not bundled"
else
    echo "ERROR: Neither .app bundle nor binary found"
    echo "Expected: ${APP_BUNDLE} or ${BINARY}"
    exit 1
fi

# ==============================================================================
# 4. 复制 vendor 库 (SKF 驱动)
# ==============================================================================
echo "--- Step 4: Copy Vendor Libraries ---"
if [ -d "vendor" ]; then
    cp -R vendor "${DIST_DIR}/vendor"
fi

# ==============================================================================
# 5. 代码签名 (Ad-hoc 签名)
# ==============================================================================
echo "--- Step 5: Code Signing ---"
if [ -d "${DIST_DIR}/${APP_NAME}.app" ]; then
    echo "Signing application bundle with ad-hoc signature..."

    # 签名 Frameworks 中的所有 dylib
    find "${DIST_DIR}/${APP_NAME}.app/Contents/Frameworks" -type f \( -name "*.dylib" -o -name "*.so" \) 2>/dev/null | while read -r lib; do
        codesign --force --sign - --timestamp=none "$lib" 2>/dev/null || true
    done

    # 签名 Frameworks 中的 framework bundle
    find "${DIST_DIR}/${APP_NAME}.app/Contents/Frameworks" -name "*.framework" -maxdepth 1 2>/dev/null | while read -r fw; do
        codesign --force --sign - --timestamp=none "$fw" 2>/dev/null || true
    done

    # 签名 PlugIns
    find "${DIST_DIR}/${APP_NAME}.app/Contents/PlugIns" -type f -name "*.dylib" 2>/dev/null | while read -r plugin; do
        codesign --force --sign - --timestamp=none "$plugin" 2>/dev/null || true
    done

    # 签名主可执行文件
    codesign --force --sign - --timestamp=none "${DIST_DIR}/${APP_NAME}.app/Contents/MacOS/${APP_NAME}" 2>/dev/null || true

    # 签名整个 .app bundle
    codesign --force --deep --sign - --timestamp=none "${DIST_DIR}/${APP_NAME}.app"

    # 验证签名
    if codesign --verify --deep --strict "${DIST_DIR}/${APP_NAME}.app" 2>&1; then
        echo "Code signing successful"
    else
        echo "WARN: Code signing verification failed, but continuing..."
    fi
else
    echo "Skipping code signing (no .app bundle)"
fi

# ==============================================================================
# 5.1 创建用户说明文件
# ==============================================================================
cat > "${DIST_DIR}/安装说明.txt" << 'README'
# wekey-skf 安装说明

## 首次打开
由于本应用未经 Apple 公证，首次打开时 macOS 会提示安全警告。
请按以下步骤操作：

方法一（推荐）：
1. 右键点击 wekey-skf.app
2. 选择“打开”
3. 在弹出的对话框中点击“打开”

方法二（终端命令）：
打开终端，执行：
  sudo xattr -rd com.apple.quarantine /Applications/wekey-skf.app

方法三（系统设置）：
1. 打开“系统设置” -> “隐私与安全性”
2. 在“安全性”部分找到“wekey-skf”相关提示
3. 点击“仍然打开”
README

# ==============================================================================
# 6. 创建 DMG
# ==============================================================================
echo "--- Step 6: Create DMG ---"
rm -f "${DMG_NAME}"
hdiutil create -volname "${APP_NAME}" \
    -srcfolder "${DIST_DIR}" \
    -ov -format UDZO \
    "${DMG_NAME}"

echo ""
echo "==> Package created: ${DMG_NAME} (arch=${ARCH})"
ls -lh "${DMG_NAME}"
