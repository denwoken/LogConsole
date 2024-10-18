#ifndef LOGLINESDISPLAYWIDGET_H
#define LOGLINESDISPLAYWIDGET_H

#include "qabstractscrollarea.h"
#include <QWidget>
#include "LogConsoleWidget.h"

namespace Tolmi::Logging
{
class LogLinesDisplayWidget : public QAbstractScrollArea
{
    Q_OBJECT
public:

    explicit LogLinesDisplayWidget(QWidget *parent = nullptr);


    // Методы для установки текста и стилей
    void setLogLines(const QVector<LogLine>& text);
    void setFont(const QFont& font);
    void setTextWrapMode(bool wordWrap); // true для wordWrap, false для NoWrap

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    QVector<LogLine> m_text;   // Текст, хранящийся построчно
    QFont m_font;         // Шрифт текста
    bool m_wordWrap;      // Режим переноса текста
    int m_lineHeight;     // Высота строки для текущего шрифта
    int m_visibleLines;   // Количество видимых строк на экране
    QPoint m_selectionStart, m_selectionEnd; // Для выделения текста
    QTimer* m_cursorTimer;
    bool m_cursorVisible;
    QPoint m_cursorPos;

    // Методы для работы с прокруткой и текстом
    void updateScrollBar();
    void calculateVisibleLines();
};
}; //Tolmi::Logging
#endif // LOGLINESDISPLAYWIDGET_H
