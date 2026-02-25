# wekey-skf 任务列表

> **版本**: 1.0.0
> **日期**: 2026-02-06
> **说明**: 所有任务遵循 TDD 原则，测试任务 (T) 在实现任务 (I) 之前
> **标记**: `[P]` = 可并行执行，`[S]` = 串行依赖前置任务

---

## 任务编号规则

```
M{里程碑}.{阶段}.{序号}{类型}

类型:
  T = 测试任务 (Test)
  I = 实现任务 (Implementation)
  C = 配置任务 (Configuration)
  D = 文档任务 (Documentation)
```

---

## M1: 基础框架

### M1.1 项目初始化

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M1.1.1C | 创建根目录 CMakeLists.txt | `CMakeLists.txt` | - | [P] |
| M1.1.2C | 创建 src 目录 CMakeLists.txt | `src/CMakeLists.txt` | M1.1.1C | [S] |
| M1.1.3C | 创建 tests 目录 CMakeLists.txt | `tests/CMakeLists.txt` | M1.1.1C | [S] |
| M1.1.4C | 创建 Makefile 便捷入口 | `Makefile` | M1.1.1C | [P] |
| M1.1.5C | 创建 .clang-format 配置 | `.clang-format` | - | [P] |
| M1.1.6C | 创建 .gitignore | `.gitignore` | - | [P] |

**M1.1.1C 详细说明:**
```
文件: CMakeLists.txt
内容:
- cmake_minimum_required(VERSION 3.20)
- project(wekey-skf VERSION 1.0.0)
- set(CMAKE_CXX_STANDARD 17)
- find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Test)
- add_subdirectory(src)
- add_subdirectory(tests)
验收: cmake -B build 成功
```

**M1.1.2C 详细说明:**
```
文件: src/CMakeLists.txt
内容:
- 定义 wekey-skf 可执行目标
- 添加各子目录 (common, config, log, plugin, core, api, gui)
- 链接 Qt6::Core, Qt6::Widgets, Qt6::Network
验收: 空项目可编译
```

**M1.1.3C 详细说明:**
```
文件: tests/CMakeLists.txt
内容:
- enable_testing()
- 添加 unit/ 和 integration/ 子目录
- 链接 Qt6::Test
验收: ctest 可运行（即使无测试）
```

**M1.1.4C 详细说明:**
```
文件: Makefile
内容:
- configure: cmake -B build
- build: cmake --build build
- run: ./build/src/wekey-skf
- test: cd build && ctest
- clean: rm -rf build
验收: make build 成功
```

---

### M1.2 公共模块 (src/common/)

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M1.2.1C | 创建 common 模块 CMakeLists.txt | `src/common/CMakeLists.txt` | M1.1.2C | [S] |
| M1.2.2T | 编写 Error 类单元测试 | `tests/unit/test_error.cpp` | M1.1.3C | [P] |
| M1.2.3I | 实现 Error.h 头文件 | `src/common/Error.h` | M1.2.1C | [S] |
| M1.2.4I | 实现 Error.cpp | `src/common/Error.cpp` | M1.2.3I | [S] |
| M1.2.5T | 编写 Result<T> 单元测试 | `tests/unit/test_result.cpp` | M1.1.3C | [P] |
| M1.2.6I | 实现 Result.h 模板 | `src/common/Result.h` | M1.2.4I | [S] |
| M1.2.7T | 编写 Utils 单元测试 | `tests/unit/test_utils.cpp` | M1.1.3C | [P] |
| M1.2.8I | 实现 Utils.h | `src/common/Utils.h` | M1.2.1C | [P] |
| M1.2.9I | 实现 Utils.cpp | `src/common/Utils.cpp` | M1.2.8I | [S] |

**M1.2.2T 详细说明:**
```
文件: tests/unit/test_error.cpp
测试用例:
- testDefaultConstruction: 默认构造 code = Success
- testCodeConstruction: 指定错误码构造
- testWithMessage: 错误码 + 消息
- testWithContext: 错误码 + 消息 + 上下文
- testFromSkf: 从 SKF 错误码构造
- testFriendlyMessage: 获取友好错误描述
- testToStringSimple: 简洁模式字符串
- testToStringDetailed: 详细模式字符串
验收: 测试编译通过，运行失败（Red）
```

**M1.2.3I 详细说明:**
```
文件: src/common/Error.h
内容:
- namespace wekey
- enum Code: uint32_t (Success, Fail, InvalidParam, ...)
- SKF 错误码映射 (SkfOk, SkfFail, SkfPinIncorrect, ...)
- 构造函数: Error(), Error(Code, QString, QString)
- Getters: code(), message(), context()
- toString(bool detailed), friendlyMessage()
- static fromSkf(uint32_t, QString)
验收: M1.2.2T 测试通过（Green）
```

**M1.2.5T 详细说明:**
```
文件: tests/unit/test_result.cpp
测试用例:
- testOkInt: Result<int>::ok(42) 值正确
- testOkString: Result<QString>::ok("hello")
- testErrInt: Result<int>::err(Error) 错误正确
- testVoidOk: Result<void>::ok() 状态正确
- testVoidErr: Result<void>::err(Error)
- testIsOkIsErr: isOk()/isErr() 互斥
- testMap: map() 转换成功值
- testMapOnErr: map() 对错误透传
- testAndThen: andThen() 链式调用
- testMoveValue: value() 右值引用移动语义
验收: 测试编译通过，运行失败（Red）
```

**M1.2.6I 详细说明:**
```
文件: src/common/Result.h
内容:
- template<typename T> class Result
- static ok(T), static err(Error)
- isOk(), isErr()
- value() const&, value() &&
- error()
- map(F), andThen(F)
- Result<void> 特化
验收: M1.2.5T 测试通过（Green）
```

---

### M1.3 配置模块 (src/config/)

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M1.3.1C | 创建 config 模块 CMakeLists.txt | `src/config/CMakeLists.txt` | M1.1.2C | [S] |
| M1.3.2I | 创建 Defaults.h 常量定义 | `src/config/Defaults.h` | M1.3.1C | [S] |
| M1.3.3T | 编写 Config 单元测试 | `tests/unit/test_config.cpp` | M1.2.6I | [S] |
| M1.3.4I | 实现 Config.h | `src/config/Config.h` | M1.3.2I | [S] |
| M1.3.5I | 实现 Config.cpp | `src/config/Config.cpp` | M1.3.4I | [S] |

**M1.3.2I 详细说明:**
```
文件: src/config/Defaults.h
内容:
- namespace wekey::defaults
- constexpr char* CONFIG_FILE = ".wekeytool.json"
- constexpr char* DEFAULT_PORT = ":9001"
- constexpr char* DEFAULT_LOG_LEVEL = "info"
- constexpr char* DEFAULT_APP_NAME = "TAGM"
- constexpr char* DEFAULT_CONTAINER_NAME = "TrustAsia"
- constexpr int ADMIN_PIN_RETRY = 6
- constexpr int USER_PIN_RETRY = 6
验收: 头文件可被包含
```

**M1.3.3T 详细说明:**
```
文件: tests/unit/test_config.cpp
测试用例:
- testSingleton: instance() 返回同一实例
- testDefaultValues: 默认值正确
- testSettersGetters: 设置后获取值正确
- testModPaths: 添加/删除模块路径
- testDefaults: 默认应用名/容器名/通用名
- testLoadNonExistent: 加载不存在文件返回 true（使用默认值）
- testSaveLoad: 保存后加载值一致
- testReset: reset() 恢复默认值
- testConfigChanged: 信号发射
验收: 测试编译通过，运行失败（Red）
```

**M1.3.4I 详细说明:**
```
文件: src/config/Config.h
内容:
- class Config : public QObject
- Q_OBJECT 宏
- static Config& instance()
- load(), save(), reset()
- Getters: listenPort(), logLevel(), logPath(), ...
- Setters: setListenPort(), setLogLevel(), ...
- modPaths(), setModPath(), removeModPath()
- defaultAppName(), defaultContainerName(), ...
- signal: configChanged()
验收: M1.3.3T 测试通过（Green）
```

