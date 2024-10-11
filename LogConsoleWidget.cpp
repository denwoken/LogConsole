#include "LogConsoleWidget.h"
#include "FunctionSelectorWidget.h"
#include "LogWidgetSettings.h"
#include "Logging.h"
#include "qdatetime.h"
#include "qdebug.h"
#include "qfileinfo.h"
#include "qlocale.h"
#include "qscrollbar.h"
#include "qsettings.h"
#include "qstandarditemmodel.h"
#include "qtextdocumentfragment.h"
#include "ui_logconsolewidget.h"
#include <QWidget>
#include <QTextEdit>
#include <QtConcurrent>


using namespace Logging;
const int line_count = 50; // count in block (for history & processing)


ConsoleFormatter::ConsoleFormatter(LogConsoleWidget *cli)
{
    m_releaseMem=false;
    this->m_settings = &cli->m_settings;
}

ConsoleFormatter::ConsoleFormatter(ConsoleSettings *settings)
{
    m_releaseMem = true;
    m_settings = new ConsoleSettings();
    *m_settings = *settings;
    m_settings->functions = nullptr;
}

ConsoleFormatter::~ConsoleFormatter(){
    //освобождаем память если она была выделена
    if(m_releaseMem)
        delete m_settings;
}


QPair<QVector<LogLine>, QStringList> ConsoleFormatter::stringList2LogLines(const QStringList &block)
{
    QVector<LogLine> logLines;
    QStringList functions;

    for(const QString& line : block){
        if(line.isEmpty()) continue;
        LogLine logLine(line);
        functions.append(logLine.functionStr);
        logLines.append(logLine);
    }
    return {logLines, functions};
}

QTextDocument* ConsoleFormatter::formatBlockToDoc(const QVector<LogLine> &block)
{
    QTextDocument* doc = new QTextDocument();
    QTextCursor curs(doc);
    curs.movePosition(QTextCursor::End);
    for(const LogLine& line : block){
        appendFormatedLine(&curs, line);
    }
    return doc;
}

void ConsoleFormatter::appendFormatedLine(QTextCursor *curs, const LogLine &line)
{
    if(line.only_message)
        appendSimpleLine(curs, line.type, line.message);
    else
        appendFormatedLine(curs, line.type, line.dateTime, line.functionStr, line.message);
}

void ConsoleFormatter::appendFormatedLine(QTextCursor* curs, QtMsgType type, const QDateTime& date,
                                          const QString& func, const QString& msg)
{
    // исключаем строки если имеют уровень логирования который отключен в настройках
    switch(type){
    case QtInfoMsg:
        if(!m_settings->enableLogMsgs.infoMsg)return;
        break;
    case QtWarningMsg:
        if(!m_settings->enableLogMsgs.warningMsg)return;
        break;
    case QtDebugMsg:
        if(!m_settings->enableLogMsgs.debugMsg)return;
        break;
    case QtCriticalMsg:
        if(!m_settings->enableLogMsgs.criticalMsg)return;
        break;
    case QtFatalMsg:
        if(!m_settings->enableLogMsgs.fatalMsg)return;
        break;
    }

    //если включено отображение данной функции
    if(!isFunctionChecked(func)) return;

    //формируем строки для даты времени и тп
    QString timeStr;
    if(m_settings->dispField.time){
        if(m_settings->dispField.timeMs)
            timeStr = date.toString("hh:mm:ss.zzz ");
        else
            timeStr = date.toString("hh:mm:ss ");
    }
    QString dateStr = date.toString("yyyy-MM-dd ");
    QString typeStr = msgTypeToString(type) + " ";
    setMsgColorFormat(type);

    // добавляем текст по курсору
    curs->beginEditBlock();
    curs->insertBlock(); // c этой строкой работает значительно быстрее
    m_settings->textFormat.setBackground(QColor(0,0,0,0));
    if(m_settings->extendedColors){
        //по очереди печатаем через курсор дату время и тп меняя цвет (медленно)
        if(m_settings->dispField.date)
        {
            m_settings->textFormat.setForeground(m_settings->colors.date);
            curs->setCharFormat(m_settings->textFormat);
            curs->insertText(dateStr);
        }
        if(m_settings->dispField.time)
        {
            m_settings->textFormat.setForeground(m_settings->colors.time);
            curs->setCharFormat(m_settings->textFormat);
            curs->insertText(timeStr);
        }
        if(m_settings->dispField.logLevel)
        {
            m_settings->textFormat.setForeground(m_settings->colors.logLevel);
            curs->setCharFormat(m_settings->textFormat);
            curs->insertText(typeStr);
        }
        if(m_settings->dispField.messageSource)
        {
            m_settings->textFormat.setForeground(m_settings->colors.messageSource);
            curs->setCharFormat(m_settings->textFormat);
            curs->insertText(func + " ");
        }
        m_settings->textFormat.setForeground(m_currMsgColor);

        if(type == QtCriticalMsg || type == QtFatalMsg)
            m_settings->textFormat.setBackground(m_currMsgBgColor);
        curs->setCharFormat(m_settings->textFormat);
        curs->insertText(">> " + msg);
    }
    else
    {
        // Склеиваем строку и печатаем ее разом через курсор (быстрее)
        QString line;
        if(m_settings->dispField.date)
            line += dateStr;
        if(m_settings->dispField.time)
            line += timeStr;
        if(m_settings->dispField.logLevel)
            line += typeStr;
        if(m_settings->dispField.messageSource)
            line += func + " ";
        line += ">> " + msg;
        m_settings->textFormat.setForeground(m_currMsgColor);
        if(type == QtCriticalMsg || type == QtFatalMsg)
            m_settings->textFormat.setBackground(m_currMsgBgColor);
        curs->setCharFormat(m_settings->textFormat);
        curs->insertText(line);
    }
    curs->endEditBlock();
}

