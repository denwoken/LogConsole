#ifndef LOGCONSOLEWIDGET_H
#define LOGCONSOLEWIDGET_H


#include "Logging.h"
#include "qdatetime.h"
#include "qmutex.h"
#include "qtextcursor.h"
#include <QWidget>
#include "qtextdocumentfragment.h"

/*! ConsoleFormatter example
 *
    using namespace Logging
    setLoggingFile("C:/Users/denwoken/Desktop/logFile.log");
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
    QString defSettings("ConsoleDefaultSettings.ini");
    if(QFileInfo::exists(defSettings))
        Console->loadSettings(defSettings);
    else
        Console->saveSettings(defSettings);

    Console->loadLogsHistory("C:/Users/denwoken/Desktop/logFile.log");
 */

QT_BEGIN_NAMESPACE
namespace Ui
{
class LogConsoleWidget;
}
QT_END_NAMESPACE

class QStandardItemModel;
class QTextDocument;
Q_DECLARE_METATYPE(QTextDocumentFragment);

namespace Logging
{

class LogConsoleWidget;
class ConsoleLogFormatter;
class FunctionSelectorWidget;

/*! \brief typedef struct ConsoleSettings
 *  тип данных для хранения настроек LogConsoleWidget
 */
typedef struct
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
    QTextCharFormat textFormat;
    //QDate filterStartDate;
    // QDate filterEndDate;
    QStandardItemModel* functions;
}ConsoleSettings;

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

class ConsoleFormatter: public QObject
{
    Q_OBJECT
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

public slots:
    void formatBlocksToDocs_thread(QList<QVector<LogLine>>* blocks);

signals:
    void sigBlockReady(QTextDocument* fr);
    void sigBlocksProcessingEnd();

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
    explicit LogConsoleWidget(QWidget *parent = nullptr);
    ~LogConsoleWidget();
    /*!
     * \brief saveSettings сохраняет настройки консоли в файл
     */
    bool saveSettings(QString path) const;
    /*!
     * \brief loadSettings загружает настройки
     */
    bool loadSettings(QString path);

    /*!
     * \brief loadLogsHistory загружает файл логов, парсит и отобраает.
     */
    bool loadLogsHistory(QString path);


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
    void insertFrontDocumentFragment(QTextDocumentFragment msg);
    /*!
     * \brief insertFrontDocumentFragment добавляет документ в консоль с начала.
     * Нельзя вызывать из других потоков!
     */
    void insertFrontDocument(QTextDocument* doc);

    //void appendLine(const QString& line);

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

    ConsoleSettings m_settings;
    ConsoleFormatter* m_formatter;
    QVector<LogLine> m_history;

    QVector<QColor> m_customColors;


    friend class ConsoleHighlighter;
    friend class LogWidgetSettings;
    friend class ConsoleFormatter;
    friend class FunctionSelectorWidget;
};




};  // namespace Logging

#endif  // LOGCONSOLEWIDGET_H
