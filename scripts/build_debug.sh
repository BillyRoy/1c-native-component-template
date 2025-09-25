#!/bin/bash

# Немедленно завершить выполнение, если какая-либо команда завершится с ошибкой
set -e

# Вывод информационных сообщений
echo "[INFO] Configuring DEBUG build..."

# Шаг 1: Установка зависимостей Conan
echo "[INFO] Running Conan install for Debug..."
conan install . -s build_type=Debug --output-folder=build/debug --build=missing

echo "[INFO] Activating build environment..."
source build/debug/conanbuild.sh

# Шаг 2: Конфигурация проекта CMake
echo "[INFO] Running CMake configuration for Debug..."
cmake -S . -B build/debug -G "Ninja" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$PWD/build/debug/conan_toolchain.cmake

# Шаг 3: Сборка проекта
echo "[INFO] Building project in Debug mode..."
cmake --build build/debug

# Шаг 4 (Опционально): Установка артефактов
echo "[INFO] Installing Debug artifacts..."
cmake --install build/debug

echo "[SUCCESS] Debug build and installation completed successfully."