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


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogconfig.h"
#include "dialogsetup.h"
#include "rigdaemon.h"
#include "rigdata.h"
#include "guidata.h"
#include "rigcommand.h"

#include <QDebug>
#include <QMessageBox>
#include <QThread>
#include <QSettings>
#include <QString>
#include <QtGlobal>
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>

#include <rig.h>    //Hamlib

extern RIG *my_rig; //Defined in rigdaemon.cpp

extern rigConnect rigCom;
extern rigSettings rigGet;
extern rigSettings rigSet;
extern rigCommand rigCmd;
extern rigCommand rigCap;
extern guiConfig guiConf;
extern guiCommand guiCmd;

int retcode;    //Return code from function
int i;  //Index
int prevDial;   //Previous dial value
int fastDial;   //Fast pushbutton state

FILE* debugFile;


QThread workerThread; //
RigDaemon *rigDaemon = new RigDaemon;


//***** MainWindow *****

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    timer = new QTimer(this);   //timer for rigDaemon thread call

    //* Debug
    rig_set_debug_level(RIG_DEBUG_WARN);  //normal
    //rig_set_debug_level(RIG_DEBUG_TRACE);   //debug
    rig_set_debug_time_stamp(true);
    if ((debugFile=fopen("catradio.log","w+")) == NULL) rig_set_debug_level(RIG_DEBUG_NONE);
    else rig_set_debug_file(debugFile);

    //* Signal and Slot connection for Slider and associated Label
    connect(ui->verticalSlider_RFpower, &QAbstractSlider::valueChanged, ui->label_RFpowerValue, QOverload<int>::of(&QLabel::setNum));
    connect(ui->verticalSlider_RFgain, &QAbstractSlider::valueChanged, ui->label_RFgainValue, QOverload<int>::of(&QLabel::setNum));
    connect(ui->verticalSlider_AFGain, &QAbstractSlider::valueChanged, ui->label_AFGainValue, QOverload<int>::of(&QLabel::setNum));
    connect(ui->verticalSlider_Squelch, &QAbstractSlider::valueChanged, ui->label_SquelchValue, QOverload<int>::of(&QLabel::setNum));
    connect(ui->horizontalSlider_clar, &QAbstractSlider::valueChanged, ui->label_clar, QOverload<int>::of(&QLabel::setNum));
    connect(ui->horizontalSlider_IFshift, &QAbstractSlider::valueChanged, ui->label_IFshiftValue,QOverload<int>::of(&QLabel::setNum));

    //* Signal and Slot connection for vfoDisplay
    connect(ui->lineEdit_vfoMain, &vfoDisplay::on_valueChanged, this, &MainWindow::on_vfoDisplayMainValueChanged);
    connect(ui->lineEdit_vfoSub, &vfoDisplay::on_valueChanged, this, &MainWindow::on_vfoDisplaySubValueChanged);

    //* Thread for RigDaemon
    rigDaemon->moveToThread(&workerThread); //
    connect(&workerThread, &QThread::finished, rigDaemon, &QObject::deleteLater);
    connect(timer, &QTimer::timeout, rigDaemon, &RigDaemon::rigUpdate);
    connect(rigDaemon, &RigDaemon::resultReady, this, &MainWindow::on_rigDaemonResultReady);
    workerThread.start();

    //* Load settings from catradio.ini
    QSettings configFile(QString("catradio.ini"), QSettings::IniFormat);
    rigCom.rigModel = configFile.value("rigModel", 0).toInt();
    rigCom.rigPort = configFile.value("rigPort").toString();
    rigCom.serialSpeed = configFile.value("serialSpeed", 9600).toInt();
    rigCom.civAddr = configFile.value("civAddress", 0).toInt();
    rigCom.netRigctl = configFile.value("netRigctl", false).toBool();
    rigCom.rigRefresh = configFile.value("rigRefresh", 100).toInt();
    rigCom.fullPoll = configFile.value("fullPolling", true).toBool();
    guiConf.vfoDisplayMode = configFile.value("vfoDisplayMode", 0).toInt();
    //Window settings
    restoreGeometry(configFile.value("WindowSettings/geometry").toByteArray());
    restoreState(configFile.value("WindowSettings/state").toByteArray());

    //* Style
    //ui->pushButton_PTT->setStyleSheet("QPushButton::checked {font: bold; color: red;}");

    QApplication::setWheelScrollLines(10);  //Mouse wheel scroll step
}

MainWindow::~MainWindow()
{
    workerThread.quit(); //
    workerThread.wait();

    if (rigCom.connected)
    {
        timer->stop();

        rigCom.connected = 0;
        rig_close(my_rig);  //Close the communication to the rig
    }

    rig_cleanup(my_rig);    //Release rig handle and free associated memory

    fclose(debugFile);  //Close debug.log

    //* Save window settings
    QSettings configFile(QString("catradio.ini"), QSettings::IniFormat);
    configFile.setValue("WindowSettings/geometry", saveGeometry());
    configFile.setValue("WindowSettings/state", saveState());

    delete ui;
}


//***** GUI function *****

