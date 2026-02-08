/**
 ** This file is part of the CatRadio project.
 ** Copyright 2022-2026 Gianfranco Sordetti IZ8EWD <iz8ewd@pianetaradio.it>.
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


#include "dialognetrigctl.h"
#include "ui_dialognetrigctl.h"

#include <QSettings>

#include "rigdata.h"
#include "guidata.h"
#include "netrigctl.h"


extern rigConnection rigCom;
extern guiConfig guiConf;


DialogNetRigctl::DialogNetRigctl(netRigCtl *netrigctlPtr, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogNetRigctl)
    , netrigctl(netrigctlPtr)
{
    ui->setupUi(this);
    ui->spinBox_portNumber->setValue(guiConf.rigctldPort);
    ui->lineEdit_arguments->setText(argumentsString);
    ui->checkBox_startup->setChecked(guiConf.autoRigctld);
}

DialogNetRigctl::~DialogNetRigctl()
{
    delete ui;
}


void DialogNetRigctl::setArguments(QString arguments)
{
    argumentsString = arguments;
    ui->lineEdit_arguments->setText(argumentsString);
}


void DialogNetRigctl::on_pushButton_start_toggled(bool checked)
{
    if (checked)    //Start
    {
        netrigctl->open();
    }
    else    //Stop
    {
        netrigctl->close();
    }
}


void DialogNetRigctl::on_spinBox_portNumber_editingFinished()
{
    guiConf.rigctldPort = ui->spinBox_portNumber->value();
    netrigctl->setRigctldArguments(rigCom.rigModel, rigCom.rigPort, rigCom.serialSpeed, rigCom.civAddr, guiConf.rigctldPort);
    setArguments(netrigctl->rigctldArguments.join(" "));
}


void DialogNetRigctl::on_buttonBox_accepted()
{
    guiConf.autoRigctld = ui->checkBox_startup->isChecked();
    guiConf.rigctldPort = ui->spinBox_portNumber->value();

    //* Save settings in catradio.ini
    QSettings configFile(QString("catradio.ini"), QSettings::IniFormat);
    configFile.setValue("autoRigctld", guiConf.autoRigctld);
    configFile.setValue("rigctldPort", guiConf.rigctldPort);
}

