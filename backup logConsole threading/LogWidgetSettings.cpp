#include "LogWidgetSettings.h"
#include "LogConsoleWidget.h"
#include "qcolordialog.h"
#include "qdebug.h"
#include "ui_LogWidgetSettings.h"

using namespace Tolmi::Logging;

LogWidgetSettings::LogWidgetSettings(LogConsoleWidget *parent):
    ui(new Ui::LogWidgetSettings), QDialog(parent) , m_console(parent)
{
    ui->setupUi(this);

    setStyleSheet(parent->styleSheet());
    m_colorDialog = new QColorDialog();
    m_colorDialog->setStyleSheet(parent->styleSheet());
    m_colorDialog->setWindowFlags(Qt::FramelessWindowHint );  //| Qt::Popup Отключаем рамки и включаем режим Popup
    m_colorDialog->setStyleSheet(m_colorDialog->styleSheet() + "QColorDialog { background-color: rgb(48,48,48); }");

    loadSettings();

    // connect(ui->spinBox_fontSize, QOverload<int>::of(&QSpinBox::valueChanged), [&](int size){
    //     QFont f = ui->fontComboBox->font();
    //     f.setPointSize(size);
    //     ui->fontComboBox->setFont(f);
    // });

    connect(ui->pushButton_LogLevelColor, &QPushButton::clicked,
            [&]() { this->openColorDialog(ui->pushButton_LogLevelColor); });
    connect(ui->pushButton_dateColor, &QPushButton::clicked,
            [&]() { this->openColorDialog(ui->pushButton_dateColor); });
    connect(ui->pushButton_funcNameColor, &QPushButton::clicked,
            [&]() { this->openColorDialog(ui->pushButton_funcNameColor); });
    connect(ui->pushButton_timeColor, &QPushButton::clicked,
            [&]() { this->openColorDialog(ui->pushButton_timeColor); });
    connect(ui->pushButton_InfoMsgColor, &QPushButton::clicked,
            [&]() { this->openColorDialog(ui->pushButton_InfoMsgColor); });
    connect(ui->pushButton_DebugMsgColor, &QPushButton::clicked,
            [&]() { this->openColorDialog(ui->pushButton_DebugMsgColor); });
    connect(ui->pushButton_WarningMsgColor, &QPushButton::clicked,
            [&]() { this->openColorDialog(ui->pushButton_WarningMsgColor); });
    connect(ui->pushButton_CriticalMsgColor, &QPushButton::clicked,
            [&]() { this->openColorDialog(ui->pushButton_CriticalMsgColor); });
    connect(ui->pushButton_CriticalMsgBgColor, &QPushButton::clicked,
            [&]() { this->openColorDialog(ui->pushButton_CriticalMsgBgColor); });
    connect(ui->pushButton_FatalMsgColor, &QPushButton::clicked,
            [&]() { this->openColorDialog(ui->pushButton_FatalMsgColor); });
    connect(ui->pushButton_FatalMsgBgColor, &QPushButton::clicked,
            [&]() { this->openColorDialog(ui->pushButton_FatalMsgBgColor); });

    connect(ui->pushButton_openLogFile, &QPushButton::clicked,
            [&]() { m_console->loadLogsHistory(ui->lineEdit_logFilePath->text()); });

}

LogWidgetSettings::~LogWidgetSettings()
{
    delete ui;
}

void LogWidgetSettings::loadSettings()
{
    if(!m_console) return;
    QMutexLocker locker(&m_console->m_mutex);

    QFont font = m_console->m_settings.textFormat.font();
    ui->fontComboBox->setCurrentFont(font);
    ui->spinBox_fontSize->setValue(font.pointSize());

    ui->checkBox_enExtendedColors->setChecked(m_console->m_settings.extendedColors);

    ui->checkBox_dispDate->setChecked(m_console->m_settings.dispField.date);
    ui->checkBox_dispTime->setChecked(m_console->m_settings.dispField.time);
    on_checkBox_dispTime_stateChanged(m_console->m_settings.dispField.time);

    ui->checkBox_dispTimeMs->setChecked(m_console->m_settings.dispField.timeMs);
    ui->checkBox_displogLevel->setChecked(m_console->m_settings.dispField.logLevel);
    ui->checkBox_dispFuncName->setChecked(m_console->m_settings.dispField.messageSource);

    setButtonColor(ui->pushButton_LogLevelColor, m_console->m_settings.colors.logLevel);
    setButtonColor(ui->pushButton_dateColor, m_console->m_settings.colors.date);
    setButtonColor(ui->pushButton_funcNameColor, m_console->m_settings.colors.messageSource);
    setButtonColor(ui->pushButton_timeColor, m_console->m_settings.colors.time);

    setButtonColor(ui->pushButton_InfoMsgColor, m_console->m_settings.colors.infoMessage);
    setButtonColor(ui->pushButton_DebugMsgColor, m_console->m_settings.colors.debugMessage);
    setButtonColor(ui->pushButton_WarningMsgColor, m_console->m_settings.colors.warningMessage);

    setButtonColor(ui->pushButton_CriticalMsgColor, m_console->m_settings.colors.criticalMessage);
    setButtonColor(ui->pushButton_CriticalMsgBgColor, m_console->m_settings.colors.criticalMessageBg);
    setButtonColor(ui->pushButton_FatalMsgColor, m_console->m_settings.colors.fatalMessage);
    setButtonColor(ui->pushButton_FatalMsgBgColor, m_console->m_settings.colors.fatalMessageBg);

    for(int i = 0; i < m_console->m_customColors.length(); i++)
        m_colorDialog->setCustomColor(i, m_console->m_customColors.at(i));
}

