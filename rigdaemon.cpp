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


#include "rigdaemon.h"
#include "rigdata.h"
#include "guidata.h"
#include "rigcommand.h"

#include <QThread>
#include <QDebug>
#include <QMessageBox>

#include <rig.h>

extern rigConnect rigCom;
extern rigSettings rigGet;
extern rigSettings rigSet;
extern rigCommand rigCmd;
extern rigCommand rigCap;
extern guiConfig guiConf;
extern guiCommand guiCmd;

int indexCmd = 0;


RigDaemon::RigDaemon(QObject *parent) : QObject(parent)
{

}

RIG *RigDaemon::rigConnect(int *retcode)
{
    RIG *my_rig = rig_init(rigCom.rigModel); //Allocate rig handle

    if (!my_rig)    //Wrong Rig number
    {
        QMessageBox msgBox; //Show error MessageBox
        msgBox.setWindowTitle("Warning");
        msgBox.setText("Rig model error");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();

        *retcode = RIG_EINVAL;   //RIG_EINVAL, Invalid parameter
        return nullptr;
    }
    else
    {
        //if (rigCom.rigModel == 2)   //Rigctld
        if (my_rig->state.port_type == RIG_PORT_NETWORK)
        {
            strncpy(my_rig->state.rigport.pathname, rigCom.rigPort.toLatin1(), HAMLIB_FILPATHLEN - 1);
        }
        else    //RIG_PORT_SERIAL
        {
            strncpy(my_rig->state.rigport.pathname, rigCom.rigPort.toLatin1(), HAMLIB_FILPATHLEN - 1);
            my_rig->state.rigport.parm.serial.rate = rigCom.serialSpeed;
            my_rig->state.rigport.parm.serial.data_bits = rigCom.serialDataBits;
            if (rigCom.serialParity == 1) my_rig->state.rigport.parm.serial.parity = RIG_PARITY_ODD;
            else if (rigCom.serialParity == 2) my_rig->state.rigport.parm.serial.parity = RIG_PARITY_EVEN;
            else my_rig->state.rigport.parm.serial.parity = RIG_PARITY_NONE;
            my_rig->state.rigport.parm.serial.stop_bits = rigCom.serialStopBits;
            if (rigCom.serialHandshake == 1) my_rig->state.rigport.parm.serial.handshake = RIG_HANDSHAKE_XONXOFF;
            else if (rigCom.serialHandshake == 2) my_rig->state.rigport.parm.serial.handshake = RIG_HANDSHAKE_HARDWARE;
            else my_rig->state.rigport.parm.serial.handshake = RIG_HANDSHAKE_NONE;

            if (rigCom.civAddr) //CI-V address Icom
            {
                std::string civaddrS = std::to_string(rigCom.civAddr);  //Convert int to string
                char const *civaddr = civaddrS.c_str(); //Convert string to char*
                rig_set_conf(my_rig, rig_token_lookup(my_rig, "civaddr"), civaddr);
            }
        }

        if (rigCom.autoPowerOn) my_rig->state.auto_power_on = 1;

        *retcode = rig_open(my_rig);

        if (*retcode != RIG_OK) return nullptr;  //Rig not connected
        else    //Rig connected
        {
            if (rig_has_get_func(my_rig, RIG_FUNCTION_GET_POWERSTAT)) rig_get_powerstat(my_rig, &rigGet.onoff);
            //if (my_rig->caps->get_powerstat != NULL) rig_get_powerstat(my_rig, &rigGet.onoff);
            else rigGet.onoff = RIG_POWER_UNKNOWN;
            return my_rig;
        }
     }
}

