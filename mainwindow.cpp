/**
 ** This file is part of the CatRadio project.
 ** Copyright 2022-2026 Gianfranco Sordetti IZ8EWD <iz8ewd@pianetaradio.it>.
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
#include "dialogvoicekeyer.h"
#include "dialogcwkeyer.h"
#include "dialogcommand.h"
#include "dialogradioinfo.h"
#include "dialognetrigctl.h"

#include "rigdaemon.h"
#include "rigdata.h"
#include "guidata.h"
#include "rigcommand.h"
#include "winkeyer.h"
#include "debuglogger.h"

#include <QDebug>
#include <QMessageBox>
#include <QThread>
#include <QString>
#include <QtGlobal>
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include <QCoreApplication>
#include <QDir>

#include <cwchar>   //c++ string library
#include <rig.h>    //Hamlib


extern rigConnect rigCom;
extern rigSettings rigGet;
extern rigSettings rigSet;
extern rigCommand rigCmd;
extern rigCommand rigCap;
extern guiConfig guiConf;
extern guiCommand guiCmd;
extern voiceKeyerConfig voiceKConf;
extern cwKeyerConfig cwKConf;

int retcode;    //Return code from function
int i;  //Index
int prevDial;   //Previous dial value
int fastDial;   //Fast pushbutton state

const float fudge = 0.003;

FILE* debugFile;


QThread workerThread; //
RigDaemon *rigDaemon = new RigDaemon;

QDialog *command = nullptr;
QDialog *radioInfo = nullptr;

WinKeyer *winkeyer = nullptr;


//***** MainWindow *****

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QDir::setCurrent(QCoreApplication::applicationDirPath());   //set current path = application path

    timer = new QTimer(this);   //timer for rigDaemon thread call

    //* Signal and Slot connection for Slider and associated Label
    connect(ui->verticalSlider_RFpower, &QAbstractSlider::valueChanged, ui->label_RFpowerValue, QOverload<int>::of(&QLabel::setNum));
    connect(ui->verticalSlider_RFgain, &QAbstractSlider::valueChanged, ui->label_RFgainValue, QOverload<int>::of(&QLabel::setNum));
    connect(ui->verticalSlider_AFGain, &QAbstractSlider::valueChanged, ui->label_AFGainValue, QOverload<int>::of(&QLabel::setNum));
    connect(ui->verticalSlider_Squelch, &QAbstractSlider::valueChanged, ui->label_SquelchValue, QOverload<int>::of(&QLabel::setNum));
    connect(ui->verticalSlider_micGain, &QAbstractSlider::valueChanged, ui->label_micGainLevel, QOverload<int>::of(&QLabel::setNum));
    connect(ui->verticalSlider_micMonitor, &QAbstractSlider::valueChanged, ui->label_micMonitorLevel, QOverload<int>::of(&QLabel::setNum));
    connect(ui->verticalSlider_micCompressor, &QAbstractSlider::valueChanged, ui->label_micCompressorLevel, QOverload<int>::of(&QLabel::setNum));
    connect(ui->horizontalSlider_clar, &QAbstractSlider::valueChanged, ui->label_clar, QOverload<int>::of(&QLabel::setNum));
    connect(ui->horizontalSlider_IFshift, &QAbstractSlider::valueChanged, ui->label_IFshiftValue,QOverload<int>::of(&QLabel::setNum));

    //* Signal and Slot connection for vfoDisplay
    connect(ui->lineEdit_vfoMain, &vfoDisplay::on_valueChanged, this, &MainWindow::on_vfoDisplayMainValueChanged);
    connect(ui->lineEdit_vfoSub, &vfoDisplay::on_valueChanged, this, &MainWindow::on_vfoDisplaySubValueChanged);

    //* Thread for RigDaemon
    rigDaemon->moveToThread(&workerThread); //
    connect(&workerThread, &QThread::finished, rigDaemon, &QObject::deleteLater);
    connect(timer, &QTimer::timeout, this, &MainWindow::rigUpdate);
    connect(rigDaemon, &RigDaemon::resultReady, this, &MainWindow::on_rigDaemonResultReady);
    workerThread.start();

    //* Load settings from catradio.ini
    loadGuiConfig("catradio.ini");  //load GUI config
    loadRigConfig("catradio.ini");  //load Rig config
    //Voice memory
    if (guiConf.voiceKeyerMode == 1)    //CatRadio Voice Keyer
    {
        ui->action_Voice_Keyer->setEnabled(true);   //enable Voice Keyer menu
        audioOutputInit("catradio.ini");     //init audio
    }
    //CW keyer
    if (guiConf.cwKeyerMode == 1)   //WinKeyer
    {
        ui->actionCW_Keyer->setEnabled(true);   //enable CW Keyer menu
        loadCwKeyerConfig("catradio.ini");  //load CW Keyer config
    }

    //* Debug
    debugLogger::install("catradio.log");
    if (guiConf.debugMode) debugLogger::setDebugLevel(QtInfoMsg);
    else debugLogger::setDebugLevel(QtFatalMsg);

    //Debug Hamlib
    if (guiConf.debugMode) rig_set_debug_level(RIG_DEBUG_VERBOSE); //debug verbose
    else rig_set_debug_level(RIG_DEBUG_WARN);  //normal
    //rig_set_debug_level(RIG_DEBUG_VERBOSE); //debug verbose
    //rig_set_debug_level(RIG_DEBUG_TRACE);   //debug trace
    rig_set_debug_time_stamp(true);
    if ((debugFile=fopen("hamlib.log","w+")) == NULL) rig_set_debug_level(RIG_DEBUG_NONE);
    else rig_set_debug_file(debugFile);

    //* Style
    //ui->pushButton_PTT->setStyleSheet("QPushButton::checked {font: bold; color: red;}");

    //display name and version in the window title
    QString version = QString::number(VERSION_MAJ)+"."+QString::number(VERSION_MIN)+"."+QString::number(VERSION_MIC);
    this->setWindowTitle("CatRadio v."+version+" (Beta)");
    qInfo() << "CatRadio v."+version;

    //Dark theme
    if (guiConf.darkTheme)
    {
        //QFile darkStyleFile(":/dark/stylesheet.qss");
        QFile darkStyleFile(":qdarkstyle/dark/darkstyle.qss");

        if (!darkStyleFile.exists()) ui->statusbar->showMessage("Unable to set stylesheet, file not found!");
        else
        {
            darkStyleFile.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&darkStyleFile);
            qApp->setStyleSheet(ts.readAll());

            ui->progressBar_Smeter->setBgColor(Qt::black);
            ui->progressBar_Smeter->setScaleColor(Qt::white);
            ui->progressBar_Smeter->setLineColor(Qt::white);
            ui->progressBar_Smeter->setProgressColor(QColor(0x66, 0x8f, 0xb8));

            ui->progressBar_subMeter->setBgColor(Qt::black);
            ui->progressBar_subMeter->setScaleColor(Qt::white);
            ui->progressBar_subMeter->setLineColor(Qt::white);
            ui->progressBar_subMeter->setProgressColor(QColor(0x66, 0x8f, 0xb8));

            ui->lineEdit_vfoMain->setBgColor(Qt::black);
            ui->lineEdit_vfoMain->setLineColor(Qt::white);
            ui->lineEdit_vfoMain->setTextColor(Qt::white);

            ui->lineEdit_vfoSub->setBgColor(Qt::black);
            ui->lineEdit_vfoSub->setLineColor(Qt::white);
            ui->lineEdit_vfoSub->setTextColor(Qt::white);
        }
    }
    //Light QFile darkStyleFile(":qdarkstyle/light/lightstyle.qss");

    QApplication::setWheelScrollLines(10);  //Mouse wheel scroll step

    //* Init
    //Meter
    ui->progressBar_Smeter->setTx(false);
    ui->progressBar_Smeter->setMaxValue(100);
    ui->progressBar_Smeter->setGateValue(80);
    ui->progressBar_Smeter->setValue(-54);
    ui->progressBar_Smeter->resetPeakValue();
    ui->progressBar_Smeter->setPeakFactor(rigCom.rigRefresh/1000.0);
    ui->progressBar_subMeter->resetPeakValue();
    ui->progressBar_subMeter->setPeakFactor(rigCom.rigRefresh/1000.0);

    //VFO
    ui->lineEdit_vfoMain->setValue(0);
    ui->lineEdit_vfoSub->setValue(0);

    //Check Hamlib version
    if (!checkHamlibVersion(4, 6, 0))
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Hamlib");
        msgBox.setText("Please, update Hamlib libraries to version 4.6 or higher.");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }

    //Auto connect
    if (rigCom.autoConnect) ui->pushButton_Connect->toggle();
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
        rig_cleanup(my_rig);    //Release rig handle and free associated memory
    }

    fclose(debugFile);  //Close hamlib.log

    if (guiConf.cwKeyerMode == 1)
    {
        if (winkeyer && winkeyer->isOpen) winkeyer->close();
        if (winkeyer)
        {
            delete winkeyer;
        }
    }

    if (command) delete command;    //deallocate *command
    if (radioInfo) delete radioInfo;  //deallocate *radioInfo

    //* Save window settings
    QSettings configFile(QString("catradio.ini"), QSettings::IniFormat);
    configFile.setValue("WindowSettings/geometry", saveGeometry());
    configFile.setValue("WindowSettings/state", saveState());

    delete ui;

    qInfo() << "Close CatRadio";
}


//***** GUI function *****

void MainWindow::guiInit()
{
    qInfo() << "guiInit";

    //* Power on/off cap
    if (rig_has_set_func(my_rig, RIG_FUNCTION_SET_POWERSTAT)==0)
    //if (my_rig->caps->set_powerstat == NULL)
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
    if (rig_has_set_func(my_rig, RIG_FUNCTION_SET_PTT)==0)
    //if (my_rig->caps->set_ptt == NULL)
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

    //* ANT comboBox
    ui->comboBox_Ant->clear();
    if (!rig_has_set_func(my_rig, RIG_FUNCTION_SET_ANT)) ui->comboBox_Ant->setEnabled(false);

    //* AGC level comboBox
    ui->comboBox_AGC->clear();
    if (!rig_has_set_level(my_rig, RIG_LEVEL_AGC)) ui->comboBox_AGC->setEnabled(false);
    else
    {
        for (i = 0; i < HAMLIB_MAX_AGC_LEVELS && i < my_rig->state.agc_level_count; i++) ui->comboBox_AGC->addItem(rig_stragclevel(my_rig->state.agc_levels[i]));
        if (i==0)   //Print all levels if list is not specified
        {
            ui->comboBox_AGC->addItem(rig_stragclevel(RIG_AGC_OFF));
            ui->comboBox_AGC->addItem(rig_stragclevel(RIG_AGC_AUTO));
            ui->comboBox_AGC->addItem(rig_stragclevel(RIG_AGC_FAST));
            ui->comboBox_AGC->addItem(rig_stragclevel(RIG_AGC_MEDIUM));
            ui->comboBox_AGC->addItem(rig_stragclevel(RIG_AGC_SLOW));
        }
    }

    //* Meters & Sub-meter comboBox
    //ui->progressBar_Smeter->setMaxValue(5); //FIXME tx_range_list
    ui->progressBar_Smeter->setPeak(guiConf.peakHold);
    ui->progressBar_subMeter->setPeak(guiConf.peakHold);
    ui->comboBox_Meter->clear();
    if (rig_has_get_level(my_rig, RIG_METER_SWR)) ui->comboBox_Meter->addItem("SWR");
    if (rig_has_get_level(my_rig, RIG_METER_ALC)) ui->comboBox_Meter->addItem("ALC");
    if (rig_has_get_level(my_rig, RIG_METER_COMP)) ui->comboBox_Meter->addItem("COMP");
    if (rig_has_get_level(my_rig, RIG_METER_IC)) ui->comboBox_Meter->addItem("ID");
    if (rig_has_get_level(my_rig, RIG_METER_VDD)) ui->comboBox_Meter->addItem("VDD");
    rigSet.meter = levelmeterstr (ui->comboBox_Meter->currentText());
    setSubMeter();
    ui->label_hiSWR->setVisible(false);

    //* Attenuator comboBox
    ui->comboBox_Att->clear();
    if (!rig_has_set_level(my_rig, RIG_LEVEL_ATT)) ui->comboBox_Att->setEnabled(false);
    ui->comboBox_Att->addItem("0");
    for (i = 0; i < HAMLIB_MAXDBLSTSIZ && my_rig->state.attenuator[i] != 0; i++)
    {
        ui->comboBox_Att->addItem(QString::number(my_rig->state.attenuator[i]));
    }

    //* Preamp comboBox
    ui->comboBox_Preamp->clear();
     if (!rig_has_set_level(my_rig, RIG_LEVEL_PREAMP)) ui->comboBox_Preamp->setEnabled(false);
    ui->comboBox_Preamp->addItem("0");
    for (i = 0; i < HAMLIB_MAXDBLSTSIZ && my_rig->state.preamp[i] != 0; i++)
    {
        ui->comboBox_Preamp->addItem(QString::number(my_rig->state.preamp[i]));
    }

    //* Levels
    if (!rig_has_set_level(my_rig, RIG_LEVEL_RFPOWER)) ui->verticalSlider_RFpower->setEnabled(false);
    if (!rig_has_set_level(my_rig, RIG_LEVEL_RF)) ui->verticalSlider_RFgain->setEnabled(false);
    if (!rig_has_set_level(my_rig, RIG_LEVEL_AF)) ui->verticalSlider_AFGain->setEnabled(false);
    if (!rig_has_set_level(my_rig, RIG_LEVEL_SQL)) ui->verticalSlider_Squelch->setEnabled(false);
    if (!rig_has_set_level(my_rig, RIG_LEVEL_MICGAIN)) ui->verticalSlider_micGain->setEnabled(false);
    if (!rig_has_set_level(my_rig, RIG_LEVEL_MONITOR_GAIN)) ui->verticalSlider_micMonitor->setEnabled(false);
    if (!rig_has_set_level(my_rig, RIG_LEVEL_COMP)) ui->verticalSlider_micCompressor->setEnabled(false);

    if (!rig_has_set_func(my_rig, RIG_FUNC_COMP)) ui->checkBox_micCompressor->setEnabled(false);
    if (!rig_has_set_func(my_rig, RIG_FUNC_MON)) ui->checkBox_micMonitor->setEnabled(false);

    //* Filter
    if (!rig_has_set_func(my_rig, RIG_FUNC_NB)) ui->checkBox_NB->setEnabled(false);
    if (!rig_has_set_func(my_rig, RIG_FUNC_NB2)) ui->checkBox_NB2->setEnabled(false);
    if (!rig_has_set_func(my_rig, RIG_FUNC_ANF)) ui->checkBox_NF->setEnabled(false);
    if (rig_has_set_func(my_rig, RIG_FUNC_NR))
    {
        int max, min, step;

        for (i = 0; i < RIG_SETTING_MAX; i++)
        {
            //qDebug()<<i<<rig_idx2setting(i)<<rig_strparm(rig_idx2setting(i));
            if (RIG_LEVEL_NR & rig_idx2setting(i)) break;
        }
        //qDebug()<<rig_strparm(RIG_LEVEL_NR & rig_idx2setting(i));
        //qDebug()<<i;

        if (RIG_LEVEL_IS_FLOAT(rig_idx2setting(i))) //float 0..1
        {
            float stepf = 1/my_rig->state.level_gran[i].step.f;
            max = (int)(my_rig->state.level_gran[i].max.f*stepf+fudge);
            step = (int)(stepf+fudge);
            min = max-step+1;
            step = max/step;
        }
        else    //integer
        {
            step = my_rig->state.level_gran[i].step.i;
            max = my_rig->state.level_gran[i].max.i;
            min = my_rig->state.level_gran[i].min.i;
        }
        //qDebug()<<max<<min<<step;

        ui->spinBox_NR->setMaximum(max);
        ui->spinBox_NR->setMinimum(min);
        ui->spinBox_NR->setSingleStep(step);
    }
    else
    {
        ui->checkBox_NR->setEnabled(false);
        ui->spinBox_NR->setEnabled(false);
    }

    //* Clarifier
    rigSet.rit = 1;
    if (!rig_has_set_func(my_rig, RIG_FUNC_XIT)) ui->radioButton_clarXIT->setEnabled(false);

    //* CW
    if (!rig_has_set_func(my_rig, RIG_FUNC_FBKIN)) ui->checkBox_BKIN->setEnabled(false);
    if (!rig_has_set_func(my_rig, RIG_FUNC_APF)) ui->checkBox_APF->setEnabled(false);
    if (guiConf.cwKeyerMode == 0 && !rig_has_set_level(my_rig, RIG_LEVEL_KEYSPD)) ui->spinBox_WPM->setEnabled(false);

    //* FM
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
    //if (my_rig->caps->rig_model != 2)   //Hamlib 4.4 has bug for rigctld and targetable_vfo, skip check
    //{
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
    //}
    //else    //NET rigctl, as workaround assume targetable_vfo
    //{
    //    rigCap.freqSub = 1;
    //    rigCap.modeSub = 1;
    //}

    //* Menu
    ui->action_Command->setEnabled(true);
    ui->action_RadioInfo->setEnabled(true);

    guiCmd.rangeList = 1;   //update range list
    guiCmd.antList = 1; //update antenna list
    guiCmd.toneList = 1;    //update tone list
    guiCmd.tabList = 1; //select tab
}


void MainWindow::loadGuiConfig(QString configFileName)
{
    QSettings configFile(configFileName, QSettings::IniFormat);

    guiConf.vfoDisplayMode = configFile.value("vfoDisplayMode", 0).toInt();
    guiConf.darkTheme = configFile.value("darkTheme", false).toBool();
    guiConf.cwKeyerMode = configFile.value("cwKeyerMode", 0).toInt();
    guiConf.voiceKeyerMode = configFile.value("voiceKeyerMode", 0).toInt();
    guiConf.peakHold = configFile.value("peakHold", true).toBool();
    guiConf.debugMode = configFile.value("debugMode", false).toBool();
    guiConf.rigModelSort = configFile.value("rigModelSort", false).toBool();

    //for (size_t i=0; i<4; i++)
    guiConf.vfoDialStep[0][0] = 10;
    guiConf.vfoDialStep[0][1] = 100;
    guiConf.vfoDialStep[0][2] = 100;
    guiConf.vfoDialStep[0][3] = 500;

    //Window settings
    restoreGeometry(configFile.value("WindowSettings/geometry").toByteArray());
    restoreState(configFile.value("WindowSettings/state").toByteArray());
}


void MainWindow::loadRigConfig(QString configFileName)
{
    QSettings configFile(configFileName, QSettings::IniFormat);

    rigCom.rigModel = configFile.value("rigModel", 0).toUInt();
    rigCom.rigPort = configFile.value("rigPort").toString();
    rigCom.serialSpeed = configFile.value("serialSpeed", 9600).toUInt();
    rigCom.civAddr = configFile.value("civAddress", 0).toInt();
    if (configFile.contains("serialDataBits"))  //For backward compatibility with CatRadio v.< 1.4.0
    {
        rigCom.serialDataBits = configFile.value("serialDataBits", 8).toUInt();
        rigCom.serialParity = configFile.value("serialParity", 0).toUInt();
        rigCom.serialStopBits = configFile.value("serialStopBits", 2).toUInt();
        rigCom.serialHandshake = configFile.value("serialHandshake", 0).toUInt();
    }
    rigCom.netRigctl = configFile.value("netRigctl", false).toBool();
    rigCom.rigRefresh = configFile.value("rigRefresh", 100).toInt();
    rigCom.fullPoll = configFile.value("fullPolling", true).toBool();
    rigCom.autoConnect = configFile.value("autoConnect", false).toBool();
    rigCom.autoPowerOn = configFile.value("autoPowerOn", false).toBool();
}


void MainWindow::audioOutputInit(QString configFileName)
{
    //Set audio file names associated to keyer memory buttons
    QSettings configFile(configFileName, QSettings::IniFormat);
    voiceKConf.memoryFile[0] = configFile.value("VoiceKeyer/voiceMemoryFile1", "").toString();
    voiceKConf.memoryFile[1] = configFile.value("VoiceKeyer/voiceMemoryFile2", "").toString();
    voiceKConf.memoryFile[2] = configFile.value("VoiceKeyer/voiceMemoryFile3", "").toString();
    voiceKConf.memoryFile[3] = configFile.value("VoiceKeyer/voiceMemoryFile4", "").toString();
    voiceKConf.memoryFile[4] = configFile.value("VoiceKeyer/voiceMemoryFile5", "").toString();
    voiceKConf.audioOutput = configFile.value("VoiceKeyer/audioOutput").toString();
    voiceKConf.audioOutputVolume = configFile.value("VoiceKeyer/audioOutputVolume", 10).toInt();

    audioPlayer = new QMediaPlayer(this);
    //Connect signal playback state change with slot on_voiceKeyerStateChanged function
    connect(audioPlayer, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::on_voiceKeyerStateChanged);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    audioOutput = new QAudioOutput(this);

    //Select audio output device
    //QAudioDevice audioDevice = configFile.value("VoiceKeyer/audioOutput", QAudioDevice::Null).value<QAudioDevice>();  //Do not work
    QAudioDevice audioDevice = QMediaDevices::defaultAudioOutput(); //Select default audio output device as first attempt
    //Search in the system audio output devices for a device with the selected name
    audioDevices = new QMediaDevices(this);
    const QList<QAudioDevice> devices = audioDevices->audioOutputs();
    for (const QAudioDevice &deviceInfo : devices)
        if (voiceKConf.audioOutput == deviceInfo.description()) audioDevice = deviceInfo;
    audioOutput->setDevice(audioDevice);
    audioPlayer->setAudioOutput(audioOutput);

    //Set output volume
    audioOutput->setVolume((float)(voiceKConf.audioOutputVolume/10));    //Float 0 min - 1 max
#else
    /* QT5 QMediaPlayer, is not possible to play sound other than the default audio device
     * possible solution using obsolete class QAudioOutputSelectorControl or play sound with QAudioOutput
    for (QAudioDeviceInfo &deviceInfo: QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
        if (voiceKConf.audioOutput == deviceInfo.deviceName())
        {
            m_audioOutput.reset(new QAudioOutput(deviceInfo, deviceInfo.preferredFormat()));
            audioDeviceInfo = deviceInfo;
            qDebug() << audioDeviceInfo.deviceName();
        }
    */

    audioPlayer->setVolume((int)(voiceKConf.audioOutputVolume*10));    //Int 0 min - 100 max