void ConsoleFormatter::appendSimpleLine(QTextCursor *curs, QtMsgType type, const QString &msg)
{
    // исключаем строки если имеют уровень логирования который отключен в настройках
    switch(type){
    case QtInfoMsg:
        if(!m_settings->enableLogMsgs.infoMsg)return;
        break;
    case QtWarningMsg:
        if(!m_settings->enableLogMsgs.warningMsg)return;
        break;
    case QtDebugMsg:
        if(!m_settings->enableLogMsgs.debugMsg)return;
        break;
    case QtCriticalMsg:
        if(!m_settings->enableLogMsgs.criticalMsg)return;
        break;
    case QtFatalMsg:
        if(!m_settings->enableLogMsgs.fatalMsg)return;
        break;
    }

    //если включено отображение данной функции
    if(!isFunctionChecked("")) return;

    setMsgColorFormat(type);

    // добавляем текст по курсору
    curs->beginEditBlock();
    curs->insertBlock(); // c этой строкой работает значительно быстрее
    m_settings->textFormat.setBackground(QColor(0,0,0,0));
    m_settings->textFormat.setForeground(m_currMsgColor);
    if(type == QtCriticalMsg || type == QtFatalMsg)
        m_settings->textFormat.setBackground(m_currMsgBgColor);
    curs->setCharFormat(m_settings->textFormat);
    curs->insertText(msg);
    curs->endEditBlock();
}

void ConsoleFormatter::setMsgColorFormat(QtMsgType type)
{
    switch(type)
    {
    case QtDebugMsg:
        m_currMsgColor = m_settings->colors.debugMessage;
        break;
    case QtWarningMsg:
        m_currMsgColor = m_settings->colors.warningMessage;
        break;
    case QtCriticalMsg:
        m_currMsgColor = m_settings->colors.criticalMessage;
        m_currMsgBgColor = m_settings->colors.criticalMessageBg;
        break;
    case QtFatalMsg:
        m_currMsgColor = m_settings->colors.fatalMessage;
        m_currMsgBgColor = m_settings->colors.fatalMessageBg;
        break;
    case QtInfoMsg:
        m_currMsgColor = m_settings->colors.infoMessage;
        break;
    }
}