void RigDaemon::rigUpdate(RIG *my_rig)
{
    int retcode;
    value_t retvalue;

    //***** Priority Command execution *****
    //* PTT
    if (rigCmd.ptt)
    {
        retcode = rig_set_ptt(my_rig, RIG_VFO_CURR, rigSet.ptt);
        if (retcode == RIG_OK) rigGet.ptt = rigSet.ptt;
        rigCmd.ptt = 0;
    }

    //* CW memory keyer (rig)
    if (guiConf.cwKeyerMode == 0 && rigCmd.cwSend && (rigGet.mode == RIG_MODE_CW || rigGet.mode == RIG_MODE_CWN || rigGet.mode == RIG_MODE_CWR))
    {
        //if (rig_has_get_func(my_rig, RIG_FUNCTION_SEND_MORSE)) rig_send_morse(my_rig, RIG_VFO_CURR, &rigSet.cwMem);
        retcode = rig_send_morse(my_rig, RIG_VFO_CURR, &rigSet.cwMem);
        if (retcode == RIG_OK) rigGet.ptt = RIG_PTT_ON; //assume PPT on if send_morse is ok
        rigCmd.cwSend = 0;
    }

    //* Voice memory keyer (rig)
    if (guiConf.voiceKeyerMode == 0 && rigCmd.voiceSend && (rigGet.mode == RIG_MODE_SSB || rigGet.mode == RIG_MODE_USB || rigGet.mode == RIG_MODE_LSB || rigGet.mode == RIG_MODE_AM || rigGet.mode == RIG_MODE_FM))
    {
        retcode = rig_send_voice_mem(my_rig, RIG_VFO_CURR, rigSet.voiceMem);
        if (retcode == RIG_OK) rigGet.ptt = RIG_PTT_ON; //assume PPT on if send_voice_mem is ok
        rigCmd.voiceSend = 0;
    }

    //* VFO
    if (rigCmd.freqMain)   //VFO Main
    {
        retcode = rig_set_freq(my_rig, RIG_VFO_CURR, rigSet.freqMain);
        if (retcode == RIG_OK) rigGet.freqMain = rigSet.freqMain;
        rigCmd.freqMain = 0;
        guiCmd.rangeList = 1;
    }

    else if (rigCmd.freqSub && rigCap.freqSub)   //VFO Sub
    {
        retcode = rig_set_freq(my_rig, rigGet.vfoSub, rigSet.freqSub);
        if (retcode == RIG_OK) rigGet.freqSub = rigSet.freqSub;
        rigCmd.freqSub = 0;
    }

    //***** Priority Poll execution *****
    else
    {
        //* PTT
        ptt_t retptt;
        retcode = rig_get_ptt(my_rig, RIG_VFO_CURR, &retptt);
        if (retcode == RIG_OK) rigGet.ptt = retptt;

        //* VFO
        freq_t retfreq;
        retcode = rig_get_freq(my_rig, RIG_VFO_CURR, &retfreq); //get VFO Main
        if (retcode == RIG_OK) rigGet.freqMain = retfreq;
        if (rigCap.freqSub)   //get sub VFO freq if targetable
        {
            retcode = rig_get_freq(my_rig, rigGet.vfoSub, &retfreq);
            if (retcode == RIG_OK) rigGet.freqSub = retfreq;
        }

        //* Meter
        if (rigGet.ptt == 1 || rigSet.ptt == 1)
        {
            rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_RFPOWER_METER, &rigGet.powerMeter);
            if (rigSet.meter != RIG_METER_NONE) rig_get_level(my_rig, RIG_VFO_CURR, rigSet.meter, &rigGet.subMeter);

            if (rig_has_get_level(my_rig, RIG_METER_SWR) && (rigSet.meter != RIG_LEVEL_SWR))
            {
                rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_SWR, &rigGet.hiSWR);
            }
            else if (rigSet.meter == RIG_LEVEL_SWR) rigGet.hiSWR = rigGet.subMeter;
        }
        else
        {
            retcode = rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_STRENGTH, &retvalue);
            if (retcode == RIG_OK) rigGet.sMeter = retvalue;
        }

        //***** Command execution *****
        if (!rigGet.ptt && !rigSet.ptt)
        {
            //* Power off
            if (rigCmd.onoff && rigCap.onoff)
            {
                retcode = rig_set_powerstat(my_rig, RIG_POWER_OFF);
                if (retcode == RIG_OK) rigGet.onoff = RIG_POWER_OFF;
            }
            rigCmd.onoff = 0;

            //* Mode
            if (rigCmd.mode && rigSet.mode != RIG_MODE_NONE)    //VFO Main
            {
                retcode = rig_set_mode(my_rig, RIG_VFO_CURR, rigSet.mode, RIG_PASSBAND_NOCHANGE);
                if (retcode == RIG_OK)
                {
                    guiCmd.bwidthList = 1;  //Command update of BW list
                    guiCmd.tabList = 1;     //Command selection of appropriate mode function tab
                    guiCmd.dialConf = 1;    //Command the tuning dial step configuration
                    indexCmd = 0;   //Update all
                    //rig_get_mode(my_rig, RIG_VFO_CURR, &rigGet.mode, &rigGet.bwidth);   //Get BW
                }
                rigCmd.mode = 0;
            }
            if (rigCmd.modeSub && rigCap.modeSub && rigSet.modeSub != RIG_MODE_NONE)  //VFO Sub
            {
                retcode = rig_set_mode(my_rig, rigGet.vfoSub, rigSet.mode, RIG_PASSBAND_NOCHANGE);
                if (retcode == RIG_OK)
                {
                }
                rigCmd.modeSub = 0;
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
                freq_t tempFreq = rigGet.freqMain;  //temporary save for non targettable sub VFO
                if (rigSet.split) retcode = rig_set_split_vfo(my_rig, rigGet.vfoMain, rigSet.split, rigGet.vfoSub); //Split on
                else retcode = rig_set_split_vfo(my_rig, rigGet.vfoMain, rigSet.split, rigGet.vfoMain); //Split off
                //retcode = rig_set_split_vfo(my_rig, RIG_VFO_RX, rigSet.split, RIG_VFO_TX);
                if (retcode == RIG_OK)
                {
                    rigGet.split = rigSet.split;
                    if (rigGet.split && (my_rig->caps->targetable_vfo & RIG_TARGETABLE_FREQ) == 0)    //if non targettable sub VFO
                    {
                        rig_get_freq(my_rig, RIG_VFO_CURR, &retfreq);
                        if (retfreq != tempFreq) rigGet.freqSub = tempFreq; //in this case VFOs were toggled, so print out the right sub VFO frequency
                    }
                }
                rigCmd.split = 0;
             }

            //* VFO Exchange
            if (rigCmd.vfoXchange)
            {
                if (my_rig->state.vfo_ops & RIG_OP_XCHG)
                //if (my_rig->caps->vfo_ops & RIG_OP_XCHG)
                {
                    mode_t tempMode = rigGet.mode;
                    retcode = rig_vfo_op(my_rig, RIG_VFO_CURR, RIG_OP_XCHG);
                    if (retcode == RIG_OK)
                    {
                        if (rigCap.modeSub == 0) rigGet.modeSub = tempMode; //If mode sub VFO not targettable, use buffer
                        indexCmd = 21;
                        //guiCmd.bwidthList = 1;
                    }
                }

                else if (my_rig->state.vfo_ops & RIG_OP_TOGGLE)
                //else if (my_rig->caps->vfo_ops & RIG_OP_TOGGLE)
                {
                    freq_t tempFreq = rigGet.freqMain;
                    mode_t tempMode = rigGet.mode;
                    retcode = rig_vfo_op(my_rig, RIG_VFO_CURR, RIG_OP_TOGGLE);
                    if (retcode == RIG_OK)
                    {
                        if (rigCap.freqSub == 0) rigGet.freqSub = tempFreq; //If freq sub VFO not targettable, use buffer
                        if (rigCap.modeSub == 0) rigGet.modeSub = tempMode; //If mode sub VFO not targettable, use buffer
                        indexCmd = 21;
                        //guiCmd.bwidthList = 1;
                    }
                }
                rigCmd.vfoXchange = 0;
            }

            //* VFO Copy
            if (rigCmd.vfoCopy)
            {
                if (my_rig->state.vfo_ops & RIG_OP_CPY)
                //if (my_rig->caps->vfo_ops & RIG_OP_CPY)
                {
                    retcode = rig_vfo_op(my_rig, RIG_VFO_CURR, RIG_OP_CPY);
                    if (retcode == RIG_OK)
                    {
                        if (rigCap.freqSub == 0) rigGet.freqSub = rigGet.freqMain;
                        if (rigCap.modeSub == 0) rigGet.modeSub = rigGet.mode;
                    }
                }
                rigCmd.vfoCopy = 0;
            }

            //* VFO Down
            if (rigCmd.vfoDown)
            {
                rig_vfo_op(my_rig, RIG_VFO_CURR, RIG_OP_DOWN);
                rigCmd.vfoDown = 0;
            }

            //* VFO Up
            if (rigCmd.vfoUp)
            {
                rig_vfo_op(my_rig, RIG_VFO_CURR, RIG_OP_UP);
                rigCmd.vfoUp = 0;
            }

            //* Band Up
            if (rigCmd.bandUp)
            {
                if (my_rig->state.vfo_ops & RIG_OP_BAND_UP)
                //if (my_rig->caps->vfo_ops & RIG_OP_BAND_UP)
                {
                    retcode = rig_vfo_op(my_rig, RIG_VFO_CURR, RIG_OP_BAND_UP);
                    if (retcode == RIG_OK) indexCmd = 21;
                }
                rigCmd.bandUp = 0;
            }

            //* Band Down
            if (rigCmd.bandDown)
            {
                if (my_rig->state.vfo_ops & RIG_OP_BAND_DOWN)
                //if (my_rig->caps->vfo_ops & RIG_OP_BAND_DOWN)
                {
                    retcode = rig_vfo_op(my_rig, RIG_VFO_CURR, RIG_OP_BAND_DOWN);
                    if (retcode == RIG_OK) indexCmd = 21;
                }
                rigCmd.bandDown = 0;
            }

            //* Band change
            if (rigCmd.bandChange)
            {
                if (rigCap.bandChange)
                {
                    retvalue.i = rigSet.band;
                    retcode = rig_set_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_BAND_SELECT, retvalue);
                    if (retcode == RIG_OK)
                    {
                        rigGet.band = rigSet.band;
                        indexCmd = 21;
                    }
                }
                rigCmd.bandChange = 0;
            }

            //* Tune
            if (rigCmd.tune)
            {
                if (my_rig->state.vfo_ops & RIG_OP_TUNE) rig_vfo_op(my_rig, RIG_VFO_CURR, RIG_OP_TUNE);
                //if (my_rig->caps->vfo_ops & RIG_OP_TUNE) rig_vfo_op(my_rig, RIG_VFO_CURR, RIG_OP_TUNE);
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

            //* AF Gain
            if (rigCmd.afGain)
            {
                retvalue.f = rigSet.afGain;
                retcode = rig_set_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_AF, retvalue);
                if (retcode == RIG_OK) rigGet.afGain = rigSet.afGain;
                rigCmd.afGain = 0;
            }

            //* Squelch
            if (rigCmd.squelch)
            {
                retvalue.f = rigSet.squelch;
                retcode = rig_set_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_SQL, retvalue);
                if (retcode == RIG_OK) rigGet.squelch = rigSet.squelch;
                rigCmd.squelch = 0;
            }

            //* MIC gain
            if (rigCmd.micGain)
            {
                retvalue.f = rigSet.micGain;
                retcode = rig_set_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_MICGAIN, retvalue);
                if (retcode == RIG_OK) rigGet.micGain = rigSet.micGain;
                rigCmd.micGain = 0;
            }

            //* MIC comp
            if (rigCmd.micCompLevel)
            {
                retvalue.f = rigSet.micCompLevel;
                retcode = rig_set_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_COMP, retvalue);
                if (retcode == RIG_OK) rigGet.micCompLevel = rigSet.micCompLevel;
                rigCmd.micCompLevel = 0;
            }
            if (rigCmd.micComp)
            {
                retcode = rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_COMP, rigSet.micComp);
                if (retcode == RIG_OK) rigGet.micComp = rigSet.micComp;
                rigCmd.micComp = 0;
            }

            //* Monitor
            if (rigCmd.micMonLevel)
            {
                retvalue.f = rigSet.micMonLevel;
                retcode = rig_set_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_MONITOR_GAIN, retvalue);
                if (retcode == RIG_OK) rigGet.micMonLevel = rigSet.micMonLevel;
                rigCmd.micMonLevel = 0;
            }
            if (rigCmd.micMon)
            {
                retcode = rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_MON, rigSet.micMon);
                if (retcode == RIG_OK) rigGet.micMon = rigSet.micMon;
                rigCmd.micMon = 0;
            }
            
            //* NB noise blanker
            if (rigCmd.noiseBlanker)
            {
                retcode = rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_NB, rigSet.noiseBlanker);
                if (retcode == RIG_OK) rigGet.noiseBlanker = rigSet.noiseBlanker;
                rigCmd.noiseBlanker = 0;
            }
            if (rigCmd.noiseBlanker2)
            {
                retcode = rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_NB2, rigSet.noiseBlanker2);
                if (retcode == RIG_OK) rigGet.noiseBlanker2 = rigSet.noiseBlanker2;
                rigCmd.noiseBlanker2 = 0;
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
                retvalue.f = rigSet.noiseReductionLevel;
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

            //* Clarifier
            if (rigCmd.clar)
            {
                if (rigSet.clar != rigGet.clar)
                {
                    if (rigSet.rit) retcode = rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_RIT, rigSet.clar);
                    else if (rigSet.xit) retcode = rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_XIT, rigSet.clar);
                    if (retcode == RIG_OK)
                    {
                        rigGet.clar = rigSet.clar;
                        rigGet.rit = rigSet.rit;
                        rigGet.xit = rigSet.xit;
                    }
                }

                if ((rigSet.rit != rigGet.rit) && rigGet.clar)
                {
                    retcode = rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_RIT, rigSet.rit);
                    if (retcode == RIG_OK) rigGet.rit = rigSet.rit;
                }
                if ((rigSet.xit != rigGet.xit) && rigGet.clar)
                {
                    retcode = rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_XIT, rigSet.xit);
                    if (retcode == RIG_OK) rigGet.xit = rigSet.xit;
                }

                if (rigSet.rit)
                {
                    retcode = rig_set_rit(my_rig, RIG_VFO_CURR, rigSet.ritOffset);
                    if (retcode == RIG_OK) rigGet.ritOffset = rigSet.ritOffset;
                }
                else if (rigSet.xit)
                {
                    retcode = rig_set_xit(my_rig, RIG_VFO_CURR, rigSet.xitOffset);
                    if (retcode == RIG_OK) rigGet.xitOffset = rigSet.xitOffset;
                }
                rigCmd.clar = 0;
            }

            //** CW
            //* CW break-in
            if (rigCmd.bkin)
            {
                retcode = rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_FBKIN, rigSet.bkin);
                if (retcode == RIG_OK) rigGet.bkin = rigSet.bkin;
                rigCmd.bkin = 0;
            }
            //* CW Auto Peak Filter
            if (rigCmd.apf)
            {
                retcode = rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_APF, rigSet.apf);
                if (retcode == RIG_OK) rigGet.apf = rigSet.apf;
                rigCmd.apf = 0;
            }
            //* CW keyer speed
            if (rigCmd.wpm)
            {
                retvalue.i = rigSet.wpm;
                retcode = rig_set_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_KEYSPD, retvalue);
                if (retcode == RIG_OK) rigGet.wpm = rigSet.wpm;
                rigCmd.wpm = 0;
            }

            //** FM
            //* Repeater shift
            if (rigCmd.rptShift)
            {
                if (rig_has_set_func(my_rig, RIG_FUNCTION_SET_RPTR_SHIFT))
                //if (my_rig->caps->set_rptr_shift)
                {
                    retcode = rig_set_rptr_shift(my_rig, RIG_VFO_CURR, rigSet.rptShift);
                    if (retcode == RIG_OK) rigGet.rptShift = rigSet.rptShift;
                }
                rigCmd.rptShift = 0;
            }
            //* Repeater offset
            if (rigCmd.rptOffset)
            {
                if (rig_has_set_func(my_rig, RIG_FUNCTION_SET_RPTR_OFFS))
                //if (my_rig->caps->set_rptr_offs)
                {
                    retcode = rig_set_rptr_offs(my_rig, RIG_VFO_CURR, rigSet.rptOffset);
                    if (retcode == RIG_OK) rigGet.rptOffset = rigSet.rptOffset;
                }
                rigCmd.rptOffset = 0;
            }
            //* Tone
            if (rigCmd.tone)
            {
                switch (rigSet.toneType)
                {
                case 1: //Burst 1750 Hz
                    retcode = rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_TBURST, true);
                    break;
                case 2: //CTCSS tone
                    retcode = rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_TONE, true);
                    if (rigSet.tone) rig_set_ctcss_tone(my_rig, RIG_VFO_CURR, rigSet.tone);
                    else rig_get_ctcss_tone(my_rig, RIG_VFO_CURR, &rigSet.tone);
                    break;
                case 3: //CTCSS tone + squelch
                    retcode = rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_TSQL, true);
                    if (rigSet.tone) rig_set_ctcss_tone(my_rig, RIG_VFO_CURR, rigSet.tone);
                    else rig_get_ctcss_tone(my_rig, RIG_VFO_CURR, &rigSet.tone);
                    break;
                case 4: //DCS tone + squelch
                    retcode = rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_CSQL, true);
                    if (rigSet.tone) rig_set_dcs_code(my_rig, RIG_VFO_CURR, rigSet.tone);
                    else rig_get_dcs_code(my_rig, RIG_VFO_CURR, &rigSet.tone);
                    break;
                 default:
                    rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_TBURST, false);
                    rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_TONE, false);
                    rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_TSQL, false);
                    rig_set_func(my_rig, RIG_VFO_CURR, RIG_FUNC_CSQL, false);
                    retcode = RIG_OK;
                    break;
                }
                if (retcode == RIG_OK)
                {
                    rigGet.toneType = rigSet.toneType;
                    rigGet.tone = rigSet.tone;
                }
                rigCmd.tone = 0;
                guiCmd.toneList = 1;
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
        //* Mode and BW
        if ((indexCmd == 1 && !rigGet.ptt && rigCom.fullPoll) || indexCmd == 0)
        {
            rmode_t tempMode;

            rig_get_mode(my_rig, RIG_VFO_CURR, &tempMode, &rigGet.bwidth);

            if (tempMode != rigGet.mode)
            {
                guiCmd.bwidthList = 1;  //Command update of BW list
                guiCmd.tabList = 1;     //Command selection of appropriate mode function tab
                guiCmd.dialConf = 1;    //Command the tuning dial step configuration
            }

            rigGet.mode = tempMode;

            if (rigGet.bwidth == rig_passband_narrow(my_rig, rigGet.mode)) rigGet.bwNarrow = 1;
            else rigGet.bwNarrow = 0;

            if (rigCap.modeSub) rig_get_mode(my_rig, rigGet.vfoSub, &rigGet.modeSub, &rigGet.bwidthSub);
        }

        //* VFO and Split
        if ((indexCmd == 2 && !rigGet.ptt && rigCom.fullPoll) || indexCmd == 0)
        {
            rig_get_split_vfo(my_rig, RIG_VFO_CURR, &rigGet.split, &rigGet.vfoTx);            

            rig_get_vfo(my_rig, &rigGet.vfoMain);
        }

        //* Tuner
        if ((indexCmd == 3 && !rigGet.ptt && rigCom.fullPoll) || indexCmd == 0) rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_TUNER, &rigGet.tuner);

        //* Antenna
        if ((indexCmd == 4 && !rigGet.ptt && rigCom.fullPoll) || indexCmd == 0)
        {
            if (rig_has_get_func(my_rig, RIG_FUNCTION_GET_ANT)) rig_get_ant(my_rig, RIG_VFO_CURR, RIG_ANT_CURR, &retvalue, &rigGet.ant, &rigGet.antTx, &rigGet.antRx);
        }

        //* AGC
        if ((indexCmd == 5 && !rigGet.ptt && rigCom.fullPoll) || indexCmd == 0)
        {
            if (rig_has_get_level(my_rig, RIG_LEVEL_AGC))
            {
                rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_AGC, &retvalue);
                rigGet.agc = levelagcvalue(retvalue.i);
            }
        }

        //* Attenuator
        if ((indexCmd == 6 && !rigGet.ptt && rigCom.fullPoll) || indexCmd == 0)
        {
            if (rig_has_get_level(my_rig, RIG_LEVEL_ATT))
            {
                rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_ATT, &retvalue);
                rigGet.att = retvalue.i;
            }
        }

        //* Preamp
        if ((indexCmd == 7 && !rigGet.ptt && rigCom.fullPoll) || indexCmd == 0)
        {
            if (rig_has_get_level(my_rig, RIG_LEVEL_PREAMP))
            {
                rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_PREAMP, &retvalue);
                rigGet.pre = retvalue.i;
            }
        }

        //* RF power
        if ((indexCmd == 8 && !rigGet.ptt && rigCom.fullPoll) || indexCmd == 0)
        {
            if (rig_has_get_level(my_rig, RIG_LEVEL_RFPOWER))
            {
                rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_RFPOWER, &retvalue);
                rigGet.rfPower = retvalue.f;
            }
        }

        //* RF gain
        if ((indexCmd == 9 && !rigGet.ptt && rigCom.fullPoll) || indexCmd == 0)
        {
            if (rig_has_get_level(my_rig, RIG_LEVEL_RF))
            {
                rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_RF, &retvalue);
                rigGet.rfGain = retvalue.f;
            }
        }

        //* AF gain
        if ((indexCmd == 10 && !rigGet.ptt && rigCom.fullPoll) || indexCmd == 0)
        {
            if (rig_has_get_level(my_rig, RIG_LEVEL_AF))
            {
                rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_AF, &retvalue);
                rigGet.afGain = retvalue.f;
            }
        }

        //* Squelch
        if ((indexCmd == 11 && !rigGet.ptt && rigCom.fullPoll) || indexCmd == 0)
        {
            if (rig_has_get_level(my_rig, RIG_LEVEL_SQL))
            {
                rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_SQL, &retvalue);
                rigGet.squelch = retvalue.f;
            }
        }

        //* MIC
        if ((indexCmd == 12 && !rigGet.ptt && rigCom.fullPoll) || indexCmd == 0)
        {
            if (rig_has_get_level(my_rig, RIG_LEVEL_MICGAIN))
            {
                rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_MICGAIN, &retvalue);
                rigGet.micGain = retvalue.f;
            }
            if (rig_has_get_func(my_rig, RIG_FUNC_COMP)) rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_COMP, &rigGet.micComp);
            if (rig_has_get_level(my_rig, RIG_LEVEL_COMP))
            {
                rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_COMP, &retvalue);
                rigGet.micCompLevel = retvalue.f;
            }
        }

        //* Monitor
        if ((indexCmd == 13 && !rigGet.ptt && rigCom.fullPoll) || indexCmd == 0)
        {
            if (rig_has_get_func(my_rig, RIG_FUNC_MON)) rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_MON, &rigGet.micMon);
            if (rig_has_get_level(my_rig, RIG_LEVEL_MONITOR_GAIN))
            {
                rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_MONITOR_GAIN, &retvalue);
                rigGet.micMonLevel = retvalue.f;
            }
        }

        //* NB noise blanker
        if ((indexCmd == 14 && !rigGet.ptt && rigCom.fullPoll) || indexCmd == 0)
        {
            if (rig_has_get_func(my_rig, RIG_FUNC_NB)) rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_NB, &rigGet.noiseBlanker);
            if (rig_has_get_func(my_rig, RIG_FUNC_NB2)) rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_NB2, &rigGet.noiseBlanker2);
        }

        //* NR noise reduction
        if ((indexCmd == 15 && !rigGet.ptt && rigCom.fullPoll) || indexCmd == 0)
        {
            if (rig_has_get_func(my_rig, RIG_FUNC_NR)) rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_NR, &rigGet.noiseReduction);
            if (rig_has_get_level(my_rig, RIG_LEVEL_NR))
            {
                rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_NR, &retvalue);
                rigGet.noiseReductionLevel = retvalue.f;
            }
        }

        //* NF notch filter
        if ((indexCmd == 16 && !rigGet.ptt && rigCom.fullPoll) || indexCmd == 0)
        {
            if (rig_has_get_func(my_rig, RIG_FUNC_ANF)) rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_ANF, &rigGet.notchFilter);
        }

        //* IF Shift
        if ((indexCmd == 17 && !rigGet.ptt && rigCom.fullPoll) || indexCmd == 0)
        {
            if (rig_has_get_level(my_rig, RIG_LEVEL_IF))
            {
                rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_IF, &retvalue);
                rigGet.ifShift = retvalue.i;
            }
        }

        //* Clarifier
        if ((indexCmd == 18 && !rigGet.ptt && rigCom.fullPoll) || indexCmd == 0)
        {
            if (rig_has_get_func(my_rig, RIG_FUNC_RIT)) rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_RIT, &rigGet.rit);  //RIT
            if (rig_has_get_func(my_rig, RIG_FUNC_XIT)) rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_XIT, &rigGet.xit);  //XIT
            rigGet.clar = rigGet.rit || rigGet.xit;
            //qDebug() << rigGet.clar << rigGet.rit << rigGet.xit;
            if (rigSet.rit && my_rig->caps->get_rit) rig_get_rit(my_rig, RIG_VFO_CURR, &rigGet.ritOffset);
            else if (rigSet.xit && my_rig->caps->get_xit) rig_get_xit(my_rig, RIG_VFO_CURR, &rigGet.xitOffset);
            //else rigGet.clarOffset = rigSet.clarOffset;
        }

        //* CW
        if ((indexCmd == 19 && !rigGet.ptt && rigCom.fullPoll && (rigGet.mode == RIG_MODE_CW || rigGet.mode == RIG_MODE_CWN || rigGet.mode == RIG_MODE_CWR)) || indexCmd == 0)
        {
            if (rig_has_get_func(my_rig, RIG_FUNC_FBKIN)) rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_FBKIN, &rigGet.bkin);   //Break-in
            if (rig_has_get_func(my_rig, RIG_FUNC_APF)) rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_APF, &rigGet.apf);      //Audio Peak Filter
            if (guiConf.cwKeyerMode == 0 && rig_has_get_level(my_rig, RIG_LEVEL_KEYSPD)) rig_get_level(my_rig, RIG_VFO_CURR, RIG_LEVEL_KEYSPD, &retvalue);   //Keyer speed WPM
            rigGet.wpm = retvalue.i;
        }

        //* FM
        if ((indexCmd == 20 && !rigGet.ptt && rigCom.fullPoll && (rigGet.mode == RIG_MODE_FM || rigGet.mode == RIG_MODE_WFM || rigGet.mode == RIG_MODE_FMN)) || indexCmd == 0)
        {
            rig_get_rptr_shift(my_rig, RIG_VFO_CURR, &rigGet.rptShift);     //Repeater Shift
            rig_get_rptr_offs(my_rig, RIG_VFO_CURR, &rigGet.rptOffset);     //Repeater Offset

            int status = false;
            if (!(my_rig->caps->has_get_func & RIG_FUNC_TONE)) status = 1;   //If get cap is not available skip
            if (!status)
            {
                rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_TBURST, &status);   //1750 Hz Tone burst
                if (status) rigGet.toneType = 1;
            }
            if (!status)
            {
                rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_TONE, &status);     //CTCSS Tone Tx
                if (status) rigGet.toneType = 2;
            }
            if (!status)
            {
                rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_TSQL, &status);     //CTCSS Tone Tx and Rx Squelch
                if (status) rigGet.toneType = 3;
            }
            if (!status)
            {
                rig_get_func(my_rig, RIG_VFO_CURR, RIG_FUNC_CSQL, &status);     //DCS Code
                if (status) rigGet.toneType = 4;
            }
            if (!status) rigGet.toneType = 0;

            if ((rigGet.toneType == 2 || rigGet.toneType == 3) && my_rig->caps->get_ctcss_tone) rig_get_ctcss_tone(my_rig, RIG_VFO_CURR, &rigGet.tone);
            else if (rigGet.toneType == 4 && my_rig->caps->get_dcs_code) rig_get_dcs_code(my_rig, RIG_VFO_CURR, &rigGet.tone);

            /*if (rigGet.toneType && rigGet.tone == 0)
            {
                rigSet.toneType = 0;
                rigCmd.tone = 1;
            }*/

            if (rigGet.toneType != rigSet.toneType) guiCmd.toneList = 1;    //update tone list
        }

        indexCmd ++;
        if (indexCmd >= 21) indexCmd = 1;
    }

    emit resultReady();
}
