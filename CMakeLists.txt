CMAKE_MINIMUM_REQUIRED(VERSION 3.15 FATAL_ERROR)

PROJECT(AddInNative CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Включение поддержки файлов ресурсов для Windows
IF(WIN32)
    ENABLE_LANGUAGE(RC)
ENDIF()

# Список исходных файлов проекта
SET(AddInNative_SRC 
    src/AddInNative.cpp
    src/dllmain.cpp
    src/stdafx.cpp
)

# Добавление специфичных для Windows файлов
if (WIN32)
    LIST(APPEND AddInNative_SRC src/AddInNative.rc src/AddInNative.def)
endif()

# Поиск и подключение зависимости OpenCV
#find_package(OpenCV REQUIRED)

# Добавление путей к заголовочным файлам
#include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/include)

# Специфичные для Windows определения препроцессора
if (NOT UNIX)
    add_definitions(-DUNICODE -DWIN32 -D_USRDLL -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -DADDINCPP_EXPORTS)
endif()

# Создание разделяемой библиотеки (DLL)
add_library(${PROJECT_NAME} SHARED ${AddInNative_SRC})

# Определяем имя ОС в нижнем регистре
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(OS_NAME "windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(OS_NAME "linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(OS_NAME "macos")
else()
    set(OS_NAME ${CMAKE_SYSTEM_NAME})
endif()

# Формирование имени выходного файла с добавлением архитектуры (например, AddInNative_x86_64)
# CMAKE_SYSTEM_PROCESSOR устанавливается сборочной средой (например, в AMD64 для 64-битной Windows)
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
    set(ARCH_SUFFIX "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")
    set(ARCH_SUFFIX "x86")
else()
    set(ARCH_SUFFIX ${CMAKE_SYSTEM_PROCESSOR})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "AddInNative_${ARCH_SUFFIX}")

# Устанавливаем префикс установки в 'install/<os>/<architecture>'
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install/${OS_NAME}/${ARCH_SUFFIX}")

# Пути к заголовочным файлам
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_BINARY_DIR}"
)

# Определения препроцессора
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        UNICODE
        WIN32
        _USRDLL
        _CRT_SECURE_NO_WARNINGS
        _CRT_SECURE_NO_DEPRECATE
        ADDINCPP_EXPORTS
    )
endif()

# Линковка с OpenCV
#target_link_libraries(${PROJECT_NAME} PRIVATE opencv::opencv)

# Использование предкомпилированных заголовков в Windows
if (WIN32)
    target_precompile_headers(${PROJECT_NAME} PUBLIC include/stdafx.h)
endif()

# Установка свойств проекта
set_target_properties( ${PROJECT_NAME} PROPERTIES
       POSITION_INDEPENDENT_CODE ON 
       CXX_STANDARD 17
       CXX_STANDARD_REQUIRED ON
       OUTPUT_NAME "AddInNative_${ARCH_SUFFIX}"
   )

# Специфичные для Unix флаги линковщика
IF (UNIX)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,--no-undefined -Xlinker --version-script -Xlinker ${CMAKE_CURRENT_SOURCE_DIR}/version.script" )
ENDIF()

# Копирование директории с весами в директорию установки.
#install(DIRECTORY weights/ DESTINATION .)

# Установка артефактов сборки (библиотек).
install(TARGETS ${PROJECT_NAME}
    # Для .dll (Windows) и .so (Linux).
    # Помещаем в корень директории установки (например, install/x86_64/).
    RUNTIME DESTINATION .

    # Для .so (Linux), .dylib (macOS).
    LIBRARY DESTINATION .

    # Для .lib (библиотека импорта в Windows).
    ARCHIVE DESTINATION .
)