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


#include "smeter.h"
#include <QPainter>
#include <math.h>

SMeter::SMeter(QWidget *parent) : QWidget(parent)
{
    lineColor = QColor(Qt::black);
    bgColor = QColor(Qt::white);
    progressColor = QColor(Qt::green);

    //Default value
    minValue = 0;
    maxValue = 100;
    gateValue = 80;
    longStep = 20;
    shortStep = 10;
    precision = 0;

    meterTx = 0;

    currentValue = -54;
}

void SMeter::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    QFont font;
    font = painter.font();
    font.setPointSize(font.pointSize() - 2);
    painter.setFont(font);

    drawMeter(&painter);
    drawProgress(&painter);
    drawScaleSMeter(&painter);
    drawScalePWRMeter(&painter);
}

void SMeter::drawMeter(QPainter *painter)
{
    painter->save();

    QPen pen(lineColor, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(bgColor);
    painter->drawRect(0, height()/3+2, width()-12, height()/3-4);
    painter->restore();
}

void SMeter::drawProgress(QPainter *painter)
{
    double max, min;
    double gate;

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(progressColor);

    if (meterTx)    //RF power meter
    {
        max = maxValue;
        min = minValue;
        gate = gateValue;
    }
    else    //SMeter
    {
        max = 60;
        min = -54;
        gate = 0;
    }

    double length = width()-14;
    double increment = length / (max - min);
    double initX, initXX;

    if (currentValue>gate)
    {
        initX = (gate - min) * increment;
        QRect rect(1, height()/3+2+1, initX, height()/3-4-2);
        painter->drawRect(rect);

        //Red bar
        initXX = (currentValue - gate) * increment;
        QRect rect2(initX+1, height()/3+2+1, initXX, height()/3-4-2);
        painter->setBrush(Qt::red);
        painter->drawRect(rect2);
    }
    else
    {
        initX = (currentValue - min) * increment;
        QRect rect(1, height()/3+2+1, initX, height()/3-4-2);
        painter->drawRect(rect);
    }

    painter->restore();
}

void SMeter::drawScalePWRMeter(QPainter *painter)
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

    //Draw scale and scale values based on range values Long lines
    int stepNumber = (maxValue - minValue) / shortStep;
    for (int i = 0; i <= stepNumber; i++)
    {
        double j = i * shortStep + minValue;
        if (fmod(j,longStep) == 0)
        {
            if (j == minValue)  //Do not draw the first value
            {
                QPointF textPot = QPointF(initX, initTopY + textHeight);
                painter->drawText(textPot, "PO");

                initX += increment * shortStep;
                continue;
            }

            QPointF topPot = QPointF(initX, initTopY);
            QPointF bottomPot = QPointF(initX, initTopY + longLineLen);
            painter->drawLine(topPot, bottomPot);

            QString strValue = QString("%1").arg(j, 0, 'f', precision);
            double textWidth = fontMetrics().horizontalAdvance(strValue);

            QPointF textPot = QPointF(initX - textWidth / 2, initTopY + textHeight + longLineLen - 2);
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

void SMeter::drawScaleSMeter(QPainter *painter)
{
    int minValue = -54; //Set min and max value according to Hamlib SMeter definition
    int maxValue = 60;

    int longLineLen = 6;
    int shortLineLen = 3;

    double initX = 0;
    int sUnit = 0;

    double initBottomY = height()/3+2;
    double length = width()-12;
    double increment = length / (maxValue - minValue);

    QFontMetrics meterFont(painter->font());
    double textHeight = meterFont.height();

    painter->save();
    painter->setPen(lineColor);

    //Draw scale and scale values based on range values
    int longStep = 12;
    int shortStep = 6;

    for (int i = minValue; i <= 0; i = i + shortStep)   //S0 to S9
    {
        if (i == minValue)
        {
            QPointF textPot = QPointF(initX, initBottomY - 2);
            painter->drawText(textPot, "S");

            initX += increment * shortStep;
            sUnit = sUnit + 1;
            continue;
        }

        if (i % longStep == 0)
        {
            QPointF topPot = QPointF(initX, initBottomY);
            QPointF bottomPot = QPointF(initX, initBottomY - longLineLen);
            painter->drawLine(topPot, bottomPot);

            QString strValue = QString::number(sUnit);
            double textWidth = fontMetrics().horizontalAdvance(strValue);
            QPointF textPot = QPointF(initX - textWidth / 2, initBottomY - textHeight / 2 - longLineLen + 2);
            painter->drawText(textPot, strValue);
        }
        else
        {
            QPointF topPot = QPointF(initX, initBottomY);
            QPointF bottomPot = QPointF(initX, initBottomY - shortLineLen);
            painter->drawLine(topPot, bottomPot);
        }

        sUnit = sUnit + 1;
        initX += increment * shortStep;
    }
    initX -= increment * shortStep;

    shortStep = 10;
    longStep = 20;

    for (int i = 0; i <= maxValue; i = i + shortStep)   //S9+ to S9+60
    {
        if (i % longStep == 0)
        {
            QPointF topPot = QPointF(initX, initBottomY);
            QPointF bottomPot = QPointF(initX, initBottomY - longLineLen);
            painter->drawLine(topPot, bottomPot);

            if (i == 0)    //Do not draw the first value
            {
                initX += increment * shortStep;
                continue;
            }

            QString strValue = "+" + QString::number(i);
            double textWidth = fontMetrics().horizontalAdvance(strValue);

            QPointF textPot = QPointF(initX - textWidth / 2, initBottomY - textHeight / 2 - longLineLen + 2);
            painter->drawText(textPot, strValue);
        }
        else
        {
            QPointF topPot = QPointF(initX, initBottomY);
            QPointF bottomPot = QPointF(initX, initBottomY - shortLineLen);
            painter->drawLine(topPot, bottomPot);
        }

        initX += increment * shortStep;
    }

    painter->restore();
}

void SMeter::setMinValue(double value)
{
    minValue = value;
    update();
}

void SMeter::setMaxValue(double value)
{
    maxValue = value;
    update();
}

void SMeter::setGateValue(double value)
{
    gateValue = value;
}

void SMeter::setLongStep(double value)
{
    longStep = value;
    update();
}

void SMeter::setShortStep(double value)
{
    shortStep = value;
    update();
}

void SMeter::setPrecision(int value)
{
    precision = value;
    update();
}

void SMeter::setValue(double value)
{
    currentValue = value;
    update();
}

void SMeter::setValue(int value)
{
    setValue(double(value));
}

void SMeter::setTx(bool Tx)
{
    meterTx = Tx;
}
