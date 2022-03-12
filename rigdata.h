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


#ifndef RIGDATA_H
#define RIGDATA_H

#endif // RIGDATA_H

#include <rig.h>    //Hamlib

#include <QString>

typedef struct {
    unsigned rigModel;  //Hamlib rig model
    QString rigPort;    //COM port or IP address
    unsigned serialSpeed;   //Serial port baud rate
    int civAddr;  //CI-V address (decimal, Icom radio)
    unsigned rigRefresh;    //GUI refresh interval (ms)
    int connected;  //connected flag
    bool fullPoll;  //full polling flag
} rigConnect;

typedef struct {
    powerstat_t onoff;  //on/off status
    freq_t freqMain, freqSub;    //Frequency
    rmode_t mode, modeSub;  //Mode
    pbwidth_t bwidth, bwidthSub;    //IF filter bandwidth
    int bwNarrow;   //IF narrow filter
    vfo_t vfoMain, vfoSub;  //VFO
    vfo_t vfoTx;
    split_t split;  //Split
    ptt_t ptt;  //PTT
    value_t sMeter; //Smeter signal strenght
    value_t powerMeter; //RF power meter
    value_t subMeter; //secondary meter
    unsigned meter; //secondary meter type
    float rfPower;    //RF power output level
    float rfGain;   //RF gain rx
    agc_level_e agc;    //AGC level
    int att;    //Attenuator
    int pre;    //Preamplifier
    ant_t ant, antTx, antRx;    //Antenna
    int rangeListTxIndex, rangeListRxIndex;
    int tuner;  //Tuner
    int bkin;   //CW Break-in
    int apf;
    int noiseBlanker;
    int noiseReduction, noiseReductionLevel;
    int notchFilter;
    int ifShift;
} rigSettings;

typedef struct {
    int onoff;
    int freqMain;
    int freqSub;
    int mode;
    int bwidth;
    int bwidthList;
    int vfo;
    int split;
    int vfoXchange;
    int vfoCopy;
    int ptt;
    int rfPower;
    int rfGain;
    int agc;
    int att;
    int pre;
    int ant;
    int antList;
    int rangeList;
    int tuner;
    int tune;
    int bandUp;
    int bandDown;
    int bandChange;
    int bkin;
    int apf;
    int noiseBlanker;
    int noiseReduction, noiseReductionLevel;
    int notchFilter;
    int ifShift;
} rigCommand;
