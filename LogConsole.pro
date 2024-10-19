QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    FunctionSelectorWidget.cpp \
    LogConsoleWidget.cpp \
    LogWidgetSettings.cpp \
    Logging.cpp \
    main.cpp



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    LogConsoleWidget.ui \
    LogWidgetSettings.ui

HEADERS += \
    FunctionSelectorWidget.h \
    LogConsoleWidget.h \
    LogWidgetSettings.h \
    Logging.h

RESOURCES += \
    resources.qrc

DISTFILES += \
    CMakeLists.txt \
    LogConsole.pri
