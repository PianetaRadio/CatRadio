#include "dialogsetup.h"
#include "ui_dialogsetup.h"

#include "guidata.h"

#include <QSettings>
#include <QFile>


extern guiConfig guiConf;


DialogSetup::DialogSetup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSetup)
{
    ui->setupUi(this);

   if (guiConf.vfoDisplayMode) ui->radioButton_vfoDispMode_UD->setChecked(true);
}

DialogSetup::~DialogSetup()
{
    delete ui;
}

void DialogSetup::on_radioButton_vfoDispMode_LR_toggled(bool checked)
{
    if (checked) guiConf.vfoDisplayMode=0;
}

void DialogSetup::on_radioButton_vfoDispMode_UD_toggled(bool checked)
{
    if (checked) guiConf.vfoDisplayMode=1;
}

void DialogSetup::on_buttonBox_accepted()
{
    //* Save settings in catradio.ini
    QSettings configFile(QString("catradio.ini"), QSettings::IniFormat);
    configFile.setValue("vfoDisplayMode", guiConf.vfoDisplayMode);
}

