#ifndef VFODISPLAY_H
#define VFODISPLAY_H

#include <QWidget>

class vfoDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit vfoDisplay(QWidget *parent = nullptr);

public slots:
    void setValue(unsigned long value);
    void setMode(int mode);

    void setBgColor(QColor color);
    void setLineColor(QColor color);
    void setTextColor(QColor color);

signals:
    void on_valueChanged(int value);

protected:
    void paintEvent(QPaintEvent *);
    void drawDisplay(QPainter *painter);
    void drawText(QPainter *painter);
    void mousePressEvent(QMouseEvent *event);   //Mouse buttons
    void wheelEvent(QWheelEvent *event);   //Mouse wheel

private:
    QColor bgColor; //background color
    QColor lineColor;   //line color
    QColor textColor;   //text color

    unsigned long currentValue;  //current frequency value (Hz)
    unsigned long value; //target value

    int vfoDisplayMode; //0: use Left/Right mouse button, 1: click digit Up or Down

    int textWidth;  //number width
};

#endif // VFODISPLAY_H
