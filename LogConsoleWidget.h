#ifndef LOGCONSOLEWIDGET_H
#define LOGCONSOLEWIDGET_H


#include "Logging.h"
#include "qdatetime.h"
#include "qmutex.h"
#include "qtextcursor.h"
#include <QWidget>
#include "Logging.h"

/*! ConsoleFormatter example
 *
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
    QFile f(":/SteleSheets/UiResources/Stylesheets/StyleSheetTolmi1.qss");
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

    //загружаем историю
    //Console->loadLogsHistory(logDir.absoluteFilePath(logFileName));
    //тк создали новый файл, загружать нечего...

 */

QT_BEGIN_NAMESPACE
namespace Ui
{
class LogConsoleWidget;
}
QT_END_NAMESPACE

class QStandardItemModel;
class QTextDocument;
namespace Logging
{

class LogConsoleWidget;
class ConsoleLogFormatter;
class FunctionSelectorWidget;

/*! \brief typedef struct ConsoleSettings
 *  тип данных для хранения настроек LogConsoleWidget
 */
struct ConsoleSettings
{
    struct
    {
        int date : 1;
        int time : 1;
        int timeMs : 1;
        int logLevel : 1;
        int messageSource : 1;
    } dispField;
    struct
    {
        int warningMsg : 1;
        int debugMsg : 1;
        int infoMsg : 1;
        int criticalMsg : 1;
        int fatalMsg : 1;
    } enableLogMsgs;
    struct
    {
        QColor date;
        QColor time;
        QColor logLevel;
        QColor messageSource;

        QColor infoMessage;
        QColor warningMessage;
        QColor debugMessage;
        QColor fatalMessage;
        QColor criticalMessage;

        QColor fatalMessageBg;
        QColor criticalMessageBg;

    }colors;
    bool extendedColors;
    bool restoreWindowPosSize;
    QTextCharFormat textFormat;
    //QDate filterStartDate;
    // QDate filterEndDate;
    QStandardItemModel* functions;
};

/*!
 * \brief The LogLine class класс хранящий содержимое строки логов.
 * сделал не структуру а класс чтобы добавить парсер в конструктор.
 */
class LogLine
{
public:
    LogLine(){};
    LogLine(const QString &line);
    LogLine(const QtMsgType& logLevel, const QDateTime& date,
            const QString& func, const QString msg):
        dateTime(date), type(logLevel), functionStr(func), message(msg){};
    ~LogLine(){};
    inline QString toQString(){
        if(only_message) return message;
        QString timeDateStr = dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
        return QString("%1 %2 %3 >> %4")
            .arg(timeDateStr, msgTypeToString(type), functionStr, message);
    }

    QDateTime dateTime;
    QtMsgType type;
    QString functionStr;
    QString message;
    bool only_message = false;
};

/*!
 * \brief The ConsoleFormatter class класс содержит фнкции формирования текста для консоли
 * Содержит копию или ссылку на структуру настроек консоли для последующего использования.
 * ConsoleFormatter содержит функции для блочной обработки данных и формирования фрагментов документов.
 * В будущем будет формироваться копии экземпляра данного класса для параллельной обработки.
 */
class ConsoleFormatter
{
public:
    ConsoleFormatter(LogConsoleWidget* cli);// взять указатель на ConsoleSettings из LogConsoleWidget
    ConsoleFormatter(ConsoleSettings* settings);// скопировать экземпляр ConsoleSettings
    ~ConsoleFormatter();


    static QPair<QVector<LogLine>, QStringList> stringList2LogLines(const QStringList &block);
    QTextDocument* formatBlockToDoc(const QVector<LogLine> &block);

