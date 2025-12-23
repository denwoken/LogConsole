QT       += core gui concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17


SOURCES += \
    $$PWD/FunctionSelectorWidget.cpp \
    $$PWD/LogConsoleWidget.cpp \
    $$PWD/LogWidgetSettings.cpp \
    $$PWD/Logging.cpp \
    $$PWD/main.cpp

FORMS += \
    $$PWD/LogConsoleWidget.ui \
    $$PWD/LogWidgetSettings.ui

HEADERS += \
    $$PWD/FunctionSelectorWidget.h \
    $$PWD/LogConsoleWidget.h \
    $$PWD/LogWidgetSettings.h \
    $$PWD/Logging.h	\
		LoggingEncoder.h

RESOURCES += \
    $$PWD/ConsoleResources.qrc



#для подключения модуля:
# include($$PWD/LogConsole/LogConsole.pri)
# INCLUDEPATH += $$PWD/LogConsole