void MainWindow::guiInit()
{
    ui->statusbar->showMessage(my_rig->caps->model_name);

    //* Power on/off cap
    //if (rig_has_set_func(my_rig, RIG_FUNCTION_SET_POWERSTAT)==0)
    if (my_rig->caps->set_powerstat == NULL)
    {
        ui->pushButton_Power->setDisabled(true);    //Power pushbutton disabled
        rigCap.onoff = 0;
    }
    else
    {
        ui->pushButton_Power->setDisabled(false);
        rigCap.onoff = 1;
    }

    //* PTT cap
    //if (rig_has_set_func(my_rig, RIG_FUNCTION_SET_PTT)==0)
    if (my_rig->caps->set_ptt == NULL)
    {
        ui->pushButton_PTT->setDisabled(true);  //PTT pushbutton disabled
        rigCap.ptt = 0;
    }
    else
    {
        ui->pushButton_PTT->setDisabled(false);
        rigCap.ptt = 1;
    }

    //* Band select cap
    if (rig_has_set_level(my_rig, RIG_LEVEL_BAND_SELECT)) rigCap.bandChange = 1;
    else rigCap.bandChange = 0;

    //* Mode combobox
    if (my_rig->state.mode_list == RIG_MODE_NONE)
    {
        ui->comboBox_Mode->setDisabled(true);
        ui->comboBox_ModeSub->setDisabled(true);
    }
    else
    {
        ui->comboBox_Mode->clear(); //Clear old contents
        ui->comboBox_ModeSub->clear();
        for (i=0; i<HAMLIB_MAX_MODES; i++)
        {
            if (my_rig->state.mode_list & (1ULL << i))
            {
                ui->comboBox_Mode->addItem(rig_strrmode(my_rig->state.mode_list & (1ULL << i)));
                ui->comboBox_ModeSub->addItem(rig_strrmode(my_rig->state.mode_list & (1ULL << i)));
            }
        }
        ui->comboBox_ModeSub->addItem(rig_strrmode(RIG_MODE_NONE)); //add None mode in the modeSub list, used in case mode sub vfo is not targetable
        guiCmd.bwidthList = 1;  //Command to populate BW combobox in guiUpdate()
    }

    //* AGC level comboBox
    ui->comboBox_AGC->clear();
    /* It seems that has_set_level or has_get_level is not well implemented in Hamlib, at the moment it is skipped in favour of fix values entry
    agc_level_e agcLevel;
    for (i = 0; i < 7; i++)
    {
        agcLevel = levelagcvalue(i);
        if (rig_has_set_level(my_rig, agcLevel))
        {
            ui->comboBox_AGC->addItem(rig_stragclevel(agcLevel));
        }
    }*/
    ui->comboBox_AGC->addItem(rig_stragclevel(RIG_AGC_OFF));
    ui->comboBox_AGC->addItem(rig_stragclevel(RIG_AGC_AUTO));
    ui->comboBox_AGC->addItem(rig_stragclevel(RIG_AGC_FAST));
    ui->comboBox_AGC->addItem(rig_stragclevel(RIG_AGC_MEDIUM));
    ui->comboBox_AGC->addItem(rig_stragclevel(RIG_AGC_SLOW));

    //* Meter comboBox
    ui->comboBox_Meter->clear();
    if (rig_has_get_level(my_rig, RIG_METER_SWR)) ui->comboBox_Meter->addItem("SWR");
    if (rig_has_get_level(my_rig, RIG_METER_ALC)) ui->comboBox_Meter->addItem("ALC");
    if (rig_has_get_level(my_rig, RIG_METER_COMP)) ui->comboBox_Meter->addItem("COMP");
    if (rig_has_get_level(my_rig, RIG_METER_IC)) ui->comboBox_Meter->addItem("ID");
    if (rig_has_get_level(my_rig, RIG_METER_VDD)) ui->comboBox_Meter->addItem("VDD");
    rigSet.meter = levelmeterstr (ui->comboBox_Meter->currentText());
    setSubMeter();

    //* Attenuator comboBox
    ui->comboBox_Att->clear();
    ui->comboBox_Att->addItem("0");
    for (i = 0; i < HAMLIB_MAXDBLSTSIZ && my_rig->state.attenuator[i] != 0; i++)
    {
        ui->comboBox_Att->addItem(QString::number(my_rig->state.attenuator[i]));
    }

    //* Preamp comboBox
    ui->comboBox_Preamp->clear();
    ui->comboBox_Preamp->addItem("0");
    for (i = 0; i < HAMLIB_MAXDBLSTSIZ && my_rig->state.preamp[i] != 0; i++)
    {
        ui->comboBox_Preamp->addItem(QString::number(my_rig->state.preamp[i]));
    }

    //* Clarifier
    rigSet.rit = 1;
    if (!rig_has_set_func(my_rig, RIG_FUNC_XIT)) ui->radioButton_clarXIT->setEnabled(false);

    //* Filter
    if (!rig_has_set_func(my_rig, RIG_FUNC_NB)) ui->checkBox_NB->setEnabled(false);
    if (!rig_has_set_func(my_rig, RIG_FUNC_NB2)) ui->checkBox_NB2->setEnabled(false);
    if (!rig_has_set_func(my_rig, RIG_FUNC_NR)) {ui->checkBox_NR->setEnabled(false); ui->spinBox_NR->setEnabled(false);}
    if (!rig_has_set_func(my_rig, RIG_FUNC_ANF)) ui->checkBox_NF->setEnabled(false);

    //* Tone
    ui->comboBox_toneType->clear();
    ui->comboBox_toneType->addItem("");        //None
    if (rig_has_set_func(my_rig, RIG_FUNC_TBURST)) ui->comboBox_toneType->addItem("1750Hz");   //Burst 1750 Hz
    if (rig_has_set_func(my_rig, RIG_FUNC_TONE)) ui->comboBox_toneType->addItem("TONE");     //CTCSS Tx
    if (rig_has_set_func(my_rig, RIG_FUNC_TSQL)) ui->comboBox_toneType->addItem("TSQL");     //CTCSS Tx + Rx squelch
    if (rig_has_set_func(my_rig, RIG_FUNC_CSQL)) ui->comboBox_toneType->addItem("DCS");    //DCS

    //* VFO
    ui->lineEdit_vfoMain->setMode(guiConf.vfoDisplayMode);
    ui->lineEdit_vfoSub->setMode(guiConf.vfoDisplayMode);

    //check for targetable sub VFO
    if (my_rig->caps->rig_model != 2)   //Hamlib 4.4 has bug for rigctld and targetable_vfo, skip check
    {
        if (my_rig->caps->targetable_vfo & RIG_TARGETABLE_FREQ) rigCap.freqSub = 1;    //targetable frequency
        else rigCap.freqSub = 0;
        if (my_rig->caps->targetable_vfo & RIG_TARGETABLE_MODE) rigCap.modeSub = 1;    //targetable mode
        else rigCap.modeSub = 0;
        if (my_rig->caps->targetable_vfo == RIG_TARGETABLE_NONE)
        {
        rigCap.freqSub = 0; //disable get/set freq for subVFO
        rigCap.modeSub = 0; //disable get/set mode for subVFO
        ui->radioButton_VFOSub->setCheckable(false);    //disable VFOsub radio button
        }
    }
    else    //NET rigctl, as workaround assume targetable_vfo
    {
        rigCap.freqSub = 1;
        rigCap.modeSub = 1;
    }

    guiCmd.rangeList = 1;   //update range list
    guiCmd.antList = 1; //update antenna list
    guiCmd.toneList = 1;    //update tone list
    guiCmd.tabList = 1; //select tab
}