---

### M1.4 日志模块 (src/log/)

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M1.4.1C | 创建 log 模块 CMakeLists.txt | `src/log/CMakeLists.txt` | M1.1.2C | [S] |
| M1.4.2T | 编写 Logger 单元测试 | `tests/unit/test_logger.cpp` | M1.3.5I | [S] |
| M1.4.3I | 实现 Logger.h | `src/log/Logger.h` | M1.4.1C | [S] |
| M1.4.4I | 实现 Logger.cpp | `src/log/Logger.cpp` | M1.4.3I | [S] |
| M1.4.5T | 编写 LogModel 单元测试 | `tests/unit/test_logmodel.cpp` | M1.4.4I | [S] |
| M1.4.6I | 实现 LogModel.h | `src/log/LogModel.h` | M1.4.4I | [S] |
| M1.4.7I | 实现 LogModel.cpp | `src/log/LogModel.cpp` | M1.4.6I | [S] |

**M1.4.2T 详细说明:**
```
文件: tests/unit/test_logger.cpp
测试用例:
- testSingleton: instance() 返回同一实例
- testDefaultLevel: 默认级别为 Info
- testSetLevel: 设置级别后过滤低级别日志
- testDebug: debug() 在 Debug 级别输出
- testInfo: info() 在 Info 级别输出
- testWarn: warn() 在 Warn 级别输出
- testError: error() 在 Error 级别输出
- testLogAddedSignal: 日志添加时发出信号
- testFileOutput: 设置路径后写入文件
- testLogFormat: 日志格式正确（时间戳、级别、来源、消息）
验收: 测试编译通过，运行失败（Red）
```

**M1.4.5T 详细说明:**
```
文件: tests/unit/test_logmodel.cpp
测试用例:
- testRowCount: 行数与日志条目数一致
- testColumnCount: 列数为 4（时间、级别、来源、消息）
- testData: data() 返回正确内容
- testHeaderData: 表头正确
- testOnLogAdded: 添加日志后行数增加
- testClear: clear() 后行数为 0
- testSetFilter: 按级别过滤
- testSetSearchText: 按文本搜索
验收: 测试编译通过，运行失败（Red）
```

---

### M1.5 插件接口 (src/plugin/interface/)

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M1.5.1C | 创建 plugin 模块 CMakeLists.txt | `src/plugin/CMakeLists.txt` | M1.1.2C | [S] |
| M1.5.2C | 创建 interface 子目录 CMakeLists.txt | `src/plugin/interface/CMakeLists.txt` | M1.5.1C | [S] |
| M1.5.3I | 实现 PluginTypes.h 数据结构 | `src/plugin/interface/PluginTypes.h` | M1.5.2C | [S] |
| M1.5.4I | 实现 IDriverPlugin.h 接口 | `src/plugin/interface/IDriverPlugin.h` | M1.5.3I, M1.2.6I | [S] |

**M1.5.3I 详细说明:**
```
文件: src/plugin/interface/PluginTypes.h
内容:
- namespace wekey
- struct DeviceInfo { deviceName, devicePath, manufacturer, label, serialNumber, hwVersion, fwVersion, isLoggedIn }
- struct AppInfo { appName, isLoggedIn }
- struct ContainerInfo { containerName, keyGenerated, keyType, certImported }
- struct CertInfo { serialNumber, subject, commonName, issuer, notBefore, notAfter, certType, certPem, publicKeyHash }
- enum class KeyType { Unknown, RSA_2048, RSA_3072, RSA_4096, SM2 }
- enum class DeviceEvent { None, Inserted, Removed }
- Q_DECLARE_METATYPE 宏
验收: 头文件可被包含，类型可实例化
```

**M1.5.4I 详细说明:**
```
文件: src/plugin/interface/IDriverPlugin.h
内容:
- class IDriverPlugin (纯虚接口)
- 设备管理: enumDevices, changeDeviceAuth, setDeviceLabel, waitForDeviceEvent
- 应用管理: enumApps, createApp, deleteApp, loginApp, logoutApp, updateAppPin, unblockApp, getRetryCount
- 容器管理: enumContainers, createContainer, deleteContainer
- 证书操作: generateCsr, importCertificate, exportCertificate, verifyCertificate
- 签名验签: sign, verify
- 文件操作: enumFiles, createFile, readFile, deleteFile
- 其他: generateRandom
- Q_DECLARE_INTERFACE 宏
验收: 接口可被继承
```

---

### M1.6 应用入口 (src/app/)

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M1.6.1C | 创建 app 模块 CMakeLists.txt | `src/app/CMakeLists.txt` | M1.1.2C | [S] |
| M1.6.2T | 编写 Application 单元测试 | `tests/unit/test_application.cpp` | M1.4.4I | [S] |
| M1.6.3I | 实现 Application.h | `src/app/Application.h` | M1.6.1C | [S] |
| M1.6.4I | 实现 Application.cpp | `src/app/Application.cpp` | M1.6.3I | [S] |
| M1.6.5I | 实现 main.cpp | `src/app/main.cpp` | M1.6.4I | [S] |

**M1.6.2T 详细说明:**
```
文件: tests/unit/test_application.cpp
测试用例:
- testSingleInstance: 第二个实例检测到已有实例
- testInitialize: 初始化加载配置和日志
验收: 测试编译通过，运行失败（Red）
```

**M1.6.3I 详细说明:**
```
文件: src/app/Application.h
内容:
- class Application : public QApplication
- 单实例控制 (QLockFile)
- bool initialize()
- bool isRunning() - 检测是否已有实例
- signal: anotherInstanceStarted()
验收: M1.6.2T 测试通过（Green）
```

**M1.6.5I 详细说明:**
```
文件: src/app/main.cpp
内容:
- int main(int argc, char* argv[])
- 创建 Application 实例
- 检测单实例
- 加载配置
- 初始化日志
- 返回 app.exec()
验收: make run 启动应用（空窗口）
```

---

### M1.7 验收检查点

| ID | 任务 | 依赖 | 验收标准 |
|----|------|------|----------|
| M1.7.1 | M1 集成验证 | M1.6.5I | `make build` 成功 |
| M1.7.2 | M1 测试验证 | M1.7.1 | `make test` 所有测试通过 |

---

## M2: SKF 驱动插件

### M2.1 SKF C API 封装

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M2.1.1C | 创建 skf 插件目录 CMakeLists.txt | `src/plugin/skf/CMakeLists.txt` | M1.5.1C | [S] |
| M2.1.2I | 定义 SKF C API 头文件 | `src/plugin/skf/SkfApi.h` | M2.1.1C | [S] |
| M2.1.3I | 定义 SKF 类型别名 | `src/plugin/skf/SkfTypes.h` | M2.1.2I | [S] |
| M2.1.4C | 创建插件元数据 JSON | `src/plugin/skf/skf.json` | M2.1.1C | [P] |

**M2.1.2I 详细说明:**
```
文件: src/plugin/skf/SkfApi.h
内容:
- #pragma pack(push, 1) 结构体对齐
- DEVHANDLE, HAPPLICATION, HCONTAINER 句柄类型
- BYTE, LPSTR, ULONG 基本类型
- VERSION, DEVINFO 结构体
- ECCPUBLICKEYBLOB, ECCSIGNATUREBLOB, ECCCIPHERBLOB 密码结构
- SAR_OK, SAR_FAIL, SAR_PIN_INCORRECT 等错误码
- PFN_SKF_* 函数指针类型定义 (30+ 个)
验收: 头文件可被包含，结构体大小正确
```

