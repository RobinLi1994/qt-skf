# ==============================================================================
# Windows 打包脚本
#
# 构建 Release 版本并生成 .zip 分发包
#
# 用法: powershell -ExecutionPolicy Bypass -File scripts/package_win.ps1
# 依赖: cmake, Qt 6 (windeployqt)
# ==============================================================================

$ErrorActionPreference = "Stop"

$AppName = "wekey-skf"
$DistDir = "dist"
# 优先使用 CI 传入的 PKG_ARCH，本地运行时默认 amd64
$PkgArch = if ($env:PKG_ARCH) { $env:PKG_ARCH } else { "amd64" }
$ZipName = "${AppName}-windows-${PkgArch}.zip"

Write-Host "==> Windows Packaging: ${AppName} [${PkgArch}]"

# ==============================================================================
# 1. Release 构建（CI 环境中使用已有的 build 目录，跳过重新构建）
# ==============================================================================
Write-Host "--- Step 1: Release Build ---"
if ($env:CI -and (Test-Path "build")) {
    # CI 环境：直接使用 CI 已经构建好的产物
    $BuildDir = "build"
    Write-Host "CI environment detected, using existing build directory: $BuildDir"
} else {
    # 本地环境：执行完整构建
    $BuildDir = "build-release"
    cmake -B $BuildDir `
        -DCMAKE_BUILD_TYPE=Release `
        -DCMAKE_EXPORT_COMPILE_COMMANDS=OFF

    $Nproc = [Environment]::ProcessorCount
    cmake --build $BuildDir --config Release -j $Nproc
}

# ==============================================================================
# 2. 准备分发目录
# ==============================================================================
Write-Host "--- Step 2: Prepare Distribution ---"
if (Test-Path $DistDir) {
    Remove-Item -Recurse -Force $DistDir
}
New-Item -ItemType Directory -Path $DistDir | Out-Null

$Binary = Join-Path $BuildDir "src/app/Release/${AppName}.exe"
if (-not (Test-Path $Binary)) {
    $Binary = Join-Path $BuildDir "src/app/${AppName}.exe"
}
if (-not (Test-Path $Binary)) {
    Write-Error "Binary not found"
    exit 1
}

Copy-Item $Binary -Destination $DistDir

# ==============================================================================
# 3. windeployqt (打包 Qt 依赖)
# ==============================================================================
Write-Host "--- Step 3: Deploy Qt Dependencies ---"
$WinDeployQt = Get-Command windeployqt -ErrorAction SilentlyContinue
if ($WinDeployQt) {
    & windeployqt "${DistDir}/${AppName}.exe"
} else {
    Write-Host "WARN: windeployqt not found, skipping Qt dependency bundling"
}

# ==============================================================================
# 4. 复制 OpenSSL DLL
# ==============================================================================
Write-Host "--- Step 4: Copy OpenSSL Libraries ---"
$OpenSSLPaths = @(
    $env:OPENSSL_ROOT_DIR,
    "C:\Program Files\OpenSSL",
    "C:\Program Files\OpenSSL-Win64",
    "C:\OpenSSL-Win64"
)
$OpenSSLFound = $false
foreach ($sslPath in $OpenSSLPaths) {
    if ($sslPath -and (Test-Path "$sslPath\bin\libcrypto-3-x64.dll")) {
        Write-Host "Found OpenSSL at: $sslPath"
        Copy-Item "$sslPath\bin\libcrypto-3-x64.dll" -Destination $DistDir
        Copy-Item "$sslPath\bin\libssl-3-x64.dll" -Destination $DistDir
        $OpenSSLFound = $true
        break
    }
}
if (-not $OpenSSLFound) {
    # 尝试从 PATH 中查找
    $cryptoDll = Get-Command "libcrypto-3-x64.dll" -ErrorAction SilentlyContinue
    if ($cryptoDll) {
        Write-Host "Found OpenSSL DLLs in PATH"
        Copy-Item $cryptoDll.Source -Destination $DistDir
        $sslDll = Get-Command "libssl-3-x64.dll" -ErrorAction SilentlyContinue
        if ($sslDll) { Copy-Item $sslDll.Source -Destination $DistDir }
    } else {
        Write-Host "WARN: OpenSSL DLLs not found, application may fail to start"
    }
}

# ==============================================================================
# 5. 嵌入内置 SKF 库到 exe 同目录
# ==============================================================================
Write-Host "--- Step 5: Embed Built-in SKF Library ---"
$SkfLibSrc = "resources\lib\win\mtoken_gm3000.dll"
if (Test-Path $SkfLibSrc) {
    Copy-Item $SkfLibSrc -Destination "${DistDir}\mtoken_gm3000.dll"
    Write-Host "Embedded SKF library: ${SkfLibSrc} -> ${DistDir}\mtoken_gm3000.dll"
} else {
    Write-Host "WARN: Built-in SKF library not found: ${SkfLibSrc}"
}

# ==============================================================================
# 6. 创建 ZIP
# ==============================================================================
Write-Host "--- Step 6: Create ZIP ---"
if (Test-Path $ZipName) {
    Remove-Item $ZipName
}
Compress-Archive -Path "${DistDir}/*" -DestinationPath $ZipName

Write-Host ""
Write-Host "==> Package created: ${ZipName} (arch=${PkgArch})"
Get-Item $ZipName | Format-Table Name, Length
