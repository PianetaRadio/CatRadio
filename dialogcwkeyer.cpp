/**
 ** This file is part of the CatRadio project.
 ** Copyright 2022-2025 Gianfranco Sordetti IZ8EWD <iz8ewd@pianetaradio.it>.
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


#include "dialogcwkeyer.h"
#include "ui_dialogcwkeyer.h"

#include "guidata.h"
#include "winkeyer.h"

#include <QSettings>
#include <QFile>
#include <QSerialPortInfo>

#include <QDebug>
#include <QThread>


extern cwKeyerConfig cwKConf;


DialogCWKeyer::DialogCWKeyer(WinKeyer *winkeyer, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogCWKeyer)
    , wk(winkeyer)
{
    ui->setupUi(this);

    ui->lineEdit_cwK1->setText(cwKConf.memoryString[0]);
    ui->lineEdit_cwK2->setText(cwKConf.memoryString[1]);
    ui->lineEdit_cwK3->setText(cwKConf.memoryString[2]);
    ui->lineEdit_cwK4->setText(cwKConf.memoryString[3]);
    ui->lineEdit_cwK5->setText(cwKConf.memoryString[4]);

    ui->radioButton_autoConnect->setChecked(cwKConf.autoConnect);

    if (wk->isOpen) //WinKeyer already open
    {
        ui->pushButton_Connect->setChecked(true);
        ui->label_wkVersion->setText(QString("WinKeyer v. %1").arg(wk->version));
    }

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
    ui->comboBox_comPort->setCurrentText(cwKConf.comPort);
}

DialogCWKeyer::~DialogCWKeyer()
{
    delete ui;
}

void DialogCWKeyer::on_pushButton_Connect_toggled(bool checked)
{
    if (checked && !wk->isOpen) //Connect
    {
        if (!wk->init(ui->comboBox_comPort->currentText())) //Open serial port
        {
            //wait 2 seconds, serial port open causes reset in Arduino for K3NG CW keyer
            QThread::msleep(2000);

            wk->version = wk->open();   //Open WinKeyer host

            if (wk->version)    //Read WinKeyer version
            {
                ui->label_wkVersion->setText(QString("WinKeyer v. %1").arg(wk->version));
                wk->isOpen = true;

                wk->setWpmSpeed(20);    //Set default WPM speed
            }
            else    //Winkeyer error
            {
                ui->label_wkVersion->setText("WinKeyer error");
                ui->pushButton_Connect->setChecked(false);
            }
        }
        else    //Serial port error
        {
            ui->label_wkVersion->setText("Serial error");
            ui->pushButton_Connect->setChecked(false);
        }
    }
    else if (!checked && wk->isOpen)    //Disconnect
    {
        ui->label_wkVersion->setText("Disconnected");
        wk->close();
        wk->isOpen = false;
    }
}

void DialogCWKeyer::on_buttonBox_accepted()
{
    cwKConf.comPort = ui->comboBox_comPort->currentText();
    cwKConf.memoryString[0] = ui->lineEdit_cwK1->text().toUtf8();
    cwKConf.memoryString[1] = ui->lineEdit_cwK2->text().toUtf8();
    cwKConf.memoryString[2] = ui->lineEdit_cwK3->text().toUtf8();
    cwKConf.memoryString[3] = ui->lineEdit_cwK4->text().toUtf8();
    cwKConf.memoryString[4] = ui->lineEdit_cwK5->text().toUtf8();

    //* Save settings in catradio.ini
    QSettings configFile(QString("catradio.ini"), QSettings::IniFormat);
    configFile.setValue("CWKeyer/comPort", cwKConf.comPort);
    configFile.setValue("CWKeyer/autoConnect", ui->radioButton_autoConnect->isChecked());
    configFile.setValue("CWKeyer/cwMemoryString1", cwKConf.memoryString[0]);
    configFile.setValue("CWKeyer/cwMemoryString2", cwKConf.memoryString[1]);
    configFile.setValue("CWKeyer/cwMemoryString3", cwKConf.memoryString[2]);
    configFile.setValue("CWKeyer/cwMemoryString4", cwKConf.memoryString[3]);
    configFile.setValue("CWKeyer/cwMemoryString5", cwKConf.memoryString[4]);
}