**M2.1.4C 详细说明:**
```
文件: src/plugin/skf/skf.json
内容:
{
    "Keys": ["skf"],
    "Name": "SKF Driver Plugin",
    "Version": "1.0.0",
    "Vendor": "TrustAsia"
}
验收: JSON 格式正确
```

---

### M2.2 动态库加载

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M2.2.1T | 编写 SkfLibrary 单元测试 | `tests/unit/test_skflibrary.cpp` | M2.1.3I | [S] |
| M2.2.2I | 实现 SkfLibrary.h | `src/plugin/skf/SkfLibrary.h` | M2.1.3I | [S] |
| M2.2.3I | 实现 SkfLibrary.cpp | `src/plugin/skf/SkfLibrary.cpp` | M2.2.2I | [S] |

**M2.2.1T 详细说明:**
```
文件: tests/unit/test_skflibrary.cpp
测试用例:
- testLoadNonExistent: 加载不存在文件返回错误
- testIsLoaded: 加载成功后 isLoaded() 返回 true
- testErrorString: 失败时 errorString() 非空
- testSymbolsLoaded: 核心函数指针非空 (Mock 库)
  - EnumDev != nullptr
  - ConnectDev != nullptr
  - GetDevInfo != nullptr
  - ...
验收: 测试编译通过（需要 Mock 库或跳过真实设备测试）
```

**M2.2.2I 详细说明:**
```
文件: src/plugin/skf/SkfLibrary.h
内容:
- class SkfLibrary
- 构造函数: SkfLibrary(const QString& path)
- isLoaded(), errorString()
- 所有 PFN_SKF_* 函数指针成员变量
- loadSymbols() 私有方法
- QLibrary lib_ 成员
验收: M2.2.1T 测试通过（Green）
```

---

### M2.3 SKF 插件实现 - 设备管理

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M2.3.1T | 编写 SkfPlugin 设备管理测试 | `tests/unit/test_skfplugin_device.cpp` | M2.2.3I | [S] |
| M2.3.2I | 实现 SkfPlugin.h 框架 | `src/plugin/skf/SkfPlugin.h` | M2.2.3I | [S] |
| M2.3.3I | 实现 SkfPlugin 构造/析构 | `src/plugin/skf/SkfPlugin.cpp` | M2.3.2I | [S] |
| M2.3.4I | 实现 SkfPlugin::enumDevices | `src/plugin/skf/SkfPlugin.cpp` | M2.3.3I | [S] |
| M2.3.5I | 实现 SkfPlugin::changeDeviceAuth | `src/plugin/skf/SkfPlugin.cpp` | M2.3.4I | [S] |
| M2.3.6I | 实现 SkfPlugin::setDeviceLabel | `src/plugin/skf/SkfPlugin.cpp` | M2.3.5I | [S] |
| M2.3.7I | 实现 SkfPlugin::waitForDeviceEvent | `src/plugin/skf/SkfPlugin.cpp` | M2.3.6I | [S] |

**M2.3.1T 详细说明:**
```
文件: tests/unit/test_skfplugin_device.cpp
测试用例:
- testEnumDevicesEmpty: 无设备时返回空列表
- testEnumDevicesWithDevice: 有设备时返回正确信息
- testEnumDevicesWithLogin: login=true 时返回登录状态
- testChangeDeviceAuth: 修改认证密钥成功
- testChangeDeviceAuthWrongPin: 错误旧密钥返回错误
- testSetDeviceLabel: 设置标签成功
- testSetDeviceLabelTooLong: 标签过长返回错误
验收: 测试编译通过，运行失败（Red）
```

---

### M2.4 SKF 插件实现 - 应用管理

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M2.4.1T | 编写 SkfPlugin 应用管理测试 | `tests/unit/test_skfplugin_app.cpp` | M2.3.7I | [S] |
| M2.4.2I | 实现 SkfPlugin::enumApps | `src/plugin/skf/SkfPlugin.cpp` | M2.3.7I | [S] |
| M2.4.3I | 实现 SkfPlugin::createApp | `src/plugin/skf/SkfPlugin.cpp` | M2.4.2I | [S] |
| M2.4.4I | 实现 SkfPlugin::deleteApp | `src/plugin/skf/SkfPlugin.cpp` | M2.4.3I | [S] |
| M2.4.5I | 实现 SkfPlugin::loginApp | `src/plugin/skf/SkfPlugin.cpp` | M2.4.4I | [S] |
| M2.4.6I | 实现 SkfPlugin::logoutApp | `src/plugin/skf/SkfPlugin.cpp` | M2.4.5I | [S] |
| M2.4.7I | 实现 SkfPlugin::updateAppPin | `src/plugin/skf/SkfPlugin.cpp` | M2.4.6I | [S] |
| M2.4.8I | 实现 SkfPlugin::unblockApp | `src/plugin/skf/SkfPlugin.cpp` | M2.4.7I | [S] |
| M2.4.9I | 实现 SkfPlugin::getRetryCount | `src/plugin/skf/SkfPlugin.cpp` | M2.4.8I | [S] |

**M2.4.1T 详细说明:**
```
文件: tests/unit/test_skfplugin_app.cpp
测试用例:
- testEnumAppsEmpty: 无应用返回空列表
- testEnumAppsWithApps: 有应用返回正确信息
- testCreateApp: 创建应用成功
- testCreateAppDuplicate: 重复创建返回错误
- testDeleteApp: 删除应用成功
- testDeleteAppNotExists: 删除不存在应用返回错误
- testLoginAppUser: 用户角色登录成功
- testLoginAppAdmin: 管理员角色登录成功
- testLoginAppWrongPin: 错误 PIN 返回错误
- testLoginAppLocked: PIN 锁定返回特定错误
- testLogoutApp: 登出成功
- testUpdateAppPin: 修改 PIN 成功
- testUnblockApp: 解锁成功
- testGetRetryCount: 返回剩余次数
验收: 测试编译通过，运行失败（Red）
```

---

### M2.5 SKF 插件实现 - 容器管理

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M2.5.1T | 编写 SkfPlugin 容器管理测试 | `tests/unit/test_skfplugin_container.cpp` | M2.4.9I | [S] |
| M2.5.2I | 实现 SkfPlugin::enumContainers | `src/plugin/skf/SkfPlugin.cpp` | M2.4.9I | [S] |
| M2.5.3I | 实现 SkfPlugin::createContainer | `src/plugin/skf/SkfPlugin.cpp` | M2.5.2I | [S] |
| M2.5.4I | 实现 SkfPlugin::deleteContainer | `src/plugin/skf/SkfPlugin.cpp` | M2.5.3I | [S] |

**M2.5.1T 详细说明:**
```
文件: tests/unit/test_skfplugin_container.cpp
测试用例:
- testEnumContainersEmpty: 无容器返回空列表
- testEnumContainersWithContainers: 有容器返回正确信息
- testCreateContainer: 创建容器成功
- testCreateContainerDuplicate: 重复创建返回错误
- testDeleteContainer: 删除容器成功
- testDeleteContainerNotExists: 删除不存在容器返回错误
验收: 测试编译通过，运行失败（Red）
```

---

### M2.6 SKF 插件实现 - 证书操作

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M2.6.1T | 编写 SkfPlugin 证书操作测试 | `tests/unit/test_skfplugin_cert.cpp` | M2.5.4I | [S] |
| M2.6.2I | 实现 SkfPlugin::generateCsr | `src/plugin/skf/SkfPlugin.cpp` | M2.5.4I | [S] |
| M2.6.3I | 实现 SkfPlugin::importCertificate | `src/plugin/skf/SkfPlugin.cpp` | M2.6.2I | [S] |
| M2.6.4I | 实现 SkfPlugin::exportCertificate | `src/plugin/skf/SkfPlugin.cpp` | M2.6.3I | [S] |
| M2.6.5I | 实现 SkfPlugin::verifyCertificate | `src/plugin/skf/SkfPlugin.cpp` | M2.6.4I | [S] |

