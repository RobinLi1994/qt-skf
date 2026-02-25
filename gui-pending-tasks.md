# GUI 页面功能实现 — 原子化任务列表

> **版本**: 1.0
> **日期**: 2026-02-08
> **原则**: 严格遵循 TDD (Red → Green → Refactor) + constitution.md

---

## 任务编号规则

```
GP{阶段}.{序号}{类型}

类型:
  T = 测试任务 (Test)    — 先写失败测试 (Red)
  I = 实现任务 (Impl)    — 让测试通过 (Green)
```

---

## 总览

| 阶段 | 页面 | 任务数 | 复杂度 |
|------|------|--------|--------|
| GP1 | DevicePage | 6 | 低 |
| GP2 | AppPage | 8 | 中 |
| GP3 | ContainerPage | 8 | 高 |

**依赖链**: `GP1 → GP2 → GP3`（AppPage 复用 DevicePage 的设备刷新模式，ContainerPage 复用两者的联动模式）

---

## GP1: DevicePage — 设备管理页

### 依赖

- `DeviceService::instance()` — `enumDevices()`, `setDeviceLabel()`, `changeDeviceAuth()`
- `DeviceService` 信号 — `deviceListChanged`, `deviceInserted`, `deviceRemoved`
- `MessageBox::error()` — 错误提示

### 任务列表

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| GP1.1T | 编写 DevicePage 功能测试 | `tests/unit/test_devicepage.cpp` | - | [P] |
| GP1.2I | 更新 DevicePage.h 声明 | `src/gui/pages/DevicePage.h` | GP1.1T | [S] |
| GP1.3I | 实现 DevicePage 信号连接与刷新 | `src/gui/pages/DevicePage.cpp` | GP1.2I | [S] |
| GP1.4I | 实现 DevicePage 设备详情显示 | `src/gui/pages/DevicePage.cpp` | GP1.3I | [S] |
| GP1.5I | 实现 DevicePage 设置标签 | `src/gui/pages/DevicePage.cpp` | GP1.4I | [S] |
| GP1.6I | 实现 DevicePage 修改认证密钥 | `src/gui/pages/DevicePage.cpp` | GP1.5I | [S] |

---

#### GP1.1T — 编写 DevicePage 功能测试

**文件**: `tests/unit/test_devicepage.cpp`

**前置**: 无（可立即开始）

**说明**: 保留已有的 7 个 UI 骨架测试，新增功能测试。需要 `StubPlugin` + `PluginManager::registerPluginInstance()` 注入 mock（参考 `test_modulepage.cpp` 模式）。

**新增测试用例**:

```
testRefreshTable:
  - 注册 StubPlugin（预置 2 个 DeviceInfo）
  - 设置为激活插件
  - 创建 DevicePage，调用 refreshTable()
  - 验证 table()->rowCount() == 2
  - 验证第 0 列 (序列号)、第 1 列 (标签)、第 2 列 (制造商) 内容正确

testRefreshTableShowsOperationButtons:
  - 注册 StubPlugin（预置 1 个设备）
  - refreshTable() 后验证第 3 列 (操作) 有 cellWidget
  - cellWidget 中包含 objectName="setLabelButton" 的 QPushButton
  - cellWidget 中包含 objectName="changeAuthButton" 的 QPushButton

testDeviceDetailsUpdate:
  - 注册 StubPlugin（预置 1 个设备，带 manufacturer/hwVersion/fwVersion）
  - refreshTable() → 选中第 0 行
  - 验证 detailsGroup_ 中的 QLabel 显示正确的制造商/硬件版本/固件版本

testSignalTriggersRefresh:
  - 创建 DevicePage（构造时连接信号）
  - 之后注册 StubPlugin 并设为激活
  - 手动 emit DeviceService::deviceListChanged()
  - 验证表格已自动刷新

testEmptyWhenNoActivePlugin:
  - 不注册任何插件
  - 创建 DevicePage，调用 refreshTable()
  - 验证 table()->rowCount() == 0（不崩溃）
```

**验收**: 测试编译通过，新增测试全部失败 (Red)

---

#### GP1.2I — 更新 DevicePage.h 声明

**文件**: `src/gui/pages/DevicePage.h`

