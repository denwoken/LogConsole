cmake_minimum_required(VERSION 3.16)

#Для подключения модуля как статической библиотеки
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/LogConsole)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/LogConsole)
#target_link_libraries(${PROJECT_NAME} PUBLIC LogConsoleLibrary)

# Название проекта
set(PROJECT_NAME LogConsoleLibrary)
project(${PROJECT_NAME} VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Установим C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Найдем необходимые модули Qt
set(QT_MODULES Core Gui Widgets Concurrent)
find_package(QT NAMES Qt5 REQUIRED COMPONENTS ${QT_MODULES})
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${QT_MODULES})



# Добавим исходные файлы
set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/FunctionSelectorWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LogConsoleWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LogWidgetSettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Logging.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# Добавим заголовочные файлы
set(PROJECT_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/FunctionSelectorWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Logging.h
)

# Добавим файлы форм
set(PROJECT_UI_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/LogConsoleWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/LogWidgetSettings.h
)
set(PROJECT_UI
    ${CMAKE_CURRENT_SOURCE_DIR}/LogConsoleWidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/LogWidgetSettings.ui
)

# Добавим ресурсы
set(PROJECT_RESOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc
)



# compile
add_library(${PROJECT_NAME} STATIC
#add_executable(${PROJECT_NAME} WIN32
    ${PROJECT_SOURCES}
    ${PROJECT_RESOURCES}
    ${PROJECT_UI}

    ${PROJECT_HEADERS}
    ${PROJECT_UI_HEADERS}
)

# target_include_directories(${PROJECT_NAME} PUBLIC
#     ${PROJECT_HEADERS}
#     ${PROJECT_UI_HEADERS}
# )

# Подключим необходимые модули Qt
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Concurrent
)


