/**
 ** This file is part of the CatRadio project.
 ** Copyright 2022-2024 Gianfranco Sordetti IZ8EWD <iz8ewd@pianetaradio.it>.
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

   if (guiConf.vfoDisplayMode == 1) ui->radioButton_vfoDispMode_UD->setChecked(true);
   if (guiConf.darkTheme) ui->radioButton_themeDark->setChecked(true);
   if (guiConf.voiceKeyerMode == 1) ui->radioButton_voiceKeyerMode_CatRadio->setChecked(true);
   if (guiConf.peakHold) ui->checkBox_peakHold->setChecked(true);
   if (guiConf.debugMode) ui->checkBox_debug->setChecked(true);
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
    guiConf.voiceKeyerMode = ui->radioButton_voiceKeyerMode_CatRadio->isChecked();
    guiConf.peakHold = ui->checkBox_peakHold->isChecked();
    guiConf.debugMode = ui->checkBox_debug->isChecked();

    //* Save settings in catradio.ini
    QSettings configFile(QString("catradio.ini"), QSettings::IniFormat);
    configFile.setValue("vfoDisplayMode", guiConf.vfoDisplayMode);
    configFile.setValue("darkTheme", guiConf.darkTheme);
    configFile.setValue("voiceKeyerMode", guiConf.voiceKeyerMode);
    configFile.setValue("peakHold", guiConf.peakHold);
    configFile.setValue("debugMode", guiConf.debugMode);
}
