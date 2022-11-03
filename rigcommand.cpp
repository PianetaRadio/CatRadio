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


#include "rigcommand.h"
#include "rigdata.h"

#include <rig.h>    //Hamlib

#include <QDebug>

extern rigSettings rigGet;
extern rigSettings rigSet;
extern rigCommand rigCmd;
extern rigCommand rigCap;

void set_band (int band)
{
    if (rigCap.bandChange==0)
    {
        freq_t freq;

        switch (band)
        {
        case 160: freq = 1840000; break;
        case 80: freq = 3600000; break;
        case 60: freq = 5355000; break;
        case 40: freq = 7100000; break;
        case 30: freq = 10130000; break;
        case 20: freq = 14100000; break;
        case 17: freq = 18120000; break;
        case 15: freq = 21150000; break;
        case 12: freq = 24940000; break;
        case 10: freq = 28320000; break;
        case 6: freq = 50150000; break;
        case 2: freq = 144300000; break;
        case 70: freq = 432200000; break;
        case 1: freq = 5000000; break;  //General
        default: freq = rigSet.freqMain; break;
        }

        rigSet.freqMain = freq;
        rigCmd.freqMain = 1;
    }
    else
    {
        int bandt;
        switch (band)
        {
        case 160: bandt = RIG_BAND_160M; break;
        case 80: bandt = RIG_BAND_80M; break;
        case 60: bandt = RIG_BAND_60M; break;
        case 40: bandt = RIG_BAND_40M; break;
        case 30: bandt = RIG_BAND_30M; break;
        case 20: bandt = RIG_BAND_20M; break;
        case 17: bandt = RIG_BAND_17M; break;
        case 15: bandt = RIG_BAND_15M; break;
        case 12: bandt = RIG_BAND_12M; break;
        case 10: bandt = RIG_BAND_10M; break;
        case 6: bandt = RIG_BAND_6M; break;
        case 2: bandt = RIG_BAND_144MHZ; break;
        case 70: bandt = RIG_BAND_430MHZ; break;
        case 1: bandt = RIG_BAND_GEN; break;
        default: bandt = RIG_BAND_20M; break;
        }

        rigSet.band = bandt;
        rigCmd.bandChange = 1;
    }
}

void quick_split ()
{
    rigSet.freqSub = rigGet.freqMain + 5000;
    rigSet.modeSub = rigGet.mode;
    rigSet.split = RIG_SPLIT_ON;
    rigCmd.freqSub = 1;
    rigCmd.modeSub = 1;
    rigCmd.split = 1;
}

agc_level_e levelagcvalue (int agcValue)
{
    agc_level_e agcLevel;

    switch (agcValue)
    {
    case 0: agcLevel = RIG_AGC_OFF; break;
    case 1: agcLevel = RIG_AGC_SUPERFAST; break;
    case 2: agcLevel = RIG_AGC_FAST; break;
    case 3: agcLevel = RIG_AGC_SLOW; break;
    case 4: agcLevel = RIG_AGC_USER; break;
    case 5: agcLevel = RIG_AGC_MEDIUM; break;
    case 6: agcLevel = RIG_AGC_AUTO; break;
    default: agcLevel = RIG_AGC_AUTO; break;
    }

    return agcLevel;
}

agc_level_e levelagcstr (QString agcString)
{
    agc_level_e agcLevel;

    if (agcString == "OFF") agcLevel = RIG_AGC_OFF;
    else if (agcString == "SUPERFAST") agcLevel = RIG_AGC_SUPERFAST;
    else if (agcString == "FAST") agcLevel = RIG_AGC_FAST;
    else if (agcString == "SLOW") agcLevel = RIG_AGC_SLOW;
    else if (agcString == "USER") agcLevel = RIG_AGC_USER;
    else if (agcString == "MEDIUM") agcLevel = RIG_AGC_MEDIUM;
    else agcLevel = RIG_AGC_AUTO;

    return agcLevel;
}

value_t valueagclevel (agc_level_e agcLevel)
{
    value_t value;

    if (agcLevel == RIG_AGC_OFF) value.i = 0;
    else if (agcLevel == RIG_AGC_SUPERFAST) value.i = 1;
    else if (agcLevel == RIG_AGC_FAST) value.i = 2;
    else if (agcLevel == RIG_AGC_SLOW) value.i = 3;
    else if (agcLevel == RIG_AGC_USER) value.i = 4;
    else if (agcLevel == RIG_AGC_MEDIUM) value.i = 5;
    else value.i = 6; //RIG_AGC_AUTO

    return value;
}

ant_t antstr (QString antString)
{
    ant_t ant;

    if (antString == "NONE") ant = RIG_ANT_NONE;
    else if (antString == "ANT1") ant = RIG_ANT_1;
    else if (antString == "ANT2") ant = RIG_ANT_2;
    else if (antString == "ANT3") ant = RIG_ANT_3;
    else if (antString == "ANT4") ant = RIG_ANT_4;
    else if (antString == "ANT5") ant = RIG_ANT_5;
    else if (antString == "UNK") ant = RIG_ANT_UNKNOWN;
    else if (antString == "CURR") ant = RIG_ANT_CURR;
    else ant = RIG_ANT_UNKNOWN;

    return ant;
}

unsigned long long levelmeterstr (QString meterString)
{
    unsigned long long levelMeter;

    if (meterString == "SWR") levelMeter = RIG_LEVEL_SWR;
    else if (meterString == "ALC") levelMeter = RIG_LEVEL_ALC;
    else if (meterString == "COMP") levelMeter = RIG_LEVEL_COMP;
    else if (meterString == "ID") levelMeter = RIG_LEVEL_ID_METER;
    else if (meterString == "VDD") levelMeter = RIG_LEVEL_VD_METER;
    else levelMeter = RIG_METER_NONE;
    return levelMeter;
}
