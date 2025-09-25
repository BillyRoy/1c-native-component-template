#!/bin/bash

# Немедленно завершить выполнение, если какая-либо команда завершится с ошибкой
set -e

# Вывод информационных сообщений
echo "[INFO] Configuring RELEASE build..."

# Шаг 1: Установка зависимостей Conan
echo "[INFO] Running Conan install for Release..."
conan install . -s build_type=Release --output-folder=build/release --build=missing

echo "[INFO] Activating build environment..."
source build/release/conanbuild.sh

# Шаг 2: Конфигурация проекта CMake
echo "[INFO] Running CMake configuration for Release..."
cmake -S . -B build/release -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$PWD/build/release/conan_toolchain.cmake

# Шаг 3: Сборка проекта
echo "[INFO] Building project in Release mode..."
cmake --build build/release

# Шаг 4 (Опционально): Установка артефактов
echo "[INFO] Installing Release artifacts..."
cmake --install build/release

echo "[SUCCESS] Release build and installation completed successfully."