**M2.6.1T 详细说明:**
```
文件: tests/unit/test_skfplugin_cert.cpp
测试用例:
- testGenerateCsrSm2: SM2 密钥 CSR 生成
- testGenerateCsrRsa2048: RSA-2048 密钥 CSR 生成
- testGenerateCsrRsa3072: RSA-3072 密钥 CSR 生成
- testGenerateCsrRsa4096: RSA-4096 密钥 CSR 生成
- testGenerateCsrWithSubject: CSR 包含正确主题
- testGenerateCsrRenew: 重新生成密钥对
- testImportCertificate: 导入单个证书
- testImportCertificateChain: 导入证书链
- testExportCertificate: 导出证书
- testVerifyCertificate: 验证证书有效
- testVerifyCertificateExpired: 验证过期证书
验收: 测试编译通过，运行失败（Red）
```

---

### M2.7 SKF 插件实现 - 签名和其他

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M2.7.1T | 编写 SkfPlugin 签名测试 | `tests/unit/test_skfplugin_sign.cpp` | M2.6.5I | [S] |
| M2.7.2I | 实现 SkfPlugin::sign | `src/plugin/skf/SkfPlugin.cpp` | M2.6.5I | [S] |
| M2.7.3I | 实现 SkfPlugin::verify | `src/plugin/skf/SkfPlugin.cpp` | M2.7.2I | [S] |
| M2.7.4T | 编写 SkfPlugin 文件操作测试 | `tests/unit/test_skfplugin_file.cpp` | M2.7.3I | [S] |
| M2.7.5I | 实现 SkfPlugin::enumFiles | `src/plugin/skf/SkfPlugin.cpp` | M2.7.3I | [S] |
| M2.7.6I | 实现 SkfPlugin::createFile | `src/plugin/skf/SkfPlugin.cpp` | M2.7.5I | [S] |
| M2.7.7I | 实现 SkfPlugin::readFile | `src/plugin/skf/SkfPlugin.cpp` | M2.7.6I | [S] |
| M2.7.8I | 实现 SkfPlugin::deleteFile | `src/plugin/skf/SkfPlugin.cpp` | M2.7.7I | [S] |
| M2.7.9I | 实现 SkfPlugin::generateRandom | `src/plugin/skf/SkfPlugin.cpp` | M2.7.8I | [S] |

**M2.7.1T 详细说明:**
```
文件: tests/unit/test_skfplugin_sign.cpp
测试用例:
- testSignSm2: SM2 签名成功
- testSignRsa: RSA 签名成功
- testSignNotLoggedIn: 未登录返回错误
- testVerifyValid: 验证有效签名
- testVerifyInvalid: 验证无效签名返回错误
- testVerifyTampered: 验证被篡改数据返回错误
验收: 测试编译通过，运行失败（Red）
```

---

### M2.8 插件管理器

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M2.8.1T | 编写 PluginManager 单元测试 | `tests/unit/test_pluginmanager.cpp` | M2.7.9I | [S] |
| M2.8.2I | 实现 PluginManager.h | `src/plugin/PluginManager.h` | M1.5.4I | [S] |
| M2.8.3I | 实现 PluginManager.cpp | `src/plugin/PluginManager.cpp` | M2.8.2I | [S] |

**M2.8.1T 详细说明:**
```
文件: tests/unit/test_pluginmanager.cpp
测试用例:
- testSingleton: instance() 返回同一实例
- testRegisterPlugin: 注册插件成功
- testRegisterPluginInvalidPath: 无效路径返回错误
- testUnregisterPlugin: 卸载插件成功
- testGetPlugin: 获取已注册插件
- testGetPluginNotExists: 获取未注册插件返回 nullptr
- testActivePlugin: 获取当前激活插件
- testSetActivePlugin: 设置激活插件
- testListPlugins: 列出所有已注册插件
- testPluginRegisteredSignal: 注册时发出信号
- testPluginUnregisteredSignal: 卸载时发出信号
- testActivePluginChangedSignal: 切换时发出信号
验收: 测试编译通过，运行失败（Red）
```

---

### M2.9 验收检查点

| ID | 任务 | 依赖 | 验收标准 |
|----|------|------|----------|
| M2.9.1 | M2 集成验证 | M2.8.3I | SKF 插件可加载 |
| M2.9.2 | M2 测试验证 | M2.9.1 | `make test` SKF 相关测试通过 |

---

## M3: 核心功能

### M3.1 核心模块结构

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M3.1.1C | 创建 core 模块 CMakeLists.txt | `src/core/CMakeLists.txt` | M1.1.2C | [S] |
| M3.1.2C | 创建 device 子目录 CMakeLists.txt | `src/core/device/CMakeLists.txt` | M3.1.1C | [P] |
| M3.1.3C | 创建 application 子目录 CMakeLists.txt | `src/core/application/CMakeLists.txt` | M3.1.1C | [P] |
| M3.1.4C | 创建 container 子目录 CMakeLists.txt | `src/core/container/CMakeLists.txt` | M3.1.1C | [P] |
| M3.1.5C | 创建 crypto 子目录 CMakeLists.txt | `src/core/crypto/CMakeLists.txt` | M3.1.1C | [P] |
| M3.1.6C | 创建 file 子目录 CMakeLists.txt | `src/core/file/CMakeLists.txt` | M3.1.1C | [P] |

---

### M3.2 设备服务

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M3.2.1T | 编写 DeviceService 单元测试 | `tests/unit/test_deviceservice.cpp` | M2.8.3I | [S] |
| M3.2.2I | 实现 DeviceService.h | `src/core/device/DeviceService.h` | M3.1.2C | [S] |
| M3.2.3I | 实现 DeviceService.cpp | `src/core/device/DeviceService.cpp` | M3.2.2I | [S] |

**M3.2.1T 详细说明:**
```
文件: tests/unit/test_deviceservice.cpp
测试用例:
- testSingleton: instance() 返回同一实例
- testEnumDevices: 枚举设备委托给插件
- testEnumDevicesNoActivePlugin: 无激活插件返回错误
- testChangeDeviceAuth: 修改认证密钥委托给插件
- testSetDeviceLabel: 设置标签委托给插件
- testStartDeviceMonitor: 启动监听线程
- testStopDeviceMonitor: 停止监听线程
- testDeviceInsertedSignal: 设备插入时发出信号
- testDeviceRemovedSignal: 设备移除时发出信号
验收: 测试编译通过，运行失败（Red）
```

**M3.2.2I 详细说明:**
```
文件: src/core/device/DeviceService.h
内容:
- class DeviceService : public QObject
- static DeviceService& instance()
- enumDevices(bool login = false)
- changeDeviceAuth(devName, oldPin, newPin)
- setDeviceLabel(devName, label)
- startDeviceMonitor()
- stopDeviceMonitor()
- signals: deviceInserted, deviceRemoved, deviceListChanged
- private: monitorLoop(), monitorThread_, monitoring_
验收: M3.2.1T 测试通过（Green）
```

---

### M3.3 应用服务

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M3.3.1T | 编写 AppService 单元测试 | `tests/unit/test_appservice.cpp` | M3.2.3I | [S] |
| M3.3.2I | 实现 AppService.h | `src/core/application/AppService.h` | M3.1.3C | [S] |
| M3.3.3I | 实现 AppService.cpp | `src/core/application/AppService.cpp` | M3.3.2I | [S] |

**M3.3.1T 详细说明:**
```
文件: tests/unit/test_appservice.cpp
测试用例:
- testEnumApps: 枚举应用委托给插件
- testCreateApp: 创建应用委托给插件
- testDeleteApp: 删除应用委托给插件
- testLogin: 登录应用委托给插件
- testLogout: 登出应用委托给插件
- testChangePin: 修改 PIN 委托给插件
- testUnblockPin: 解锁 PIN 委托给插件
- testGetRetryCount: 获取剩余次数委托给插件
- testLoginStateChangedSignal: 登录状态变化时发出信号
- testPinErrorSignal: PIN 错误时发出信号（含剩余次数）
- testPinLockedSignal: PIN 锁定时发出信号
验收: 测试编译通过，运行失败（Red）
```