void ConsoleFormatter::setMsgColorFormat(const QString &typeStr)
{
    int cursor = 0;
    while(cursor<1){
        char first_char = typeStr.at(cursor).toLatin1();

        if(first_char == 'I')
        {
            m_currMsgColor = m_settings->colors.infoMessage;
            return;
        } else if(first_char == 'D')
        {
            m_currMsgColor = m_settings->colors.debugMessage;
            return;
        } else if(first_char == 'W')
        {
            m_currMsgColor = m_settings->colors.warningMessage;
            return;
        } else if(first_char == 'C')
        {
            m_currMsgColor = m_settings->colors.criticalMessage;
            m_currMsgBgColor = m_settings->colors.criticalMessageBg;
            return;
        } else if(first_char == 'F')
        {
            m_currMsgColor = m_settings->colors.fatalMessage;
            m_currMsgBgColor = m_settings->colors.fatalMessageBg;
            return;
        }
        else  cursor++;
    }

    // если запись выглядела по другому, на будущее
    if(typeStr.contains("INFO", Qt::CaseInsensitive)){
        m_currMsgColor = m_settings->colors.infoMessage;
    } else if(typeStr.contains("DEBUG", Qt::CaseInsensitive))
    {
        m_currMsgColor = m_settings->colors.debugMessage;
    } else if(typeStr.contains("WARNING", Qt::CaseInsensitive))
    {
        m_currMsgColor = m_settings->colors.warningMessage;
    } else if(typeStr.contains("CRITICAL", Qt::CaseInsensitive))
    {
        m_currMsgColor = m_settings->colors.criticalMessage;
        m_currMsgBgColor = m_settings->colors.criticalMessageBg;
    } else if(typeStr.contains("FATAL", Qt::CaseInsensitive))
    {
        m_currMsgColor = m_settings->colors.fatalMessage;
        m_currMsgBgColor = m_settings->colors.fatalMessageBg;
    }
}

bool ConsoleFormatter::isFunctionChecked(const QString &func)
{
    //модель содержит список в виде древа включенных и выключнных функций
    QStandardItemModel* model = m_settings->functions;
    if(!model) return true;
    const QStandardItem* item = model->invisibleRootItem();
    const QStringList list = func.split("::");

    for(int i = 0; i<list.size(); i++){
        QStandardItem* child;
        int itemCount = item->rowCount()-1;
        for(int j = 0; j<=itemCount; j++){
            child = item->child(j);
            if(!child)continue;
            if(list[i] == child->text()){
                int state = child->data(Qt::UserRole).toInt();
                if(state == Qt::CheckState::Checked)
                    return true;
                if(state == Qt::CheckState::Unchecked)
                    return false;
                if(state == Qt::CheckState::PartiallyChecked){
                    item = child;
                     break;
                }
            }
            if(j==itemCount ){
                // если функция не найдена
                return true; // считаем что она отмечена и отображение включено
            }
        }
    }
}