**前置**: GP1.1T

**新增内容**:

```cpp
public:
    void refreshTable();

private:
    void connectSignals();
    void onSetLabel(const QString& devName);
    void onChangeAuth(const QString& devName);
    void updateDetails(int row);

    // 详情区的 QLabel 指针（需从 setupUi 中提取保存）
    QLabel* manufacturerValue_ = nullptr;
    QLabel* hwVersionValue_ = nullptr;
    QLabel* fwVersionValue_ = nullptr;
```

**新增 include**: `<QLabel>`

**验收**: 头文件编译通过

---

#### GP1.3I — 实现 DevicePage 信号连接与刷新

**文件**: `src/gui/pages/DevicePage.cpp`

**前置**: GP1.2I

**修改内容**:

1. 构造函数中调用 `connectSignals()` 和 `refreshTable()`
2. `setupUi()` 中将 detailsGroup 内的 3 个 QLabel 保存到成员指针
3. `connectSignals()`:
   - `refreshButton_ clicked → refreshTable()`
   - `DeviceService::deviceListChanged → refreshTable()`
   - `DeviceService::deviceInserted → refreshTable()`
   - `DeviceService::deviceRemoved → refreshTable()`
   - `table_ currentRowChanged → updateDetails(row)`
4. `refreshTable()`:
   - `DeviceService::instance().enumDevices(false)`
   - 失败时：清空表格，return
   - 成功时：遍历 `QList<DeviceInfo>` 填充表格
   - 每行第 0 列=serialNumber，第 1 列=label，第 2 列=manufacturer
   - 第 3 列=操作 widget（含"设置标签"+"修改认证"按钮）
   - 按钮通过 lambda 捕获 `devName` 调用 `onSetLabel`/`onChangeAuth`

**新增 include**: `"core/device/DeviceService.h"`, `"gui/dialogs/MessageBox.h"`, `<QHBoxLayout>`, `<QPushButton>`

**验收**: `testRefreshTable`, `testRefreshTableShowsOperationButtons`, `testSignalTriggersRefresh`, `testEmptyWhenNoActivePlugin` 通过

---

#### GP1.4I — 实现 DevicePage 设备详情显示

**文件**: `src/gui/pages/DevicePage.cpp`

**前置**: GP1.3I

**实现 `updateDetails(int row)`**:

1. 如果 row < 0 或 row >= rowCount，详情标签显示 "-"
2. 否则，从 `refreshTable()` 中缓存的设备列表取出 `DeviceInfo`
3. 更新: `manufacturerValue_->setText(info.manufacturer)` 等

**注意**: 需要增加 `QList<DeviceInfo> devices_` 成员变量缓存刷新结果，或从 table item 的 UserRole data 中读取

**验收**: `testDeviceDetailsUpdate` 通过

---

#### GP1.5I — 实现 DevicePage 设置标签

**文件**: `src/gui/pages/DevicePage.cpp`

**前置**: GP1.4I

**实现 `onSetLabel(const QString& devName)`**:

1. `QInputDialog::getText()` 弹出输入新标签
2. 用户取消则 return
3. `DeviceService::instance().setDeviceLabel(devName, newLabel)`
4. 失败 → `MessageBox::error()`
5. 成功 → 自动刷新（`deviceListChanged` 信号触发，或手动 `refreshTable()`）

**新增 include**: `<QInputDialog>`

**验收**: 手动测试可弹出输入框（单元测试中 QInputDialog 为 modal，不便自动测试）

---

#### GP1.6I — 实现 DevicePage 修改认证密钥

**文件**: `src/gui/pages/DevicePage.cpp`

**前置**: GP1.5I

**实现 `onChangeAuth(const QString& devName)`**:

1. 弹出一个简单对话框（QDialog），包含两个 QLineEdit（旧密钥、新密钥），密码模式
2. 用户确认后调用 `DeviceService::instance().changeDeviceAuth(devName, oldPin, newPin)`
3. 失败 → `MessageBox::error()`
4. 成功 → 提示成功

**验收**: `make test` 全部通过，DevicePage 新增测试全部 Green

---

## GP2: AppPage — 应用管理页

### 依赖

