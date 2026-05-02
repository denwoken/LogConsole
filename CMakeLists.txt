cmake_minimum_required(VERSION 3.16)

#Для подключения модуля как статической библиотеки
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/LogConsole)
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
set(QT_MODULES 
    Core 
    Gui 
    Widgets 
    Concurrent
)
find_package(QT NAMES Qt5 REQUIRED COMPONENTS ${QT_MODULES})
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${QT_MODULES})



# Добавим исходные файлы
set(PROJECT_SOURCES
    FunctionSelectorWidget.cpp
    LogConsoleWidget.cpp
    LogWidgetSettings.cpp
    Logging.cpp
    main.cpp

    FunctionSelectorWidget.h
    Logging.h
    LogConsoleWidget.h
    LogWidgetSettings.h
		LoggingEncoder.h
)

# Добавим файлы форм
set(PROJECT_UI
    LogConsoleWidget.ui
    LogWidgetSettings.ui
)

# Добавим ресурсы
set(PROJECT_RESOURCES
    ConsoleResources.qrc
)

# compile
add_library(${PROJECT_NAME} STATIC
#add_executable(${PROJECT_NAME} WIN32
    ${PROJECT_SOURCES}
    ${PROJECT_RESOURCES}
    ${PROJECT_UI}
)

target_include_directories(${PROJECT_NAME} PUBLIC ./)

# Подключим необходимые модули Qt
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Concurrent
)