LogConsoleWidget::LogConsoleWidget(QWidget *parent) : QWidget(parent), ui(new Ui::LogConsoleWidget)
{
    qRegisterMetaType<QTextCursor>("QTextCursor");
    qRegisterMetaType<QTextFormat>("QTextCharFormat");
    qRegisterMetaType<QtMsgType>("QtMsgType");
    ui->setupUi(this);
    resize(800, 400);
    setWindowTitle("Console");

    m_settings.dispField.date = false;
    m_settings.dispField.logLevel = false;
    m_settings.dispField.time = true;
    m_settings.dispField.timeMs = true;
    m_settings.dispField.messageSource = true;
    m_settings.enableLogMsgs.criticalMsg = true;
    m_settings.enableLogMsgs.debugMsg = true;
    m_settings.enableLogMsgs.fatalMsg = true;
    m_settings.enableLogMsgs.infoMsg = true;
    m_settings.enableLogMsgs.warningMsg = true;
    m_settings.functions = nullptr;
    m_settings.extendedColors = true;
    m_settings.textFormat.setFontFamily("Arial Black");
    m_settings.textFormat.setFontPointSize(11);

    m_settings.colors.date = QColor(0, 0, 0xff);                 //"#0000ff"
    m_settings.colors.time = QColor(0xaa, 0, 0xff);              //"#aa00ff"
    m_settings.colors.logLevel = QColor(0x55, 0xff, 0);          //"#55ff00"
    m_settings.colors.messageSource = QColor(0x55, 0xaa, 0xff);  //"#55aaff"

    m_settings.colors.infoMessage = QColor(0, 0, 0xff);        //"#0000ff"
    m_settings.colors.warningMessage = QColor(0xff, 0xaa, 0);  //"#ffaa00"
    m_settings.colors.debugMessage = QColor(0, 0, 0);          //"#000000"
    m_settings.colors.fatalMessage = QColor(0xff, 0, 0);       //"#ff0000"
    m_settings.colors.criticalMessage = QColor(0xff, 0, 0);    //"#ff0000"

    m_settings.colors.fatalMessageBg = QColor(0, 0, 0);              //"#000000"
    m_settings.colors.criticalMessageBg = QColor(0x86, 0x86, 0x86);  //"#868686"



    m_formatter = new ConsoleFormatter(this);

    //подключаем кнопку к включению и отключению LineWrapMode
    connect(ui->checkBox_lineWrap, &QCheckBox::stateChanged, [=](bool wrap) {
        if(wrap)
        {
            //ui->textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            ui->textEdit->setLineWrapMode(QTextEdit::LineWrapMode::WidgetWidth);
        }
        else
        {
            //ui->textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            ui->textEdit->setLineWrapMode(QTextEdit::LineWrapMode::NoWrap);
        }
    });

    //подключаем кнопку очистки консоли к очистке истории и textEdit
    connect(ui->pushButton_clear, &QPushButton::clicked, [&](){
        QMutexLocker locker(&m_mutex);
        ui->textEdit->clear();
        m_history.clear();
    });



    // Подключаем кнопку к окрытию диалога настроек
    connect(ui->settingsButton, &QPushButton::clicked, [&]() {
        LogWidgetSettings *optWidget = new LogWidgetSettings(this);
        QPoint pos = ui->settingsButton->mapToGlobal(QPoint(-this->width(), ui->settingsButton->height()));
        optWidget->move(pos);
        if(optWidget->exec() == QDialog::Accepted) {
            updateContent();
            ui->pushButton_GoDown->click();
        }
        optWidget->deleteLater();
    });

    // настройка testEdit
    ui->textEdit->setReadOnly(true);
    ui->textEdit->document()->setUndoRedoEnabled(false);
    // ui->textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->textEdit->setLineWrapMode(QTextEdit::LineWrapMode::NoWrap);
    ui->textEdit->setWordWrapMode(QTextOption::WordWrap);
    ui->textEdit->setAlignment(Qt::AlignLeft);

    //подключаем сигналы добавления строки
    connect(this, QOverload<QtMsgType, QDateTime, QString, const QString>::of(&LogConsoleWidget::sigAppendFormatedLine),
            this, QOverload<QtMsgType, QDateTime, QString, const QString>::of(&LogConsoleWidget::appendFormatedLine),
            Qt::QueuedConnection);
    connect(this, QOverload<const QString>::of(&LogConsoleWidget::sigAppendFormatedLine),
            this, QOverload<const QString&>::of(&LogConsoleWidget::appendFormatedLine),
            Qt::QueuedConnection);

    //создаю виджет для изменяющий правила сортировки
    m_FuncSelector = new FunctionSelectorWidget(this);
    m_FuncSelector->resize(300, 350);

    // Подключаем кнопку к открытию меню правил сортировки (FunctionSelectorWidget)
    connect(ui->FilterButton, &QPushButton::clicked, [=]() {
        QPoint pos = QPoint(1+ui->FilterButton->width() - m_FuncSelector->width(), ui->FilterButton->height());
        pos = ui->FilterButton->mapToGlobal(pos);
        m_FuncSelector->move(pos);
        m_FuncSelector->sortItems();
        //m_FuncSelector->show();
        m_FuncSelector->exec();
        updateContent();
        ui->pushButton_GoDown->click();
    });
    //подключаем кнопку прокрутки консоли в самый низ
    connect(ui->pushButton_GoDown, &QPushButton::clicked, [=]() {
        QScrollBar* scroll = ui->textEdit->verticalScrollBar();
        scroll->setValue(scroll->maximum());
    });

    QIcon icon1(":/resources/trash_bin_icon.png");
    ui->pushButton_clear->setIcon(icon1);
    QIcon icon2(":/resources/filters_icon.png");
    ui->FilterButton->setIcon(icon2);
    QIcon icon3(":/resources/settings_icon.png");
    ui->settingsButton->setIcon(icon3);
}

LogConsoleWidget::~LogConsoleWidget()
{
    delete ui;
    delete m_formatter;
    if(m_settings.functions)
        delete m_settings.functions;
}