#endif
}


void MainWindow::loadCwKeyerConfig(QString configFileName)
{
    QSettings configFile(configFileName, QSettings::IniFormat);

    //Set keyer COM port name
    cwKConf.comPort = configFile.value("CWKeyer/comPort", "").toString();

    //Set auto-connect flag
    cwKConf.autoConnect = configFile.value("CWKeyer/autoConnect", false).toBool();

    //Set CW strings associated to keyer memory buttons
    cwKConf.memoryString[0] = configFile.value("CWKeyer/cwMemoryString1", "").toByteArray();
    cwKConf.memoryString[1] = configFile.value("CWKeyer/cwMemoryString2", "").toByteArray();
    cwKConf.memoryString[2] = configFile.value("CWKeyer/cwMemoryString3", "").toByteArray();
    cwKConf.memoryString[3] = configFile.value("CWKeyer/cwMemoryString4", "").toByteArray();
    cwKConf.memoryString[4] = configFile.value("CWKeyer/cwMemoryString5", "").toByteArray();

    if (!winkeyer) winkeyer = new WinKeyer;
}


void MainWindow::guiUpdate()
{
    //* Power button
    if (rigGet.onoff == RIG_POWER_ON)
    {
        ui->pushButton_Power->setChecked(true);
        //ui->pushButton_Power->setStyleSheet("QPushButton {color: limegreen;}");
    }
    else if (rigGet.onoff == RIG_POWER_OFF)
    {
        if(timer->isActive())
        {
            timer->stop();

            ui->pushButton_Power->setChecked(false);
            //ui->pushButton_Power->setStyleSheet("");

            //Reset Smeter
            rigGet.sMeter.i = -54;
            ui->progressBar_Smeter->resetPeakValue();

            //Reset VFOs
            rigGet.freqMain = 0;
            rigGet.freqSub = 0;

            ui->statusbar->showMessage("Radio off");
        }
    }

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
    if (!rigCmd.mode) ui->comboBox_Mode->setCurrentText(rig_strrmode(rigGet.mode));
    if (!rigCmd.modeSub) ui->comboBox_ModeSub->setCurrentText(rig_strrmode(rigGet.modeSub));

    if (guiCmd.tabList) //Select appropriate mode function tab
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
                int j = 0;
                //qDebug() << modes << mode << bwidth_list.width;
                QRegularExpression rx("\\b"+mode+"?\\s");
                if (modes.contains (rx) && bwidth_list.width != RIG_FLT_ANY)
                {   //sort the BW list
                    if (ui->comboBox_BW->count() == 0) ui->comboBox_BW->addItem(QString::number(bwidth_list.width));    //first line
                    else while (j <= ui->comboBox_BW->count()) //sort descending by filter width
                    {
                        ui->comboBox_BW->setCurrentIndex(j);
                        QString bwidthCurrent = ui->comboBox_BW->currentText();
                        if (bwidth_list.width > bwidthCurrent.toLong())
                        {
                            ui->comboBox_BW->insertItem(j, QString::number(bwidth_list.width));
                            break;
                        }
                        else j++;
                    }
                }
            }
            //else qDebug() << "vuoto" << rigGet.mode;
       }
       //ui->comboBox_BW->model()->sort(0, Qt::DescendingOrder);
       guiCmd.bwidthList = 0;
    }

    if (!rigCmd.bwidth) ui->comboBox_BW->setCurrentText(QString::number(rigGet.bwidth));
    ui->checkBox_NAR->setChecked(rigGet.bwNarrow);

    //* Range list
    if (guiCmd.rangeList)
    {
        for (i=0; i<HAMLIB_FRQRANGESIZ; i++)    //Tx range list
        {
            //qDebug()<<rigGet.freqMain<<my_rig->state.tx_range_list[i].startf<<my_rig->state.tx_range_list[i].endf;
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
        if (!rig_has_get_func(my_rig, RIG_FUNCTION_GET_ANT)) ui->comboBox_Ant->addItem("");
        else if (my_rig->state.tx_range_list[rigGet.rangeListRxIndex].ant == RIG_ANT_NONE) ui->comboBox_Ant->addItem("NONE");  //RIG_ANT_NONE
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
    if (!rigCmd.tuner) ui->radioButton_Tuner->setChecked(rigGet.tuner);
    if (!rigCmd.agc) ui->comboBox_AGC->setCurrentText(rig_stragclevel(rigGet.agc));
    if (!rigCmd.att) ui->comboBox_Att->setCurrentText(QString::number(rigGet.att));
    if (!rigCmd.pre) ui->comboBox_Preamp->setCurrentText(QString::number(rigGet.pre));

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

        //Smeter and subMeter
        if (!ui->progressBar_Smeter->getTx())
        {
            ui->progressBar_Smeter->setTx(true);
            ui->progressBar_Smeter->setValue(0);
            ui->progressBar_Smeter->resetPeakValue();
        }
        ui->progressBar_Smeter->setValue(rigGet.powerMeter.f*100);
        ui->progressBar_subMeter->setValue(rigGet.subMeter.f);
        if (rigGet.hiSWR.f > 2) ui->label_hiSWR->setVisible(true);

        //Voice keyer
        if (guiConf.voiceKeyerMode == 1 && rigCmd.voiceSend) audioPlayer->play();
    }
    else    //RIG_PTT_OFF
    {
        //ui->pushButton_PTT->setChecked(false);
        if (rigGet.vfoTx == rigGet.vfoSub) ui->label_vfoSub->setStyleSheet("QLabel {}");
        else ui->label_vfoMain->setStyleSheet("QLabel {}");

        if (ui->progressBar_Smeter->getTx())
        {
            ui->progressBar_Smeter->setTx(false);
            ui->progressBar_Smeter->setValue(-54);
            ui->progressBar_Smeter->resetPeakValue();
            ui->label_hiSWR->setVisible(false);
            //if (rigSet.meter == RIG_LEVEL_SWR) ui->progressBar_subMeter->setValue(1.0);
            //else ui->progressBar_subMeter->setValue(0.0);
            //ui->progressBar_subMeter->resetPeakValue();
        }
        ui->progressBar_Smeter->setValue(rigGet.sMeter.i);
        if (rigSet.meter == RIG_LEVEL_SWR) ui->progressBar_subMeter->setValue(1.0);
        else ui->progressBar_subMeter->setValue(0.0);
    }

    //* Levels
    if (!ui->verticalSlider_RFpower->isSliderDown() && !rigCmd.rfPower) ui->verticalSlider_RFpower->setValue((int)(rigGet.rfPower*100+fudge));
    if (!ui->verticalSlider_RFgain->isSliderDown() && !rigCmd.rfGain) ui->verticalSlider_RFgain->setValue((int)(rigGet.rfGain*100+fudge));
    if (!ui->verticalSlider_AFGain->isSliderDown() && !rigCmd.afGain) ui->verticalSlider_AFGain->setValue((int)(rigGet.afGain*100+fudge));
    if (!ui->verticalSlider_Squelch->isSliderDown() && !rigCmd.squelch) ui->verticalSlider_Squelch->setValue((int)(rigGet.squelch*100+fudge));

    //* MIC
    if (!ui->verticalSlider_micGain->isSliderDown() && !rigCmd.micGain) ui->verticalSlider_micGain->setValue((int)(rigGet.micGain*100+fudge));
    if (!ui->verticalSlider_micMonitor->isSliderDown() && !rigCmd.micMonLevel) ui->verticalSlider_micMonitor->setValue((int)(rigGet.micMonLevel*100+fudge));
    if (!ui->verticalSlider_micCompressor->isSliderDown() && !rigCmd.micCompLevel) ui->verticalSlider_micCompressor->setValue((int)(rigGet.micCompLevel*100+fudge));
    if (!rigCmd.micComp) ui->checkBox_micCompressor->setChecked(rigGet.micComp);
    if (!rigCmd.micMon) ui->checkBox_micMonitor->setChecked(rigGet.micMon);

    //* Filter
    if (!rigCmd.noiseBlanker) ui->checkBox_NB->setChecked(rigGet.noiseBlanker);
    if (!rigCmd.noiseBlanker2) ui->checkBox_NB2->setChecked(rigGet.noiseBlanker2);
    if (!rigCmd.noiseReduction) ui->checkBox_NR->setChecked(rigGet.noiseReduction);
    if (!rigCmd.noiseReductionLevel) ui->spinBox_NR->setValue((int)(rigGet.noiseReductionLevel*ui->spinBox_NR->maximum()+fudge));
    if (!rigCmd.notchFilter) ui->checkBox_NF->setChecked(rigGet.notchFilter);
    if (!ui->horizontalSlider_IFshift->isSliderDown() && !rigCmd.ifShift) ui->horizontalSlider_IFshift->setValue(rigGet.ifShift);

    //* Clarifier
    if (!rigCmd.clar) ui->checkBox_clar->setChecked(rigGet.clar);
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
    if (!rigCmd.bkin) ui->checkBox_BKIN->setChecked(rigGet.bkin);
    if (!rigCmd.apf) ui->checkBox_APF->setChecked(rigGet.apf);
    if (guiConf.cwKeyerMode == 0 && !rigCmd.wpm) ui->spinBox_WPM->setValue(rigGet.wpm);

    //* FM
    if (rigGet.rptShift == RIG_RPT_SHIFT_MINUS && !rigCmd.rptShift) ui->radioButton_RPTshiftMinus->setChecked(true);    //-
    else if (rigGet.rptShift == RIG_RPT_SHIFT_PLUS && !rigCmd.rptShift) ui->radioButton_RPTshiftPlus->setChecked(true); //+
    else ui->radioButton_RPTshiftSimplex->setChecked(true); //Simplex
    if (!rigCmd.rptOffset) ui->spinBox_RPToffset->setValue(rigGet.rptOffset/1000);  //Offset (kHz)

    switch (rigGet.toneType)
    {
    case (1): ui->comboBox_toneType->setCurrentText("1750Hz"); break;
    case (2): ui->comboBox_toneType->setCurrentText("TONE"); break;
    case (3): ui->comboBox_toneType->setCurrentText("TSQL"); break;
    case (4): ui->comboBox_toneType->setCurrentText("DCS"); break;
    default: ui->comboBox_toneType->setCurrentText(""); break;
    }

    if (rigGet.toneType == 2 || rigGet.toneType == 3) ui->comboBox_toneFreq->setCurrentText(QString::number(rigGet.tone/10.0));  //CTCSS
    else if (rigGet.toneType == 4) ui->comboBox_toneFreq->setCurrentText(QString::number(rigGet.tone));  //DCS
}