---

### M3.4 容器服务

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M3.4.1T | 编写 ContainerService 单元测试 | `tests/unit/test_containerservice.cpp` | M3.3.3I | [S] |
| M3.4.2I | 实现 ContainerService.h | `src/core/container/ContainerService.h` | M3.1.4C | [S] |
| M3.4.3I | 实现 ContainerService.cpp | `src/core/container/ContainerService.cpp` | M3.4.2I | [S] |

**M3.4.1T 详细说明:**
```
文件: tests/unit/test_containerservice.cpp
测试用例:
- testEnumContainers: 枚举容器委托给插件
- testCreateContainer: 创建容器委托给插件
- testDeleteContainer: 删除容器委托给插件
验收: 测试编译通过，运行失败（Red）
```

---

### M3.5 证书服务

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M3.5.1T | 编写 CertService 单元测试 | `tests/unit/test_certservice.cpp` | M3.4.3I | [S] |
| M3.5.2I | 实现 CertService.h | `src/core/crypto/CertService.h` | M3.1.5C | [S] |
| M3.5.3I | 实现 CertService.cpp | `src/core/crypto/CertService.cpp` | M3.5.2I | [S] |

**M3.5.1T 详细说明:**
```
文件: tests/unit/test_certservice.cpp
测试用例:
- testGenerateCsr: 生成 CSR 委托给插件
- testImportCertificate: 导入证书委托给插件
- testImportCertificateChain: 导入证书链正确拆分
- testExportCertificate: 导出证书委托给插件
- testVerifyCertificate: 验证证书委托给插件
- testSign: 签名委托给插件
- testVerify: 验签委托给插件
验收: 测试编译通过，运行失败（Red）
```

---

### M3.6 文件服务

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M3.6.1T | 编写 FileService 单元测试 | `tests/unit/test_fileservice.cpp` | M3.5.3I | [S] |
| M3.6.2I | 实现 FileService.h | `src/core/file/FileService.h` | M3.1.6C | [S] |
| M3.6.3I | 实现 FileService.cpp | `src/core/file/FileService.cpp` | M3.6.2I | [S] |

**M3.6.1T 详细说明:**
```
文件: tests/unit/test_fileservice.cpp
测试用例:
- testEnumFiles: 枚举文件委托给插件
- testCreateFile: 创建文件委托给插件
- testReadFile: 读取文件委托给插件
- testDeleteFile: 删除文件委托给插件
- testGenerateRandom: 生成随机数委托给插件
验收: 测试编译通过，运行失败（Red）
```

---

### M3.7 验收检查点

| ID | 任务 | 依赖 | 验收标准 |
|----|------|------|----------|
| M3.7.1 | M3 集成验证 | M3.6.3I | 所有服务可实例化 |
| M3.7.2 | M3 测试验证 | M3.7.1 | `make test` 核心服务测试通过 |

---

## M4: HTTP API

### M4.1 HTTP 基础设施

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M4.1.1C | 创建 api 模块 CMakeLists.txt | `src/api/CMakeLists.txt` | M1.1.2C | [S] |
| M4.1.2I | 定义 HTTP 请求/响应结构 | `src/api/dto/HttpTypes.h` | M4.1.1C | [S] |
| M4.1.3I | 定义 API 响应 DTO | `src/api/dto/Response.h` | M4.1.2I | [S] |
| M4.1.4I | 定义 API 请求 DTO | `src/api/dto/Request.h` | M4.1.3I | [S] |

**M4.1.2I 详细说明:**
```
文件: src/api/dto/HttpTypes.h
内容:
- struct HttpRequest { method, path, headers, queryParams, body }
- struct HttpResponse { statusCode, statusText, headers, body }
- HttpResponse::setJson(QJsonObject)
- HttpResponse::setError(int code, QString message)
验收: 结构体可实例化
```

---

### M4.2 HTTP 服务器

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M4.2.1T | 编写 HttpServer 单元测试 | `tests/unit/test_httpserver.cpp` | M4.1.4I | [S] |
| M4.2.2I | 实现 HttpServer.h | `src/api/HttpServer.h` | M4.1.4I | [S] |
| M4.2.3I | 实现 HttpServer.cpp | `src/api/HttpServer.cpp` | M4.2.2I | [S] |

**M4.2.1T 详细说明:**
```
文件: tests/unit/test_httpserver.cpp
测试用例:
- testStartStop: 启动和停止服务器
- testPortInUse: 端口被占用返回错误
- testIsRunning: isRunning() 状态正确
- testPort: port() 返回正确端口
- testStartedSignal: 启动时发出信号
- testStoppedSignal: 停止时发出信号
验收: 测试编译通过，运行失败（Red）
```

---

### M4.3 路由器

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M4.3.1T | 编写 ApiRouter 单元测试 | `tests/unit/test_apirouter.cpp` | M4.2.3I | [S] |
| M4.3.2I | 实现 ApiRouter.h | `src/api/ApiRouter.h` | M4.2.3I | [S] |
| M4.3.3I | 实现 ApiRouter.cpp 框架 | `src/api/ApiRouter.cpp` | M4.3.2I | [S] |

**M4.3.1T 详细说明:**
```
文件: tests/unit/test_apirouter.cpp
测试用例:
- testAddRoute: 添加路由成功
- testHandleRequest: 路由匹配正确处理器
- testHandleRequestNotFound: 未匹配路由返回 404
- testHandleRequestMethodNotAllowed: 方法不匹配返回 405
验收: 测试编译通过，运行失败（Red）
```

---

### M4.4 公共接口处理器

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M4.4.1C | 创建 handlers 子目录 CMakeLists.txt | `src/api/handlers/CMakeLists.txt` | M4.1.1C | [S] |
| M4.4.2T | 编写公共接口处理器测试 | `tests/unit/test_publichandlers.cpp` | M4.3.3I | [S] |
| M4.4.3I | 实现 PublicHandlers.h | `src/api/handlers/PublicHandlers.h` | M4.4.1C | [S] |
| M4.4.4I | 实现 PublicHandlers.cpp | `src/api/handlers/PublicHandlers.cpp` | M4.4.3I | [S] |

**M4.4.2T 详细说明:**
```
文件: tests/unit/test_publichandlers.cpp
测试用例:
- testHealth: GET /health 返回 {"status":"ok","version":"1.0.0"}
- testExit: GET /exit 触发退出信号
验收: 测试编译通过，运行失败（Red）
```

---

### M4.5 业务接口处理器

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M4.5.1T | 编写业务接口测试 - 设备 | `tests/unit/test_businesshandlers_device.cpp` | M4.4.4I | [S] |
| M4.5.2I | 实现 BusinessHandlers.h | `src/api/handlers/BusinessHandlers.h` | M4.4.1C | [S] |
| M4.5.3I | 实现 handleEnumDev | `src/api/handlers/BusinessHandlers.cpp` | M4.5.2I | [S] |
| M4.5.4T | 编写业务接口测试 - 登录 | `tests/unit/test_businesshandlers_login.cpp` | M4.5.3I | [S] |
| M4.5.5I | 实现 handleLogin | `src/api/handlers/BusinessHandlers.cpp` | M4.5.3I | [S] |
| M4.5.6T | 编写业务接口测试 - CSR | `tests/unit/test_businesshandlers_csr.cpp` | M4.5.5I | [S] |
| M4.5.7I | 实现 handleGenCsr | `src/api/handlers/BusinessHandlers.cpp` | M4.5.5I | [S] |
| M4.5.8T | 编写业务接口测试 - 证书 | `tests/unit/test_businesshandlers_cert.cpp` | M4.5.7I | [S] |
| M4.5.9I | 实现 handleImportCert | `src/api/handlers/BusinessHandlers.cpp` | M4.5.7I | [S] |
| M4.5.10I | 实现 handleExportCert | `src/api/handlers/BusinessHandlers.cpp` | M4.5.9I | [S] |
| M4.5.11T | 编写业务接口测试 - 签名 | `tests/unit/test_businesshandlers_sign.cpp` | M4.5.10I | [S] |
| M4.5.12I | 实现 handleSign | `src/api/handlers/BusinessHandlers.cpp` | M4.5.10I | [S] |
| M4.5.13I | 实现 handleVerify | `src/api/handlers/BusinessHandlers.cpp` | M4.5.12I | [S] |
| M4.5.14I | 实现 handleRandom | `src/api/handlers/BusinessHandlers.cpp` | M4.5.13I | [S] |