bool LogConsoleWidget::saveSettings(QString path)
{
    if(!path.endsWith(".ini", Qt::CaseInsensitive)) return true;
    m_settingsFilePath = path;
    QSettings saving(path, QSettings::IniFormat);
    // saving.beginGroup("LogConsoleSettings");
    // saving.endGroup();

    saving.beginGroup("EnableLogMsgs");
    saving.setValue("warningMsg", QVariant((bool)m_settings.enableLogMsgs.warningMsg));
    saving.setValue("debugMsg", QVariant((bool)m_settings.enableLogMsgs.debugMsg));
    saving.setValue("infoMsg", QVariant((bool)m_settings.enableLogMsgs.infoMsg));
    saving.setValue("criticalMsg", QVariant((bool)m_settings.enableLogMsgs.criticalMsg));
    saving.setValue("fatalMsg", QVariant((bool)m_settings.enableLogMsgs.fatalMsg));
    saving.endGroup();

    saving.beginGroup("DisplayField");
    saving.setValue("date", QVariant((bool)m_settings.dispField.date));
    saving.setValue("time", QVariant((bool)m_settings.dispField.time));
    saving.setValue("timeMilliseconds", QVariant((bool)m_settings.dispField.timeMs));
    saving.setValue("logLevel", QVariant((bool)m_settings.dispField.logLevel));
    saving.setValue("messageSource", QVariant((bool)m_settings.dispField.messageSource));
    saving.endGroup();

    saving.beginGroup("ConsoleColors");
    saving.setValue("dateColor", m_settings.colors.date.name());
    saving.setValue("timeColor", m_settings.colors.time.name());
    saving.setValue("logLevelColor", m_settings.colors.logLevel.name());
    saving.setValue("messageSourceColor", m_settings.colors.messageSource.name());

    saving.setValue("infoMessage", m_settings.colors.infoMessage.name());
    saving.setValue("warningMessage", m_settings.colors.warningMessage.name());
    saving.setValue("debugMessage", m_settings.colors.debugMessage.name());
    saving.setValue("fatalMessage", m_settings.colors.fatalMessage.name());
    saving.setValue("criticalMessage", m_settings.colors.criticalMessage.name());

    saving.setValue("fatalMessageBg", m_settings.colors.fatalMessageBg.name());
    saving.setValue("criticalMessageBg", m_settings.colors.criticalMessageBg.name());

    saving.setValue("customColorsNum", m_customColors.length());
    for(int i = 0; i < m_customColors.length(); i++)
        saving.setValue(QString("customColor_%1").arg(i), m_customColors.at(i).name());
    saving.endGroup();

    saving.setValue("extendedColors", QVariant((bool)m_settings.extendedColors));
    saving.setValue("FontName", m_settings.textFormat.fontFamily());
    saving.setValue("FontSize", m_settings.textFormat.fontPointSize());


    saving.beginGroup("FunctionFilter");
    QVector<QPair<QString, bool>> vector = m_FuncSelector->convertToVector();
    for(auto pair : vector)
        saving.setValue(pair.first, QVariant((bool)pair.second));
    saving.endGroup();

    saving.sync();

    return false;
}