- GP1 完成（复用设备刷新模式）
- `DeviceService::instance()` — `enumDevices()`（填充设备下拉框）
- `AppService::instance()` — `enumApps()`, `createApp()`, `deleteApp()`, `login()`, `logout()`, `changePin()`, `unlockPin()`, `getRetryCount()`
- `LoginDialog` — 登录对话框（已实现）
- `MessageBox::error()` — 错误提示

### 任务列表

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| GP2.1T | 编写 AppPage 功能测试 | `tests/unit/test_apppage.cpp` | GP1 完成 | [P] |
| GP2.2I | 更新 AppPage.h 声明 | `src/gui/pages/AppPage.h` | GP2.1T | [S] |
| GP2.3I | 实现 AppPage 信号连接与设备下拉刷新 | `src/gui/pages/AppPage.cpp` | GP2.2I | [S] |
| GP2.4I | 实现 AppPage 应用列表刷新 | `src/gui/pages/AppPage.cpp` | GP2.3I | [S] |
| GP2.5I | 实现 AppPage 创建/删除应用 | `src/gui/pages/AppPage.cpp` | GP2.4I | [S] |
| GP2.6I | 实现 AppPage 登录/登出 | `src/gui/pages/AppPage.cpp` | GP2.5I | [S] |
| GP2.7I | 实现 AppPage 修改 PIN | `src/gui/pages/AppPage.cpp` | GP2.6I | [S] |
| GP2.8I | 实现 AppPage 解锁 PIN | `src/gui/pages/AppPage.cpp` | GP2.7I | [S] |

---

#### GP2.1T — 编写 AppPage 功能测试

**文件**: `tests/unit/test_apppage.cpp`

**前置**: 无（可与 GP1 并行编写，但执行需 GP1 完成后的模式参考）

**说明**: 保留已有的 6 个 UI 骨架测试。新增功能测试需要 StubPlugin（预置设备+应用），注入 PluginManager。

**新增测试用例**:

```
testRefreshDevices:
  - 注册 StubPlugin（预置 "DEV001" 设备），设为激活
  - 创建 AppPage，调用 refreshDevices()
  - 验证 deviceCombo()->count() == 1
  - 验证 deviceCombo()->itemText(0) == "DEV001"

testRefreshApps:
  - 注册 StubPlugin（预置 "DEV001" + 2 个 App: "APP1", "APP2"），设为激活
  - 创建 AppPage，refreshDevices()，选中 "DEV001"
  - 调用 refreshApps()
  - 验证 table()->rowCount() == 2
  - 验证第 0 列包含 "APP1" 和 "APP2"

testLoginStatusDisplay:
  - 注册 StubPlugin（预置设备 + 已登录的 APP），设为激活
  - refreshApps() 后
  - 验证登录状态列显示 "已登录" / "未登录"

testOperationButtonsForLoggedOut:
  - 未登录的应用行，操作列包含 "登录" 按钮 (objectName="loginButton")
  - 包含 "删除" 按钮 (objectName="deleteButton")

testOperationButtonsForLoggedIn:
  - 已登录的应用行，操作列包含 "登出" 按钮 (objectName="logoutButton")
  - 包含 "修改PIN" 按钮 (objectName="changePinButton")

testDeviceComboTriggersRefresh:
  - 创建 AppPage，注册含 2 个设备的 StubPlugin
  - refreshDevices()，切换 deviceCombo 到第二个设备
  - 验证 table 内容对应第二个设备的应用列表

testEmptyWhenNoDevice:
  - 不注册插件
  - 创建 AppPage
  - 验证 deviceCombo()->count() == 0
  - 验证 table()->rowCount() == 0
```

**验收**: 测试编译通过，新增测试全部失败 (Red)

---

#### GP2.2I — 更新 AppPage.h 声明

**文件**: `src/gui/pages/AppPage.h`

**前置**: GP2.1T

**新增内容**:

```cpp
public:
    void refreshDevices();
    void refreshApps();

private:
    void connectSignals();
    void onCreateApp();
    void onDeleteApp(const QString& devName, const QString& appName);
    void onLogin(const QString& devName, const QString& appName);
    void onLogout(const QString& devName, const QString& appName);
    void onChangePin(const QString& devName, const QString& appName);
    void onUnlockPin(const QString& devName, const QString& appName);

    QString currentDevice() const;  // 返回 deviceCombo_ 当前选中的设备名
```

