# ==============================================================================
# wekey-skf Source CMakeLists.txt
# ==============================================================================

# ==============================================================================
# 子目录 - 按依赖顺序添加
# ==============================================================================

# 基础模块 (无依赖)
add_subdirectory(common)

# 配置模块
add_subdirectory(config)

# 日志模块
add_subdirectory(log)

# 插件接口模块 (M1.5 接口定义)
add_subdirectory(plugin)

# 核心服务模块
add_subdirectory(core)

# API 模块 (M4)
add_subdirectory(api)

# GUI 模块 (M5)
add_subdirectory(gui)

# 应用入口模块
add_subdirectory(app)
