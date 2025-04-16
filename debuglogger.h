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


#ifndef DEBUGLOGGER_H
#define DEBUGLOGGER_H

#include <QString>
#include <QFile>
#include <QMutex>
#include <QtGlobal>


class debugLogger
{
public:
    debugLogger();
    static void install(QString fileName);
    static void setDebugLevel(QtMsgType level);

private:
    static QFile logFile;
    static QMutex mutex;
    static QtMsgType currentDebugLevel;

    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
};

#endif // DEBUGLOGGER_H
