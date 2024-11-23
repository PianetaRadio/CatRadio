#include "dialognetrigctl.h"
#include "ui_dialognetrigctl.h"

DialogNetRigctl::DialogNetRigctl(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogNetRigctl)
{
    ui->setupUi(this);
}

DialogNetRigctl::~DialogNetRigctl()
{
    delete ui;
}
