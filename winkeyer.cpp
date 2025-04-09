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


#include "winkeyer.h"

#include <QSerialPort>
#include <QDebug>


int WinKeyer::init(QString portName)    //Serial port init
{
    serial = new QSerialPort();

    if (serial && !serial->isOpen())
    {
        //Set serial port parameters
        serial->setPortName(portName);
        serial->setBaudRate(QSerialPort::Baud1200);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);

        //Open the serial port
        serial->open(QIODevice::ReadWrite);

        return serial->error();
    }
    return QSerialPort::OpenError;
}

int WinKeyer::open()    //Open WK
{
    if (serial->isOpen())
    {
        //Command mode \x00 and Host open \x02
        QByteArray dataToSend = QByteArray::fromRawData("\x00\x02", 2);
        serial->write(dataToSend);

        if (serial->waitForBytesWritten(1000))
        {
            if (serial->waitForReadyRead(1000))
            {
                QByteArray receivedData = serial->readAll(); //receive the WK revision code

                bool okInt;
                int intValue = receivedData.toHex().toInt(&okInt, 16);
                if (okInt)
                {
                    isOpen = true;
                    return intValue;
                }
                else
                {
                    isOpen = false;
                    return 0;
                }
            }
        }
    }
    return 0;
}

void WinKeyer::close()  //Close WK and serial port
{
    if (serial->isOpen())
    {
        //Command mode x00 and Host close \x03
        QByteArray dataToSend = QByteArray::fromRawData("\x00\x03", 2);
        serial->write(dataToSend);

        //Close the serial port
        serial->close();
        delete serial;
        serial = nullptr;

        isOpen = false;
    }
}

void WinKeyer::setWpmSpeed(int wpm)
{
    if (serial->isOpen())
    {
        if (wpm < 5) wpm = 5;
        else if (wpm > 99) wpm = 99;

        //Set WPM speed \x02 + \xnn WPM
        QByteArray dataToSend = QByteArray::fromRawData("\x02", 1);
        dataToSend.append(intToByte(wpm));

        serial->write(dataToSend);
    }
}

void WinKeyer::sendString(QByteArray string)
{
    if (serial->isOpen())
    {
        serial->write(string.toUpper());
    }
}

QByteArray WinKeyer::intToByte (int value)
{
    QByteArray byteArray;
    byteArray.append(static_cast<char>(value & 0xFF));
    return byteArray;
}