bool LogConsoleWidget::loadSettings(QString path)
{
    QMutexLocker locker(&m_mutex);
    if(!path.endsWith(".ini", Qt::CaseInsensitive)) return true;
    m_settingsFilePath = path;
    QSettings saving(path, QSettings::IniFormat);

    //if(!saving.childGroups().contains("LogConsoleSettings")) return true;


    saving.beginGroup("EnableLogMsgs");
    m_settings.enableLogMsgs.warningMsg = saving.value("warningMsg").toBool();
    m_settings.enableLogMsgs.debugMsg = saving.value("debugMsg").toBool();
    m_settings.enableLogMsgs.infoMsg = saving.value("infoMsg").toBool();
    m_settings.enableLogMsgs.criticalMsg = saving.value("criticalMsg").toBool();
    m_settings.enableLogMsgs.fatalMsg = saving.value("fatalMsg").toBool();
    saving.endGroup();

    saving.beginGroup("DisplayField");
    m_settings.dispField.date = saving.value("date").toBool();
    m_settings.dispField.time = saving.value("time").toBool();
    m_settings.dispField.timeMs = saving.value("timeMilliseconds").toBool();
    m_settings.dispField.logLevel = saving.value("logLevel").toBool();
    m_settings.dispField.messageSource = saving.value("messageSource").toBool();
    saving.endGroup();

    saving.beginGroup("ConsoleColors");
    m_settings.colors.date.setNamedColor(saving.value("dateColor").toString());
    m_settings.colors.time.setNamedColor(saving.value("timeColor").toString());
    m_settings.colors.logLevel.setNamedColor(saving.value("logLevelColor").toString());
    m_settings.colors.messageSource.setNamedColor(saving.value("messageSourceColor").toString());

    m_settings.colors.infoMessage.setNamedColor(saving.value("infoMessage").toString());
    m_settings.colors.warningMessage.setNamedColor(saving.value("warningMessage").toString());
    m_settings.colors.debugMessage.setNamedColor(saving.value("debugMessage").toString());
    m_settings.colors.fatalMessage.setNamedColor(saving.value("fatalMessage").toString());
    m_settings.colors.criticalMessage.setNamedColor(saving.value("criticalMessage").toString());

    m_settings.colors.fatalMessageBg.setNamedColor(saving.value("fatalMessageBg").toString());
    m_settings.colors.criticalMessageBg.setNamedColor(saving.value("criticalMessageBg").toString());

    int num = saving.value(QString("customColorsNum")).toInt();
    m_customColors.clear();
    for(int i = 0; i < num; i++)
    {
        QColor c;
        c.setNamedColor(saving.value(QString("customColor_%1").arg(i)).toString());
        m_customColors.append(c);
    }
    saving.endGroup();

    m_settings.extendedColors = saving.value("extendedColors").toBool();
    m_settings.textFormat.setFontFamily(saving.value("FontName").toString());
    m_settings.textFormat.setFontPointSize(saving.value("FontSize").toInt());


    saving.beginGroup("FunctionFilter");
    QStringList list = saving.childKeys();
    for(auto keyName : list){
        if(keyName.isEmpty())continue;
        bool st = saving.value(keyName).toBool();
        m_FuncSelector->addFunction(keyName);
        m_FuncSelector->setCheckStateFunction(keyName, st);
    }
    saving.endGroup();

    m_FuncSelector->updateEnablesLogMsgs();

    return false;
}

bool LogConsoleWidget::loadLogsHistory(QString path)
{
    if(QFileInfo::exists(path) && (path.endsWith(".txt") || path.endsWith(".log")))
    {
        m_logFilePath = path;
        QFile f(path);
        if(!f.open(QFile::ReadOnly | QFile::Text)) return true;

        QTextStream in(&f);

        //QElapsedTimer timer;
        //timer.start();
        //формируем блоки строк
        QList<QStringList> stringListBlocks;
        bool at_end = false;
        while(!at_end)
        {
            QStringList block;
            QString line;
            for(int i = 0; i<line_count; i++){
                if(in.atEnd()){
                    at_end = true;
                    break;
                }
                line = in.readLine();
                block.append(line);
            }
            if(block.size())
                stringListBlocks.append(std::move(block));
        }
        //float t0 = timer.nsecsElapsed()/1000;
        //std::cout << "create  QVector<QStringList> " << t0/1000 << " ms;"<< std::endl;
        //float t1, t2;

        QList<QVector<LogLine>> Blocks;
        bool useConcurrent = (stringListBlocks.size()*line_count) >= 1500 ; // если больше 1500 строк используем многопоток (небольшой прирост производительности)
        if(useConcurrent){
            //timer.restart();
            QThreadPool *pool = QThreadPool::globalInstance();
            int previousThreadCount = pool->maxThreadCount();
            pool->setMaxThreadCount(3);
            QFuture<QPair<QVector<LogLine>, QStringList>> future;
            future = QtConcurrent::mapped(stringListBlocks, ConsoleFormatter::stringList2LogLines);
            // Ждем завершения выполнения задачи
            future.waitForFinished();
            pool->setMaxThreadCount(previousThreadCount);
            //t1 = timer.nsecsElapsed()/1000;
            //std::cout << "translate stringList to LogLines " << t1/1000 << " ms;"<< std::endl;

            //timer.restart();
            //добавляем функции в m_FuncSelector
            for(int i=0; i<future.resultCount(); i++){
                const QPair<QVector<LogLine>, QStringList>& result = future.resultAt(i);
                for(auto func : result.second)
                    m_FuncSelector->addFunction(std::move(func));
                Blocks.append(std::move(result.first));
            }
            //t2 = timer.nsecsElapsed()/1000;
            //std::cout << "add Functions to selector " << t2/1000 << " ms;" << std::endl;
        }
        else
        {
            //timer.restart();
            QStringList Functions;
            for(auto stringBlock : stringListBlocks){
                auto pair = m_formatter->stringList2LogLines(stringBlock);
                Blocks.append(std::move(pair.first));
                Functions.append(std::move(pair.second));
            }
            //t1 = timer.nsecsElapsed()/1000;
            //std::cout << "translate stringList to LogLines " << t1/1000 << " ms;"<< std::endl;

            //timer.restart();
            for(auto func : Functions)
                m_FuncSelector->addFunction(func);
            //t2 = timer.nsecsElapsed()/1000;
            //std::cout << "add Functions to selector " << t2/1000 << " ms;"<< std::endl;
        }

        //добавляем строки в историю
        //timer.restart();
        for(auto block : Blocks)
            for(auto line : block)
                m_history.append(line);
        //float t3 = timer.nsecsElapsed()/1000;
        //std::cout << "add LogLines to history " << t3/1000 << " ms;"<< std::endl;

        QList<QTextDocumentFragment> docFragments;
        //timer.restart();
        //преобразование блоков текста в документ
        for(auto block : Blocks){
            QTextDocument* doc = m_formatter->formatBlockToDoc(block);
            QTextDocumentFragment fr(doc);
            docFragments.append(std::move(fr));
            delete doc;
        }

        //float t4 = timer.nsecsElapsed()/1000;
        //std::cout << "translate LogLinesBlocks to docFrag " << t4/1000 << " ms;"<< std::endl;


        //timer.restart();
        //добавление документов
        for(auto fragment : docFragments){
            appendDocumentFragment(fragment);
        }
        //float t5 = timer.nsecsElapsed()/1000;
        //std::cout << "add docFragments to textEdit " << t5/1000 << " ms;"<< std::endl;
        //std::cout << "total load Time " << (t0+t1+t2+t3+t4+t5)/1000 << " ms;"<< std::endl;

        f.close();

        int val = ui->textEdit->verticalScrollBar()->maximum();
        ui->textEdit->verticalScrollBar()->setValue(val);
        return false;
    }

    return true;
}


