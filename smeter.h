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


#ifndef SMETER_H
#define SMETER_H

#include <QWidget>

class SMeter : public QWidget
{
    Q_OBJECT

public:
    explicit SMeter(QWidget *parent = nullptr);

public slots:
    void setMinValue(double value);
    void setMaxValue(double value);
    void setGateValue(double value);
    void setLongStep(double value);
    void setShortStep(double value);
    void setPrecision(int value);

    void setBgColor(QColor color);
    void setLineColor(QColor color);
    void setProgressColor(QColor color);
    void setScaleColor(QColor color);

    void setValue(double value);
    void setValue(int value);

    void setTx(bool Tx);
    bool getTx();
    void setPeak(bool Peak);
    void setPeakFactor(double factor);
    void resetPeakValue();

protected:
    void paintEvent(QPaintEvent *);
    void drawMeter(QPainter *painter);
    void drawProgress(QPainter *painter);
    void drawPeak(QPainter *painter);
    void drawScaleSMeter(QPainter *painter);
    void drawScalePWRMeter(QPainter *painter);

private:
    double minValue;    //Minimum scale value
    double maxValue;    //Maximum scale value
    double gateValue;   //Gate value to bar color change
    double longStep;   //Long lines with equal steps
    double shortStep;  //Short lines with equal steps
    int precision;  //Precision, the last few decimal places

    double value;   //target value
    double currentValue;    //current value
    double peakValue;   //peak value
    double peakFactor;  //peak factor

    bool meterTx;  //Switch between SMeter or PowerMeter
    bool peakHold;  //Peak hold

    QColor bgColor; //background color
    QColor lineColor;   //line color
    QColor progressColor;   //progress color
    QColor scaleColor;   //scale text color

signals:

};

#endif // SMETER_H
