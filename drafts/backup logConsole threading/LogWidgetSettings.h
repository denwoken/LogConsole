#ifndef LOGWIDGETSETTINGS_H
#define LOGWIDGETSETTINGS_H

#include "qdialog.h"
#include "qpushbutton.h"
#include <QWidget>
#include <QMap>

namespace Ui {
class LogWidgetSettings;
}

class QColorDialog;
namespace Logging
{


class LogConsoleWidget;
class LogWidgetSettings : public QDialog
{
    Q_OBJECT

public:
    explicit LogWidgetSettings(LogConsoleWidget *parent);
    ~LogWidgetSettings();

private:
    void on_pushButton_apply_clicked();

    void on_checkBox_dispTime_stateChanged(int arg1);

    void on_pushButton_saveConsoleSettings_clicked();

    void on_pushButton_loadConsoleSettings_clicked();

private:
    Ui::LogWidgetSettings *ui;
    LogConsoleWidget *m_console;

    void loadSettings();//загружает настройки из класса LogConsoleWidget
    void saveSettings();// сохраняет настройки в класс LogConsoleWidget
    /*!
     * \brief setButtonColor устанавливает цвет кнопки выбора цветовой палитры
     */
    void setButtonColor(QPushButton *b, const QColor& color);
    /*!
     * \brief openColorDialog открывает диалог выбора цвета при нажатии на соотв. кнопку
     */
    void openColorDialog(QPushButton *b);
    /*!
     * \brief m_buttonColors содержит ассоциацию кнопки и цвета
     */
    QMap<QPushButton*, QColor> m_buttonColors;

    QColorDialog *m_colorDialog;
};

};//namespace Logging
#endif // LOGWIDGETSETTINGS_H
