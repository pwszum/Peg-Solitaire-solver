#include "square.h"

#include <QPainter>

Square::Square(QWidget *parent): QWidget{parent}
{
    setCursor(Qt::PointingHandCursor);
    isPegPresent = true;
}

void Square::changePegState()
{
    if(isPegPresent) isPegPresent = false;
    else isPegPresent = true;
}

void Square::setPegState(const bool& state)
{
    isPegPresent = state;
}

bool Square::getPegState()
{
    return isPegPresent;
}

void Square::paintEvent(QPaintEvent*)
{
    int size;
    if(QWidget::height() > QWidget::width()) size = QWidget::height();
    else size = QWidget::width();

    QRectF rect;
    QPainter painter;
    painter.begin(this);

    rect = QRectF(0, 0, size, size);
    painter.fillRect(rect, QColor::fromRgb(0xf0b260));

    if(isPegPresent) {
        rect = QRectF(size/6.f, size/6.f, size*4/6.f, size*4/6.f);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor::fromRgb(0x232424));
        painter.drawEllipse(rect);
    }

    painter.end();
}

void Square::mousePressEvent(QMouseEvent*)
{
    changePegState();
    repaint();
}
