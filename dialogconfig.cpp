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


#include "dialogconfig.h"
#include "ui_dialogconfig.h"

#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QSerialPortInfo>
#include <QMessageBox>

#include "rigdata.h"

#include <hamlib/rig.h>


extern rigConnect rigCom;


QString rigListFile = "rig.lst";    //Text file containing the list of rig supported by hamlib
QFile rigFile(rigListFile);


DialogConfig::DialogConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConfig)
{
    ui->setupUi(this);

    //* rigModel comboBox
    if (!rigFile.exists()) //Create file rig.lst if not exists
    {
        createRigFile();
    }
    else rigFile.open(QIODevice::ReadOnly);    //Open file rig.lst and populate the combobox
    rigFile.seek(0);

    QString versionFile = rigFile.readLine();   //Update rigFile if old version
    if (versionFile.trimmed() != hamlib_version)
    {
        rigFile.remove();
        createRigFile();
        rigFile.seek(0);
        rigFile.readLine();
    }

    ui->comboBox_rigModel->clear();
    ui->comboBox_rigModel->addItem("");
    while(!rigFile.atEnd())
    {
        QString line = rigFile.readLine();
        ui->comboBox_rigModel->addItem(line.trimmed());
    }
    rigFile.close();

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
    ui->comboBox_rigModel->setCurrentIndex(ui->comboBox_rigModel->findText(QString::number(rigCom.rigModel),Qt::MatchStartsWith));
    if (rigCom.netRigctl)
    {
        ui->checkBox_netRigctl->setChecked(rigCom.netRigctl);
        ui->lineEdit_ip->setText(rigCom.rigPort);
    }
    else
    {
        ui->comboBox_comPort->setCurrentText(rigCom.rigPort);
        ui->comboBox_serialSpeed->setCurrentText(QString::number(rigCom.serialSpeed));
        if (rigCom.civAddr) ui->lineEdit_civAddr->setText(QString::number(rigCom.civAddr,16));
        setDialogSerialConfig(rigCom.serialDataBits, rigCom.serialParity, rigCom.serialStopBits, rigCom.serialHandshake);
    }
    ui->spinBox_RefreshRate->setValue(rigCom.rigRefresh);
    ui->checkBox_fullPoll->setChecked(rigCom.fullPoll);
    ui->checkBox_autoConnect->setChecked(rigCom.autoConnect);
    ui->checkBox_autoPowerOn->setChecked(rigCom.autoPowerOn);
}

DialogConfig::~DialogConfig()
{
    delete ui;
}


int DialogConfig::findRigModel(QString rigModel)
{
    QRegularExpression regexp("[0-9]+");
    QRegularExpressionMatch rigNumber = regexp.match(rigModel);
    return rigNumber.captured(0).toInt();
}


void DialogConfig::setDialogSerialConfig(int dataBits, int parity, int stopBits, int handshake)
{
    switch (dataBits)
    {
    case 7: ui->radioButton_dataBits7->setChecked(true); break;
    case 8: ui->radioButton_dataBits8->setChecked(true); break;
    }

    switch (parity)
    {
    case 0: ui->radioButton_parityNone->setChecked(true); break;
    case 1: ui->radioButton_parityOdd->setChecked(true); break;
    case 2: ui->radioButton_parityEven->setChecked(true); break;
    }

    switch (stopBits)
    {
    case 1: ui->radioButton_stopBits1->setChecked(true); break;
    case 2: ui->radioButton_stopBits2->setChecked(true); break;
    }

    switch (handshake)
    {
    case 0: ui->radioButton_handshakeNone->setChecked(true); break;
    case 1: ui->radioButton_handshakeXonXoff->setChecked(true); break;
    case 2: ui->radioButton_handshakeHardware->setChecked(true); break;
    }
}


void DialogConfig::setRigSerialConfigFromDialog()
{
    if (ui->radioButton_dataBits7->isChecked()) rigCom.serialDataBits = 7;
    else rigCom.serialDataBits = 8;

    if (ui->radioButton_parityOdd->isChecked()) rigCom.serialParity = RIG_PARITY_ODD;
    else if (ui->radioButton_parityEven->isChecked()) rigCom.serialParity = RIG_PARITY_EVEN;
    else rigCom.serialParity = RIG_PARITY_NONE;

    if (ui->radioButton_stopBits1->isChecked()) rigCom.serialStopBits = 1;
    else rigCom.serialStopBits = 2;

    if (ui->radioButton_handshakeXonXoff->isChecked()) rigCom.serialHandshake = RIG_HANDSHAKE_XONXOFF;
    else if (ui->radioButton_handshakeHardware->isChecked()) rigCom.serialHandshake = RIG_HANDSHAKE_HARDWARE;
    else rigCom.serialHandshake = RIG_HANDSHAKE_NONE;
}


