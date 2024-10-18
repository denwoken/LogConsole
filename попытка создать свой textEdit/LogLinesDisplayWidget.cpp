#include "LogLinesDisplayWidget.h"
#include "qapplication.h"
#include "qclipboard.h"
#include "qdebug.h"
#include "qelapsedtimer.h"
#include "qevent.h"
#include "qpainter.h"
#include "qscrollbar.h"
#include "qtimer.h"
#include <iostream>
#include <ostream>

using namespace Tolmi::Logging;
void LogLinesDisplayWidget::paintEvent(QPaintEvent *event)
{
    QElapsedTimer timer;
    timer.start();

    QPainter painter(viewport());
    painter.setFont(m_font);

    int startLine = verticalScrollBar()->value(); // С какой строки начинать отображение
    int endLine = qMin(startLine + m_visibleLines, m_text.size());

    qDebug() << "startLine " << startLine;
    qDebug() << "endLine " << endLine;


    int paceSize = painter.fontMetrics().width("_");

    int y = 0; // Начальная координата по Y
    for (int i = startLine; i < endLine; ++i) {
        LogLine& line = m_text[i];
        QString t = line.message;
        if (true) {//m_wordWrap
            t = painter.fontMetrics().elidedText(t, Qt::ElideRight, viewport()->width()/2);
        }
        //painter.fontMetrics().el

        // Рисуем выделение, если текст находится в области выделения
        if (i >= m_selectionStartLine && i <= m_selectionEndLine) {
            painter.setPen(Qt::black);
            painter.fillRect(0, y - fontMetrics().height(), width(), fontMetrics().height(), QColor(200, 200, 255, 150)); // Светло-синий фон для выделения
        }

        const QString date = line.dateTime.toString("yyyy-MM-dd ");
        const QString time = line.dateTime.toString("hh:mm:ss.zzz ");
        const QString type = msgTypeToString(line.type);

        painter.setPen(QColor(0,0,255));
        painter.drawText(0, y, date);
        int textWidth = painter.fontMetrics().width(date) + paceSize;

        painter.setPen(QColor(0, 125, 125));
        painter.drawText(textWidth, y, time);
        textWidth += painter.fontMetrics().width(time) + paceSize;

        painter.setPen(QColor(50, 255, 0));
        painter.drawText(textWidth, y, type);
        textWidth += painter.fontMetrics().width(type) + paceSize;

        painter.setPen(QColor(255, 0, 255));
        painter.drawText(textWidth, y, line.functionStr);
        textWidth += painter.fontMetrics().width(line.functionStr) + paceSize;

        painter.setPen(QColor(0, 0, 0));
        painter.drawText(textWidth, y, ">>");
        textWidth += painter.fontMetrics().width(">>") + paceSize;

        painter.setPen(QColor(0,0,0));
        painter.drawText(textWidth, y, line.message);

        y += m_lineHeight+1;
    }
    std::cout << "paint time " << timer.nsecsElapsed()/1000/1000 << " ms" << std::endl;
}


void LogLinesDisplayWidget::resizeEvent(QResizeEvent* event) {
    calculateVisibleLines();
    updateScrollBar();
}

void LogLinesDisplayWidget::calculateVisibleLines() {
    m_lineHeight = fontMetrics().height();
    m_visibleLines = viewport()->height() / m_lineHeight;
}

void LogLinesDisplayWidget::updateScrollBar() {
    verticalScrollBar()->setRange(0, m_text.size() - m_visibleLines);
    verticalScrollBar()->setPageStep(m_visibleLines);
}


void LogLinesDisplayWidget::mousePressEvent(QMouseEvent* event) {
    m_selectionStart = event->pos();
    m_selectionEnd = m_selectionStart;
    viewport()->update();
}

void LogLinesDisplayWidget::mouseMoveEvent(QMouseEvent* event) {
    m_selectionEnd = event->pos();
    viewport()->update();
}

// Пример выделения и копирования
void LogLinesDisplayWidget::keyPressEvent(QKeyEvent* event) {
    if (event->matches(QKeySequence::Copy)) {
        QString selectedText;
        // Логика для получения выделенного текста
        QApplication::clipboard()->setText(selectedText);
    }
}

LogLinesDisplayWidget::LogLinesDisplayWidget(QWidget *parent) :
    QAbstractScrollArea(parent), m_wordWrap(false) {
    // Установить начальный шрифт
    m_font.setPointSize(12);
    // Таймер для мерцания курсора
    m_cursorTimer = new QTimer(this);
    connect(m_cursorTimer, &QTimer::timeout, this, &LogLinesDisplayWidget::toggleCursorVisibility);
    m_cursorTimer->start(500); // Курсор будет обновляться каждые 500 мс
}

void LogLinesDisplayWidget::setLogLines(const QVector<LogLine> &text)
{
    m_text = text;
    viewport()->update();
    viewport()->repaint();
}

void LogLinesDisplayWidget::setTextWrapMode(bool wordWrap) {
    m_wordWrap = wordWrap;
    if (m_wordWrap) {
        horizontalScrollBar()->setRange(0, 0);
    } else {
        int maxWidth = 0;
        for (const auto& line : m_text) {
            maxWidth = qMax(maxWidth, fontMetrics().width(line.message));
        }
        horizontalScrollBar()->setRange(0, maxWidth - viewport()->width());
    }
    viewport()->update();
}