void LogConsoleWidget::appendFormatedLine(const QString &line)
{
    QMutexLocker locker(&m_mutex);
    // Сохраняем текущую позицию скролла
    QScrollBar* scroll = ui->textEdit->verticalScrollBar();
    int scrollValue = scroll->value();
    bool atBottom = (scroll->maximum() - scrollValue <= 50);


    ui->textEdit->setUpdatesEnabled(false);

    QTextDocument* doc = ui->textEdit->document();
    QTextCursor curs(doc);
    curs.movePosition(QTextCursor::End);

    LogLine logLine(line);
    m_history.append(logLine);
    m_FuncSelector->addFunction(logLine.functionStr);
    m_formatter->appendFormatedLine(&curs, line);

    ui->textEdit->setUpdatesEnabled(true);

    // Переносим позицию скролла если окно отображения в низу консоли (эффект прилипания)
    if(!atBottom)
        scroll->setValue(scrollValue);
    else
        scroll->setValue(scroll->maximum());
}

void LogConsoleWidget::appendFormatedLine(QtMsgType type, QDateTime date, QString func, const QString msg)
{
    QMutexLocker locker(&m_mutex);
    // Сохраняем текущую позицию скролла
    QScrollBar* scroll = ui->textEdit->verticalScrollBar();
    int scrollValue = scroll->value();
    bool atBottom = (scroll->maximum() - scrollValue <= 50);


    ui->textEdit->setUpdatesEnabled(false);

    QTextDocument* doc = ui->textEdit->document();
    QTextCursor curs(doc);
    curs.movePosition(QTextCursor::End);

    LogLine logLine(type, date, func, msg);
    m_history.append(logLine);
    m_FuncSelector->addFunction(func);
    m_formatter->appendFormatedLine(&curs, type, date, func, msg);

    ui->textEdit->setUpdatesEnabled(true);

    // Переносим позицию скролла если окно отображения в низу консоли (эффект прилипания)
    if(!atBottom)
        scroll->setValue(scrollValue);
    else
        scroll->setValue(scroll->maximum());
}