void MainWindow::guiUpdate()
{
    //* Power button
    if (rigGet.onoff == RIG_POWER_ON || rigGet.onoff == RIG_POWER_UNKNOWN) ui->pushButton_Power->setChecked(true);

    //* VFOs
    if (!rigCmd.freqMain) ui->lineEdit_vfoMain->setValue(rigGet.freqMain);
    if (!rigCmd.freqSub) ui->lineEdit_vfoSub->setValue(rigGet.freqSub);

    ui->label_vfoMain->setText(rig_strvfo(rigGet.vfoMain));
    switch (rigGet.vfoMain)
    {
    case RIG_VFO_A: rigGet.vfoSub = RIG_VFO_B; break;
    case RIG_VFO_B: rigGet.vfoSub = RIG_VFO_A; break;
    case RIG_VFO_MAIN: rigGet.vfoSub = RIG_VFO_SUB; break;
    case RIG_VFO_SUB: rigGet.vfoSub = RIG_VFO_MAIN; break;
    }
    ui->label_vfoSub->setText(rig_strvfo(rigGet.vfoSub));

    //* Mode
    ui->comboBox_Mode->setCurrentText(rig_strrmode(rigGet.mode));
    ui->comboBox_ModeSub->setCurrentText(rig_strrmode(rigGet.modeSub));

    if (guiCmd.tabList)
    {
        if (rigGet.mode == RIG_MODE_SSB || rigGet.mode == RIG_MODE_USB || rigGet.mode == RIG_MODE_LSB) ui->tabWidget->setCurrentIndex(0); //Clarifier tab
        if (rigGet.mode == RIG_MODE_CW || rigGet.mode == RIG_MODE_CWR || rigGet.mode == RIG_MODE_CWN) ui->tabWidget->setCurrentIndex(1);    //CW tab
        if (rigGet.mode == RIG_MODE_FM || rigGet.mode == RIG_MODE_FMN || rigGet.mode == RIG_MODE_PKTFM || rigGet.mode == RIG_MODE_PKTFMN || rigGet.mode == RIG_MODE_C4FM || rigGet.mode == RIG_MODE_DSTAR) ui->tabWidget->setCurrentIndex(2);   //FM tab
        guiCmd.tabList = 0;
    }

    //* BW combobox
    if (guiCmd.bwidthList)
    {
        ui->comboBox_BW->clear();
        filter_list bwidth_list;    //IF filter bandwidth per mode
        for (i=0; i<HAMLIB_FLTLSTSIZ && !RIG_IS_FLT_END(my_rig->state.filters[i]); i++)
        {
            char modeslist[1000] = "";
            QString modes, mode;
            bwidth_list = my_rig->state.filters[i];
            rig_strrmodes(bwidth_list.modes, modeslist, sizeof(modeslist));
            modes = modeslist;
            modes.append(" ");
            mode = rig_strrmode(rigGet.mode);
            if (mode != "")
            {
                //qDebug() << modes << mode << bwidth_list.width;
                QRegularExpression rx("\\b"+mode+"?\\s");
                if (modes.contains (rx) && bwidth_list.width != RIG_FLT_ANY) ui->comboBox_BW->addItem(QString::number(bwidth_list.width));
            }
            //else qDebug() << "vuoto" << rigGet.mode;
       }
       ui->comboBox_BW->model()->sort(0, Qt::DescendingOrder);
       guiCmd.bwidthList = 0;
    }

    ui->comboBox_BW->setCurrentText(QString::number(rigGet.bwidth));
    ui->checkBox_NAR->setChecked(rigGet.bwNarrow);

    //* Range list
    if (guiCmd.rangeList)
    {
        for (i=0; i<HAMLIB_FRQRANGESIZ; i++)    //Tx range list
        {
            if (rigGet.freqMain >= my_rig->state.tx_range_list[i].startf && rigGet.freqMain <= my_rig->state.tx_range_list[i].endf) break;
        }
        rigGet.rangeListTxIndex = i;

        for (i=0; i<HAMLIB_FRQRANGESIZ; i++)    //Rx range list
        {
            if (rigGet.freqMain >= my_rig->state.rx_range_list[i].startf && rigGet.freqMain <= my_rig->state.rx_range_list[i].endf) break;
        }
        if (rigGet.rangeListRxIndex != i) guiCmd.antList = 1;
        rigGet.rangeListRxIndex = i;

        guiCmd.rangeList = 0;
    }

    //* Antenna list
    if (guiCmd.antList)
    {
        ui->comboBox_Ant->clear();
        if (my_rig->state.tx_range_list[rigGet.rangeListRxIndex].ant == RIG_ANT_NONE) ui->comboBox_Ant->addItem("NONE");  //RIG_ANT_NONE
        else for (i=0; i < RIG_ANT_MAX; i++)
        {
            if (my_rig->state.tx_range_list[rigGet.rangeListRxIndex].ant & (1UL << i))
            {
                switch (i)
                {
                case 0: ui->comboBox_Ant->addItem("ANT1"); break;   //RIG_ANT_1
                case 1: ui->comboBox_Ant->addItem("ANT2"); break;   //RIG_ANT_2
                case 2: ui->comboBox_Ant->addItem("ANT3"); break;   //RIG_ANT_3
                case 3: ui->comboBox_Ant->addItem("ANT4"); break;   //RIG_ANT_4
                case 4: ui->comboBox_Ant->addItem("ANT5"); break;   //RIG_ANT_5
                case 30: ui->comboBox_Ant->addItem("UNK"); break;   //RIG_ANT_UNKNOWN
                case 31: ui->comboBox_Ant->addItem("CURR"); break;  //RIG_ANT_CURR
                default: ui->comboBox_Ant->addItem(""); break;
                }
            }
        }
        guiCmd.antList = 0;
    }

    //* Tone list
    if (guiCmd.toneList)
    {
        ui->comboBox_toneFreq->clear();

        if (rigGet.toneType == 2 || rigGet.toneType == 3)
        {
            for (i = 0; i < CTCSS_LIST_SIZE; i++)   //CTCSS tone
            {
            if (my_rig->caps->ctcss_list[i] == 0) break;
            ui->comboBox_toneFreq->addItem(QString::number(my_rig->caps->ctcss_list[i]/10.0,'f',1));
            }
        }

        if (rigGet.toneType == 4)
        {
            for (i = 0; i < DCS_LIST_SIZE; i++)   //DCS code
            {
                if (my_rig->caps->dcs_list[i] == 0) break;
                ui->comboBox_toneFreq->addItem(QString::number(my_rig->caps->dcs_list[i]));
            }
        }

        guiCmd.toneList = 0;
    }

    //* RF
    ui->radioButton_Tuner->setChecked(rigGet.tuner);
    ui->comboBox_AGC->setCurrentText(rig_stragclevel(rigGet.agc));
    ui->comboBox_Att->setCurrentText(QString::number(rigGet.att));
    ui->comboBox_Preamp->setCurrentText(QString::number(rigGet.pre));

    //* Split
    if (rigGet.split == RIG_SPLIT_ON)
    {
        ui->pushButton_Split->setChecked(true);
        if (rigGet.vfoSub == rigGet.vfoTx)
        {
            ui->label_vfoMainRxTx->setText("RX");
            ui->label_vfoSubRxTx->setText("TX");
        }
        else
        {
            ui->label_vfoMainRxTx->setText("TX");
            ui->label_vfoSubRxTx->setText("RX");
        }
    }
    else
    {
        ui->pushButton_Split->setChecked(false);
        ui->label_vfoMainRxTx->setText("RX TX");
        ui->label_vfoSubRxTx->setText("");
    }

    //* PTT & Meter
    if (rigGet.ptt == RIG_PTT_ON)
    {
        //ui->pushButton_PTT->setChecked(true);

        if (rigGet.vfoTx == rigGet.vfoSub) ui->label_vfoSub->setStyleSheet("QLabel {background-color: red}");
        else ui->label_vfoMain->setStyleSheet("QLabel {background-color: red}");

        ui->progressBar_Smeter->setTx(true);
        ui->progressBar_Smeter->setValue(rigGet.powerMeter.f*100);
        ui->progressBar_subMeter->setValue(rigGet.subMeter.f);
    }
    else    //RIG_PTT_OFF
    {
        //ui->pushButton_PTT->setChecked(false);

        if (rigGet.vfoTx == rigGet.vfoSub) ui->label_vfoSub->setStyleSheet("QLabel {}");
        else ui->label_vfoMain->setStyleSheet("QLabel {}");

        ui->progressBar_Smeter->setTx(false);
        ui->progressBar_Smeter->setValue(rigGet.sMeter.i);
        if (rigSet.meter == RIG_LEVEL_SWR) ui->progressBar_subMeter->setValue(1.0);
        else ui->progressBar_subMeter->setValue(0.0);
    }

    //* Sliders
    if (!ui->verticalSlider_RFpower->isSliderDown() && !rigCmd.rfPower) ui->verticalSlider_RFpower->setValue((int)(rigGet.rfPower*100));
    if (!ui->verticalSlider_RFgain->isSliderDown() && !rigCmd.rfGain) ui->verticalSlider_RFgain->setValue((int)(rigGet.rfGain*100));
    if (!ui->verticalSlider_AFGain->isSliderDown() && !rigCmd.afGain) ui->verticalSlider_AFGain->setValue((int)(rigGet.afGain*100));
    if (!ui->verticalSlider_Squelch->isSliderDown() && !rigCmd.squelch) ui->verticalSlider_Squelch->setValue((int)(rigGet.squelch*100));

    //* Filter
    ui->checkBox_NB->setChecked(rigGet.noiseBlanker);
    ui->checkBox_NB2->setChecked(rigGet.noiseBlanker2);
    ui->checkBox_NR->setChecked(rigGet.noiseReduction);
    ui->spinBox_NR->setValue(rigGet.noiseReductionLevel);
    ui->checkBox_NF->setChecked(rigGet.notchFilter);
    if (!ui->horizontalSlider_IFshift->isSliderDown() && !rigCmd.ifShift) ui->horizontalSlider_IFshift->setValue(rigGet.ifShift);

    //* Clarifier
    ui->checkBox_clar->setChecked(rigGet.clar);
    if (rigSet.xit)
    {
        ui->radioButton_clarXIT->setChecked(true);
        if (!ui->horizontalSlider_clar->isSliderDown() && !rigCmd.clar) ui->horizontalSlider_clar->setValue(rigGet.xitOffset);
    }
    else    //rigSet.rit
    {
        ui->radioButton_clarRIT->setChecked(true);
        if (!ui->horizontalSlider_clar->isSliderDown() && !rigCmd.clar) ui->horizontalSlider_clar->setValue(rigGet.ritOffset);
    }

    //* CW
    ui->checkBox_BKIN->setChecked(rigGet.bkin);
    ui->checkBox_APF->setChecked(rigGet.apf);
    ui->spinBox_WPM->setValue(rigGet.wpm);

    //* FM
    if (rigGet.rptShift == RIG_RPT_SHIFT_MINUS) ui->radioButton_RPTshiftMinus->setChecked(true);    //-
    else if (rigGet.rptShift == RIG_RPT_SHIFT_PLUS) ui->radioButton_RPTshiftPlus->setChecked(true); //+
    else ui->radioButton_RPTshiftSimplex->setChecked(true); //Simplex
    ui->spinBox_RPToffset->setValue(rigGet.rptOffset/1000);  //Offset (kHz)

    switch (rigGet.toneType)
    {
    case (1): ui->comboBox_toneType->setCurrentText("1750Hz"); break;
    case (2): ui->comboBox_toneType->setCurrentText("TONE"); break;
    case (3): ui->comboBox_toneType->setCurrentText("TSQL"); break;
    case (4): ui->comboBox_toneType->setCurrentText("DCS"); break;
    default: ui->comboBox_toneType->setCurrentText(""); break;
    }

    if (rigGet.toneType == 2 || rigGet.toneType ==3) ui->comboBox_toneFreq->setCurrentText(QString::number(rigGet.tone/10.0));  //CTCSS
    else if (rigGet.toneType == 4) ui->comboBox_toneFreq->setCurrentText(QString::number(rigGet.tone));  //DCS
}


