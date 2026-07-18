@echo off
chcp 65001 >nul
echo ============================================
echo BrushEngine Build Script
echo ============================================

where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] CMake not found in PATH!
    echo Please install CMake: https://cmake.org/download/
    pause
    exit /b 1
)

if "%VULKAN_SDK%"=="" (
    echo [ERROR] Vulkan SDK not found!
    echo Please install Vulkan SDK: https://vulkan.lunarg.com/sdk/home#windows
    echo Make sure to check "Add to PATH" during installation.
    pause
    exit /b 1
)

echo Vulkan SDK: %VULKAN_SDK%

if not exist build mkdir build
cd build

echo Configuring...
cmake .. -A x64 -DCMAKE_BUILD_TYPE=Release

if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed!
    pause
    exit /b 1
)

echo Building...
cmake --build . --config Release --parallel

if %errorlevel% neq 0 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

echo ============================================
echo Build successful!
echo Executable: build\src\Release\BrushEngine.exe
echo ============================================
pause