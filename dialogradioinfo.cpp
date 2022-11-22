#include "dialogradioinfo.h"
#include "ui_dialogradioinfo.h"

#include "rig.h"


DialogRadioInfo::DialogRadioInfo(RIG *rig, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRadioInfo)
{
    ui->setupUi(this);

    my_rig = rig;

    QString text;

    text = "Model: ";
    text.append(QString::number(my_rig->caps->rig_model));
    ui->plainTextEdit_RadioInfo->appendPlainText(text);

    text = "Model name: ";
    text.append(my_rig->caps->model_name);
    ui->plainTextEdit_RadioInfo->appendPlainText(text);

    text = "Mfg name: ";
    text.append(my_rig->caps->mfg_name);
    ui->plainTextEdit_RadioInfo->appendPlainText(text);

    text = "Backend version: ";
    text.append(my_rig->caps->version);
    ui->plainTextEdit_RadioInfo->appendPlainText(text);
}

DialogRadioInfo::~DialogRadioInfo()
{
    delete ui;
}