//* RigDaemon handle results
void MainWindow::on_rigDaemonResultReady()
{
    guiUpdate();
}

//* SubMeter
void MainWindow::setSubMeter()
{
    if (rigSet.meter == RIG_LEVEL_SWR)
    {
        ui->progressBar_subMeter->setMeterSWR(true);
        ui->progressBar_subMeter->setMaxValue(4.0);
        ui->progressBar_subMeter->setGateValue(2.0);
        ui->progressBar_subMeter->setLongStep(0.5);
        ui->progressBar_subMeter->setShortStep(0.1);
        ui->progressBar_subMeter->setValue(1.0);
    }
    else if (rigSet.meter == RIG_LEVEL_ID_METER)
    {
        ui->progressBar_subMeter->setMeterSWR(false);
        ui->progressBar_subMeter->setMaxValue(25.0);
        ui->progressBar_subMeter->setMinValue(0.0);
        ui->progressBar_subMeter->setGateValue(10.0);
        ui->progressBar_subMeter->setPrecision(0);
        ui->progressBar_subMeter->setLongStep(5.0);
        ui->progressBar_subMeter->setShortStep(1.0);
        ui->progressBar_subMeter->setValue(0);
    }
    else if (rigSet.meter == RIG_LEVEL_VD_METER)
    {
        ui->progressBar_subMeter->setMeterSWR(false);
        ui->progressBar_subMeter->setMaxValue(15.0);
        ui->progressBar_subMeter->setMinValue(0.0);
        ui->progressBar_subMeter->setGateValue(13.8);
        ui->progressBar_subMeter->setPrecision(1);
        ui->progressBar_subMeter->setLongStep(5.0);
        ui->progressBar_subMeter->setShortStep(1.0);
        ui->progressBar_subMeter->setValue(0);
    }
    else if (rigSet.meter == RIG_LEVEL_COMP_METER)
    {
        ui->progressBar_subMeter->setMeterSWR(false);
        ui->progressBar_subMeter->setMaxValue(20.0);
        ui->progressBar_subMeter->setMinValue(0.0);
        ui->progressBar_subMeter->setGateValue(15.0);
        ui->progressBar_subMeter->setPrecision(0);
        ui->progressBar_subMeter->setLongStep(5.0);
        ui->progressBar_subMeter->setShortStep(1.0);
        ui->progressBar_subMeter->setValue(0);
    }
    else
    {
        ui->progressBar_subMeter->setMeterSWR(false);
        ui->progressBar_subMeter->setMaxValue(1.0);
        ui->progressBar_subMeter->setMinValue(0.0);
        ui->progressBar_subMeter->setGateValue(0.5);
        ui->progressBar_subMeter->setPrecision(1);
        ui->progressBar_subMeter->setLongStep(0.5);
        ui->progressBar_subMeter->setShortStep(0.1);
        ui->progressBar_subMeter->setValue(0);
    }
}


