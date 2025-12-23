#include "Logging.h"

#include <QTextStream>
#include <QDateTime>
#include <QFile>
#include <QMutex>
#include "LogConsoleWidget.h"
#include "qcoreapplication.h"
#include "qdebug.h"
#include "qdir.h"
#include "LoggingEncoder.h"
#include "qthread.h"

static QMutex mutex; // только для защиты QFile
static QScopedPointer<QFile> m_logFile;

static std::atomic_bool m_fileExist = false;
static std::atomic_bool m_enableConsole = true;
static std::atomic_bool m_enableFile = true;
static std::atomic_bool m_enableDebug = true;
static std::atomic_bool m_enableFileEncoding = true;




static std::atomic<Logging::LogConsoleWidget*> m_consoleInstance = nullptr;


QString Logging::msgTypeToString(const QtMsgType type)
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
    return "";
}
QtMsgType Logging::StringToMsgType(const QString& str)
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
    return QtDebugMsg;
}


/*!
 * \brief Функция устанавливаем файл для логирования. Если файл не существует
 * логи в файл не пишутся
 */
void Logging::setLoggingFile(const QString &filePath)
{

    if(QFile::exists(filePath))
    {
        QMutexLocker locker(&mutex);
        // Устанавливаем файл логирования
        m_logFile.reset(new QFile(filePath));
        // Открываем файл логирования
        m_logFile.data()->open(QFile::Append | QFile::Text);
        if(!m_logFile->isOpen())
            qCritical() << "Could not open log file for writing";
        //Выставляе флаг разрешения
        m_fileExist = true;
    }
    else
    {
        m_fileExist = false;
    }
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
 * \brief Logging::setEnableFileEncoding Включает или отключает кодрование логов в файле
 */
void Logging::setEnableFileEncoding(bool enable)
{
    m_enableFileEncoding = enable;
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
    //QString timeDateStr = date.toString("yyyy-MM-dd hh:mm:ss.zzz");



    // Ищем подстроку, которая начинается с последнего пробела
    QString func = context.function;
    int lastIndex = func.lastIndexOf('(');
    func = func.left(lastIndex).trimmed();

    int firstIndex = func.lastIndexOf(' ');
    func = func.mid(firstIndex+1);
    //QStringList list = func.split("::");


    QString str;
    //По типу определяем, к какому уровню относится сообщение
    // QString typeMsg = msgTypeToString(type);
    // str = QString("%1 %2 %3 >> %4").arg(timeDateStr, typeMsg, func, msg);
    //LogLine(type, date, func, msg);
    str = LogLine(type, date, func, msg).toQString();
    QString encodedStr;
    if(m_enableFileEncoding && m_fileExist && m_enableFile)
    {
        encodedStr = Logging::Encoder::encodeLineString(str);
    }

    //В зависимости от установленных флагов выводим сообщение
    //в консоль и пишем в файл
    {
        QMutexLocker locker(&mutex);
        if(m_enableConsole)
        {
            QTextStream out(stdout);
            out << str << '\n';
            //std::cout << str.toStdString() << std::endl;
            out.flush();
            if (out.status() != QTextStream::Ok) {
                qCritical() << "Console write error!";
            }
        }
        if(m_fileExist && m_enableFile)
        {
            QTextStream out(m_logFile.data());
            if(!encodedStr.isEmpty())
                out << encodedStr << Qt::endl;
            else
                out << str << Qt::endl;
            out.flush();
            // Проверка ошибки после записи
            if (out.status() != QTextStream::Ok ||
                m_logFile->error() != QFile::NoError) {
                qCritical() << "File log write error:" << m_logFile->errorString();
            }
        }
    }

    auto c = m_consoleInstance.load();
    if(c){
        if(QThread::currentThread() != qApp->thread()){
            emit c->sigAppendFormatedLine(type, date, func, msg);
        } else {
            c->appendFormatedLine(type, date, func, msg);
        }
    }
}

void Logging::setLogConsole(LogConsoleWidget *c)
{
    m_consoleInstance = c;
}
Logging::LogConsoleWidget* Logging::getLogConsole()
{
    return m_consoleInstance;
}

Logging::LogConsoleWidget *Logging::quickNewConsole(QWidget *parent, Qt::WindowFlags f)
{
    setEnableFileLogging(true);
    setEnableConsoleLogging(true);
    setEnableDebug(true);
    setEnableFileEncoding(false);
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


    if(parent == nullptr){
    // установка SteleSheets
        QFile f(":/Console/resources/StyleSheetTolmi1.qss");
        f.open(QFile::ReadOnly);
        QString style = f.readAll();
        f.close();
        Console->setStyleSheet(style);
    }

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


