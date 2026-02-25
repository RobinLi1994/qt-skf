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
$BuildDir = "build-release"
$DistDir = "dist"
# 优先使用 CI 传入的 PKG_ARCH，本地运行时默认 amd64
$PkgArch = if ($env:PKG_ARCH) { $env:PKG_ARCH } else { "amd64" }
$ZipName = "${AppName}-windows-${PkgArch}.zip"

Write-Host "==> Windows Packaging: ${AppName} [${PkgArch}]"

# ==============================================================================
# 1. Release 构建
# ==============================================================================
Write-Host "--- Step 1: Release Build ---"
cmake -B $BuildDir `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_EXPORT_COMPILE_COMMANDS=OFF

$Nproc = [Environment]::ProcessorCount
cmake --build $BuildDir --config Release -j $Nproc

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
# 4. 复制 vendor 库 (SKF 驱动)
# ==============================================================================
Write-Host "--- Step 4: Copy Vendor Libraries ---"
if (Test-Path "vendor") {
    Copy-Item -Recurse "vendor" -Destination "${DistDir}/vendor"
}

# ==============================================================================
# 5. 创建 ZIP
# ==============================================================================
Write-Host "--- Step 5: Create ZIP ---"
if (Test-Path $ZipName) {
    Remove-Item $ZipName
}
Compress-Archive -Path "${DistDir}/*" -DestinationPath $ZipName

Write-Host ""
Write-Host "==> Package created: ${ZipName} (arch=${PkgArch})"
Get-Item $ZipName | Format-Table Name, Length