//***** PushButton *****

void MainWindow::on_pushButton_Connect_toggled(bool checked)
{
    if (checked && rigCom.connected == 0)
    {
       retcode = rigDaemon->rigConnect();   //Open Rig connection

       if (retcode != RIG_OK)   //Connection error
       {
           rigCom.connected = 0;
           ui->statusbar->showMessage(rigerror(retcode));
           ui->pushButton_Connect->setChecked(false);  //Uncheck the button
       }
       else    //Rig connected
       {
           rigCom.connected = 1;
           guiInit();
           if (rigCap.onoff == 0 || rigGet.onoff == RIG_POWER_ON || rigGet.onoff == RIG_POWER_UNKNOWN) timer->start(rigCom.rigRefresh);
       }
    }
    else if (rigCom.connected)   //Button unchecked
    {
        rigCom.connected = 0;
        timer->stop();
        rig_close(my_rig);  //Close the communication to the rig
        //rig_cleanup(my_rig);    //Release rig handle and free associated memory
    }
}

void MainWindow::on_pushButton_Power_toggled(bool checked)
{
    if (checked && !rigGet.onoff)
    {
        retcode = rig_set_powerstat(my_rig, RIG_POWER_ON);
        if (retcode != RIG_OK)
        {
            ui->pushButton_Power->setChecked(false);  //Uncheck the button
            ui->statusbar->showMessage(rigerror(retcode));
        }
        else timer->start(rigCom.rigRefresh);
    }
    else if (!checked && rigGet.onoff)
    {
        retcode = rig_set_powerstat(my_rig, RIG_POWER_OFF);
        if (retcode == RIG_OK)
        {
            ui->pushButton_Power->setChecked(false);  //Uncheck the button
            timer->stop();
        }
    }
}

void MainWindow::on_pushButton_PTT_toggled(bool checked)
{
    if (checked)
    {
        rigSet.ptt = RIG_PTT_ON;
        rigCmd.ptt = 1;
    }
    else    //!checked
    {
        rigSet.ptt = RIG_PTT_OFF;
        rigCmd.ptt = 1;
    }
}