void DialogConfig::on_buttonBox_accepted()
{
    bool civAddrConv;

    //* Read settings from GUI
    if (ui->comboBox_rigModel->currentText() == "") //No backend selected
    {
        QMessageBox msgBox; //Show error MessageBox
        msgBox.setWindowTitle("Warning");
        msgBox.setText("Rig model not selected");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }
    else
    {
        QString rigModel = ui->comboBox_rigModel->currentText();
        //QRegularExpression regexp("[0-9]+");
        //QRegularExpressionMatch rigNumber = regexp.match(rigModel);
        //rigCom.rigModel = rigNumber.captured(0).toInt();
        rigCom.rigModel = findRigModel(rigModel);

        if (ui->checkBox_netRigctl->isChecked())   //TCP port
        {
            rigCom.netRigctl = true;
            rigCom.rigPort = ui->lineEdit_ip->text();

            if (rigCom.rigPort == "")
            {
                QMessageBox msgBox; //Show error MessageBox
                msgBox.setWindowTitle("Warning");
                msgBox.setText(rigModel + "\nIP address not valid");
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.exec();
            }
        }
        else    //COM port
        {
            rigCom.netRigctl = false;
            rigCom.rigPort = ui->comboBox_comPort->currentText();
            rigCom.serialSpeed = ui->comboBox_serialSpeed->currentText().toInt();
            rigCom.civAddr = ui->lineEdit_civAddr->text().toInt(&civAddrConv,16);
            setRigSerialConfigFromDialog();

            if (rigCom.rigPort == "" && rigCom.rigModel != 1 && rigCom.rigModel != 6)
            {
                QMessageBox msgBox; //Show error MessageBox
                msgBox.setWindowTitle("Warning");
                msgBox.setText(rigModel + "\nCOM port not valid");
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.exec();
            }
        }
    }

    rigCom.rigRefresh = ui->spinBox_RefreshRate->value();
    rigCom.fullPoll = ui->checkBox_fullPoll->isChecked();
    rigCom.autoConnect = ui->checkBox_autoConnect->isChecked();
    rigCom.autoPowerOn = ui->checkBox_autoPowerOn->isChecked();

    //* Save settings in catradio.ini
    QSettings configFile(QString("catradio.ini"), QSettings::IniFormat);
    configFile.setValue("rigModel", rigCom.rigModel);
    configFile.setValue("rigPort", rigCom.rigPort);
    configFile.setValue("serialSpeed", ui->comboBox_serialSpeed->currentText());
    configFile.setValue("civAddress", ui->lineEdit_civAddr->text().toInt(&civAddrConv,16));
    configFile.setValue("serialDataBits", rigCom.serialDataBits);
    configFile.setValue("serialParity", rigCom.serialParity);
    configFile.setValue("serialStopBits", rigCom.serialStopBits);
    configFile.setValue("serialHandshake", rigCom.serialHandshake);
    configFile.setValue("netRigctl", ui->checkBox_netRigctl->isChecked());
    configFile.setValue("rigRefresh", ui->spinBox_RefreshRate->value());
    configFile.setValue("fullPolling", ui->checkBox_fullPoll->isChecked());
    configFile.setValue("autoConnect", ui->checkBox_autoConnect->isChecked());
    configFile.setValue("autoPowerOn", ui->checkBox_autoPowerOn->isChecked());
}

#ifdef RIGCAPS_NOT_CONST    //rig_caps is no longer constant starting from hamlib v.4.6
int printRigList(struct rig_caps *rigCaps, void *data)    //Load rig list from hamlib and save into file rig.lst
{
    if (data) return 0;
    QTextStream stream(&rigFile);
    stream << rigCaps->rig_model << " " << rigCaps->mfg_name << " " << rigCaps->model_name << "\n";
    return 1;
}
#else
int printRigList(const struct rig_caps *rigCaps, void *data)
{
    if (data) return 0;
    QTextStream stream(&rigFile);
    stream << rigCaps->rig_model << " " << rigCaps->mfg_name << " " << rigCaps->model_name << "\n";
    return 1;
}
#endif

bool createRigFile()
{
    bool ret = rigFile.open(QIODevice::ReadWrite);
    rigFile.write(hamlib_version);  //Write current Hamlib version in the file header
    rigFile.write("\n");
    rig_load_all_backends();    //Load all backends information
    rig_list_foreach(printRigList, NULL);   //Create and write the rig list
    return ret;
}

void DialogConfig::on_checkBox_netRigctl_toggled(bool checked)
{
    if (checked)    //TCP port
    {
        ui->comboBox_comPort->setCurrentText("");   //clear COM port
    }
    else    //COM port
    {
        ui->lineEdit_ip->setText("");   //clear IP address
    }
}

void DialogConfig::on_comboBox_rigModel_currentIndexChanged(int index)
{
    int currentRig = 0;
    RIG *rig;

    if (index)
    {
        QString rigModel = ui->comboBox_rigModel->currentText();
        currentRig = findRigModel(rigModel);
    }

    if (currentRig)
    {
        rig = rig_init(currentRig);
        if (rig->caps->port_type == RIG_PORT_SERIAL)
        {
            ui->checkBox_netRigctl->setChecked(false);
            ui->tabWidget_Config->setCurrentIndex(0);
            setDialogSerialConfig(rig->caps->serial_data_bits, rig->caps->serial_parity, rig->caps->serial_stop_bits, rig->caps->serial_handshake);
            //qDebug() << rig->caps->serial_data_bits << rig->caps->serial_parity << rig->caps->serial_stop_bits << rig->caps->serial_handshake;
        }
        else if (rig->caps->port_type == RIG_PORT_NETWORK)
        {
            ui->checkBox_netRigctl->setChecked(true);
            ui->tabWidget_Config->setCurrentIndex(1);
        }
        else if (rig->caps->port_type == RIG_PORT_NONE)
        {
            ui->checkBox_netRigctl->setChecked(false);
            ui->tabWidget_Config->setCurrentIndex(0);
            ui->comboBox_comPort->clear();
        }
    }

    //if (index == 2 || index == 3 || index == 4)
    //{
    //    ui->checkBox_netRigctl->setChecked(true);
    //    ui->tabWidget_Config->setCurrentIndex(1);
    //}
}

void DialogConfig::on_comboBox_comPort_currentIndexChanged(int index)
{
    if (index) ui->checkBox_netRigctl->setChecked(false);   //uncheck TCP
}
