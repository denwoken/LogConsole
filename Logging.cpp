#include "Logging.h"

#include <QTextStream>
#include <QDateTime>
#include <QFile>
#include <QMutex>
#include "LogConsoleWidget.h"
#include "qdebug.h"
#include "qdir.h"


static QScopedPointer<QFile> m_logFile;
static bool m_fileExist = false;
static bool m_enableConsole = true;
static bool m_enableFile = true;
static bool m_enableDebug = true;
static QMutex mutex;

static QScopedPointer<Logging::LogConsoleWidget> Console;


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


    //Код ниже является костылем для решения проблемы. Задаваемый ранее паттерн нормально работал
    //только если методы печати вызывались из методов класса находящихся в неймспейсе,
    //по тиу такого ClassName::MethodName. Если, например, неймспейса
    //не было или печать из свободной функции, то условие rx.indexIn(context.function) != -1
    //не проходило проверку, и ни чего не печаталось в консоль/файл. Поэтому
    //теперь 4 отдельный QRegExp для случаев с 1, 2, 3, 4 вложености. Это наверняка
    //можно решить изящно и красиво, но нет ни времени, ни желания разбираться с тем
    //как работают регулярные выражения
    /*
    //На случай Namespace::Namespace::Class::Method
    QRegExp rx("([\\w-]+::[\\w-]+::[\\w-]+::[\\w-]+)");
    //На случай Namespace::Class::Method
    QRegExp rx2("([\\w-]+::[\\w-]+::[\\w-]+)");
    //На случай Class::Method
    QRegExp rx3("([\\w-]+::[\\w-]+)");
    //На случай свободной функции
    QRegExp rx4("([\\w-]+)");

    if(rx.indexIn(context.function) != -1)
        str = time + typeMsg + rx.cap(1) + ": " + msg;
    else if(rx2.indexIn(context.function) != -1)
        str = time + typeMsg + rx2.cap(1) + ": " + msg;
    else if(rx3.indexIn(context.function) != -1)
        str = time + typeMsg + rx3.cap(1) + ": " + msg;
    else if(rx4.indexIn(context.function) != -1)
        str = time + typeMsg + rx4.cap(1) + ": " + msg;
    else
        str = time + typeMsg + ": " + msg + " (Logging RegExp error)";
    */

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
    if(Console)
        emit Console->sigAppendFormatedLine(type, date, func, msg);
}


void Logging::setLogConsole(LogConsoleWidget *c)
{
    if(c)
        Console.reset(c);
}



Logging::LogConsoleWidget *Logging::quickNewConsole()
{
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

    //загрузка настроек
    QString defSettings(logDir.absoluteFilePath("ConsoleDefaultSettings.ini"));
    if(QFileInfo::exists(defSettings))
        Console->loadSettings(defSettings);
    else
        Console->saveSettings(defSettings); // сохраняем дефолтные настройки


    //QElapsedTimer tim;
    //tim.start();
    //загружаем историю
    Console->loadLogsHistory(logDir.absoluteFilePath(logFileName));
    return Console;

}