**验收**: 头文件编译通过

---

#### GP2.3I — 实现 AppPage 信号连接与设备下拉刷新

**文件**: `src/gui/pages/AppPage.cpp`

**前置**: GP2.2I

**修改内容**:

1. 构造函数调用 `connectSignals()`, `refreshDevices()`
2. `connectSignals()`:
   - `createButton_ clicked → onCreateApp()`
   - `deviceCombo_ currentTextChanged → refreshApps()`
   - `DeviceService::deviceListChanged → refreshDevices()`
3. `refreshDevices()`:
   - 保存当前选中项
   - `deviceCombo_->clear()`
   - `DeviceService::instance().enumDevices(false)`
   - 成功: 遍历设备添加 `deviceCombo_->addItem(info.deviceName)`
   - 恢复之前的选中项（如果还存在）

**新增 include**: `"core/device/DeviceService.h"`, `"core/application/AppService.h"`, `"gui/dialogs/MessageBox.h"`, `"gui/dialogs/LoginDialog.h"`, `<QHBoxLayout>`, `<QPushButton>`, `<QInputDialog>`

**验收**: `testRefreshDevices`, `testEmptyWhenNoDevice`, `testDeviceComboTriggersRefresh` 通过

---

#### GP2.4I — 实现 AppPage 应用列表刷新

**文件**: `src/gui/pages/AppPage.cpp`

**前置**: GP2.3I

**实现 `refreshApps()`**:

1. `table_->setRowCount(0)`
2. 获取 `currentDevice()`，为空则 return
3. `AppService::instance().enumApps(devName)`
4. 失败: return（不 MessageBox，只是清空）
5. 成功: 遍历 `QList<AppInfo>` 填充表格
   - 第 0 列=appName
   - 第 1 列=isLoggedIn ? "已登录" : "未登录"
   - 第 2 列=操作 widget:
     - 未登录: [登录][修改PIN][解锁PIN][删除]
     - 已登录: [登出][修改PIN][删除]
   - 各按钮通过 lambda 捕获 devName+appName 调用对应 slot

**验收**: `testRefreshApps`, `testLoginStatusDisplay`, `testOperationButtonsForLoggedOut`, `testOperationButtonsForLoggedIn` 通过

---

#### GP2.5I — 实现 AppPage 创建/删除应用

**文件**: `src/gui/pages/AppPage.cpp`

**前置**: GP2.4I

**实现 `onCreateApp()`**:

1. 获取 `currentDevice()`，为空则提示
2. `QInputDialog::getText()` 输入应用名（默认 Config::instance().defaultAppName()）
3. 构建 args QVariantMap（adminPin + userPin 也可用简单 QInputDialog 获取，或硬编码默认值）
4. `AppService::instance().createApp(devName, appName, args)`
5. 失败 → MessageBox
6. 成功 → refreshApps()

**实现 `onDeleteApp(devName, appName)`**:

1. `QMessageBox::question()` 确认
2. `AppService::instance().deleteApp(devName, appName)`
3. 失败 → MessageBox
4. 成功 → refreshApps()

**验收**: 编译通过，手动测试创建/删除流程

---

#### GP2.6I — 实现 AppPage 登录/登出

**文件**: `src/gui/pages/AppPage.cpp`

**前置**: GP2.5I

**实现 `onLogin(devName, appName)`**:

1. 创建 `LoginDialog`
2. 先获取 `AppService::getRetryCount(devName, appName, "user")` 设置剩余次数
3. `dialog.exec()` == Accepted 后获取 pin/role
4. `AppService::instance().login(devName, appName, role, pin)`
5. 失败 → MessageBox
6. 成功 → refreshApps()

**实现 `onLogout(devName, appName)`**:

1. `AppService::instance().logout(devName, appName)`
2. 失败 → MessageBox
3. 成功 → refreshApps()

**验收**: 编译通过

---

#### GP2.7I — 实现 AppPage 修改 PIN

**文件**: `src/gui/pages/AppPage.cpp`

**前置**: GP2.6I

**实现 `onChangePin(devName, appName)`**:

