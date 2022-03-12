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

    int textWidth;  //number width
};

#endif // VFODISPLAY_H
