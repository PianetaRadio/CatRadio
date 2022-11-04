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
    bool netRigctl; //TCP NET Rigctl
    unsigned rigRefresh;    //GUI refresh interval (ms)
    int connected;  //connected flag
    bool fullPoll;  //full polling flag
} rigConnect;

typedef struct {
    powerstat_t onoff;  //on/off status
    freq_t freqMain, freqSub;    //Frequency (Hz)
    int band;    //Band
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
    unsigned long long meter; //secondary meter type
    float rfPower;    //RF power output level
    float rfGain;   //RF gain rx
    float afGain;   //AF gain level
    float squelch;  //Squelch level
    agc_level_e agc;    //AGC level
    int att;    //Attenuator
    int pre;    //Preamplifier
    ant_t ant, antTx, antRx;    //Antenna
    int rangeListTxIndex, rangeListRxIndex;
    int tuner;  //Tuner
    int bkin;   //CW Break-in
    int wpm;    //CW Keyer speed WPM
    int apf;    //Audio Peak Filter
    int noiseBlanker;
    int noiseReduction, noiseReductionLevel;
    int notchFilter;
    int ifShift;
    int clar, rit, xit;   //Clarifier Rx or Tx
    shortfreq_t ritOffset, xitOffset;   //Clarifier offset (Hz)
    rptr_shift_t rptShift;  //Repeater shift
    shortfreq_t rptOffset;  //Repeater offset (Hz)
    int toneType;   //0 none, 1 burst 1750, 2 CTCSS, 3 CTCSS SQL, 4 DCS
    tone_t tone;    //CTCSS or DCS tone
    int micComp, micMon;    //Flag MIC Compressor, MIC Monitor
    float micGain, micMonLevel;  //Level MIC Gain, MIC Monitor
} rigSettings;

typedef struct {
    int onoff;
    int freqMain, freqSub;
    int mode, modeSub;
    int bwidth;
    int vfo;
    int split;
    int vfoXchange, vfoCopy;
    int vfoDown, vfoUp;
    int ptt;
    int rfPower;
    int rfGain;
    int afGain;
    int squelch;
    int agc;
    int att;
    int pre;
    int ant;
    int tuner;
    int tune;
    int bandUp, bandDown;
    int bandChange;
    int bkin;
    int wpm;
    int apf;
    int noiseBlanker;
    int noiseReduction, noiseReductionLevel;
    int notchFilter;
    int ifShift;
    int clar;
    int rptShift, rptOffset;
    int tone;
    int micComp, micMon;
    int micGain, micMonLevel;
} rigCommand;
