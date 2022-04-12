/**
 ** This file is part of the CatRadio project.
 ** Copyright 2022 Gianfranco Sordetti IZ8EWD <iz8ewd@pianetaradio.it>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/


#include "vfodisplay.h"

#include <QPainter>
#include <QInputEvent>
#include <QDebug>
#include <QtMath>
#include <math.h>


vfoDisplay::vfoDisplay(QWidget *parent) : QWidget(parent)
{
    lineColor = QColor(Qt::black);
    bgColor = QColor(Qt::white);
    textColor = QColor(Qt::black);

    vfoDisplayMode = 0;
}

void vfoDisplay::paintEvent(QPaintEvent *)
{
    //setMouseTracking(true);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    QFont font;
    font = painter.font();
    font.setPointSize(height() - 18);
    painter.setFont(font);

    drawDisplay(&painter);
    drawText(&painter);
}

void vfoDisplay::drawDisplay(QPainter *painter)
{
    painter->save();

    QPen pen(lineColor, 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
    painter->setPen(pen);
    painter->setBrush(bgColor);
    painter->drawRect(1, 1, width()-3, height()-2);
    painter->restore();
}

void vfoDisplay::drawText(QPainter *painter)
{
    painter->save();
    painter->setPen(textColor);

    QFontMetrics meterFont(painter->font());
    double textHeight = meterFont.ascent();
    textWidth = meterFont.horizontalAdvance("0",-1);

    QPointF textPot;

    long val = currentValue;

    for (int i = 1; i < 10; i++)
    {
        val = qFloor(val/10);
        int d = val % 10;

        if (i==2)
        {
            textPot = QPointF(width()-3-(textWidth+2)*i-4, textHeight);
            painter->drawText(textPot, ".");
        }

        textPot = QPointF(width()-3-(textWidth+2)*i, textHeight);

        if (val==0 && d==0) painter->drawText(textPot, " ");
        else painter->drawText(textPot, QString::number(d));
    }

    painter->restore();
}

void vfoDisplay::setValue(unsigned long value)
{
    currentValue = value;
    update();
}

void vfoDisplay::setMode(int mode)
{
    vfoDisplayMode = mode;
}

//* Tuning using mouse buttons
void vfoDisplay::mousePressEvent(QMouseEvent *event)
{
    event->accept();

    QPoint pointerPos = event->pos();

    for (int i = 1; i < 10; i ++)
    {
        if (pointerPos.x() > (width()-3-(textWidth+2)*i+1) && pointerPos.x() < (width()-3-(textWidth+2)*(i-1)-1))
        {
            if (vfoDisplayMode && event->button() == Qt::LeftButton)    //Up/Down mode
            {
                if (pointerPos.y() < height()/2) currentValue = currentValue + pow(10,i);   //Up
                else if (currentValue - pow(10,i) > 0) currentValue = currentValue - pow(10,i);   //Down
            }
            else if (!vfoDisplayMode)   //Left/Right mode
            {
                if (event->button() == Qt::LeftButton) currentValue = currentValue + pow(10,i);    //LeftButton
                else if (currentValue - pow(10,i) > 0) currentValue = currentValue - pow(10,i);   //RightButton
            }

            break;
        }
    }

    update();
    emit on_valueChanged(currentValue);
}

//* Tuning using mouse wheel
void vfoDisplay::wheelEvent(QWheelEvent *event)
{
    event->accept();

    QPointF pointerPos = event->position();

    for (int i = 1; i < 10; i ++)
    {
        if (pointerPos.x() > (width()-3-(textWidth+2)*i+1) && pointerPos.x() < (width()-3-(textWidth+2)*(i-1)-1))
        {
            if (event->angleDelta().y() > 0) currentValue = currentValue + pow(10,i);   //Wheel up
            else if (currentValue - pow(10,i) > 0) currentValue = currentValue - pow(10,i);   //Wheel down
        }
    }

    update();
    emit on_valueChanged(currentValue);
}
