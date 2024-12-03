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


#ifndef GUIDATA_H
#define GUIDATA_H

#endif // GUIDATA_H

#include <QString>


typedef struct {
    int vfoDisplayMode; //0: use Left/Right mouse button, 1: click digit Up or Down
    bool darkTheme; //flag for Dark theme
    bool peakHold;  //meters peak hold
    bool debugMode; //flag for debug log
    int cwKeyerMode;    //0: Radio
    int voiceKeyerMode; //0: Radio, 1: CatRadio
} guiConfig;


typedef struct {
    int bwidthList;
    int antList;
    int rangeList;
    int tabList;
    int toneList;
} guiCommand;


typedef struct {
    QString memoryFile[5];  //Audio files
    QString audioOutput;    //Audio output device name
    int audioOutputVolume;    //Audio output volume integer 0..10
} voiceKeyerConfig;