**M4.5.1T 详细说明:**
```
文件: tests/unit/test_businesshandlers_device.cpp
测试用例:
- testEnumDevSuccess: GET /api/v1/enum-dev 返回设备列表
- testEnumDevNoModule: 无激活模块返回错误
验收: 测试编译通过，运行失败（Red）
```

---

### M4.6 管理接口处理器

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M4.6.1T | 编写管理接口测试 - 模块 | `tests/unit/test_adminhandlers_mod.cpp` | M4.5.14I | [S] |
| M4.6.2I | 实现 AdminHandlers.h | `src/api/handlers/AdminHandlers.h` | M4.4.1C | [S] |
| M4.6.3I | 实现模块管理接口 | `src/api/handlers/AdminHandlers.cpp` | M4.6.2I | [S] |
| M4.6.4T | 编写管理接口测试 - 设备 | `tests/unit/test_adminhandlers_dev.cpp` | M4.6.3I | [S] |
| M4.6.5I | 实现设备管理接口 | `src/api/handlers/AdminHandlers.cpp` | M4.6.3I | [S] |
| M4.6.6T | 编写管理接口测试 - 应用 | `tests/unit/test_adminhandlers_app.cpp` | M4.6.5I | [S] |
| M4.6.7I | 实现应用管理接口 | `src/api/handlers/AdminHandlers.cpp` | M4.6.5I | [S] |
| M4.6.8T | 编写管理接口测试 - 容器 | `tests/unit/test_adminhandlers_container.cpp` | M4.6.7I | [S] |
| M4.6.9I | 实现容器管理接口 | `src/api/handlers/AdminHandlers.cpp` | M4.6.7I | [S] |
| M4.6.10T | 编写管理接口测试 - 文件 | `tests/unit/test_adminhandlers_file.cpp` | M4.6.9I | [S] |
| M4.6.11I | 实现文件管理接口 | `src/api/handlers/AdminHandlers.cpp` | M4.6.9I | [S] |
| M4.6.12T | 编写管理接口测试 - 设置 | `tests/unit/test_adminhandlers_settings.cpp` | M4.6.11I | [S] |
| M4.6.13I | 实现设置管理接口 | `src/api/handlers/AdminHandlers.cpp` | M4.6.11I | [S] |

**M4.6.1T 详细说明:**
```
文件: tests/unit/test_adminhandlers_mod.cpp
测试用例:
- testModList: GET /admin/mod/list 返回模块列表
- testModCreate: POST /admin/mod/create 创建模块
- testModActive: POST /admin/mod/active 激活模块
- testModDelete: DELETE /admin/mod/delete 删除模块
验收: 测试编译通过，运行失败（Red）
```

---

### M4.7 路由注册

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M4.7.1I | 注册所有路由到 ApiRouter | `src/api/ApiRouter.cpp` | M4.6.13I | [S] |

**M4.7.1I 详细说明:**
```
文件: src/api/ApiRouter.cpp
修改内容:
- setupRoutes() 中注册所有路由:
  - GET /health -> PublicHandlers::handleHealth
  - GET /exit -> PublicHandlers::handleExit
  - GET /api/v1/enum-dev -> BusinessHandlers::handleEnumDev
  - POST /api/v1/login -> BusinessHandlers::handleLogin
  - ... (所有 /api/v1/* 和 /admin/* 路由)
验收: 所有 API 端点可访问
```

---

### M4.8 验收检查点

| ID | 任务 | 依赖 | 验收标准 |
|----|------|------|----------|
| M4.8.1 | M4 集成验证 | M4.7.1I | HTTP 服务器可启动 |
| M4.8.2 | M4 测试验证 | M4.8.1 | `make test` API 测试通过 |
| M4.8.3 | API 兼容性测试 | M4.8.2 | 运行兼容性测试脚本通过 |

---

## M5: GUI 实现

### M5.1 GUI 基础设施

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M5.1.1C | 创建 gui 模块 CMakeLists.txt | `src/gui/CMakeLists.txt` | M1.1.2C | [S] |
| M5.1.2C | 创建 pages 子目录 CMakeLists.txt | `src/gui/pages/CMakeLists.txt` | M5.1.1C | [P] |
| M5.1.3C | 创建 widgets 子目录 CMakeLists.txt | `src/gui/widgets/CMakeLists.txt` | M5.1.1C | [P] |
| M5.1.4C | 创建 dialogs 子目录 CMakeLists.txt | `src/gui/dialogs/CMakeLists.txt` | M5.1.1C | [P] |
| M5.1.5I | 创建 Qt 资源文件 | `resources/wekey-skf.qrc` | M5.1.1C | [P] |
| M5.1.6I | 添加应用图标 | `resources/icons/app.png` | M5.1.5I | [P] |

---

### M5.2 主窗口

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M5.2.1T | 编写 MainWindow 单元测试 | `tests/unit/test_mainwindow.cpp` | M3.7.2 | [S] |
| M5.2.2I | 实现 MainWindow.h | `src/gui/MainWindow.h` | M5.1.1C | [S] |
| M5.2.3I | 实现 MainWindow.cpp - 框架 | `src/gui/MainWindow.cpp` | M5.2.2I | [S] |
| M5.2.4I | 实现 MainWindow::setupUi | `src/gui/MainWindow.cpp` | M5.2.3I | [S] |
| M5.2.5I | 实现 MainWindow::setupNavigation | `src/gui/MainWindow.cpp` | M5.2.4I | [S] |
| M5.2.6I | 实现 MainWindow::setupStatusBar | `src/gui/MainWindow.cpp` | M5.2.5I | [S] |
| M5.2.7I | 实现 MainWindow::closeEvent | `src/gui/MainWindow.cpp` | M5.2.6I | [S] |

**M5.2.1T 详细说明:**
```
文件: tests/unit/test_mainwindow.cpp
测试用例:
- testConstructor: 窗口可构造
- testWindowTitle: 标题为 "wekey-skf"
- testMinimumSize: 最小尺寸 900x600
- testNavigationCount: 导航项数量为 6
- testPageStackCount: 页面栈数量为 6
验收: 测试编译通过，运行失败（Red）
```

---

### M5.3 系统托盘

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M5.3.1T | 编写 SystemTray 单元测试 | `tests/unit/test_systemtray.cpp` | M5.2.7I | [S] |
| M5.3.2I | 实现 SystemTray.h | `src/gui/SystemTray.h` | M5.1.1C | [S] |
| M5.3.3I | 实现 SystemTray.cpp | `src/gui/SystemTray.cpp` | M5.3.2I | [S] |
| M5.3.4I | 集成 SystemTray 到 MainWindow | `src/gui/MainWindow.cpp` | M5.3.3I | [S] |

**M5.3.1T 详细说明:**
```
文件: tests/unit/test_systemtray.cpp
测试用例:
- testConstructor: 托盘可构造
- testIcon: 图标非空
- testMenu: 菜单包含"显示主窗口"和"退出"
- testShowWindowRequestedSignal: 双击发出信号
- testExitRequestedSignal: 点击退出发出信号
验收: 测试编译通过，运行失败（Red）
```

