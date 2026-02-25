#!/usr/bin/env bash
# ==============================================================================
# API 兼容性测试脚本
#
# 用法:
#   1. 先启动服务: make run &
#   2. 等待启动:   sleep 2
#   3. 运行测试:   bash tests/integration/test_api_compat.sh [port]
#
# 默认端口: 9527
# ==============================================================================

set -euo pipefail

PORT="${1:-9527}"
BASE="http://localhost:${PORT}"
PASS=0
FAIL=0
TOTAL=0

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# ==============================================================================
# 辅助函数
# ==============================================================================

assert_status() {
    local desc="$1"
    local expected_status="$2"
    local method="$3"
    local url="$4"
    local body="${5:-}"
    TOTAL=$((TOTAL + 1))

    local args=(-s -o /dev/null -w "%{http_code}" -X "$method")
    if [ -n "$body" ]; then
        args+=(-H "Content-Type: application/json" -d "$body")
    fi
    args+=("${BASE}${url}")

    local actual_status
    actual_status=$(curl "${args[@]}" 2>/dev/null || echo "000")

    if [ "$actual_status" = "$expected_status" ]; then
        echo -e "  ${GREEN}PASS${NC} [${actual_status}] ${desc}"
        PASS=$((PASS + 1))
    else
        echo -e "  ${RED}FAIL${NC} [${actual_status}] ${desc} (expected ${expected_status})"
        FAIL=$((FAIL + 1))
    fi
}

assert_body_contains() {
    local desc="$1"
    local expected_str="$2"
    local method="$3"
    local url="$4"
    TOTAL=$((TOTAL + 1))

    local response
    response=$(curl -s -X "$method" "${BASE}${url}" 2>/dev/null || echo "")

    if echo "$response" | grep -q "$expected_str"; then
        echo -e "  ${GREEN}PASS${NC} ${desc}"
        PASS=$((PASS + 1))
    else
        echo -e "  ${RED}FAIL${NC} ${desc} (response: ${response})"
        FAIL=$((FAIL + 1))
    fi
}

# ==============================================================================
# 检查服务是否运行
# ==============================================================================
echo -e "${YELLOW}==> API Compatibility Tests (port: ${PORT})${NC}"
echo ""

if ! curl -s -o /dev/null -w "" "${BASE}/health" 2>/dev/null; then
    echo -e "${RED}ERROR: Server not running on port ${PORT}${NC}"
    echo "Start the server first: make run &"
    exit 1
fi

# ==============================================================================
# 公共端点
# ==============================================================================
echo "--- Public Endpoints ---"
assert_status "GET /health returns 200" "200" "GET" "/health"
assert_body_contains "GET /health contains status:ok" "ok" "GET" "/health"

# ==============================================================================
# 404 处理
# ==============================================================================
echo ""
echo "--- 404 Handling ---"
assert_status "GET /nonexistent returns 404" "404" "GET" "/nonexistent"
assert_status "GET /api/v1/nonexistent returns 404" "404" "GET" "/api/v1/nonexistent"

# ==============================================================================
# 405 Method Not Allowed
# ==============================================================================
echo ""
echo "--- Method Not Allowed ---"
assert_status "POST /health returns 405" "405" "POST" "/health"
assert_status "DELETE /health returns 405" "405" "DELETE" "/health"

# ==============================================================================
# 管理端点 (无需认证)
# ==============================================================================
echo ""
echo "--- Admin Module Endpoints ---"
assert_status "GET /admin/mod/list returns 200" "200" "GET" "/admin/mod/list"

# ==============================================================================
# 业务端点 (需要模块)
# ==============================================================================
echo ""
echo "--- Business Endpoints ---"
assert_status "GET /api/v1/enum-dev returns response" "200" "GET" "/api/v1/enum-dev"

# ==============================================================================
# Content-Type 验证
# ==============================================================================
echo ""
echo "--- Content-Type ---"
TOTAL=$((TOTAL + 1))
content_type=$(curl -s -o /dev/null -w "%{content_type}" "${BASE}/health" 2>/dev/null || echo "")
if echo "$content_type" | grep -q "application/json"; then
    echo -e "  ${GREEN}PASS${NC} /health returns application/json"
    PASS=$((PASS + 1))
else
    echo -e "  ${RED}FAIL${NC} /health Content-Type: ${content_type} (expected application/json)"
    FAIL=$((FAIL + 1))
fi

# ==============================================================================
# 结果汇总
# ==============================================================================
echo ""
echo "=========================================="
if [ "$FAIL" -eq 0 ]; then
    echo -e "${GREEN}ALL PASSED: ${PASS}/${TOTAL} tests${NC}"
    exit 0
else
    echo -e "${RED}FAILED: ${FAIL}/${TOTAL} tests failed${NC}"
    exit 1
fi