void MainWindow::on_pushButton_Split_toggled(bool checked)
{
    if (checked && !rigGet.split)
    {
        rigSet.split = RIG_SPLIT_ON;
        rigCmd.split = 1;
    }
    else if (!checked && rigGet.split)
    {
        rigSet.split = RIG_SPLIT_OFF;
        rigCmd.split = 1;
    }
}

void MainWindow::on_pushButton_QSplit_clicked()
{
    quick_split();
}

void MainWindow::on_pushButton_AB_clicked()
{
    rigCmd.vfoXchange = 1;
}

void MainWindow::on_pushButton_AeqB_clicked()
{
    rigCmd.vfoCopy = 1;
}

void MainWindow::on_pushButton_Fast_toggled(bool checked)
{
    if (checked) fastDial = 1;
    else fastDial = 0;
}

void MainWindow::on_pushButton_left_clicked()
{
    rigCmd.vfoDown = 1;
}


void MainWindow::on_pushButton_right_clicked()
{
    rigCmd.vfoUp = 1;
}

void MainWindow::on_pushButton_Tune_clicked()
{
    rigCmd.tune = 1;
}

void MainWindow::on_pushButton_clarClear_clicked()
{
    if (rigSet.rit) rigSet.ritOffset = 0;
    else rigSet.xitOffset = 0;  //rigSet.xit
    rigCmd.clar = 1;
}

void MainWindow::on_pushButton_Band160_clicked()
{
    set_band(160);
}

void MainWindow::on_pushButton_Band80_clicked()
{
    set_band(80);
}

void MainWindow::on_pushButton_Band60_clicked()
{
    set_band(60);
}

void MainWindow::on_pushButton_Band40_clicked()
{
    set_band(40);
}

void MainWindow::on_pushButton_Band30_clicked()
{
    set_band(30);
}

void MainWindow::on_pushButton_Band20_clicked()
{
    set_band(20);
}

void MainWindow::on_pushButton_Band17_clicked()
{
    set_band(17);
}

void MainWindow::on_pushButton_Band15_clicked()
{
    set_band(15);
}

void MainWindow::on_pushButton_Band12_clicked()
{
    set_band(12);
}

void MainWindow::on_pushButton_Band10_clicked()
{
    set_band(10);
}

void MainWindow::on_pushButton_Band6_clicked()
{
    set_band(6);
}

void MainWindow::on_pushButton_Band2_clicked()
{
    set_band(2);
}

void MainWindow::on_pushButton_Band70_clicked()
{
    set_band(70);
}

void MainWindow::on_pushButton_BandGen_clicked()
{
    set_band(1);
}

void MainWindow::on_pushButton_BandDown_clicked()
{
    rigCmd.bandDown = 1;
}

void MainWindow::on_pushButton_BandUp_clicked()
{
    rigCmd.bandUp = 1;
}

//***** CheckBox *****

void MainWindow::on_checkBox_NAR_toggled(bool checked)
{
    if (checked && !rigGet.bwNarrow)
    {
        rigSet.bwidth = rig_passband_narrow(my_rig, rigGet.mode);
        rigCmd.bwidth = 1;
    }
    else if (!checked && rigGet.bwNarrow)
    {
        rigSet.bwidth = RIG_PASSBAND_NORMAL;
        rigCmd.bwidth = 1;
    }
}

void MainWindow::on_checkBox_BKIN_toggled(bool checked)
{
    if (checked && !rigGet.bkin)
    {
        rigSet.bkin = 1;
        rigCmd.bkin = 1;
    }
    else if (!checked && rigGet.bkin)
    {
        rigSet.bkin = 0;
        rigCmd.bkin = 1;
    }
}

void MainWindow::on_checkBox_NB_toggled(bool checked)
{
    if (checked && !rigGet.noiseBlanker)
    {
        rigSet.noiseBlanker = 1;
        rigCmd.noiseBlanker = 1;
    }
    else if (!checked && rigGet.noiseBlanker)
    {
        rigSet.noiseBlanker = 0;
        rigCmd.noiseBlanker = 1;
    }
}

void MainWindow::on_checkBox_NB2_toggled(bool checked)
{
    if (checked && !rigGet.noiseBlanker2)
    {
        rigSet.noiseBlanker2 = 1;
        rigCmd.noiseBlanker2 = 1;
    }
    else if (!checked && rigGet.noiseBlanker2)
    {
        rigSet.noiseBlanker2 = 0;
        rigCmd.noiseBlanker2 = 1;
    }
}

void MainWindow::on_checkBox_NR_toggled(bool checked)
{
    if (checked && !rigGet.noiseReduction)
    {
        rigSet.noiseReduction = 1;
        rigCmd.noiseReduction = 1;
    }
    else if (!checked && rigGet.noiseReduction)
    {
        rigSet.noiseReduction = 0;
        rigCmd.noiseReduction = 1;
    }
}

void MainWindow::on_checkBox_NF_toggled(bool checked)
{
    if (checked && !rigGet.notchFilter)
    {
        rigSet.notchFilter = 1;
        rigCmd.notchFilter = 1;
    }
    else if (!checked && rigGet.notchFilter)
    {
        rigSet.notchFilter = 0;
        rigCmd.notchFilter = 1;
    }
}

void MainWindow::on_checkBox_clar_toggled(bool checked)
{
    if (checked && !rigGet.clar)
    {
        rigSet.clar = 1;
        rigCmd.clar = 1;
    }
    else if (!checked && rigGet.clar)
    {
        rigSet.clar = 0;
        rigCmd.clar = 1;
    }
}

void MainWindow::on_checkBox_APF_toggled(bool checked)
{
    if (checked && !rigGet.apf)
    {
        rigSet.apf = 1;
        rigCmd.apf = 1;
    }
    else if (!checked && rigGet.apf)
    {
        rigSet.apf = 0;
        rigCmd.apf = 1;
    }
}