void LogConsoleWidget::appendDocumentFragment(const QTextDocumentFragment &fragment)
{
    QMutexLocker locker(&m_mutex);
    // Получаем QTextCursor из существующего QTextEdit
    QTextCursor cursor = ui->textEdit->textCursor();

    // Перемещаем курсор в конец текста, чтобы вставить фрагмент туда
    cursor.movePosition(QTextCursor::End);
    cursor.beginEditBlock();
    // Вставляем сформированный фрагмент в текущее положение курсора
    cursor.insertFragment(fragment);
    cursor.endEditBlock();
}

void LogConsoleWidget::insertFrontDocumentFragment(const QTextDocumentFragment &fragment)
{
    QMutexLocker locker(&m_mutex);
    QTextCursor cursor = ui->textEdit->textCursor();

    // Перемещаем курсор в начало текста, чтобы вставить фрагмент туда
    cursor.movePosition(QTextCursor::Start);
    cursor.beginEditBlock();
    // Вставляем сформированный фрагмент в текущее положение курсора
    cursor.insertFragment(fragment);
    cursor.endEditBlock();
}

QList<QVector<LogLine>> LogConsoleWidget::separateIntoBlocks(QVector<LogLine> &list)
{
    QList<QVector<LogLine>> vector;
    int linesNum = list.size();
    int blocksNum = (linesNum + line_count - 1) / line_count;
    vector.reserve(blocksNum);
    for(int b = 0; b < blocksNum; b++)
    {
        QVector<LogLine> block;
        int size = std::min(linesNum, line_count);
        block.reserve(size);
        for(int i = 0; i < size; i++)
            block.append(list[b * line_count + i]);

        linesNum -= size;
        vector.append(std::move(block));
    }
    return vector;
}

void LogConsoleWidget::updateContent()
{
    //QElapsedTimer timer;
    //timer.start();

    m_mutex.lock();
    ui->textEdit->clear();
    QList<QVector<LogLine>> blocks = separateIntoBlocks(m_history);
    QStandardItemModel* mod = m_FuncSelector->convertToStandardItemModel();
    m_mutex.unlock();

    if(m_settings.functions)
        delete m_settings.functions;
    m_settings.functions = mod;

    for(auto block : blocks){
        QTextDocument* doc = m_formatter->formatBlockToDoc(block);
        QTextDocumentFragment fr(doc);
        //docFragments.append(std::move(fr));
        appendDocumentFragment(std::move(fr));
        delete doc;
    }
    //std::cout << "time " << timer.nsecsElapsed()/1000/1000 << " ms;" << std::endl;
}



LogLine::LogLine(const QString &line){
    if(line.isEmpty()) return;
    int msgSepIndex = line.indexOf(" >> ");  // промт разделяющий сообщение и информацию
    if(msgSepIndex == -1){
        message = line;
        only_message = true;
        return;
    }
    message = line.mid(msgSepIndex + 4);

    QStringList list = line.left(msgSepIndex).split(" ",  QString::SkipEmptyParts);
    if(list.size() >= 3){
        // при имспользовании некоторых функций QDate QTime QDateTime включая парсер,
        // регистрируется календарь QCalendar причем это происходит непотокобезопасно
        // поэтому создаю его статическим
        static QCalendar calendar;
        //парсим дату
        QStringList dateList = list[0].split("-");
        if(dateList.size() >= 3){
            int yy = dateList[0].toLong();
            int mm = dateList[1].toLong();
            int dd = dateList[2].toLong();
            QDate date(yy, mm, dd, calendar);
            dateTime.setDate(date);
        }

        //парсим время
        QStringList timeList = list[1].split(":");
        if(timeList.size() >=3){
            int hh = timeList[0].toLong();
            int MM = timeList[1].toLong();
            QStringList timeMsS = timeList[2].split(".");
            int ss = timeMsS[0].toLong();
            if(timeList.size() >=2){
                int zzz= timeMsS[1].toLong();
                QTime time(hh, MM, ss, zzz);
                dateTime.setTime(time);
            }
            else
            {
                QTime time(hh, MM, ss);
                dateTime.setTime(time);
            }
        }

        //dateTime.setDate(QDate::fromString(list[0], "yyyy-MM-dd"));
        //dateTime.setTime(QTime::fromString(list[1], "hh:mm:ss.zzz"));

        type = StringToMsgType(list[2]);
        if(list.size() >= 4)
            functionStr = list[3];
    }
    else
    {
        message = line;
        only_message = true;
        return;
    }
}
