#!/bin/bash
# 测试内置模块自动注册

echo "=== 测试内置 SKF 库自动注册 ==="
echo ""

# 1. 检查可执行文件位置
EXEC_PATH="build/src/app/wekey-skf"
echo "1. 可执行文件: $EXEC_PATH"
if [ -f "$EXEC_PATH" ]; then
    echo "   ✅ 存在"
else
    echo "   ❌ 不存在"
    exit 1
fi
echo ""

# 2. 检查库文件位置
LIB_PATH="build/lib/libgm3000.dylib"
echo "2. SKF 库文件: $LIB_PATH"
if [ -f "$LIB_PATH" ]; then
    echo "   ✅ 存在"
    ls -lh "$LIB_PATH"
else
    echo "   ❌ 不存在"
    exit 1
fi
echo ""

# 3. 从可执行文件目录计算相对路径
cd build/src/app
RELATIVE_PATH="../../lib/libgm3000.dylib"
echo "3. 相对路径测试: $RELATIVE_PATH"
if [ -f "$RELATIVE_PATH" ]; then
    echo "   ✅ 可以通过相对路径访问"
    ABSOLUTE_PATH=$(cd ../../lib && pwd)/libgm3000.dylib
    echo "   绝对路径: $ABSOLUTE_PATH"
else
    echo "   ❌ 无法通过相对路径访问"
    exit 1
fi
cd ../../..
echo ""

# 4. 检查配置文件
CONFIG_FILE="$HOME/.config/TrustAsia/wekey-skf.conf"
echo "4. 配置文件: $CONFIG_FILE"
if [ -f "$CONFIG_FILE" ]; then
    echo "   ✅ 存在"
    echo "   内容预览:"
    head -20 "$CONFIG_FILE" | sed 's/^/   /'
else
    echo "   ⚠️  不存在（首次运行时会自动创建）"
fi
echo ""

echo "=== 测试完成 ==="