void MainWindow::rigUpdate()
{
    rigDaemon->rigUpdate(my_rig);
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

    ui->progressBar_subMeter->resetPeakValue();
}


void MainWindow::on_voiceKeyerStateChanged()
{
    //qDebug() << audioPlayer->mediaStatus() << rigCmd.voiceSend << rigGet.ptt;
    if (rigCmd.voiceSend >= 1)
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        if (audioPlayer->mediaStatus() == QMediaPlayer::LoadedMedia && audioPlayer->source() == QUrl::fromLocalFile(voiceKConf.memoryFile[rigCmd.voiceSend - 1]))    //LoadedMedia
#else
        if (audioPlayer->mediaStatus() == QMediaPlayer::LoadedMedia && audioPlayer->media() == QUrl::fromLocalFile(voiceKConf.memoryFile[rigCmd.voiceSend - 1]))
#endif
        {
            ui->pushButton_PTT->toggle();
            ui->statusbar->showMessage("Playing audio...");
        }
        else if (audioPlayer->mediaStatus() == QMediaPlayer::NoMedia || audioPlayer->mediaStatus() == QMediaPlayer::InvalidMedia)    //NoMedia or InvalidMedia
        {
            rigCmd.voiceSend = 0;
            ui->statusbar->showMessage("Audio file error!", 5000);
        }
        else if (audioPlayer->mediaStatus() == QMediaPlayer::EndOfMedia && rigGet.ptt) //EndOfMedia
        {
            rigCmd.voiceSend = 0;
            ui->pushButton_PTT->toggle();
            ui->statusbar->clearMessage();
        }
    }
}


