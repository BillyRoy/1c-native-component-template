# Проверяем, была ли определена переменная BUILD_TYPE
if(NOT DEFINED BUILD_TYPE)
    message(FATAL_ERROR "Ошибка: Тип сборки не указан. \nПример: cmake -DBUILD_TYPE=release -P build.cmake")
endif()

# Приводим значение к нижнему регистру для удобства
string(TOLOWER "${BUILD_TYPE}" build_type)

# Проверяем корректность значения
if(NOT (build_type STREQUAL "debug" OR build_type STREQUAL "release"))
    message(FATAL_ERROR "Ошибка: Неверный тип сборки '${build_type}'. Используйте 'debug' или 'release'.")
endif()

# Определяем ОС и имя скрипта
if(WIN32)
    set(script_name "build_${build_type}.bat")
else()
    set(script_name "build_${build_type}.sh")
endif()

# Формируем полный путь к скрипту
set(script_path "${CMAKE_CURRENT_LIST_DIR}/scripts/${script_name}")

if(NOT EXISTS ${script_path})
    message(FATAL_ERROR "Ошибка: Скрипт сборки не найден по пути: ${script_path}")
endif()

message(STATUS "Запуск скрипта: ${script_name}")

# Запускаем скрипт из корневой директории проекта
execute_process(
    COMMAND ${script_path}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    RESULT_VARIABLE result
)

# Проверяем результат выполнения
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Ошибка: Сборка завершилась с кодом ${result}.")
else()
    message(STATUS "Сборка успешно завершена.")
endif()