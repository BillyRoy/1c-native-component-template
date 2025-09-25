@echo off
setlocal

ECHO [INFO] Configuring RELEASE build...

REM Шаг 1: Установка зависимостей Conan
ECHO [INFO] Running Conan install for Release...
conan install . -s build_type=Release --output-folder=build/release --build=missing
if %errorlevel% neq 0 (
    ECHO [ERROR] Conan install failed.
    exit /b %errorlevel%
)

REM Активируем окружение компилятора, настроенное Conan.
ECHO [INFO] Activating build environment...
call build\release\conanbuild.bat

REM Шаг 2: Конфигурация проекта CMake
ECHO [INFO] Running CMake configuration for Release...
cmake -S . -B build/release -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%cd%/build/release/conan_toolchain.cmake
if %errorlevel% neq 0 (
    ECHO [ERROR] CMake configuration failed.
    exit /b %errorlevel%
)

REM Шаг 3: Сборка проекта
ECHO [INFO] Building project in Release mode...
cmake --build build/release --config Release
if %errorlevel% neq 0 (
    ECHO [ERROR] CMake build failed.
    exit /b %errorlevel%
)

REM Шаг 4 (Опционально): Установка артефактов
ECHO [INFO] Installing Release artifacts...
cmake --install build/release --config Release
if %errorlevel% neq 0 (
    ECHO [ERROR] CMake install failed.
    exit /b %errorlevel%
)

ECHO [SUCCESS] Release build and installation completed successfully.