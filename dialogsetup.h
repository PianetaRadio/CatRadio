#ifndef DIALOGSETUP_H
#define DIALOGSETUP_H

#include <QDialog>

namespace Ui {
class DialogSetup;
}

class DialogSetup : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSetup(QWidget *parent = nullptr);
    ~DialogSetup();

private slots:


    void on_radioButton_vfoDispMode_LR_toggled(bool checked);

    void on_radioButton_vfoDispMode_UD_toggled(bool checked);

    void on_buttonBox_accepted();

private:
    Ui::DialogSetup *ui;
};

#endif // DIALOGSETUP_H
