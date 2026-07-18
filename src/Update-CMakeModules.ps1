# .\Update-CMakeModules.ps1
# 放在 BrushEngine\src\ 下运行，自动更新所有子模块的 CMakeLists.txt
# 新增模块只需要在 cmake-modules.json 里加一段配置，再运行脚本即可。不需要改脚本代码。

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ConfigFile = Join-Path $ScriptDir "cmake-modules.json"

if (-not (Test-Path $ConfigFile)) {
    Write-Error "Config file not found: $ConfigFile"
    exit 1
}

# 读取配置
$config = Get-Content $ConfigFile -Raw | ConvertFrom-Json

foreach ($module in $config.modules) {
    $moduleDir = Join-Path $ScriptDir $module.dir
    $cmakeFile = Join-Path $moduleDir "CMakeLists.txt"

    if (-not (Test-Path $moduleDir)) {
        Write-Warning "Module directory not found: $moduleDir, skipping."
        continue
    }

    # 扫描 .cpp 和 .h
    $sources = Get-ChildItem -Path $moduleDir -File -Filter "*.cpp" | 
        Sort-Object Name | 
        ForEach-Object { "    `${CMAKE_CURRENT_LIST_DIR}/$($_.Name)" }

    $headers = Get-ChildItem -Path $moduleDir -File -Filter "*.h" | 
        Sort-Object Name | 
        ForEach-Object { "    `${CMAKE_CURRENT_LIST_DIR}/$($_.Name)" }

    # 构建 CMakeLists.txt 内容
    $lines = [System.Collections.Generic.List[string]]::new()

    # 可选的 cmake_minimum_required
    if ($module.cmake_min_version) {
        $lines.Add("cmake_minimum_required(VERSION $($module.cmake_min_version))")
        $lines.Add("")
    }

    # 可选的注释
    if ($module.comment) {
        $lines.Add("# $($module.comment)")
    }

    # set(SOURCES ...)
    $lines.Add("set($($module.sources_var)")
    foreach ($s in $sources) { $lines.Add($s) }
    $lines.Add(")")
    $lines.Add("")

    # set(HEADERS ...)
    $lines.Add("set($($module.headers_var)")
    foreach ($h in $headers) { $lines.Add($h) }
    $lines.Add(")")
    $lines.Add("")

    # add_library
    $lines.Add("add_library($($module.lib_name) STATIC `${$($module.sources_var)} `${$($module.headers_var)})")
    $lines.Add("")

    # target_include_directories
    if ($module.includes -and $module.includes.Count -gt 0) {
        $lines.Add("target_include_directories($($module.lib_name) PUBLIC")
        foreach ($inc in $module.includes) {
            $lines.Add("    $inc")
        }
        $lines.Add(")")
        $lines.Add("")
    }

    # target_link_libraries
    if ($module.links -and $module.links.Count -gt 0) {
        $lines.Add("target_link_libraries($($module.lib_name) PUBLIC")
        foreach ($link in $module.links) {
            $lines.Add("    $link")
        }
        $lines.Add(")")
        $lines.Add("")
    }

    # target_compile_definitions
    if ($module.definitions -and $module.definitions.Count -gt 0) {
        $lines.Add("target_compile_definitions($($module.lib_name) PRIVATE")
        foreach ($def in $module.definitions) {
            $lines.Add("    $def")
        }
        $lines.Add(")")
        $lines.Add("")
    }

    # 写入文件
    $lines | Out-File -FilePath $cmakeFile -Encoding utf8
    Write-Host "[$($module.lib_name)] Updated $cmakeFile"
    Write-Host "  Sources: $($sources.Count), Headers: $($headers.Count)"
}

Write-Host ""
Write-Host "All modules updated!"