//***** RadioButton *****

void MainWindow::on_radioButton_Tuner_toggled(bool checked)
{
   if (checked && !rigGet.tuner)
   {
       rigSet.tuner = 1;
       rigCmd.tuner = 1;
   }
   else if (!checked && rigGet.tuner)
   {
       rigSet.tuner = 0;
       rigCmd.tuner = 1;
   }
}

void MainWindow::on_radioButton_clarRIT_toggled(bool checked)
{
    if (checked)
    {
        rigSet.rit = 1;
        rigSet.xit = 0;
        rigCmd.clar = 1;
    }
}

void MainWindow::on_radioButton_clarXIT_toggled(bool checked)
{
    if (checked)
    {
        rigSet.rit = 0;
        rigSet.xit = 1;
        rigCmd.clar = 1;
    }
}

void MainWindow::on_radioButton_RPTshiftSimplex_toggled(bool checked)
{
    if (checked)
    {
        rigSet.rptShift = RIG_RPT_SHIFT_NONE;
        rigCmd.rptShift = 1;
    }
}

void MainWindow::on_radioButton_RPTshiftMinus_toggled(bool checked)
{
    if (checked)
    {
        rigSet.rptShift = RIG_RPT_SHIFT_MINUS;
        rigCmd.rptShift = 1;
    }
}


void MainWindow::on_radioButton_RPTshiftPlus_toggled(bool checked)
{
    if (checked)
    {
        rigSet.rptShift = RIG_RPT_SHIFT_PLUS;
        rigCmd.rptShift = 1;
    }
}


//***** Dial *****

void MainWindow::on_dial_valueChanged(int value)
{
    int step;

    step = value - prevDial;
    if (step<=99 && step>50) step = value - 100 - prevDial;
    if (step>=-99 && step<-50) step = value + 100 - prevDial;
    prevDial = value;

    if (ui->radioButton_VFOSub->isChecked()) //dial VFO Sub
    {
        rigSet.freqSub = rigGet.freqSub + step*(!fastDial*10+fastDial*100);
        rigCmd.freqSub = 1;
    }
    else    //dial VFO Main
    {
        rigSet.freqMain = rigGet.freqMain + step*(!fastDial*10+fastDial*100);
        rigCmd.freqMain = 1;
    }
}

void MainWindow::on_vfoDisplayMainValueChanged(int value)
{
    rigSet.freqMain = value;
    rigCmd.freqMain = 1;
}

void MainWindow::on_vfoDisplaySubValueChanged(int value)
{
    rigSet.freqSub = value;
    rigCmd.freqSub = 1;
}

//***** ComboBox *****

void MainWindow::on_comboBox_Mode_activated(int index)
{
    rigSet.mode = rig_parse_mode(ui->comboBox_Mode->itemText(index).toLatin1());
    guiCmd.tabList = 1; //update tab
    rigCmd.mode = 1;
}

void MainWindow::on_comboBox_ModeSub_activated(int index)
{
    rigSet.modeSub = rig_parse_mode(ui->comboBox_ModeSub->itemText(index).toLatin1());
    rigCmd.modeSub = 1;
}

void MainWindow::on_comboBox_BW_activated(int index)
{
    rigSet.bwidth = ui->comboBox_BW->itemText(index).toInt();
    rigCmd.bwidth = 1;
}

void MainWindow::on_comboBox_AGC_activated(int index)
{
    rigSet.agc = levelagcstr(ui->comboBox_AGC->itemText(index));
    rigCmd.agc = 1;
}

void MainWindow::on_comboBox_Att_activated(int index)
{
    rigSet.att = ui->comboBox_Att->itemText(index).toInt();
    rigCmd.att = 1;
}

void MainWindow::on_comboBox_Preamp_activated(int index)
{
    rigSet.pre = ui->comboBox_Preamp->itemText(index).toInt();
    rigCmd.pre = 1;
}

void MainWindow::on_comboBox_Ant_activated(int index)
{
    rigSet.ant = antstr(ui->comboBox_Ant->itemText(index));
    rigCmd.ant = 1;
}

void MainWindow::on_comboBox_Meter_activated(int index)
{
    rigSet.meter = levelmeterstr (ui->comboBox_Meter->itemText(index));
    setSubMeter();
}

void MainWindow::on_comboBox_toneType_activated(int index)
{
    QString toneType = ui->comboBox_toneType->itemText(index);
    if (toneType == "1750Hz") rigSet.toneType = 1;
    else if (toneType == "TONE") rigSet.toneType = 2;
    else if (toneType == "TSQL") rigSet.toneType = 3;
    else if (toneType == "DCS") rigSet.toneType = 4;
    else rigSet.toneType = 0;

    guiCmd.toneList = 1;    //update tone list
    rigCmd.tone = 1;
}

void MainWindow::on_comboBox_toneFreq_activated(int index)
{
    if (rigGet.toneType == 2 || rigGet.toneType == 3)   //CTCSS
    {
        QString arg = ui->comboBox_toneFreq->itemText(index);
        arg = arg.remove(".");  //Remove '.' from CTCSS (as for hamlib CTCSS tone list)
        rigSet.tone = arg.toUInt();
    }
    else if (rigGet.toneType == 4) rigSet.tone = ui->comboBox_toneFreq->itemText(index).toInt();  //DCS
    else return;

    rigCmd.tone = 1;
}


//***** Spin Box *****

void MainWindow::on_spinBox_NR_valueChanged(int arg1)
{
    rigSet.noiseReductionLevel = arg1;
    rigCmd.noiseReductionLevel = 1;
}

void MainWindow::on_spinBox_WPM_valueChanged(int arg1)
{
    rigSet.wpm = arg1;
    rigCmd.wpm = 1;
}

void MainWindow::on_spinBox_RPToffset_valueChanged(int arg1)
{
    rigSet.rptOffset = arg1*1000;
    rigCmd.rptOffset = 1;
}

//***** Slider *****

