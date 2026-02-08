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


#ifndef RIGDAEMON_H
#define RIGDAEMON_H

#include <QObject>
#include <rig.h>


class RigDaemon : public QObject
{
    Q_OBJECT

public:
    explicit RigDaemon(QObject *parent = nullptr);
    RIG *rigConnect(unsigned rigModel, QString rigPort, unsigned serialSpeed, unsigned serialDataBits, unsigned serialParity, unsigned serialStopBits, unsigned serialHandshake, int civAddr, bool autoPowerOn, int *retcode);
    RIG *rigConnect(unsigned rigModel, QString rigPort, bool autoPowerOn, int *retcode);
    void rigUpdate(RIG *my_rig, bool fullPoll);

public slots:

signals:
    void resultReady();

};

#endif // RIGDAEMON_H
