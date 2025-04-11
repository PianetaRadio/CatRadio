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


#ifndef GUIDATA_H
#define GUIDATA_H

#endif // GUIDATA_H

#include <QString>


typedef struct {
    int vfoDisplayMode; //0: use Left/Right mouse button, 1: click digit Up or Down
    int vfoDialStep[6][4];  //VFO dial step
                            //           0      1            2      3
                            //           Single Page    Fast Single Page
                            //0 Current
                            //1 SSB
                            //2 CW
                            //3 DIGI
                            //4 FM
                            //5 AM
    bool darkTheme; //flag for Dark theme
    bool peakHold;  //meters peak hold
    bool debugMode; //flag for debug log
    int cwKeyerMode;    //0: Radio, 1: WinKeyer
    int voiceKeyerMode; //0: Radio, 1: CatRadio
} guiConfig;


typedef struct {
    int bwidthList; //Flag to command the update of the filter bandwidth combo box list
    int antList;    //Flag to command the update of the antenna combo box list
    int rangeList;  //Flag to command the  update the rig range list
    int tabList;    //Flag to select the right function tab
    int toneList;   //Flag to command the update of the tone combo box list
    int dialConf;   //Flag to command the update of the tuning dial step configuration
} guiCommand;


typedef struct {
    QString memoryFile[5];  //Audio files
    QString audioOutput;    //Audio output device name
    int audioOutputVolume;    //Audio output volume integer 0..10
} voiceKeyerConfig;


typedef struct {
    QString comPort;    //COM port name
    QByteArray memoryString[5];   //CW strings
    int wpm;    //WPM
    bool autoConnect; //Auto-connect flag
} cwKeyerConfig;
