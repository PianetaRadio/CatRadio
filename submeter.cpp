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


#include "submeter.h"
#include <QPainter>
#include <QDebug>

SubMeter::SubMeter(QWidget *parent) : QWidget(parent)
{
    lineColor = QColor(Qt::black);
    bgColor = QColor(Qt::white);
    progressColor = QColor(Qt::green);

    //Default value
    minValue = 0;
    maxValue = 10;
    gateValue = 10;
    longStep = 5;
    shortStep = 1;
    precision = 0;

    currentValue = 0;
}

void SubMeter::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    QFont font;
    font = painter.font();
    font.setPointSize(font.pointSize() - 2);
    painter.setFont(font);

    drawMeter(&painter);
    drawProgress(&painter);
    drawScale(&painter);
}

void SubMeter::drawMeter(QPainter *painter)
{
    painter->save();
    QPen pen(lineColor, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(bgColor);
    painter->drawRect(0, height()/3+2, width()-12, height()/3-4);
    painter->restore();
}

void SubMeter::drawProgress(QPainter *painter)
{
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(progressColor);

    double length = width()-14;
    double increment = length / (maxValue - minValue);

    double initX, initXX;

    if (currentValue>gateValue)
    {
        initX = (gateValue - minValue) * increment;
        QRect rect(1, height()/3+2+1, initX, height()/3-4-2);
        painter->drawRect(rect);

        //Red bar
        initXX = (currentValue - gateValue) * increment;
        QRect rect2(initX+1, height()/3+2+1, initXX, height()/3-4-2);
        painter->setBrush(Qt::red);
        painter->drawRect(rect2);
    }
    else
    {
        initX = (currentValue - minValue) * increment;
        QRect rect(1, height()/3+2+1, initX, height()/3-4-2);
        painter->drawRect(rect);
    }

    painter->restore();
}

void SubMeter::drawScale(QPainter *painter)
{
    painter->save();
    painter->setPen(lineColor);

    double initX = 0;
    double initTopY = height()*2/3-2;
    double length = width()-12;
    double increment = length / (maxValue - minValue);

    int longLineLen = 6;
    int shortLineLen = 3;

    QFontMetrics meterFont(painter->font());
    double textHeight = meterFont.height();

    //Draw scale values and scale values based on range values
    int stepNumber = (maxValue - minValue) / shortStep;
    for (int i = 0; i <= stepNumber; i++)
    {
        double j = i * shortStep + minValue;
        if (fmod(j,longStep) == 0)
        {
            if (j == minValue)  //Do not draw the first value
            {
                initX += increment * shortStep;
                continue;
            }

            QPointF topPot = QPointF(initX, initTopY);
            QPointF bottomPot = QPointF(initX, initTopY + longLineLen);
            painter->drawLine(topPot, bottomPot);

            QString strValue = QString("%1").arg(j, 0, 'f', precision);
            double textWidth = fontMetrics().horizontalAdvance(strValue);

            QPointF textPot = QPointF(initX - textWidth / 2, initTopY + textHeight + longLineLen);
            painter->drawText(textPot, strValue);
        }
        else
        {
            QPointF topPot = QPointF(initX, initTopY);
            QPointF bottomPot = QPointF(initX, initTopY + shortLineLen);
            painter->drawLine(topPot, bottomPot);
        }

        initX += increment * shortStep;
    }

    painter->restore();
}

void SubMeter::setMinValue(double value)
{
    minValue = value;
    update();
}

void SubMeter::setMaxValue(double value)
{
    maxValue = value;
    update();
}

void SubMeter::setGateValue(double value)
{
    gateValue = value;
}

void SubMeter::setLongStep(double value)
{
    longStep = value;
    update();
}

void SubMeter::setShortStep(double value)
{
    shortStep = value;
    update();
}

void SubMeter::setPrecision(int value)
{
    precision = value;
    update();
}

void SubMeter::setValue(double value)
{
    currentValue = value;
    update();
}

void SubMeter::setValue(int value)
{
    setValue(double(value));
}