1. 弹出 QDialog，包含:
   - 角色选择（user/admin radio）
   - 旧 PIN（QLineEdit, password mode）
   - 新 PIN（QLineEdit, password mode）
2. 用户确认后: `AppService::instance().changePin(devName, appName, role, oldPin, newPin)`
3. 失败 → MessageBox
4. 成功 → 提示成功

**验收**: 编译通过

---

#### GP2.8I — 实现 AppPage 解锁 PIN

**文件**: `src/gui/pages/AppPage.cpp`

**前置**: GP2.7I

**实现 `onUnlockPin(devName, appName)`**:

1. 弹出 QDialog，包含:
   - 管理员 PIN（QLineEdit, password mode）
   - 新用户 PIN（QLineEdit, password mode）
2. 用户确认后: `AppService::instance().unlockPin(devName, appName, adminPin, newUserPin, {})`
3. 失败 → MessageBox
4. 成功 → 提示成功，refreshApps()

**验收**: `make test` 全部通过，AppPage 新增测试全部 Green

---

## GP3: ContainerPage — 容器管理页

### 依赖

- GP2 完成（复用设备+应用联动模式）
- `DeviceService::instance()` — `enumDevices()`
- `AppService::instance()` — `enumApps()`
- `ContainerService::instance()` — `enumContainers()`, `createContainer()`, `deleteContainer()`
- `CertService::instance()` — `generateKeyPair()`, `importCert()`, `exportCert()`
- `CsrDialog` — CSR 对话框（已实现）
- `ImportCertDialog` — 导入证书对话框（已实现）

### 任务列表

| ID | 任务 | 文件 | 依赖 | 并行 |
|----|------|------|------|------|
| GP3.1T | 编写 ContainerPage 功能测试 | `tests/unit/test_containerpage.cpp` | GP2 完成 | [P] |
| GP3.2I | 更新 ContainerPage.h 声明 | `src/gui/pages/ContainerPage.h` | GP3.1T | [S] |
| GP3.3I | 实现 ContainerPage 信号连接与设备/应用联动 | `src/gui/pages/ContainerPage.cpp` | GP3.2I | [S] |
| GP3.4I | 实现 ContainerPage 容器列表刷新 | `src/gui/pages/ContainerPage.cpp` | GP3.3I | [S] |
| GP3.5I | 实现 ContainerPage 创建/删除容器 | `src/gui/pages/ContainerPage.cpp` | GP3.4I | [S] |
| GP3.6I | 实现 ContainerPage 生成 CSR | `src/gui/pages/ContainerPage.cpp` | GP3.5I | [S] |
| GP3.7I | 实现 ContainerPage 导入证书 | `src/gui/pages/ContainerPage.cpp` | GP3.6I | [S] |
| GP3.8I | 实现 ContainerPage 导出证书 | `src/gui/pages/ContainerPage.cpp` | GP3.7I | [S] |

---

#### GP3.1T — 编写 ContainerPage 功能测试

**文件**: `tests/unit/test_containerpage.cpp`

**前置**: 无（可与 GP2 并行编写）

**说明**: 保留已有的 7 个 UI 骨架测试。新增功能测试需 StubPlugin（预置设备+应用+容器）。

**新增测试用例**:

```
testRefreshDevices:
  - 注册 StubPlugin（预置 "DEV001"），设为激活
  - 创建 ContainerPage，调用 refreshDevices()
  - 验证 deviceCombo()->count() == 1

testRefreshApps:
  - 注册 StubPlugin（预置设备 + "APP1"），设为激活
  - refreshDevices()，选中设备
  - refreshApps()
  - 验证 appCombo()->count() == 1

testRefreshContainers:
  - 注册 StubPlugin（预置设备 + 应用 + 2 个容器），设为激活
  - refreshDevices()，refreshApps()，refreshContainers()
  - 验证 table()->rowCount() == 2
  - 验证第 0 列包含容器名

testContainerTableColumns:
  - 验证容器行的列内容:
    - 第 1 列=密钥类型 ("SM2"/"RSA"/"未知")
    - 第 2 列=密钥状态 ("已生成"/"未生成")
    - 第 3 列=证书状态 ("已导入"/"未导入")

testOperationButtons:
  - 操作列 (第 4 列) 存在 cellWidget
  - 包含 "生成CSR" 按钮 (objectName="genCsrButton")
  - 包含 "导入证书" 按钮 (objectName="importCertButton")
  - 包含 "导出证书" 按钮 (objectName="exportCertButton")
  - 包含 "删除" 按钮 (objectName="deleteButton")

testDeviceComboTriggersAppRefresh:
  - 切换 deviceCombo → appCombo 内容更新

testAppComboTriggersContainerRefresh:
  - 切换 appCombo → table 内容更新
```