void LogWidgetSettings::saveSettings()
{
    if(!m_console) return;
    QMutexLocker locker(&m_console->m_mutex);

    QFont font = ui->fontComboBox->currentFont();
    font.setPointSize(ui->spinBox_fontSize->value());
    m_console->m_settings.textFormat.setFont(font);

    m_console->m_settings.extendedColors = ui->checkBox_enExtendedColors->isChecked();

    m_console->m_settings.dispField.date = ui->checkBox_dispDate->isChecked();
    m_console->m_settings.dispField.time = ui->checkBox_dispTime->isChecked();
    m_console->m_settings.dispField.timeMs = ui->checkBox_dispTimeMs->isChecked();
    m_console->m_settings.dispField.logLevel = ui->checkBox_displogLevel->isChecked();
    m_console->m_settings.dispField.messageSource = ui->checkBox_dispFuncName->isChecked();

    m_console->m_settings.colors.logLevel = m_buttonColors[ui->pushButton_LogLevelColor];
    m_console->m_settings.colors.date = m_buttonColors[ui->pushButton_dateColor];
    m_console->m_settings.colors.messageSource = m_buttonColors[ui->pushButton_funcNameColor];
    m_console->m_settings.colors.time = m_buttonColors[ui->pushButton_timeColor];

    m_console->m_settings.colors.infoMessage = m_buttonColors[ui->pushButton_InfoMsgColor];
    m_console->m_settings.colors.debugMessage = m_buttonColors[ui->pushButton_DebugMsgColor];
    m_console->m_settings.colors.warningMessage = m_buttonColors[ui->pushButton_WarningMsgColor];

    m_console->m_settings.colors.criticalMessage = m_buttonColors[ui->pushButton_CriticalMsgColor];
    m_console->m_settings.colors.criticalMessageBg = m_buttonColors[ui->pushButton_CriticalMsgBgColor];
    m_console->m_settings.colors.fatalMessage = m_buttonColors[ui->pushButton_FatalMsgColor];
    m_console->m_settings.colors.fatalMessageBg = m_buttonColors[ui->pushButton_FatalMsgBgColor];

    m_console->m_customColors.clear();
    for(int i = 0; i < m_colorDialog->customCount(); i++)
        m_console->m_customColors.append(m_colorDialog->customColor(i));
}

void LogWidgetSettings::setButtonColor(QPushButton *b, const QColor &color)
{
    if(!b || !color.isValid()) return;
    QString str = "border: 1px solid gray;";
    str += "border-radius: 0px;";
    str += QString("background-color: %1;").arg(color.name());
    b->setStyleSheet(str);
    if(m_buttonColors.contains(b))
        m_buttonColors[b] = color;
    else
        m_buttonColors.insert(b, color);
}

void LogWidgetSettings::openColorDialog(QPushButton *button)
{
    if(m_colorDialog->exec() == QDialog::Accepted)
        setButtonColor(button, m_colorDialog->currentColor());
}

void LogWidgetSettings::on_pushButton_apply_clicked()
{
    saveSettings();
    this->accept();
}

void LogWidgetSettings::on_checkBox_dispTime_stateChanged(int arg1)
{
    ui->checkBox_dispTimeMs->setEnabled(arg1);
}

void LogWidgetSettings::on_pushButton_saveConsoleSettings_clicked()
{
    QString str = ui->lineEdit_ConsoleSettingsPath->text();
    saveSettings();
    m_console->saveSettings(str);
    this->accept();
}


void LogWidgetSettings::on_pushButton_loadConsoleSettings_clicked()
{
    QString str = ui->lineEdit_ConsoleSettingsPath->text();
    m_console->loadSettings(str);
}

