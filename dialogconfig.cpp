/**
 ** This file is part of the CatRadio project.
 ** Copyright 2022 Gianfranco Sordetti IZ8EWD <iz8ewd@pianetaradio.it>.
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


#include "dialogconfig.h"
#include "ui_dialogconfig.h"

#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QSerialPortInfo>

#include "rigdata.h"

#include <hamlib/rig.h>


extern rigConnect rigCom;


QString rigListFile = "rig.lst";    //Text file containing the list of rig supported by hamlib
QFile file(rigListFile);


DialogConfig::DialogConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConfig)
{
    ui->setupUi(this);

    //* rigModel comboBox
    if (!file.exists()) //Create file rig.lst if not exists
    {
        file.open(QIODevice::ReadWrite);
        rig_load_all_backends();    //Load all backends information
        rig_list_foreach(printRigList, NULL);   //Create the rig list
    }
    else file.open(QIODevice::ReadOnly);    //Open file rig.lst and populate the combobox
    file.seek(0);
    ui->comboBox_rigModel->clear();
    ui->comboBox_rigModel->addItem("");
    while(!file.atEnd())
    {
        QString line = file.readLine();
        ui->comboBox_rigModel->addItem(line.trimmed());
    }
    file.close();

    //* COM port
    ui->comboBox_comPort->clear();
    ui->comboBox_comPort->addItem("");
    foreach (const QSerialPortInfo &comPort, QSerialPortInfo::availablePorts())  //search available COM port
    {
#ifdef Q_OS_WIN
        ui->comboBox_comPort->addItem(comPort.portName());
#endif
#ifdef Q_OS_LINUX
        ui->comboBox_comPort->addItem("/dev/"+comPort.portName());
#endif
    }

    //* serialSpeed
    ui->comboBox_serialSpeed->clear();
    ui->comboBox_serialSpeed->addItem("");
    ui->comboBox_serialSpeed->addItem("4800");
    ui->comboBox_serialSpeed->addItem("9600");
    ui->comboBox_serialSpeed->addItem("19200");
    ui->comboBox_serialSpeed->addItem("38400");
    ui->comboBox_serialSpeed->addItem("57600");
    ui->comboBox_serialSpeed->addItem("115200");

    //* Update values in the GUI
    if (rigCom.netRigctl)
    {
        ui->checkBox_netRigctl->setChecked(rigCom.netRigctl);
        ui->lineEdit_ip->setText(rigCom.rigPort);
        ui->comboBox_rigModel->setCurrentIndex(0);
    }
    else
    {
        ui->comboBox_rigModel->setCurrentIndex(ui->comboBox_rigModel->findText(QString::number(rigCom.rigModel),Qt::MatchStartsWith));
        ui->comboBox_comPort->setCurrentText(rigCom.rigPort);
        ui->comboBox_serialSpeed->setCurrentText(QString::number(rigCom.serialSpeed));
        if (rigCom.civAddr) ui->lineEdit_civAddr->setText(QString::number(rigCom.civAddr,16));
    }
    ui->spinBox_RefreshRate->setValue(rigCom.rigRefresh);
    ui->checkBox_fullPoll->setChecked(rigCom.fullPoll);
}

DialogConfig::~DialogConfig()
{
    delete ui;
}

void DialogConfig::on_buttonBox_accepted()
{
    bool civAddrConv;

    //* Read settings from GUI
    if (ui->checkBox_netRigctl->isChecked())
    {
        rigCom.rigModel = 2;
        rigCom.netRigctl = true;
        rigCom.rigPort = ui->lineEdit_ip->text();
    }
    else
    {
    QString rigModel = ui->comboBox_rigModel->currentText();
    QRegularExpression regexp("[0-9]+");
    QRegularExpressionMatch rigNumber = regexp.match(rigModel);
    rigCom.rigModel = rigNumber.captured(0).toInt();

    rigCom.rigPort = ui->comboBox_comPort->currentText();
    rigCom.serialSpeed = ui->comboBox_serialSpeed->currentText().toInt();
    rigCom.civAddr = ui->lineEdit_civAddr->text().toInt(&civAddrConv,16);
    }
    rigCom.rigRefresh = ui->spinBox_RefreshRate->value();
    rigCom.fullPoll = ui->checkBox_fullPoll->isChecked();

    //* Save settings in catradio.ini
    QSettings configFile(QString("catradio.ini"), QSettings::IniFormat);
    configFile.setValue("rigModel", rigCom.rigModel);
    configFile.setValue("rigPort", rigCom.rigPort);
    configFile.setValue("serialSpeed", ui->comboBox_serialSpeed->currentText());
    configFile.setValue("civAddress", ui->lineEdit_civAddr->text().toInt(&civAddrConv,16));
    configFile.setValue("netRigctl", ui->checkBox_netRigctl->isChecked());
    configFile.setValue("rigRefresh", ui->spinBox_RefreshRate->value());
    configFile.setValue("fullPolling", ui->checkBox_fullPoll->isChecked());
}

int printRigList(const struct rig_caps *rigCaps, void *data)    //Load rig list from hamlib and save into file rig.lst
{
    if (data) return 0;
    QTextStream stream(&file);
    stream << rigCaps->rig_model << " " << rigCaps->mfg_name << " " << rigCaps->model_name << "\n";
    return 1;
}

void DialogConfig::on_checkBox_netRigctl_toggled(bool checked)
{
    if (checked)
    {
        ui->comboBox_rigModel->setCurrentIndex(2);  //set NET rigctl
        ui->comboBox_comPort->setCurrentText("");
    }
    else
    {
        ui->comboBox_rigModel->setCurrentIndex(0);  //set void
        ui->lineEdit_ip->setText("");   //clear IP address
    }
}