---

### M5.4 模块管理页

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M5.4.1T | 编写 ModulePage 单元测试 | `tests/unit/test_modulepage.cpp` | M5.2.7I | [S] |
| M5.4.2I | 实现 ModulePage.h | `src/gui/pages/ModulePage.h` | M5.1.2C | [S] |
| M5.4.3I | 实现 ModulePage.cpp | `src/gui/pages/ModulePage.cpp` | M5.4.2I | [S] |

**M5.4.1T 详细说明:**
```
文件: tests/unit/test_modulepage.cpp
测试用例:
- testConstructor: 页面可构造
- testAddModuleButton: 存在"添加模块"按钮
- testModuleTable: 存在模块表格
- testRefresh: refresh() 更新表格
验收: 测试编译通过，运行失败（Red）
```

---

### M5.5 设备管理页

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M5.5.1T | 编写 DevicePage 单元测试 | `tests/unit/test_devicepage.cpp` | M5.4.3I | [P] |
| M5.5.2I | 实现 DevicePage.h | `src/gui/pages/DevicePage.h` | M5.1.2C | [S] |
| M5.5.3I | 实现 DevicePage.cpp | `src/gui/pages/DevicePage.cpp` | M5.5.2I | [S] |

**M5.5.1T 详细说明:**
```
文件: tests/unit/test_devicepage.cpp
测试用例:
- testConstructor: 页面可构造
- testRefreshButton: 存在"刷新"按钮
- testDeviceTable: 存在设备表格
- testDeviceDetails: 存在设备详情区
- testRefresh: refresh() 更新设备列表
验收: 测试编译通过，运行失败（Red）
```

---

### M5.6 应用管理页

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M5.6.1T | 编写 AppPage 单元测试 | `tests/unit/test_apppage.cpp` | M5.5.3I | [P] |
| M5.6.2I | 实现 AppPage.h | `src/gui/pages/AppPage.h` | M5.1.2C | [S] |
| M5.6.3I | 实现 AppPage.cpp | `src/gui/pages/AppPage.cpp` | M5.6.2I | [S] |

**M5.6.1T 详细说明:**
```
文件: tests/unit/test_apppage.cpp
测试用例:
- testConstructor: 页面可构造
- testDeviceComboBox: 存在设备下拉框
- testCreateAppButton: 存在"创建应用"按钮
- testAppTable: 存在应用表格
验收: 测试编译通过，运行失败（Red）
```

---

### M5.7 容器管理页

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M5.7.1T | 编写 ContainerPage 单元测试 | `tests/unit/test_containerpage.cpp` | M5.6.3I | [P] |
| M5.7.2I | 实现 ContainerPage.h | `src/gui/pages/ContainerPage.h` | M5.1.2C | [S] |
| M5.7.3I | 实现 ContainerPage.cpp | `src/gui/pages/ContainerPage.cpp` | M5.7.2I | [S] |

**M5.7.1T 详细说明:**
```
文件: tests/unit/test_containerpage.cpp
测试用例:
- testConstructor: 页面可构造
- testDeviceComboBox: 存在设备下拉框
- testAppComboBox: 存在应用下拉框
- testCreateContainerButton: 存在"创建容器"按钮
- testContainerTable: 存在容器表格
验收: 测试编译通过，运行失败（Red）
```

---

### M5.8 配置管理页

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M5.8.1T | 编写 ConfigPage 单元测试 | `tests/unit/test_configpage.cpp` | M5.7.3I | [P] |
| M5.8.2I | 实现 ConfigPage.h | `src/gui/pages/ConfigPage.h` | M5.1.2C | [S] |
| M5.8.3I | 实现 ConfigPage.cpp | `src/gui/pages/ConfigPage.cpp` | M5.8.2I | [S] |

**M5.8.1T 详细说明:**
```
文件: tests/unit/test_configpage.cpp
测试用例:
- testConstructor: 页面可构造
- testDefaultAppNameEdit: 存在默认应用名输入框
- testDefaultContainerNameEdit: 存在默认容器名输入框
- testPortEdit: 存在端口输入框
- testErrorModeRadios: 存在错误模式单选按钮
- testSaveButton: 存在保存按钮
- testResetButton: 存在恢复默认按钮
验收: 测试编译通过，运行失败（Red）
```

---

### M5.9 日志查看页

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M5.9.1T | 编写 LogPage 单元测试 | `tests/unit/test_logpage.cpp` | M5.8.3I | [P] |
| M5.9.2I | 实现 LogPage.h | `src/gui/pages/LogPage.h` | M5.1.2C | [S] |
| M5.9.3I | 实现 LogPage.cpp | `src/gui/pages/LogPage.cpp` | M5.9.2I | [S] |

**M5.9.1T 详细说明:**
```
文件: tests/unit/test_logpage.cpp
测试用例:
- testConstructor: 页面可构造
- testSearchEdit: 存在搜索输入框
- testLevelComboBox: 存在级别下拉框
- testClearButton: 存在清空按钮
- testLogTable: 存在日志表格
验收: 测试编译通过，运行失败（Red）
```

---

### M5.10 对话框

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M5.10.1T | 编写 LoginDialog 单元测试 | `tests/unit/test_logindialog.cpp` | M5.1.4C | [P] |
| M5.10.2I | 实现 LoginDialog.h | `src/gui/dialogs/LoginDialog.h` | M5.1.4C | [S] |
| M5.10.3I | 实现 LoginDialog.cpp | `src/gui/dialogs/LoginDialog.cpp` | M5.10.2I | [S] |
| M5.10.4T | 编写 CsrDialog 单元测试 | `tests/unit/test_csrdialog.cpp` | M5.10.3I | [P] |
| M5.10.5I | 实现 CsrDialog.h | `src/gui/dialogs/CsrDialog.h` | M5.1.4C | [S] |
| M5.10.6I | 实现 CsrDialog.cpp | `src/gui/dialogs/CsrDialog.cpp` | M5.10.5I | [S] |
| M5.10.7T | 编写 ImportCertDialog 单元测试 | `tests/unit/test_importcertdialog.cpp` | M5.10.6I | [P] |
| M5.10.8I | 实现 ImportCertDialog.h | `src/gui/dialogs/ImportCertDialog.h` | M5.1.4C | [S] |
| M5.10.9I | 实现 ImportCertDialog.cpp | `src/gui/dialogs/ImportCertDialog.cpp` | M5.10.8I | [S] |
| M5.10.10T | 编写 MessageBox 工具测试 | `tests/unit/test_messagebox.cpp` | M5.10.9I | [P] |
| M5.10.11I | 实现 MessageBox.h 工具类 | `src/gui/dialogs/MessageBox.h` | M5.1.4C | [S] |
| M5.10.12I | 实现 MessageBox.cpp | `src/gui/dialogs/MessageBox.cpp` | M5.10.11I | [S] |

**M5.10.1T 详细说明:**
```
文件: tests/unit/test_logindialog.cpp
测试用例:
- testConstructor: 对话框可构造
- testPinEdit: 存在 PIN 输入框（密码模式）
- testRoleRadios: 存在用户/管理员单选按钮
- testRetryLabel: 存在剩余次数标签
- testPin: pin() 返回输入的 PIN
- testRole: role() 返回选中的角色
- testSetRetryCount: setRetryCount() 更新标签
验收: 测试编译通过，运行失败（Red）
```

---

### M5.11 文件管理页

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M5.11.1T | 编写 FilePage 单元测试 | `tests/unit/test_filepage.cpp` | M5.10.12I | [P] |
| M5.11.2I | 实现 FilePage.h | `src/gui/pages/FilePage.h` | M5.1.2C | [S] |
| M5.11.3I | 实现 FilePage.cpp | `src/gui/pages/FilePage.cpp` | M5.11.2I | [S] |
| M5.11.4I | 实现文件创建对话框 | `src/gui/dialogs/CreateFileDialog.h/cpp` | M5.11.3I | [S] |