**验收**: 测试编译通过，新增测试全部失败 (Red)

---

#### GP3.2I — 更新 ContainerPage.h 声明

**文件**: `src/gui/pages/ContainerPage.h`

**前置**: GP3.1T

**新增内容**:

```cpp
public:
    void refreshDevices();
    void refreshApps();
    void refreshContainers();

private:
    void connectSignals();
    void onCreateContainer();
    void onDeleteContainer(const QString& devName, const QString& appName, const QString& containerName);
    void onGenCsr(const QString& devName, const QString& appName, const QString& containerName);
    void onImportCert(const QString& devName, const QString& appName, const QString& containerName);
    void onExportCert(const QString& devName, const QString& appName, const QString& containerName);

    QString currentDevice() const;
    QString currentApp() const;
```

**验收**: 头文件编译通过

---

#### GP3.3I — 实现 ContainerPage 信号连接与设备/应用联动

**文件**: `src/gui/pages/ContainerPage.cpp`

**前置**: GP3.2I

**修改内容**:

1. 构造函数调用 `connectSignals()`, `refreshDevices()`
2. `connectSignals()`:
   - `createButton_ clicked → onCreateContainer()`
   - `deviceCombo_ currentTextChanged → refreshApps()`
   - `appCombo_ currentTextChanged → refreshContainers()`
   - `DeviceService::deviceListChanged → refreshDevices()`
3. `refreshDevices()`: 同 AppPage 模式
4. `refreshApps()`:
   - 保存当前选中应用
   - `appCombo_->clear()`
   - `AppService::instance().enumApps(currentDevice())`
   - 填充 appCombo_
   - 恢复选中（触发 refreshContainers）

**新增 include**: `"core/device/DeviceService.h"`, `"core/application/AppService.h"`, `"core/container/ContainerService.h"`, `"core/crypto/CertService.h"`, `"gui/dialogs/MessageBox.h"`, `"gui/dialogs/CsrDialog.h"`, `"gui/dialogs/ImportCertDialog.h"`, `<QHBoxLayout>`, `<QPushButton>`, `<QInputDialog>`, `<QFileDialog>`

**验收**: `testRefreshDevices`, `testRefreshApps`, `testDeviceComboTriggersAppRefresh` 通过

---

#### GP3.4I — 实现 ContainerPage 容器列表刷新

**文件**: `src/gui/pages/ContainerPage.cpp`

**前置**: GP3.3I

**实现 `refreshContainers()`**:

1. `table_->setRowCount(0)`
2. 获取 `currentDevice()` + `currentApp()`，任一为空则 return
3. `ContainerService::instance().enumContainers(devName, appName)`
4. 失败: return
5. 成功: 遍历 `QList<ContainerInfo>` 填充表格
   - 第 0 列=containerName
   - 第 1 列=密钥类型: keyType==2 → "SM2", keyType==1 → "RSA", 其他 → "未知"
   - 第 2 列=keyGenerated ? "已生成" : "未生成"
   - 第 3 列=certImported ? "已导入" : "未导入"
   - 第 4 列=操作 widget: [生成CSR][导入证书][导出证书][删除]

**验收**: `testRefreshContainers`, `testContainerTableColumns`, `testOperationButtons`, `testAppComboTriggersContainerRefresh` 通过

---

#### GP3.5I — 实现 ContainerPage 创建/删除容器

**文件**: `src/gui/pages/ContainerPage.cpp`

**前置**: GP3.4I

**实现 `onCreateContainer()`**:

1. 获取 currentDevice + currentApp，为空则提示
2. `QInputDialog::getText()` 输入容器名（默认 Config::instance().defaultContainerName()）
3. `ContainerService::instance().createContainer(devName, appName, name)`
4. 失败 → MessageBox
5. 成功 → refreshContainers()

**实现 `onDeleteContainer(devName, appName, containerName)`**:

1. `QMessageBox::question()` 确认
2. `ContainerService::instance().deleteContainer(devName, appName, containerName)`
3. 失败 → MessageBox
4. 成功 → refreshContainers()

**验收**: 编译通过

---

#### GP3.6I — 实现 ContainerPage 生成 CSR

**文件**: `src/gui/pages/ContainerPage.cpp`

**前置**: GP3.5I

**实现 `onGenCsr(devName, appName, containerName)`**:

1. 创建 `CsrDialog dialog(this)`
2. 从 Config 设置默认值到 dialog 的各 edit 字段
3. `dialog.exec()` == Accepted 后:
   - 如果 `dialog.regenerateKey()` 或首次生成:
     - `CertService::instance().generateKeyPair(devName, appName, containerName, dialog.keyType())`
     - 失败 → MessageBox，return
4. CSR 生成逻辑（目前 CertService 没有 generateCsr，用 generateKeyPair 代替）
5. 成功 → 提示结果 / 显示公钥信息
6. refreshContainers()

**验收**: 编译通过

---

#### GP3.7I — 实现 ContainerPage 导入证书

**文件**: `src/gui/pages/ContainerPage.cpp`

**前置**: GP3.6I

**实现 `onImportCert(devName, appName, containerName)`**:

1. 创建 `ImportCertDialog dialog(this)`
2. `dialog.exec()` == Accepted 后:
   - 获取 `dialog.certPem()`
   - `CertService::instance().importCert(devName, appName, containerName, certData, true)`
   - 失败 → MessageBox
   - 成功 → refreshContainers()

**验收**: 编译通过

---

#### GP3.8I — 实现 ContainerPage 导出证书

**文件**: `src/gui/pages/ContainerPage.cpp`

**前置**: GP3.7I

**实现 `onExportCert(devName, appName, containerName)`**:

1. `CertService::instance().exportCert(devName, appName, containerName, true)`
2. 失败 → MessageBox，return
3. 成功 → `QFileDialog::getSaveFileName()` 选择保存路径
4. 用户取消则 return
5. 写入文件 `QFile`
6. 提示导出成功

**验收**: `make test` 全部通过，ContainerPage 新增测试全部 Green

---

## 执行计划

```
阶段 1 (可并行):
  GP1.1T  编写 DevicePage 测试
  GP2.1T  编写 AppPage 测试      [P]
  GP3.1T  编写 ContainerPage 测试 [P]

阶段 2 (串行 — DevicePage):
  GP1.2I → GP1.3I → GP1.4I → GP1.5I → GP1.6I

阶段 3 (串行 — AppPage):
  GP2.2I → GP2.3I → GP2.4I → GP2.5I → GP2.6I → GP2.7I → GP2.8I

阶段 4 (串行 — ContainerPage):
  GP3.2I → GP3.3I → GP3.4I → GP3.5I → GP3.6I → GP3.7I → GP3.8I

每阶段结束后:
  make test  # 确保全部 Green
```

---

## 任务统计

| 类型 | 数量 |
|------|------|
| 测试任务 (T) | 3 |
| 实现任务 (I) | 19 |
| **总计** | **22** |

---

## StubPlugin 共享说明

三个测试文件（`test_devicepage.cpp`, `test_apppage.cpp`, `test_containerpage.cpp`）都需要一个简单的 `StubPlugin`，它需要能预置 `DeviceInfo`/`AppInfo`/`ContainerInfo` 返回数据。

可参考 `test_modulepage.cpp` 中已定义的 `StubPlugin`（空实现），但需扩展为可预置返回数据的版本。

**建议**: 在每个测试文件中内联定义（避免增加额外共享文件），或从 `tests/integration/MockPlugin.h` 复用。由于 `MockPlugin.h` 已经完整实现了所有功能，**推荐直接 include `integration/MockPlugin.h`**（tests CMakeLists 的 gui_test 已配置 `${CMAKE_SOURCE_DIR}/tests` 为 include 路径）。

---

## 文档结束
