#ifndef DIALOGRADIOINFO_H
#define DIALOGRADIOINFO_H

#include <QDialog>

#include "rig.h"

namespace Ui {
class DialogRadioInfo;
}

class DialogRadioInfo : public QDialog
{
    Q_OBJECT

public:
    explicit DialogRadioInfo(RIG *rig, QWidget *parent = nullptr);
    ~DialogRadioInfo();

private:
    Ui::DialogRadioInfo *ui;

    RIG *my_rig;
};

#endif // DIALOGRADIOINFO_H
