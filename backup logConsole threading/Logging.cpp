#include "Logging.h"

#include <QTextStream>
#include <QDateTime>
#include <QFile>
#include <QMutex>
#include "LogConsoleWidget.h"


static QScopedPointer<QFile> m_logFile;
static bool m_fileExist = false;
static bool m_enableConsole = true;
static bool m_enableFile = true;
static bool m_enableDebug = true;
static QMutex mutex;

static Tolmi::Logging::LogConsoleWidget *Console = nullptr;


QString Tolmi::Logging::msgTypeToString(QtMsgType type)
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
    }
}
QtMsgType Tolmi::Logging::StringToMsgType(QString str)
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
}

/*!
 * \brief Функция устанавливаем файл для логирования. Если файл не существует
 * логи в файл не пишутся
 */
void Tolmi::Logging::setLoggingFile(const QString &filePath)
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
void Tolmi::Logging::setEnableFileLogging(bool enable)
{
    m_enableFile = enable;
}

/*!
 * \brief Функция устанавливает разрешение на отображение логов в консоли
 */
void Tolmi::Logging::setEnableConsoleLogging(bool enable)
{
    m_enableConsole = enable;
}

/*!
 * \brief Функция устанавливает разрешение на запись/отображение сообщения типа "Debug"
 */
void Tolmi::Logging::setEnableDebug(bool enable)
{
    m_enableDebug = enable;
}

/*!
 * \brief Функция логирования для установки в qInstallMessageHandler.
 *  Пример:
    qInstallMessageHandler(Tolmi::Logging::messageHandler);
    Tolmi::Logging::setFilePath(logPath);
    Tolmi::Logging::setEnableFileLogging(true);
    Tolmi::Logging::setEnableConsoleLogging(true);
    Tolmi::Logging::setEnableDebug(true);
 */
void Tolmi::Logging::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
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
    //по тиу такого Tolmi::ClassName::MethodName. Если, например, неймспейса
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

void Tolmi::Logging::setLogConsole(LogConsoleWidget *c)
{
    Console = c;
}