    inline void appendFormatedLine(QTextCursor* curs, const LogLine& line);
    inline void appendFormatedLine(QTextCursor* curs, QtMsgType type, const QDateTime& date,
                            const QString& func, const QString& msg);
    inline void appendSimpleLine(QTextCursor* curs, QtMsgType type, const QString& msg);

private:
    void setMsgColorFormat(QtMsgType type); // устанавливает цветовой формат основного сообщения по QtMsgType
    void setMsgColorFormat(const QString& type); // устанавливает цветовой формат основного сообщения по строке ex:"DEBUG"
    bool isFunctionChecked(const QString& func); //возвращает false если функция отключена в сортировке
    ConsoleSettings* m_settings = nullptr;
    bool m_releaseMem = false;
    QColor m_currMsgColor;
    QColor m_currMsgBgColor;
};

/*!
 * \brief The LogConsoleWidget class класс виджета консоли
 *
 */
class LogConsoleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LogConsoleWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~LogConsoleWidget();
    /*!
     * \brief saveSettings сохраняет настройки консоли в файл
     */
    bool saveSettings(QString path);
    /*!
     * \brief loadSettings загружает настройки
     */
    bool loadSettings(QString path);

    /*!
     * \brief loadLogsHistory загружает файл логов, парсит и отобраает.
     */
    bool loadLogsHistory(QString path);

    /*!
     * \brief getLogFilePath геттер для получения пути до файла в котором храняться логи.
     */
    QString getLogFilePath();
    /*!
     * \brief setLogFilePath устанавливает путь до файла с логами, но не загружает историю логов!
     *  loadLogsHistory(..) загружает историю и устанавливает setLogFilePath(..)
     *  Даная функция по факту является не нужной тк класс не пишет логи в файл сам..
     */
    void setLogFilePath(const QString& path);

    QString getConsoleSettingsPath(){
        return m_settingsFilePath;
    }

    /*!
     * \brief appendFormatedLine добавляет строку в виджет с заданным шаблоном.
     * Нельзя вызывать из других потоков!
     * шаблон: 2024-09-17 21:31:40.175 INFO functionName >> Debug info from worker thread hash
     */
    void appendFormatedLine(const QString& line);
    /*!
     * \brief appendFormatedLine добавляет строку в виджет.
     * Нельзя вызывать из других потоков!
     */
    void appendFormatedLine(QtMsgType type, QDateTime date, QString func, const QString msg);

    /*!
     * \brief appendDocumentFragment добавляет фрагмент документа в консоль с конца.
     * Нельзя вызывать из других потоков!
     */
    void appendDocumentFragment(const QTextDocumentFragment& msg);
    /*!
     * \brief insertFrontDocumentFragment добавляет фрагмент документа в консоль с начала.
     * Нельзя вызывать из других потоков!
     */
    void insertFrontDocumentFragment(const QTextDocumentFragment& msg);


    //void appendLine(const QString& line);
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    /*!
     * \brief sigAppendFormatedLine сигнал добавления новой строки в виджет
     */
    void sigAppendFormatedLine(QtMsgType type, QDateTime date, QString func, const QString msg);
    /*!
     * \brief sigAppendFormatedLine сигнал добавления новой строки с заданным шаблоном
     */
    void sigAppendFormatedLine(const QString line);

private:
    /*!
     * \brief separateIntoBlocks делит вектор из строк логов в вектор из блоков
     *  содержащих по несколько десятков строк логов
     */
    static QList<QVector<LogLine>> separateIntoBlocks(QVector<LogLine>& list);
    /*!
     * \brief updateContent перезагружает содеримое виджета
     * удаляет содержимое textEdit и загружает его заново из истории.
     * необходимо вызывать после обновления любых настроек по цветовой политре или сортировке
     */
    void updateContent();

    QMutex m_mutex;
    Ui::LogConsoleWidget *ui;
    FunctionSelectorWidget* m_FuncSelector = nullptr;
    QString m_logFilePath;
    QString m_settingsFilePath;

    ConsoleSettings m_settings;
    ConsoleFormatter* m_formatter;
    QVector<LogLine> m_history;

    QVector<QColor> m_customColors;

    bool m_dragging = false;
    QPoint m_dragPosition;


signals:
    void appendedNewLine(const Logging::LogLine line);


    friend class ConsoleHighlighter;
    friend class LogWidgetSettings;
    friend class ConsoleFormatter;
    friend class FunctionSelectorWidget;
};




};  // namespace Logging

#endif  // LOGCONSOLEWIDGET_H
