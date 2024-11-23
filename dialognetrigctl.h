#ifndef DIALOGNETRIGCTL_H
#define DIALOGNETRIGCTL_H

#include <QDialog>

namespace Ui {
class DialogNetRigctl;
}

class DialogNetRigctl : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNetRigctl(QWidget *parent = nullptr);
    ~DialogNetRigctl();

private:
    Ui::DialogNetRigctl *ui;
};

#endif // DIALOGNETRIGCTL_H