**M5.11.1T 详细说明:**
```
文件: tests/unit/test_filepage.cpp
测试用例:
- testConstructor: 页面可构造
- testDeviceComboBox: 存在设备下拉框
- testAppComboBox: 存在应用下拉框
- testCreateFileButton: 存在"创建文件"按钮
- testFileTable: 存在文件列表表格
- testRefresh: refresh() 更新文件列表
验收: 测试编译通过，运行失败（Red）
```

**M5.11.2I 详细说明:**
```
文件: src/gui/pages/FilePage.h
内容:
- class FilePage : public QWidget
- 设备选择器 QComboBox
- 应用选择器 QComboBox
- 文件列表 QTableWidget
- 创建/读取/删除按钮
- slots: onCreateFile(), onReadFile(), onDeleteFile()
验收: M5.11.1T 测试通过（Green）
```

**M5.11.3I 详细说明:**
```
文件: src/gui/pages/FilePage.cpp
功能:
- 枚举文件列表（通过 FileService）
- 显示文件名和大小
- 创建文件（打开对话框）
- 读取文件（保存到本地）
- 删除文件（二次确认）
验收: 界面正常显示，交互流畅
```

**M5.11.4I 详细说明:**
```
文件: src/gui/dialogs/CreateFileDialog.h/cpp
内容:
- 文件名输入框
- 文件选择按钮（QFileDialog）
- 显示选中文件大小
- 读权限下拉框（Everyone/UserOnly/AdminOnly）
- 写权限下拉框（Everyone/UserOnly/AdminOnly）
- 文件大小限制提示（8KB）
- 验证逻辑：大小 > 8KB 时禁用确定按钮
验收: 对话框功能完整，数据验证正确
```

---

### M5.12 页面集成

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M5.12.1I | 集成所有页面到 MainWindow | `src/gui/MainWindow.cpp` | M5.9.3I, M5.10.12I, M5.11.4I | [S] |

**M5.12.1I 详细说明:**
```
文件: src/gui/MainWindow.cpp
修改内容:
- setupPages() 中创建并添加所有页面:
  - ModulePage
  - DevicePage
  - AppPage
  - ContainerPage
  - ConfigPage
  - LogPage
  - FilePage （新增）
- 连接页面信号到对应的对话框
验收: 所有页面可切换，功能正常
```

---

### M5.13 验收检查点

| ID | 任务 | 依赖 | 验收标准 |
|----|------|------|----------|
| M5.13.1 | M5 集成验证 | M5.12.1I | GUI 可启动并显示 |
| M5.13.2 | M5 测试验证 | M5.13.1 | `make test` GUI 测试通过 |

---

## M6: 测试与打包

### M6.1 集成测试

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M6.1.1C | 创建集成测试 CMakeLists.txt | `tests/integration/CMakeLists.txt` | M1.1.3C | [S] |
| M6.1.2T | 编写端到端工作流测试 | `tests/integration/test_e2e.cpp` | M5.13.2 | [S] |
| M6.1.3T | 编写设备热插拔测试 | `tests/integration/test_hotplug.cpp` | M6.1.2T | [S] |
| M6.1.4T | 编写 PIN 锁定解锁测试 | `tests/integration/test_pin.cpp` | M6.1.3T | [S] |
| M6.1.5T | 编写 API 兼容性测试脚本 | `tests/integration/test_api_compat.sh` | M6.1.4T | [S] |

**M6.1.2T 详细说明:**
```
文件: tests/integration/test_e2e.cpp
测试用例:
- testFullWorkflow:
  1. 启动应用
  2. 加载 SKF 插件
  3. 枚举设备
  4. 登录应用
  5. 创建容器
  6. 生成 CSR
  7. 签名数据
  8. 验证签名
  9. 登出应用
  10. 关闭应用
验收: 端到端流程无错误
```

---

### M6.2 内存检测

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M6.2.1C | 添加 AddressSanitizer CMake 选项 | `CMakeLists.txt` | M6.1.5T | [S] |
| M6.2.2T | 运行 ASAN 内存检测 | - | M6.2.1C | [S] |

**M6.2.1C 详细说明:**
```
文件: CMakeLists.txt
修改内容:
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()
验收: cmake -DENABLE_ASAN=ON 可配置
```

---

### M6.3 Windows 打包

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M6.3.1C | 创建 Windows 打包脚本 | `scripts/package_win.ps1` | M6.2.2T | [S] |
| M6.3.2I | 执行 Windows 打包 | - | M6.3.1C | [S] |

**M6.3.1C 详细说明:**
```
文件: scripts/package_win.ps1
内容:
- 编译 Release 版本
- 复制 wekey-skf.exe
- 运行 windeployqt
- 复制插件到 plugins/
- 复制厂商库到 lib/
- 创建 ZIP 压缩包
验收: 生成 wekey-skf-win64.zip
```

---

### M6.4 macOS 打包

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M6.4.1C | 创建 macOS 打包脚本 | `scripts/package_mac.sh` | M6.2.2T | [P] |
| M6.4.2I | 执行 macOS 打包 | - | M6.4.1C | [S] |

**M6.4.1C 详细说明:**
```
文件: scripts/package_mac.sh
内容:
- 编译 Release 版本
- 复制 app bundle
- 运行 macdeployqt
- 复制厂商库到 Resources/lib/
- 创建 DMG 安装包
验收: 生成 wekey-skf.dmg
```

---

### M6.5 CI/CD 配置

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| M6.5.1C | 创建 GitHub Actions 配置 | `.github/workflows/build.yml` | M6.4.2I | [S] |

**M6.5.1C 详细说明:**
```
文件: .github/workflows/build.yml
内容:
- Windows 构建任务
- macOS 构建任务
- 运行测试
- 上传构建产物
验收: CI 构建成功
```

---

### M6.6 最终验收

| ID | 任务 | 依赖 | 验收标准 |
|----|------|------|----------|
| M6.6.1 | 全量测试验证 | M6.5.1C | 所有测试通过 |
| M6.6.2 | Windows 安装验证 | M6.3.2I | 绿色版可运行 |
| M6.6.3 | macOS 安装验证 | M6.4.2I | DMG 安装后可运行 |
| M6.6.4 | API 兼容性最终验证 | M6.6.3 | 与原 Go 版本 API 100% 兼容 |
| M6.6.5 | 内存泄漏验证 | M6.6.4 | ASAN 无错误 |

---

## 任务统计

| 里程碑 | 配置任务 | 测试任务 | 实现任务 | 总计 |
|--------|----------|----------|----------|------|
| M1 | 12 | 9 | 16 | 37 |
| M2 | 2 | 8 | 25 | 35 |
| M3 | 6 | 6 | 9 | 21 |
| M4 | 2 | 14 | 21 | 37 |
| M5 | 5 | 14 | 25 | 44 |
| M6 | 5 | 6 | 2 | 13 |
| **总计** | **32** | **57** | **98** | **187** |

---

## 执行顺序建议

```
阶段 1 (可完全并行):
  M1.1.1C, M1.1.4C, M1.1.5C, M1.1.6C
  M1.2.2T, M1.2.5T, M1.2.7T

阶段 2 (M1 主体):
  M1.1.2C -> M1.1.3C
  M1.2.1C -> M1.2.3I -> M1.2.4I -> M1.2.6I
  M1.3.1C -> M1.3.2I -> M1.3.3T -> M1.3.4I -> M1.3.5I
  ...

阶段 3 (M2):
  M2.1.* -> M2.2.* -> M2.3.* -> ... -> M2.8.*

阶段 4 (M3 + M4 可并行):
  M3.* (核心服务)
  M4.* (HTTP API)

阶段 5 (M5):
  M5.* (GUI)

阶段 6 (M6):
  M6.* (测试与打包)
```

---

## 文档结束
