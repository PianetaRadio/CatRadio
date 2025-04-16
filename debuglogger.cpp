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


#include "debuglogger.h"

#include <QDateTime>
#include <QTextStream>
#include <QDebug>


QFile debugLogger::logFile;
QMutex debugLogger::mutex;
QtMsgType debugLogger::currentDebugLevel = QtInfoMsg;   //defaul debug level


debugLogger::debugLogger() {}


void debugLogger::install(QString fileName) //Install the debug message handler
{
    logFile.setFileName(fileName);    //Set debug log file
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Text)) logFile.close();

    qInstallMessageHandler(debugLogger::messageHandler);    //Message handler
}


void debugLogger::setDebugLevel(QtMsgType level)    //Select debug level
{
    //QMutexLocker locker(&mutex);
    currentDebugLevel = level;
}


void debugLogger::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) //Message handler
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzztt");
    QString txt;

    QString contextInfo = QString(" (%1:%2 %3 [%4])")
        .arg(context.file ? context.file : "unknown")
        .arg(context.line)
        .arg(context.function ? context.function : "unknown")
        .arg(context.category ? context.category : "default");

    switch (type)
    {
    case QtDebugMsg:    //0
        fprintf(stderr, "[Debug] %s\n", msg.toLocal8Bit().constData());
        return;
    case QtInfoMsg:     //4
        txt = QString("%1: [Info] %2 %3").arg(timestamp, msg, contextInfo);
        break;
    case QtWarningMsg:  //1
        txt = QString("%1: [Warning] %2 %3").arg(timestamp, msg, contextInfo);
        break;
    case QtCriticalMsg: //2
        txt = QString("%1: [Critical] %2 %3").arg(timestamp, msg, contextInfo);
        break;
    case QtFatalMsg:    //3
        txt = QString("%1: [Fatal] %2 %3").arg(timestamp, msg, contextInfo);
        break;
    }

    QMutexLocker locker(&mutex);

    if (type <= currentDebugLevel)
    {
        if (logFile.open(QIODevice::Append | QIODevice::Text))
        {
            QTextStream out(&logFile);
            out << txt << Qt::endl;
            logFile.close();
        }
    }

    if (type == QtFatalMsg)
    {
        abort();
    }
}