void MainWindow::on_verticalSlider_RFgain_valueChanged(int value)
{
    if (!rigCmd.rfGain && !ui->verticalSlider_RFgain->isSliderDown())
    {
        rigSet.rfGain = (float)(value)/100;
        if (rigSet.rfGain != rigGet.rfGain) rigCmd.rfGain = 1;
    }
}

void MainWindow::on_verticalSlider_RFgain_sliderReleased()
{
    if (!rigCmd.rfGain)
    {
        rigSet.rfGain = (float)(ui->verticalSlider_RFgain->value())/100;
        rigCmd.rfGain = 1;
    }
}


void MainWindow::on_verticalSlider_RFpower_valueChanged(int value)
{
    if (!rigCmd.rfPower && !ui->verticalSlider_RFpower->isSliderDown())
    {
        if (value < 5) value = 5;   //to be fixed
        rigSet.rfPower = (float)(value)/100;
        if (rigSet.rfPower != rigGet.rfPower) rigCmd.rfPower = 1;
    }
}

void MainWindow::on_verticalSlider_RFpower_sliderReleased()
{
    if (!rigCmd.rfPower)
    {
        rigSet.rfPower = (float)(ui->verticalSlider_RFpower->value())/100;
        rigCmd.rfPower = 1;
    }
}


void MainWindow::on_verticalSlider_AFGain_valueChanged(int value)
{
    if (!rigCmd.afGain && !ui->verticalSlider_AFGain->isSliderDown())
    {
        rigSet.afGain = (float)(value)/100;
        if (rigSet.afGain != rigGet.afGain) rigCmd.afGain = 1;
    }
}

void MainWindow::on_verticalSlider_AFGain_sliderReleased()
{
    if (!rigCmd.afGain)
    {
        rigSet.afGain = (float)(ui->verticalSlider_AFGain->value())/100;
        rigCmd.afGain = 1;
    }
}


void MainWindow::on_verticalSlider_Squelch_valueChanged(int value)
{
    if (!rigCmd.squelch && !ui->verticalSlider_Squelch->isSliderDown())
    {
        rigSet.squelch = (float)(value)/100;
        if (rigSet.squelch != rigGet.squelch) rigCmd.squelch = 1;
    }
}

void MainWindow::on_verticalSlider_Squelch_sliderReleased()
{
    if (!rigCmd.squelch)
    {
        rigSet.squelch = (float)(ui->verticalSlider_Squelch->value())/100;
        rigCmd.squelch = 1;
    }
}


void MainWindow::on_horizontalSlider_IFshift_valueChanged(int value)
{
    if (!rigCmd.ifShift && !ui->horizontalSlider_IFshift->isSliderDown())
    {
        rigSet.ifShift = value;
        if (rigSet.ifShift != rigGet.ifShift) rigCmd.ifShift = 1;
    }
}

void MainWindow::on_horizontalSlider_IFshift_sliderReleased()
{
    if (!rigCmd.ifShift)
    {
        rigSet.ifShift = ui->horizontalSlider_IFshift->value();
        rigCmd.ifShift = 1;
    }
}


void MainWindow::on_horizontalSlider_clar_valueChanged(int value)
{
    if (!rigCmd.clar && !ui->horizontalSlider_clar->isSliderDown())
    {
        if (rigSet.rit)
        {
            rigSet.ritOffset = value;
            if (rigSet.ritOffset != rigGet.ritOffset) rigCmd.clar = 1;
        }
        else    //rigSet.xit
        {
            rigSet.xitOffset = value;
            if (rigSet.xitOffset != rigGet.xitOffset) rigCmd.clar = 1;
        }
     }
}

void MainWindow::on_horizontalSlider_clar_sliderReleased()
{
    if (!rigCmd.clar)
    {
        if (rigSet.rit)
        {
            rigSet.ritOffset = ui->horizontalSlider_clar->value();
            rigCmd.clar = 1;
        }
        else
        {
            rigSet.xitOffset = ui->horizontalSlider_clar->value();
            rigCmd.clar = 1;
        }
     }
}


//***** Menu *****

void MainWindow::on_action_Connection_triggered()
{
    DialogConfig config;
    config.setModal(true);
    config.exec();
}

void MainWindow::on_action_Setup_triggered()
{
    DialogSetup setup;
    setup.setModal(true);
    setup.exec();

    ui->lineEdit_vfoMain->setMode(guiConf.vfoDisplayMode);
    ui->lineEdit_vfoSub->setMode(guiConf.vfoDisplayMode);
}

void MainWindow::on_action_AboutCatRadio_triggered()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("About");
    msgBox.setTextFormat(Qt::RichText);
    QString version = QString::number(VERSION_MAJ)+"."+QString::number(VERSION_MIN)+"."+QString::number(VERSION_MIC);
    msgBox.setText("<b>CatRadio</b> <i>Radio control software</i><br/>version "+version+" "+RELEASE_DATE);
    msgBox.setInformativeText("Copyright (C) 2022 Gianfranco Sordetti IZ8EWD<br/>"
                              "<a href='https://www.pianetaradio.it'>www.pianetaradio.it</a></p>"
                              "<p>This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.<br/>"
                              "This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.<br/>"
                              "You should have received a copy of the GNU General Public License along with this program.  If not, see <a href='http://www.gnu.org/licenses/'>www.gnu.org/licenses</a>.");
    msgBox.setIcon(QMessageBox::NoIcon);
    msgBox.setStandardButtons(QMessageBox::Ok);

    QPixmap icon("catradio.png");
    msgBox.setIconPixmap(icon);

    msgBox.exec();
}

void MainWindow::on_action_AboutQT_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_action_AboutHamLib_triggered()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("About Hamlib");
    msgBox.setText(rig_version());
    msgBox.setInformativeText(rig_copyright());
    //msgBox.setDetailedText(rig_license());
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

void MainWindow::on_action_CatRadioHomepage_triggered()
{
    QUrl homepage("https://www.pianetaradio.it/blog/catradio/");
    QDesktopServices::openUrl(homepage);
}
