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


#ifndef WINKEYER_H
#define WINKEYER_H

#include <QSerialPort>


class WinKeyer
{
public:
    int version;    //WinKeyer version
    bool isOpen;    //WinKeyer open flag

    int init(QString portName);
    int open();
    void close();
    void setWpmSpeed(int wpm);
    void sendString(QByteArray string);

private:
    QSerialPort *serial = nullptr;

    QByteArray intToByte (int value);    //Convert int (0 - 255) into a 1 byte QByteArray
};

#endif // WINKEYER_H
