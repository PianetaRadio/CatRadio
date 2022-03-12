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


#include "rigdaemon.h"
#include "rigdata.h"
#include "rigcommand.h"

#include <QThread>
#include <QDebug>
#include <QMessageBox>

#include <rig.h>


RIG *my_rig;

extern rigConnect rigCom;
extern rigSettings rigGet;
extern rigSettings rigSet;
extern rigCommand rigCmd;
extern rigCommand rigCap;

int commandPriority = 0;


RigDaemon::RigDaemon(QObject *parent) : QObject(parent)
{

}

int RigDaemon::rigConnect()
{
    int retcode;

    my_rig = rig_init(rigCom.rigModel); //Allocate rig handle

    if (!my_rig)    //Wrong Rig number
    {
        QMessageBox msgBox; //Show error MessageBox
        msgBox.setWindowTitle("Warning");
        msgBox.setText("Rig model error");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();

        return -1;   //RIG_EINVAL, Invalid parameter
    }
    else
    {
        if (rigCom.rigModel == 2)   //Rigctld
        {
            //myport.type.rig = RIG_PORT_NETWORK;
            strncpy(my_rig->state.rigport.pathname, rigCom.rigPort.toLatin1(), HAMLIB_FILPATHLEN - 1);
            //strncpy(my_rig->state.rigport.pathname, RIG_FILE, HAMLIB_FILPATHLEN - 1);
        }
        else
        {
            //myport.type.rig = RIG_PORT_SERIAL;
            strncpy(my_rig->state.rigport.pathname, rigCom.rigPort.toLatin1(), HAMLIB_FILPATHLEN - 1);
            my_rig->state.rigport.parm.serial.rate = rigCom.serialSpeed;
            if (rigCom.civAddr) //CI-V address Icom
            {
                std::string civaddrS = std::to_string(rigCom.civAddr);  //Convert int to string
                char const *civaddr = civaddrS.c_str(); //Convert string to char*
                rig_set_conf(my_rig, rig_token_lookup(my_rig, "civaddr"), civaddr);
            }
        }

        retcode = rig_open(my_rig);

        if (retcode != RIG_OK) return retcode;  //Rig not connected
        else    //Rig connected
        {
            rig_get_powerstat(my_rig, &rigGet.onoff);
            return 0;
        }

     }
}

