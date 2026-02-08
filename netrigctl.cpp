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


#include "netrigctl.h"
#include "qdebug.h"

#include <QProcess>


netRigCtl::netRigCtl()
{
    if (!netrigctld) netrigctld = new QProcess();
}

void netRigCtl::open()
{
    qInfo() << "Opening rigctld";
    if (!netrigctld->isOpen())
    {
#ifdef Q_OS_WIN
        if (debugMode) netrigctld->setProgram("cmd.exe");
        else netrigctld->setProgram("rigctld.exe");
#else
        netrigctld->setProgram("rigctld");
#endif
        netrigctld->setArguments(rigctldArguments);
        netrigctld->open();
        qWarning() << netrigctld->error();
        qInfo() << netrigctld->state();
        if (netrigctld->state()==QProcess::Running)
        {
            if (netrigctld->waitForReadyRead(2000)) qInfo() << netrigctld->readAllStandardOutput().constData();
            isOpen = true;
        }
        else isOpen = false;
    }
}

void netRigCtl::close()
{
    if (netrigctld->isOpen()) netrigctld->close();
}

void netRigCtl::setRigctldArguments(unsigned rigModel, QString rigPort, unsigned serialSpeed, int civAddr, unsigned rigctldPort)
{
    rigctldArguments.clear();   //removes all previous elements
#ifdef Q_OS_WIN
    if (debugMode) rigctldArguments << "/c" << "start" << "cmd.exe" << "/k" << "rigctld.exe" << "-vvvv";
#endif
    rigctldArguments << "-m" << QString::number(rigModel) << "-t" << QString::number(rigctldPort) << "-o";
    if (rigPort!="")
    {
        rigctldArguments.append("-r " + rigPort);
        if (serialSpeed) rigctldArguments.append("-s " + QString::number(serialSpeed));
        //--set-conf not implemented
    }
    if (civAddr) rigctldArguments.append("-c " + QString::number(civAddr));
}
