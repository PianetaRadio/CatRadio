#include "dialogsetup.h"
#include "ui_dialogsetup.h"

#include "guidata.h"

#include <QSettings>
#include <QFile>
#include <QMessageBox>


extern guiConfig guiConf;


DialogSetup::DialogSetup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSetup)
{
    ui->setupUi(this);

   if (guiConf.vfoDisplayMode) ui->radioButton_vfoDispMode_UD->setChecked(true);
   if (guiConf.darkTheme) ui->radioButton_themeDark->setChecked(true);
}

DialogSetup::~DialogSetup()
{
    delete ui;
}

void DialogSetup::on_buttonBox_accepted()
{
    if ((guiConf.darkTheme != ui->radioButton_themeDark->isChecked()))
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Theme");
        msgBox.setText("Please, restart CatRadio to make effective the theme.");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }

    guiConf.vfoDisplayMode = ui->radioButton_vfoDispMode_UD->isChecked();
    guiConf.darkTheme = ui->radioButton_themeDark->isChecked();

    //* Save settings in catradio.ini
    QSettings configFile(QString("catradio.ini"), QSettings::IniFormat);
    configFile.setValue("vfoDisplayMode", guiConf.vfoDisplayMode);
    configFile.setValue("darkTheme", guiConf.darkTheme);
}
