#include "Logging.h"

#include <QTextStream>
#include <QDateTime>
#include <QFile>
#include <QMutex>
#include <qapplication.h>
#include <qthread.h>
#include "LogConsoleWidget.h"
#include "qdebug.h"
#include "qdir.h"


namespace Logging
{
    static QScopedPointer<QFile> m_logFile;
    static bool m_fileExist = false;
    static bool m_enableConsole = true;
    static bool m_enableFile = true;
    static bool m_enableDebug = true;
    static QMutex mutex;
    static LogConsoleWidget *Console;
};




QString Logging::msgTypeToString(QtMsgType type)
{
    switch(type)
    {
    case QtDebugMsg:
        return "DEBUG";
    case QtWarningMsg:
        return "WARNING";
    case QtCriticalMsg:
        return "CRITICAL";
    case QtFatalMsg:
        return "FATAL";
    case QtInfoMsg:
        return "INFO";
    default:
        return "";
    }
}
QtMsgType Logging::StringToMsgType(QString str)
{
    if(str.contains("INFO", Qt::CaseInsensitive))
        return QtInfoMsg;
    else if(str.contains("DEBUG", Qt::CaseInsensitive))
        return QtDebugMsg;
    else if(str.contains("WARNING", Qt::CaseInsensitive))
        return QtWarningMsg;
    else if(str.contains("CRITICAL", Qt::CaseInsensitive))
        return QtCriticalMsg;
    else if(str.contains("FATAL", Qt::CaseInsensitive))
        return QtFatalMsg;
    return QtFatalMsg;
}

/*!
 * \brief Функция устанавливаем файл для логирования. Если файл не существует
 * логи в файл не пишутся
 */
void Logging::setLoggingFile(const QString &filePath)
{
    mutex.lock();
    if(QFile::exists(filePath))
    {
        // Устанавливаем файл логирования
        m_logFile.reset(new QFile(filePath));
        // Открываем файл логирования
        m_logFile.data()->open(QFile::Append | QFile::Text);
        //Выставляе флаг разрешения
        m_fileExist = true;
    }
    else
    {
        m_fileExist = false;
    }

    mutex.unlock();
}

/*!
 * \brief Функция устанавливает разрешение на запись логов в файл
 */
void Logging::setEnableFileLogging(bool enable)
{
    m_enableFile = enable;
}

/*!
 * \brief Функция устанавливает разрешение на отображение логов в консоли
 */
void Logging::setEnableConsoleLogging(bool enable)
{
    m_enableConsole = enable;
}

/*!
 * \brief Функция устанавливает разрешение на запись/отображение сообщения типа "Debug"
 */
void Logging::setEnableDebug(bool enable)
{
    m_enableDebug = enable;
}

/*!
 * \brief Функция логирования для установки в qInstallMessageHandler.
 *  Пример:
    qInstallMessageHandler(Logging::messageHandler);
    Logging::setFilePath(logPath);
    Logging::setEnableFileLogging(true);
    Logging::setEnableConsoleLogging(true);
    Logging::setEnableDebug(true);
 */
void Logging::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    //Если отключен вывод дебаг сообщений и приходит дебаг сообщение, то прерываем метод
    if(!m_enableDebug && type == QtDebugMsg) return;

    //Записываем дату и время
    QDateTime date = QDateTime::currentDateTime();
    QString timeDateStr = date.toString("yyyy-MM-dd hh:mm:ss.zzz");
    //По типу определяем, к какому уровню относится сообщение
    QString typeMsg = msgTypeToString(type);


    // Ищем подстроку, которая начинается с последнего пробела
    QString func = context.function;
    int lastIndex = func.lastIndexOf('(');
    func = func.left(lastIndex).trimmed();

    int firstIndex = func.lastIndexOf(' ');
    func = func.mid(firstIndex+1);
    //QStringList list = func.split("::");


    QString str;
    str = QString("%1 %2 %3 >> %4").arg(timeDateStr).arg(typeMsg).arg(func).arg(msg);


    //В зависимости от установленных флагов выводим сообщение
    //в консоль и пишем в файл
    mutex.lock();
    if(m_enableConsole)
    {
        QTextStream out(stdout);
        out << str << '\n';
        //std::cout << str.toStdString() << std::endl;
    }
    if(m_fileExist && m_enableFile)
    {
        QTextStream out(m_logFile.data());
        out << str << Qt::endl;
        out.flush();
    }
    mutex.unlock();
    if(Console){
        emit Console->sigAppendFormatedLine(type, date, func, msg);
        // if(QThread::currentThread() == QApplication::instance()->thread())
        //     Console->appendFormatedLine(type, date, func, msg); // напрямую если тотже поток
        // else
        // {
        //     emit Console->sigAppendFormatedLine(type, date, func, msg); // через очередь
        //     QApplication::processEvents(); // обрабатываем события (чтобы сообщение успело сделать вывод в виджет)
        // }

    }

}


void Logging::setLogConsole(LogConsoleWidget *c)
{
    Console = c;
}
Logging::LogConsoleWidget* Logging::getLogConsole()
{
    return Console;
}

Logging::LogConsoleWidget *Logging::quickNewConsole(QWidget *parent, Qt::WindowFlags f)
{
    setEnableFileLogging(true);
    setEnableConsoleLogging(true);
    setEnableDebug(true);
    qInstallMessageHandler(messageHandler);
    LogConsoleWidget *Console = new LogConsoleWidget(parent, f);
    setLogConsole(Console);
    QObject::connect(Console, &Logging::LogConsoleWidget::destroyed, [Console](){
        if(Console == Logging::getLogConsole())
            Logging::setLogConsole(nullptr);
    });

    QDir logDir = QDir::currentPath();
    if(!logDir.exists("Logs")) logDir.mkdir("Logs");  // создаем папку logs если ее нет
    logDir.cd("Logs");
    QDateTime date = QDateTime::currentDateTime();
    QString logFileName = date.toString("yyyy-MM-dd hh-mm-ss-zzz ") + "logFile.log";

    Console->setLogFilePath(logDir.absoluteFilePath(logFileName));
    setLoggingFile(Console->getLogFilePath());


    //загрузка настроек
    QString defSettings(logDir.absoluteFilePath("ConsoleDefaultSettings.ini"));
    if(QFileInfo::exists(defSettings))
        Console->loadSettings(defSettings);
    else
        Console->saveSettings(defSettings); // сохраняем дефолтные настройки


    //загружаем историю ? файл новый, нечего загружать
    //Console->loadLogsHistory(logDir.absoluteFilePath(logFileName));

    return Console;
}
