@echo off
setlocal

ECHO [INFO] Configuring DEBUG build...

REM Шаг 1: Установка зависимостей Conan
ECHO [INFO] Running Conan install for Debug...
conan install . -s build_type=Debug --output-folder=build/debug --build=missing
if %errorlevel% neq 0 (
    ECHO [ERROR] Conan install failed.
    exit /b %errorlevel%
)

REM Активируем окружение компилятора, настроенное Conan.
ECHO [INFO] Activating build environment...
call build\debug\conanbuild.bat

REM Шаг 2: Конфигурация проекта CMake
ECHO [INFO] Running CMake configuration for Debug...
cmake -S . -B build/debug -G "Ninja" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=%cd%/build/debug/conan_toolchain.cmake
if %errorlevel% neq 0 (
    ECHO [ERROR] CMake configuration failed.
    exit /b %errorlevel%
)

REM Шаг 3: Сборка проекта
ECHO [INFO] Building project in Debug mode...
cmake --build build/debug --config Debug
if %errorlevel% neq 0 (
    ECHO [ERROR] CMake build failed.
    exit /b %errorlevel%
)

REM Шаг 4 (Опционально): Установка артефактов
ECHO [INFO] Installing Debug artifacts...
cmake --install build/debug --config Debug
if %errorlevel% neq 0 (
    ECHO [ERROR] CMake install failed.
    exit /b %errorlevel%
)

ECHO [SUCCESS] Debug build and installation completed successfully.