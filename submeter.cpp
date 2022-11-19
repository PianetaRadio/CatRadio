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
#include <math.h>
#include <QDebug>

SubMeter::SubMeter(QWidget *parent) : QWidget(parent)
{
    lineColor = QColor(Qt::black);
    bgColor = QColor(Qt::white);
    progressColor = QColor(Qt::green);
    scaleColor = QColor(Qt::black);

    //Default value
    minValue = 0;
    maxValue = 10;
    gateValue = 10;
    longStep = 5;
    shortStep = 1;
    precision = 0;

    meterSWR = 0;

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
    double increment;

    double initX, initXX;

    if (meterSWR) increment = length / (10 * log10(maxValue));
    else increment = length / (maxValue - minValue);

    if (currentValue>gateValue)
    {
        if (currentValue > maxValue) currentValue = maxValue;   //Trim value if overload scale

        if (meterSWR) initX = 10 * log10(gateValue) * increment;
        else initX = (gateValue - minValue) * increment;
        QRect rect(1, height()/3+2+1, initX, height()/3-4-2);
        painter->drawRect(rect);

        //Red bar
        if (meterSWR)
        {
            if (currentValue > maxValue) currentValue = maxValue;   //Trim value if overload scale
            initXX = 10 * (log10(currentValue) - log10(gateValue)) * increment;
        }
        else
        {
            if (currentValue > maxValue) currentValue = maxValue;   //Trim value if overload scale
            initXX = (currentValue - gateValue) * increment;
        }
        QRect rect2(initX+1, height()/3+2+1, initXX+1, height()/3-4-2);
        painter->setBrush(Qt::red);
        painter->drawRect(rect2);
    }
    else
    {
        if (meterSWR) initX = 10 * log10(currentValue) * increment;
        else initX = (currentValue - minValue) * increment;
        QRect rect(1, height()/3+2+1, initX, height()/3-4-2);
        painter->drawRect(rect);
    }

    painter->restore();
}

void SubMeter::drawScale(QPainter *painter)
{
    painter->save();
    painter->setPen(scaleColor);

    double initX = 0;
    double initTopY = height()*2/3-2;
    double length = width()-12;
    double increment;

    int longLineLen = 6;
    int shortLineLen = 3;

    QFontMetrics meterFont(painter->font());
    double textHeight = meterFont.height();

    if (meterSWR) increment = length / (10 * log10(maxValue));
    else increment = length / (maxValue - minValue);

    if (meterSWR)   //Draw SWR meter with log scale
    {
        double j = 1.0;
        for (int i = 0; i <= 10; i++)   //1.0 to 2.0
        {
            QPointF topPot = QPointF(initX, initTopY);
            QPointF bottomPot;
            if (abs(j - 1.5) < 0.01 || abs(j - 2.0) < 0.01)
            {
                bottomPot = QPointF(initX, initTopY + longLineLen);
                QString strValue = QString::number(j, 'f', 1);
                double textWidth = fontMetrics().horizontalAdvance(strValue);
                QPointF textPot = QPointF(initX - textWidth / 2, initTopY + textHeight + longLineLen - 2);
                painter->drawText(textPot, strValue);
            }
            else bottomPot = QPointF(initX, initTopY + shortLineLen);
            painter->drawLine(topPot, bottomPot);
            j +=0.1;
            initX = 10 * log10(j) * increment;
        }
        for (int i = 11; i <= 25; i++)  //2.1 to 3.5
        {
            QPointF topPot = QPointF(initX, initTopY);
            QPointF bottomPot;
            if (abs(j - 2.5) < 0.01 || abs(j - 3.0) < 0.01 || abs(j - 3.5) < 0.01)
            {
                bottomPot = QPointF(initX, initTopY + longLineLen);
                QString strValue = QString::number(j, 'f', 1);
                double textWidth = fontMetrics().horizontalAdvance(strValue);
                QPointF textPot = QPointF(initX - textWidth / 2, initTopY + textHeight + longLineLen - 2);
                painter->drawText(textPot, strValue);
                painter->drawLine(topPot, bottomPot);
            }
            j +=0.1;
            initX = 10 * log10(j) * increment;
        }
    }
    else    //Draw scale and scale values based on range values
    {
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

void SubMeter::setBgColor(QColor color)
{
    bgColor = color;
}

void SubMeter::setLineColor(QColor color)
{
    lineColor = color;
}

void SubMeter::setProgressColor(QColor color)
{
    progressColor = color;
}

void SubMeter::setScaleColor(QColor color)
{
    scaleColor = color;
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

void SubMeter::setMeterSWR(bool swr)
{
    meterSWR = swr;
}
