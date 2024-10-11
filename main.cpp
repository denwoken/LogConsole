

#include "qdatetime.h"
#include "qdebug.h"
#include "qelapsedtimer.h"
#include "qfileinfo.h"
#include <QApplication>
#include <QDir>

#include "Logging.h"
#include "LogConsoleWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    using namespace Logging;
    QDir logDir = QDir::currentPath();
    if(!logDir.exists("logs")) logDir.mkdir("logs"); // создаем папку logs если ее нет
    logDir.cd("logs");
    QDateTime date = QDateTime::currentDateTime();
    QString logFileName = date.toString("yyyy-MM-dd hh-mm-ss-zzz ") + "logFile.log";

    // создаем файл для логов
    QFile file(logDir.absoluteFilePath(logFileName));
    if(file.open(QIODevice::WriteOnly)) file.close();
    else qCritical() << "Файл для логов не создан: " << logDir.absoluteFilePath(logFileName);

    setLoggingFile(logDir.absoluteFilePath(logFileName));
    setEnableFileLogging(true);
    setEnableConsoleLogging(true);
    setEnableDebug(true);
    qInstallMessageHandler(messageHandler);


    LogConsoleWidget *Console = new LogConsoleWidget();

    // установка SteleSheets
    QFile f(":/resources/StyleSheetTolmi1.qss");
    f.open(QFile::ReadOnly);
    QString style = f.readAll();
    f.close();
    Console->setStyleSheet(style + "QWidget { background-color: rgb(32,32,32); }");

    setLogConsole(Console);
    Console->show();

    //загрузка настроек
    QString defSettings(logDir.absoluteFilePath("ConsoleDefaultSettings.ini"));
    if(QFileInfo::exists(defSettings))
        Console->loadSettings(defSettings);
    else
        Console->saveSettings(defSettings); // сохраняем дефолтные настройки


    QElapsedTimer tim;
    tim.start();
    //загружаем историю
    //Console->loadLogsHistory(logDir.absoluteFilePath(logFileName));
    //тк создали новый файл, загружать нечего...
    qInfo() << "time elapsed: " << tim.elapsed() << " ms ";

    qDebug() << " debug string ";
    qWarning() << " warning string ";
    qInfo() << " info string ";
    qCritical() << " critical string ";


    return a.exec();
}
