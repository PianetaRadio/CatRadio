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


#include "dialogcommand.h"
#include "ui_dialogcommand.h"

#include "rig.h"

#include <stdio.h>
#include <string.h>

#include <QDebug>


DialogCommand::DialogCommand(RIG *rig, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCommand)
{
    ui->setupUi(this);
    my_rig = rig;
    int backend = RIG_BACKEND_NUM(my_rig->caps->rig_model);

    if (backend == RIG_YAESU || backend == RIG_KENWOOD) ui->radioButton_yaesu->setChecked(true);
    else if (backend == RIG_ICOM) ui->radioButton_icom->setChecked(true);
}

DialogCommand::~DialogCommand()
{
    delete ui;
}

void DialogCommand::on_pushButton_send_clicked()
{
    bool hex =  false;  //Flag for Hex command

    QString sendCmdS = ui->lineEdit_commandSend->text();
    QByteArray sendCmdA;
    if (sendCmdS.isEmpty()) return;
    if (sendCmdS.contains("0x", Qt::CaseInsensitive))   //Hex input
    {
        sendCmdS = sendCmdS.mid(2);
        sendCmdA = QByteArray::fromHex(sendCmdS.toLatin1());
        hex = true;
    }
    else sendCmdA = sendCmdS.toUtf8();  //Char input

    QByteArray termCmdA;
    termCmdA.resize(1);
    if (ui->radioButton_yaesu->isChecked()) termCmdA[0] = ';';
    else if (ui->radioButton_icom->isChecked()) termCmdA[0] = 0xfd;
    else if (ui->radioButton_CR->isChecked()) termCmdA[0] = 0x0d;
    else if (ui->radioButton_LF->isChecked()) termCmdA[0] = 0x0a;
    else termCmdA[0] = '\0';
    unsigned char *termCmd = (unsigned char*)termCmdA.data();

    sendCmdA.append(termCmdA);
    unsigned char *sendCmd = (unsigned char*)sendCmdA.data();
    int sendCmdLen = strlen((char*)sendCmd);

    unsigned char rcvdCmd[200];
    int rcvdCmdLen = sizeof(rcvdCmd);

    int retLen = rig_send_raw(my_rig, sendCmd, sendCmdLen, rcvdCmd, rcvdCmdLen, termCmd);

    if (retLen > 0)
    {
        QString rcvdCmdS;
        QByteArray rcvdCmdA(QByteArray::fromRawData((char *)rcvdCmd, retLen));
        if (hex) rcvdCmdS = rcvdCmdA.toHex();
        else rcvdCmdS = rcvdCmdA;
        ui->lineEdit_receive->setText(rcvdCmdS);
    }

    //int rig_send_raw(rig, const unsigned char *send, int send_len, unsigned char *reply, int reply_len, unsigned char *term);
    //send contains the raw command data
    //send_len is the # of bytes to send
    //If reply is NULL no answer is expected
    //reply should be as long as need for any reply
    //term is the command termination char -- could be semicolon, CR, or 0xfd for Icom rigs
}

void DialogCommand::on_pushButton_close_clicked()
{
    this->close();
}