bool MainWindow::checkHamlibVersion(int major, int minor, int revision)
{
    QString hamlibVer = rig_version();
    qInfo() << hamlibVer;
    QRegularExpression hamlibVerExp("(?P<major>\\d)\\.(?P<minor>\\d)\\.?(?P<revision>\\d)?");

    QRegularExpressionMatch hamlibVerMatch = hamlibVerExp.match(hamlibVer);

    if (hamlibVerMatch.hasMatch())
    {
        int majorVer = hamlibVerMatch.captured("major").toInt();
        int minorVer = hamlibVerMatch.captured("minor").toInt();
        int revisionVer = hamlibVerMatch.captured("revision").toInt();

        //qDebug()<<majorVer<<minorVer<<revisionVer;

        if (majorVer > major) return true;
        else if (majorVer < major) return false;
        else if (minorVer > minor) return true;    //& majorVer=major
        else if (minorVer < minor) return false;   //& majorVer=major
        else if (revisionVer < revision) return false; //& majorVer=major, minorVer=minor
        else return true;   //revisionVer>=revision & majorVer=major, minorVer=minor
    }
    else return false;
}


//***** PushButton *****

void MainWindow::on_pushButton_Connect_toggled(bool checked)
{
    qInfo() << "Connect" << checked;

    QString connectMsg;

    if (checked && rigCom.connected == 0)
    {
        int retcode;

        my_rig = rigDaemon->rigConnect(&retcode);   //Open Rig connection

        if (retcode != RIG_OK)   //Connection error
        {
            rigCom.connected = 0;
            connectMsg = "Connection error: ";
            connectMsg.append(rigerror(retcode));
            ui->pushButton_Connect->setChecked(false);  //Uncheck the button

            qCritical() << connectMsg;
        }
        else    //Rig connected
        {
            rigCom.connected = 1;
            guiInit();
            connectMsg = "Connected to ";
            connectMsg.append(my_rig->state.model_name);
            if (rigCap.onoff == 0 || rigGet.onoff == RIG_POWER_ON || rigGet.onoff == RIG_POWER_UNKNOWN)
            {
                freq_t retfreq;
                retcode = rig_get_freq(my_rig, RIG_VFO_CURR, &retfreq); //double check if rig is on, by getting the current frequency
                if (retcode==RIG_OK && retfreq!=0)
                {
                    rigGet.onoff = RIG_POWER_ON;    //force it for rigCap.onoff = 0 || rigGet.onoff = RIG_POWER_UNKNOWN
                    timer->start(rigCom.rigRefresh);
                }
                else rigGet.onoff = RIG_POWER_OFF;
            }

            if (guiConf.cwKeyerMode && cwKConf.autoConnect) //WinKeyer
            {
                if (!winkeyer->init(cwKConf.comPort)) //Open serial port
                {
                    //wait 2 seconds, serial port open causes reset in Arduino for K3NG CW keyer
                    QThread::msleep(2000);

                    winkeyer->version = winkeyer->open();   //Open WinKeyer host

                    if (winkeyer->version)    //Read WinKeyer version
                    {
                        connectMsg.append(QString(", WinKeyer v. %1").arg(winkeyer->version));
                        winkeyer->isOpen = true;

                        winkeyer->setWpmSpeed(ui->spinBox_WPM->value());    //Set WPM speed
                    }
                }
            }
        }
    }
    else   //Button unchecked
    {
        if (rigCom.connected)   //Close RIG
        {
            if (rigSet.ptt == RIG_PTT_OFF)  //Disconnect only if PTT off
            {
                rigCom.connected = 0;
                if(timer->isActive()) timer->stop();
                rig_close(my_rig);  //Close the communication to the rig
                connectMsg = "Disconnected";

                //Reset meters
                ui->progressBar_Smeter->setValue(-54);
                ui->progressBar_Smeter->resetPeakValue();
                setSubMeter();
            }
            else
            {
                ui->pushButton_Connect->setChecked(false);  //Uncheck the button
                connectMsg = "Warning PTT on!";
            }
        }

        if (guiConf.cwKeyerMode == 1)   //Close WinKeyer
        {
            if (winkeyer && winkeyer->isOpen) winkeyer->close();
        }
    }

    ui->statusbar->showMessage(connectMsg);
}

