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


DialogCommand::DialogCommand(RIG *rig, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCommand)
{
    ui->setupUi(this);
    my_rig = rig;
}

DialogCommand::~DialogCommand()
{
    delete ui;
}

void DialogCommand::on_pushButton_send_clicked()
{


    QString sendCmdS = ui->lineEdit_commandSend->text();
    const unsigned char *sendCmd = (unsigned char*)sendCmdS.toLatin1().data();
    int sendCmdLen = sendCmdS.size();
    unsigned char termCmd[] = ";";

    unsigned char rcvdCmd[100];

    int rcvdCmdLen = sizeof(rcvdCmd);

    //qDebug()<<sendCmdS<<(char*)termCmd<<(char*)sendCmd<<sendCmdLen;

    rig_send_raw(my_rig, sendCmd, sendCmdLen, rcvdCmd, rcvdCmdLen, termCmd);

    //QString rcvdCmdS = rcvdCmd;

    qDebug()<<(char *)rcvdCmd;


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
