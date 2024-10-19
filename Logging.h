#ifndef LOGGING_H
#define LOGGING_H


#include <qlogging.h>
#include "LogConsoleWidget.h"
#include "qdebug.h"

namespace Logging{

class LogConsoleWidget;

QString msgTypeToString(QtMsgType type);
QtMsgType StringToMsgType(QString str);
void setLoggingFile(const QString &filePath);
void setEnableFileLogging(bool enable);
void setEnableConsoleLogging(bool enable);
void setEnableDebug(bool enable);
void setLogConsole(LogConsoleWidget *Console);
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);


LogConsoleWidget* quickNewConsole();

}



#endif // LOGGING_H