void MainWindow::on_pushButton_Power_toggled(bool checked)
{
    qInfo() << "Power" << checked;

    if (checked && !rigGet.onoff)
    {
        retcode = rig_set_powerstat(my_rig, RIG_POWER_ON);
        if (retcode != RIG_OK)
        {
            ui->pushButton_Power->setChecked(false);  //Uncheck the button
            ui->statusbar->showMessage(rigerror(retcode));
        }
        else
        {
            rigGet.onoff = RIG_POWER_ON;
            timer->start(rigCom.rigRefresh);
        }
    }
    else if (!checked && rigGet.onoff)
    {
        rigCmd.onoff = 1;
        //Note: the onoff command works only if the PTT is off (see rigdaemon.cpp)
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

//Band
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

//CW keyer
void MainWindow::on_pushButton_CW1_clicked()
{
    if (guiConf.cwKeyerMode == 0) send_cw_mem(1);   //Radio CW keyer
    else if (guiConf.cwKeyerMode == 1 && winkeyer->isOpen && rigCmd.cwSend == 0 && cwKConf.memoryString[0]!="")  //WinKeyer
    {
        rigCmd.cwSend = 1;
        winkeyer->sendString(cwKConf.memoryString[0]);
        rigCmd.cwSend = 0;
    }
}

void MainWindow::on_pushButton_CW2_clicked()
{
    if (guiConf.cwKeyerMode == 0) send_cw_mem(2);
    else if (guiConf.cwKeyerMode == 1 && winkeyer->isOpen && rigCmd.cwSend == 0 && cwKConf.memoryString[1]!="")
    {
        rigCmd.cwSend = 1;
        winkeyer->sendString(cwKConf.memoryString[1]);
        rigCmd.cwSend = 0;
    }
}

void MainWindow::on_pushButton_CW3_clicked()
{
    if (guiConf.cwKeyerMode == 0) send_cw_mem(3);
    else if (guiConf.cwKeyerMode == 1 && winkeyer->isOpen && rigCmd.cwSend == 0 && cwKConf.memoryString[2]!="")
    {
        rigCmd.cwSend = 1;
        winkeyer->sendString(cwKConf.memoryString[2]);
        rigCmd.cwSend = 0;
    }
}

void MainWindow::on_pushButton_CW4_clicked()
{
    if (guiConf.cwKeyerMode == 0) send_cw_mem(4);
    else if (guiConf.cwKeyerMode == 1 && winkeyer->isOpen && rigCmd.cwSend == 0 && cwKConf.memoryString[3]!="")
    {
        rigCmd.cwSend = 1;
        winkeyer->sendString(cwKConf.memoryString[3]);
        rigCmd.cwSend = 0;
    }
}

void MainWindow::on_pushButton_CW5_clicked()
{
    if (guiConf.cwKeyerMode == 0) send_cw_mem(5);
    else if (guiConf.cwKeyerMode == 1 && winkeyer->isOpen && rigCmd.cwSend == 0 && cwKConf.memoryString[4]!="")
    {
        rigCmd.cwSend = 1;
        winkeyer->sendString(cwKConf.memoryString[4]);
        rigCmd.cwSend = 0;
    }
}

//Voice keyer
void MainWindow::on_pushButton_VoiceK1_clicked()
{
    if (guiConf.voiceKeyerMode == 0) send_voice_mem(1); //Radio voice keyer
    else if (rigCmd.voiceSend == 0 && voiceKConf.memoryFile[0]!="")  //CatRadio voice keyer
    {
        rigCmd.voiceSend = 1;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        audioPlayer->setSource(QUrl::fromLocalFile(voiceKConf.memoryFile[0]));  //Load audio file
#else
        audioPlayer->setMedia(QUrl::fromLocalFile(voiceKConf.memoryFile[0]));
#endif
    }
}

void MainWindow::on_pushButton_VoiceK2_clicked()
{
    if (guiConf.voiceKeyerMode == 0) send_voice_mem(2);
    else if (rigCmd.voiceSend == 0 && voiceKConf.memoryFile[1]!="")
    {
        rigCmd.voiceSend = 2;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        audioPlayer->setSource(QUrl::fromLocalFile(voiceKConf.memoryFile[1]));  //Load audio file
#else
        audioPlayer->setMedia(QUrl::fromLocalFile(voiceKConf.memoryFile[1]));
#endif
    }
}

void MainWindow::on_pushButton_VoiceK3_clicked()
{
    if (guiConf.voiceKeyerMode == 0) send_voice_mem(3);
    else if (rigCmd.voiceSend == 0 && voiceKConf.memoryFile[2]!="")
    {
        rigCmd.voiceSend = 3;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        audioPlayer->setSource(QUrl::fromLocalFile(voiceKConf.memoryFile[2]));  //Load audio file
#else
        audioPlayer->setMedia(QUrl::fromLocalFile(voiceKConf.memoryFile[2]));
#endif
    }
}

void MainWindow::on_pushButton_VoiceK4_clicked()
{
    if (guiConf.voiceKeyerMode == 0) send_voice_mem(4);
    else if (rigCmd.voiceSend == 0 && voiceKConf.memoryFile[3]!="")
    {
        rigCmd.voiceSend = 4;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        audioPlayer->setSource(QUrl::fromLocalFile(voiceKConf.memoryFile[3]));  //Load audio file
#else
        audioPlayer->setMedia(QUrl::fromLocalFile(voiceKConf.memoryFile[3]));
#endif
    }
}

void MainWindow::on_pushButton_VoiceK5_clicked()
{
    if (guiConf.voiceKeyerMode == 0) send_voice_mem(5);
    else if (rigCmd.voiceSend == 0 && voiceKConf.memoryFile[4]!="")
    {
        rigCmd.voiceSend = 5;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        audioPlayer->setSource(QUrl::fromLocalFile(voiceKConf.memoryFile[4]));  //Load audio file
#else
        audioPlayer->setMedia(QUrl::fromLocalFile(voiceKConf.memoryFile[4]));
#endif
    }
}


//***** CheckBox ***==
void MainWindow::on_checkBox_micCompressor_toggled(bool checked)
{
    if (checked && !rigGet.micComp)
    {
        rigSet.micComp = 1;
        rigCmd.micComp = 1;
    }
    else if (!checked && rigGet.micComp)
    {
        rigSet.micComp = 0;
        rigCmd.micComp = 1;
    }
}

void MainWindow::on_checkBox_micMonitor_toggled(bool checked)
{
    if (checked && !rigGet.micMon)
    {
        rigSet.micMon = 1;
        rigCmd.micMon = 1;
    }
    else if (!checked && rigGet.micMon)
    {
        rigSet.micMon = 0;
        rigCmd.micMon = 1;
    }
}

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
    int freqStep;

    step = value - prevDial;

    if (step<=99 && step>50) step = value - 100 - prevDial; //More than half turn CW
    else if (step>=-99 && step<-50) step = value + 100 - prevDial;  //More than half turn CCW

    prevDial = value;   //Store actual dial value

    if (step == 10) //Page step CW
        if (fastDial) freqStep = guiConf.vfoDialStep[0][3]; //Fast
        else freqStep = guiConf.vfoDialStep[0][1];  //Normal
    else if (step == -10) //Page step CCW
        if (fastDial) freqStep = -guiConf.vfoDialStep[0][3]; //Fast
        else freqStep = -guiConf.vfoDialStep[0][1];  //Normal
    else if (fastDial) freqStep=guiConf.vfoDialStep[0][2] * step; //Single step Fast
    else freqStep=guiConf.vfoDialStep[0][0] * step; //Single step Normal

    //if (fastDial) freqStep = freqStep * guiConf.vfoDialStep[0][2]; //Fast activated

    //qDebug() << value << step << freqStep;

    if (ui->radioButton_VFOSub->isChecked()) //dial VFO Sub
    {
        rigSet.freqSub = rigGet.freqSub + freqStep;
        rigCmd.freqSub = 1;
    }
    else    //dial VFO Main
    {
        rigSet.freqMain = rigGet.freqMain + freqStep;
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
    if (!rigCmd.mode)
    {
        rigSet.mode = rig_parse_mode(ui->comboBox_Mode->itemText(index).toLatin1());
        rigCmd.mode = 1;
    }
}

void MainWindow::on_comboBox_ModeSub_activated(int index)
{
    if (!rigCmd.modeSub)
    {
        rigSet.modeSub = rig_parse_mode(ui->comboBox_ModeSub->itemText(index).toLatin1());
        rigCmd.modeSub = 1;
    }
}

void MainWindow::on_comboBox_BW_activated(int index)
{
    if (!rigCmd.bwidth)
    {
        rigSet.bwidth = ui->comboBox_BW->itemText(index).toInt();
        rigCmd.bwidth = 1;
    }
}

void MainWindow::on_comboBox_AGC_activated(int index)
{
    if (!rigCmd.agc)
    {
        rigSet.agc = levelagcstr(ui->comboBox_AGC->itemText(index));
        rigCmd.agc = 1;
    }
}

void MainWindow::on_comboBox_Att_activated(int index)
{
    if (!rigCmd.att)
    {
        rigSet.att = ui->comboBox_Att->itemText(index).toInt();
        rigCmd.att = 1;
    }
}

void MainWindow::on_comboBox_Preamp_activated(int index)
{
    if (!rigCmd.pre)
    {
        rigSet.pre = ui->comboBox_Preamp->itemText(index).toInt();
        rigCmd.pre = 1;
    }
}

void MainWindow::on_comboBox_Ant_activated(int index)
{
    if (!rigCmd.ant)
    {
        rigSet.ant = antstr(ui->comboBox_Ant->itemText(index));
        rigCmd.ant = 1;
    }
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
    if (!rigCmd.noiseReductionLevel)
    {
        rigSet.noiseReductionLevel = (float)(arg1) / ui->spinBox_NR->maximum();
        rigCmd.noiseReductionLevel = 1;
    }
}

void MainWindow::on_spinBox_WPM_valueChanged(int arg1)
{
    if (guiConf.cwKeyerMode == 0 && !rigCmd.wpm)
    {
        rigSet.wpm = arg1;
        rigCmd.wpm = 1;
    }
    else if (guiConf.cwKeyerMode == 1 && winkeyer->isOpen && rigCmd.cwSend == 0)
    {
        winkeyer->setWpmSpeed(arg1);
    }
}

void MainWindow::on_spinBox_RPToffset_valueChanged(int arg1)
{
    if (!rigCmd.rptOffset)
    {
        rigSet.rptOffset = arg1*1000;
        rigCmd.rptOffset = 1;
    }
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


void MainWindow::on_verticalSlider_micGain_valueChanged(int value)
{
    if (!rigCmd.micGain && !ui->verticalSlider_micGain->isSliderDown())
    {
        rigSet.micGain = (float)(value)/100;
        if (rigSet.micGain != rigGet.micGain) rigCmd.micGain = 1;
    }
}

void MainWindow::on_verticalSlider_micGain_sliderReleased()
{
    if (!rigCmd.micGain)
    {
        rigSet.micGain = (float)(ui->verticalSlider_micGain->value())/100;
        rigCmd.micGain = 1;
    }
}


void MainWindow::on_verticalSlider_micCompressor_valueChanged(int value)
{
    if (!rigCmd.micCompLevel && !ui->verticalSlider_micCompressor->isSliderDown())
    {
        rigSet.micCompLevel = (float)(value)/100;
        if (rigSet.micCompLevel != rigGet.micCompLevel) rigCmd.micCompLevel = 1;
    }
}

void MainWindow::on_verticalSlider_micCompressor_sliderReleased()
{
    if (!rigCmd.micCompLevel)
    {
        rigSet.micCompLevel = (float)(ui->verticalSlider_micCompressor->value())/100;
        rigCmd.micCompLevel = 1;
    }
}


void MainWindow::on_verticalSlider_micMonitor_valueChanged(int value)
{
    if (!rigCmd.micMonLevel && !ui->verticalSlider_micMonitor->isSliderDown())
    {
        rigSet.micMonLevel = (float)(value)/100;
        if (rigSet.micMonLevel != rigGet.micMonLevel) rigCmd.micMonLevel = 1;
    }
}

void MainWindow::on_verticalSlider_micMonitor_sliderReleased()
{
    if (!rigCmd.micMonLevel)
    {
        rigSet.micMonLevel = (float)(ui->verticalSlider_micMonitor->value())/100;
        rigCmd.micMonLevel = 1;
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
    qInfo() << "DialogConfig";
    DialogConfig config;
    config.setModal(true);
    config.exec();
}

void MainWindow::on_action_Setup_triggered()
{
    qInfo() << "DialogSetup";
    DialogSetup setup;
    setup.setModal(true);
    setup.exec();

    ui->lineEdit_vfoMain->setMode(guiConf.vfoDisplayMode);
    ui->lineEdit_vfoSub->setMode(guiConf.vfoDisplayMode);

    if (guiConf.cwKeyerMode == 1)
    {
        ui->actionCW_Keyer->setEnabled(true);
        loadCwKeyerConfig("catradio.ini");
    }
    else ui->action_Voice_Keyer->setEnabled(false);

    if (guiConf.voiceKeyerMode == 1)
    {
        ui->action_Voice_Keyer->setEnabled(true);
        audioOutputInit("catradio.ini");
    }
    else ui->action_Voice_Keyer->setEnabled(false);
}

void MainWindow::on_action_Voice_Keyer_triggered()
{
    qInfo() << "DialogVoiceKeyer";
    DialogVoiceKeyer voiceKeyer;
    voiceKeyer.setModal(true);
    voiceKeyer.exec();

    audioOutputInit("catradio.ini");
}

void MainWindow::on_actionCW_Keyer_triggered()
{
    qInfo() << "DialogCWKeyer";
    DialogCWKeyer cwKeyer(winkeyer);
    cwKeyer.setModal(true);
    cwKeyer.exec();

    loadCwKeyerConfig("catradio.ini");
}

void MainWindow::on_action_RadioInfo_triggered()
{
    qInfo() << "DialogRadioInfo";
    if (!radioInfo) radioInfo = new DialogRadioInfo(my_rig, this);
    radioInfo->setModal(true);
    radioInfo->exec();
}

void MainWindow::on_action_Command_triggered()
{
    qInfo() << "DialogCommand";
    //DialogCommand command;
    //command.setModal(true);
    //command.exec();

    if (!command)
    {
        command = new DialogCommand(my_rig, this);
    }
    command->setModal(false);
    command->show();
    command->raise();
    command->activateWindow();
}

void MainWindow::on_actionNET_rigctl_triggered()
{
    qInfo() << "DialogNetRigctl";
    DialogNetRigctl configNetRigctl;
    configNetRigctl.setModal(true);
    configNetRigctl.exec();
}

void MainWindow::on_action_AboutCatRadio_triggered()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("About");
    msgBox.setTextFormat(Qt::RichText);
    QString version = QString::number(VERSION_MAJ)+"."+QString::number(VERSION_MIN)+"."+QString::number(VERSION_MIC);
    msgBox.setText("<b>CatRadio</b> <i>Radio control software</i><br/>version "+version+" "+RELEASE_DATE);
    msgBox.setInformativeText("<p>Copyright (C) 2022-2026 Gianfranco Sordetti IZ8EWD<br/>"
                              "<a href='https://www.pianetaradio.it' style='color: #668fb8'>www.pianetaradio.it</a></p>"
                              "<p>This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.<br/>"
                              "This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.<br/>"
                              "You should have received a copy of the GNU General Public License along with this program.  If not, see <a href='http://www.gnu.org/licenses/' style='color: #668fb8'>www.gnu.org/licenses</a>.</p>");
    msgBox.setIcon(QMessageBox::Information);
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

void MainWindow::on_action_AboutDarkTheme_triggered()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("About Dark Theme");
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText("<b>QDarkStyleSheet</b>");
    msgBox.setInformativeText("<p>Copyright (c) 2013-2019 Colin Duquesnoy<br/>"
                              "<a href='https://github.com/ColinDuquesnoy/QDarkStyleSheet' style='color: #668fb8'>github.com/ColinDuquesnoy/QDarkStyleSheet</a></p>"
                              "<p>The MIT License (MIT)</p>"
                              "<p>Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:<br/>"
                              "The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.<br/>"
                              "THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.</p>"
                              "<p>Images contained in this project is licensed under CC-BY license.</p>");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

void MainWindow::on_action_CatRadioHomepage_triggered()
{
    QUrl homepage("https://www.pianetaradio.it/blog/catradio/");
    QDesktopServices::openUrl(homepage);
}