void RigDaemon::rigUpdate()
{
    int retcode;
    value_t retvalue;

    //***** Command execution *****
    //* PTT
    if (rigCmd.ptt)
    {
        retcode = rig_set_ptt(my_rig, RIG_VFO_CURR, rigSet.ptt);
        if (retcode == RIG_OK) rigGet.ptt = rigSet.ptt;
        rigCmd.ptt = 0;
    }

    //* VFO
    if (rigCmd.freqMain)   //VFO Main
    {
        retcode = rig_set_freq(my_rig, RIG_VFO_CURR, rigSet.freqMain);
        if (retcode == RIG_OK) rigGet.freqMain = rigSet.freqMain;
        rigCmd.freqMain = 0;
        rigCmd.rangeList = 1;
    }

    else if (rigCmd.freqSub)   //VFO Sub
    {
        retcode = rig_set_freq(my_rig, rigGet.vfoSub, rigSet.freqSub);
        if (retcode == RIG_OK) rigGet.freqSub = rigSet.freqSub;
        rigCmd.freqSub = 0;
    }

    else
    {
        freq_t retfreq;
        retcode = rig_get_freq(my_rig, RIG_VFO_CURR, &retfreq);
        if (retcode == RIG_OK) rigGet.freqMain = retfreq;
        retcode = rig_get_freq(my_rig, rigGet.vfoSub, &retfreq);
        if (retcode == RIG_OK) rigGet.freqSub = retfreq;

        ptt_t retptt;
        retcode = rig_get_ptt(my_rig, RIG_VFO_CURR, &retptt);
        if (retcode == RIG_OK) rigGet.ptt = retptt;

        //* Meter
        if (rigGet.ptt==1)
        {
            rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_RFPOWER_METER, &rigGet.powerMeter);
            rig_get_level(my_rig, RIG_VFO_CURR, rigSet.meter, &rigGet.subMeter);
        }
        else
        {
            retcode = rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_STRENGTH, &retvalue);
            if (retcode == RIG_OK) rigGet.sMeter = retvalue;
        }

        if (!rigGet.ptt)
        {
            //* Mode
            if (rigCmd.mode)
            {
                retcode = rig_set_mode(my_rig, RIG_VFO_CURR, rigSet.mode, RIG_PASSBAND_NOCHANGE);
                if (retcode == RIG_OK)
                {
                    rigCmd.bwidthList = 1;   //Update BWidth list
                    commandPriority = 0;
                    //rig_get_mode(my_rig, RIG_VFO_CURR, &rigGet.mode, &rigGet.bwidth);   //Get BW
                }
                rigCmd.mode = 0;
            }

            //* BandWidth
            if (rigCmd.bwidth)
            {
                retcode = rig_set_mode(my_rig, RIG_VFO_CURR, rigGet.mode, rigSet.bwidth);
                if (retcode == RIG_OK) rigGet.bwidth = rigSet.bwidth;
                rigCmd.bwidth = 0;
            }

            //* Split
            if (rigCmd.split)
            {
                retcode = rig_set_split_vfo(my_rig, RIG_VFO_RX, rigSet.split, RIG_VFO_TX);
                if (retcode == RIG_OK) rigGet.split = rigSet.split;
                rigCmd.split = 0;
            }

            //* VFO Exchange
            if (rigCmd.vfoXchange)
            {
                retcode = rig_vfo_op(my_rig, RIG_VFO_CURR, RIG_OP_XCHG);
                if (retcode == RIG_OK) commandPriority = 0;
                rigCmd.vfoXchange = 0;
            }

            //* VFO Copy
            if (rigCmd.vfoCopy)
            {
                rig_vfo_op(my_rig, RIG_VFO_CURR, RIG_OP_CPY);
                rigCmd.vfoCopy = 0;
            }

            //* Band Up
            if (rigCmd.bandUp)
            {
                retcode = rig_vfo_op(my_rig, RIG_VFO_CURR, RIG_OP_BAND_UP);
                if (retcode == RIG_OK) commandPriority = 0;
                rigCmd.bandUp = 0;
            }

            //* Band Down
            if (rigCmd.bandDown)
            {
                retcode = rig_vfo_op(my_rig, RIG_VFO_CURR, RIG_OP_BAND_DOWN);
                if (retcode == RIG_OK) commandPriority = 0;
                rigCmd.bandDown = 0;
            }

            //* Band change
            if (rigCmd.bandChange) commandPriority = 0;

            //* Tune
            if (rigCmd.tune)
            {
                rig_vfo_op(my_rig, RIG_VFO_CURR, RIG_OP_TUNE);
                rigCmd.tune = 0;
            }

            //* Antenna
            if (rigCmd.ant)
            {
                retcode = rig_set_ant(my_rig, RIG_VFO_CURR, rigSet.ant, retvalue);
                if (retcode == RIG_OK) rigGet.ant = rigSet.ant;
                rigCmd.ant = 0;
            }

            //* AGC
            if (rigCmd.agc)
            {
                retcode = rig_set_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_AGC, valueagclevel(rigSet.agc));
                if (retcode == RIG_OK) rigGet.agc = rigSet.agc;
                rigCmd.agc = 0;
            }

            //* Attenuator
            if (rigCmd.att)
            {
                retvalue.i = rigSet.att;
                retcode = rig_set_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_ATT, retvalue);
                if (retcode == RIG_OK) rigGet.att = rigSet.att;
                rigCmd.att = 0;
            }

            //* Preamp
            if (rigCmd.pre)
            {
                retvalue.i = rigSet.pre;
                retcode = rig_set_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_PREAMP, retvalue);
                if (retcode == RIG_OK) rigGet.pre = rigSet.pre;
                rigCmd.pre = 0;
            }

            //* RF gain
            if (rigCmd.rfGain)
            {
                retvalue.f = rigSet.rfGain;
                retcode = rig_set_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_RF, retvalue);
                if (retcode == RIG_OK) rigGet.rfGain = rigSet.rfGain;
                rigCmd.rfGain = 0;
            }
            
            //* NB noise blanker
            if (rigCmd.noiseBlanker)
            {
                retcode = rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_NB, rigSet.noiseBlanker);
                if (retcode == RIG_OK) rigGet.noiseBlanker = rigSet.noiseBlanker;
                rigCmd.noiseBlanker = 0;
            }

            //* NR noise reduction
            if (rigCmd.noiseReduction)
            {
                retcode = rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_NR, rigSet.noiseReduction);
                if (retcode == RIG_OK) rigGet.noiseReduction = rigSet.noiseReduction;
                rigCmd.noiseReduction = 0;
            }

            if (rigCmd.noiseReductionLevel)
            {
                retvalue.i = rigSet.noiseReductionLevel;
                retcode = rig_set_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_NR, retvalue);
                if (retcode == RIG_OK) rigGet.noiseReductionLevel = rigSet.noiseReductionLevel;
                rigCmd.noiseReductionLevel = 0;
            }

            //* NF notch filter
            if (rigCmd.notchFilter)
            {
                retcode = rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_ANF, rigSet.notchFilter);
                if (retcode == RIG_OK) rigGet.notchFilter = rigSet.notchFilter;
                rigCmd.notchFilter = 0;
            }

            //* IF Shift
            if (rigCmd.ifShift)
            {
                retvalue.i = rigSet.ifShift;
                retcode = rig_set_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_IF, retvalue);
                if (retcode == RIG_OK) rigGet.ifShift = rigSet.ifShift;
                rigCmd.ifShift = 0;
            }
         }  //end if (!rigGet.ptt)

        //* Tuner
        if (rigCmd.tuner)
        {
            retcode = rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_TUNER, rigSet.tuner);
            if (retcode == RIG_OK) rigGet.tuner = rigSet.tuner;
            rigCmd.tune = 0;
        }

        //* RF power
        if (rigCmd.rfPower)
        {
            retvalue.f = rigSet.rfPower;
            retcode = rig_set_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_RFPOWER, retvalue);
            if (retcode == RIG_OK) rigGet.rfPower = rigSet.rfPower;
            rigCmd.rfPower = 0;
        }

    //***** Poll execution *****
        if ((commandPriority == 1 && !rigGet.ptt && rigCom.fullPoll) || commandPriority == 0)
        {
            //* Mode and BW
            rig_get_mode(my_rig, RIG_VFO_CURR, &rigGet.mode, &rigGet.bwidth);
            //rig_get_mode(my_rig, rigGet.vfoSub, &rigGet.modeSub, &rigGet.bwidthSub);
            if (rigGet.bwidth == rig_passband_narrow(my_rig, rigGet.mode)) rigGet.bwNarrow = 1;
            else rigGet.bwNarrow = 0;
        }

        if ((commandPriority == 2 && !rigGet.ptt && rigCom.fullPoll) || commandPriority == 0)
        {
            //* Split
            rig_get_split_vfo(my_rig, RIG_VFO_CURR, &rigGet.split, &rigGet.vfoTx);
            //else rig_get_split(my_rig, RIG_VFO_CURR, &rigGet.split);

            rig_get_vfo(my_rig, &rigGet.vfoMain);
        }

        //* Tuner
        if ((commandPriority == 3 && !rigGet.ptt && rigCom.fullPoll) || commandPriority == 0) rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_TUNER, &rigGet.tuner);

        //* Antenna
        if ((commandPriority == 4 && !rigGet.ptt && rigCom.fullPoll) || commandPriority == 0) rig_get_ant(my_rig, RIG_VFO_CURR, RIG_ANT_CURR, &retvalue, &rigGet.ant, &rigGet.antTx, &rigGet.antRx);

        //* AGC
        if ((commandPriority == 5 && !rigGet.ptt && rigCom.fullPoll) || commandPriority == 0)
        {
            rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_AGC, &retvalue);
            rigGet.agc = levelagcvalue(retvalue.i);
        }

        //* Attenuator
        if ((commandPriority == 6 && !rigGet.ptt && rigCom.fullPoll) || commandPriority == 0)
        {
            rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_ATT, &retvalue);
            rigGet.att = retvalue.i;
        }

        //* Preamp
        if ((commandPriority == 7 && !rigGet.ptt && rigCom.fullPoll) || commandPriority == 0)
        {
            rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_PREAMP, &retvalue);
            rigGet.pre = retvalue.i;
        }

        //* RF power
        if ((commandPriority == 8 && !rigGet.ptt && rigCom.fullPoll) || commandPriority == 0)
        {
            rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_RFPOWER, &retvalue);
            rigGet.rfPower = retvalue.f;
        }

        //* RF gain
        if ((commandPriority == 9 && !rigGet.ptt && rigCom.fullPoll) || commandPriority == 0)
        {
            rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_RF, &retvalue);
            rigGet.rfGain = retvalue.f;
        }

        //* NB noise blanker
        if ((commandPriority == 10 && !rigGet.ptt && rigCom.fullPoll) || commandPriority == 0) rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_NB, &rigGet.noiseBlanker);

        //* NR noise reduction
        if ((commandPriority == 11 && !rigGet.ptt && rigCom.fullPoll) || commandPriority == 0)
        {
            rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_NR, &rigGet.noiseReduction);
            rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_NR, &retvalue);
            rigGet.noiseReductionLevel = retvalue.i;
        }

        //* NF notch filter
        if ((commandPriority == 12 && !rigGet.ptt && rigCom.fullPoll) || commandPriority == 0) rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_ANF, &rigGet.notchFilter);

        //* IF Shift
        if ((commandPriority == 13 && !rigGet.ptt && rigCom.fullPoll) || commandPriority == 0)
        {
            rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_IF, &retvalue);
            rigGet.ifShift = retvalue.i;
        }

    if ((commandPriority == 14 && !rigGet.ptt && rigCom.fullPoll) || commandPriority == 0) //&& mode=CW
    {
        //* CW Full Break-in
        if (rigCmd.bkin)
        {
            retcode = rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_FBKIN, rigSet.bkin);
            if (retcode == RIG_OK) rigGet.bkin = rigSet.bkin;
            rigCmd.bkin = 0;
        }
        else rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_FBKIN, &rigGet.bkin);
    }

    commandPriority ++;
    if (commandPriority == 15) commandPriority = 1;

    }

    emit resultReady();
}
