# ==============================================================================
# wekey-skf Makefile
# ==============================================================================
# 便捷命令入口，封装 CMake 操作
# ==============================================================================

.PHONY: all configure build run test clean rebuild help
.PHONY: debug release asan
.PHONY: format lint
.PHONY: package-mac package-win test-api

# ==============================================================================
# 变量定义
# ==============================================================================
BUILD_DIR := build
BUILD_TYPE ?= Debug
CMAKE := cmake
CTEST := ctest
NPROC := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Qt 路径 (可通过环境变量覆盖)
# export QT_DIR=/path/to/Qt/6.x.x/gcc_64
ifdef QT_DIR
    CMAKE_PREFIX_PATH := -DCMAKE_PREFIX_PATH=$(QT_DIR)
endif

# ==============================================================================
# 默认目标
# ==============================================================================
all: build

# ==============================================================================
# 配置目标
# ==============================================================================
configure:
	@echo "==> Configuring with CMAKE_BUILD_TYPE=$(BUILD_TYPE)"
	$(CMAKE) -B $(BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		$(CMAKE_PREFIX_PATH)

# ==============================================================================
# 构建目标
# ==============================================================================
build: configure
	@echo "==> Building..."
	$(CMAKE) --build $(BUILD_DIR) -j$(NPROC)

# Debug 构建
debug:
	@$(MAKE) BUILD_TYPE=Debug build

# Release 构建
release:
	@$(MAKE) BUILD_TYPE=Release build

# AddressSanitizer 构建
asan:
	@echo "==> Building with AddressSanitizer..."
	$(CMAKE) -B $(BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DENABLE_ASAN=ON \
		$(CMAKE_PREFIX_PATH)
	$(CMAKE) --build $(BUILD_DIR) -j$(NPROC)

# ==============================================================================
# 运行目标
# ==============================================================================
run: build
	@echo "==> Running wekey-skf..."
	@if [ -f ./$(BUILD_DIR)/src/app/wekey-skf.app/Contents/MacOS/wekey-skf ]; then \
		./$(BUILD_DIR)/src/app/wekey-skf.app/Contents/MacOS/wekey-skf; \
	elif [ -f ./$(BUILD_DIR)/src/app/wekey-skf ]; then \
		./$(BUILD_DIR)/src/app/wekey-skf; \
	else \
		echo "Error: wekey-skf executable not found"; \
		exit 1; \
	fi

# ==============================================================================
# 测试目标
# ==============================================================================
test: build
	@echo "==> Running tests..."
	cd $(BUILD_DIR) && $(CTEST) --output-on-failure -j$(NPROC)

# 运行单个测试 (用法: make test-single TEST=test_config)
test-single: build
	@echo "==> Running test: $(TEST)"
	cd $(BUILD_DIR) && $(CTEST) --output-on-failure -R $(TEST)

# API 兼容性测试 (需要先启动服务)
test-api:
	@echo "==> Running API compatibility tests..."
	bash tests/integration/test_api_compat.sh

# ==============================================================================
# 打包目标
# ==============================================================================
package-mac:
	@echo "==> Packaging for macOS..."
	bash scripts/package_mac.sh

package-win:
	@echo "==> Packaging for Windows..."
	powershell -ExecutionPolicy Bypass -File scripts/package_win.ps1

# ==============================================================================
# 代码质量目标
# ==============================================================================
format:
	@echo "==> Formatting code..."
	find src tests -name "*.cpp" -o -name "*.h" | xargs clang-format -i

lint:
	@echo "==> Running clang-tidy..."
	find src -name "*.cpp" | xargs clang-tidy -p $(BUILD_DIR)

# ==============================================================================
# 清理目标
# ==============================================================================
clean:
	@echo "==> Cleaning build directory..."
	rm -rf $(BUILD_DIR)

# 重新构建
rebuild: clean build

# ==============================================================================
# 帮助信息
# ==============================================================================
help:
	@echo "wekey-skf Makefile"
	@echo ""
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Default, same as 'build'"
	@echo "  configure  - Configure CMake project"
	@echo "  build      - Build the project (Debug by default)"
	@echo "  debug      - Build with Debug configuration"
	@echo "  release    - Build with Release configuration"
	@echo "  asan       - Build with AddressSanitizer enabled"
	@echo "  run        - Build and run the application"
	@echo "  test       - Build and run all tests"
	@echo "  test-single TEST=<name> - Run a specific test"
	@echo "  test-api   - Run API compatibility tests (server must be running)"
	@echo "  package-mac - Build and package for macOS (.dmg)"
	@echo "  package-win - Build and package for Windows (.zip)"
	@echo "  format     - Format code with clang-format"
	@echo "  lint       - Run clang-tidy static analysis"
	@echo "  clean      - Remove build directory"
	@echo "  rebuild    - Clean and rebuild"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Environment variables:"
	@echo "  BUILD_TYPE - CMake build type (Debug/Release)"
	@echo "  QT_DIR     - Path to Qt installation"
