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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #include <QMediaDevices>
#endif
#include <QSettings>

#include "rig.h"

#define RELEASE_DATE __DATE__
#define VERSION_MAJ 1
#define VERSION_MIN 5
#define VERSION_MIC 0


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();    

public slots:
    void guiUpdate();
    void rigUpdate();   //Slot for QTimer
    void on_rigDaemonResultReady();    //Slot for rigDaemon resultReady
    void on_vfoDisplayMainValueChanged(int value); //Slot for vfoDisplay Main valueChanged
    void on_vfoDisplaySubValueChanged(int value); //Slot for vfoDisplay Sub valueChanged
    void on_voiceKeyerStateChanged();

signals:

private slots:
    void on_pushButton_Connect_toggled(bool checked);
    void on_pushButton_Power_toggled(bool checked);
    void on_pushButton_PTT_toggled(bool checked);
    void on_pushButton_Split_toggled(bool checked);
    void on_pushButton_AB_clicked();
    void on_pushButton_AeqB_clicked();

    void on_dial_valueChanged(int value);

    void on_comboBox_Mode_activated(int index);
    void on_comboBox_ModeSub_activated(int index);

    void on_pushButton_Fast_toggled(bool checked);

    void on_pushButton_Band160_clicked();
    void on_pushButton_Band80_clicked();
    void on_pushButton_Band60_clicked();
    void on_pushButton_Band40_clicked();
    void on_pushButton_Band30_clicked();
    void on_pushButton_Band20_clicked();
    void on_pushButton_Band17_clicked();
    void on_pushButton_Band15_clicked();
    void on_pushButton_Band12_clicked();
    void on_pushButton_Band10_clicked();
    void on_pushButton_Band6_clicked();

    void on_pushButton_Tune_clicked();
    void on_radioButton_Tuner_toggled(bool checked);

    void on_pushButton_BandDown_clicked();
    void on_pushButton_BandUp_clicked();

    void on_pushButton_QSplit_clicked();

    void on_action_Connection_triggered();

    void on_comboBox_BW_activated(int index);

    void on_checkBox_NAR_toggled(bool checked);

    void on_checkBox_BKIN_toggled(bool checked);

    void on_comboBox_AGC_activated(int index);

    void on_comboBox_Att_activated(int index);

    void on_comboBox_Preamp_activated(int index);

    void on_comboBox_Ant_activated(int index);

    void on_action_AboutCatRadio_triggered();

    void on_checkBox_NB_toggled(bool checked);

    void on_checkBox_NR_toggled(bool checked);

    void on_checkBox_NF_toggled(bool checked);

    void on_comboBox_Meter_activated(int index);

    void on_spinBox_NR_valueChanged(int arg1);

    void on_horizontalSlider_IFshift_valueChanged(int value);

    void on_pushButton_Band2_clicked();

    void on_pushButton_Band70_clicked();

    void on_pushButton_BandGen_clicked();

    void on_action_Setup_triggered();
    void on_action_CatRadioHomepage_triggered();
    void on_action_AboutQT_triggered();
    void on_action_AboutHamLib_triggered();

    void on_verticalSlider_AFGain_valueChanged(int value);
    void on_verticalSlider_Squelch_valueChanged(int value);
    void on_verticalSlider_RFpower_valueChanged(int value);
    void on_verticalSlider_RFgain_valueChanged(int value);

    void on_spinBox_WPM_valueChanged(int arg1);

    void on_checkBox_APF_toggled(bool checked);

    void on_radioButton_RPTshiftSimplex_toggled(bool checked);

    void on_radioButton_RPTshiftMinus_toggled(bool checked);

    void on_radioButton_RPTshiftPlus_toggled(bool checked);

    void on_comboBox_toneType_activated(int index);

    void on_comboBox_toneFreq_activated(int index);

    void on_spinBox_RPToffset_valueChanged(int arg1);

    void on_pushButton_left_clicked();
    void on_pushButton_right_clicked();

    void on_checkBox_clar_toggled(bool checked);
    void on_pushButton_clarClear_clicked();
    void on_horizontalSlider_clar_valueChanged(int value);
    void on_radioButton_clarRIT_toggled(bool checked);
    void on_radioButton_clarXIT_toggled(bool checked);

    void on_verticalSlider_RFpower_sliderReleased();

    void on_verticalSlider_RFgain_sliderReleased();

    void on_verticalSlider_AFGain_sliderReleased();

    void on_verticalSlider_Squelch_sliderReleased();

    void on_horizontalSlider_IFshift_sliderReleased();

    void on_horizontalSlider_clar_sliderReleased();

    void on_checkBox_NB2_toggled(bool checked);

    void on_verticalSlider_micGain_valueChanged(int value);

    void on_verticalSlider_micGain_sliderReleased();

    void on_verticalSlider_micCompressor_valueChanged(int value);

    void on_verticalSlider_micCompressor_sliderReleased();

    void on_verticalSlider_micMonitor_valueChanged(int value);

    void on_verticalSlider_micMonitor_sliderReleased();

    void on_checkBox_micCompressor_toggled(bool checked);

    void on_checkBox_micMonitor_toggled(bool checked);

    void on_action_Command_triggered();

    void on_action_RadioInfo_triggered();

    void on_action_AboutDarkTheme_triggered();

    void on_pushButton_CW1_clicked();
    void on_pushButton_CW2_clicked();
    void on_pushButton_CW3_clicked();
    void on_pushButton_CW4_clicked();
    void on_pushButton_CW5_clicked();

    void on_actionNET_rigctl_triggered();

    void on_action_Voice_Keyer_triggered();
    void on_pushButton_VoiceK1_clicked();
    void on_pushButton_VoiceK2_clicked();
    void on_pushButton_VoiceK3_clicked();
    void on_pushButton_VoiceK4_clicked();
    void on_pushButton_VoiceK5_clicked();

private:
    Ui::MainWindow *ui;
    QTimer *timer;

    QMediaPlayer *audioPlayer;
    QAudioOutput *audioOutput;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QMediaDevices *audioDevices;
#endif

    RIG *my_rig;

    void guiInit();
    void audioOutputInit(QString configFileName);
    void setSubMeter();
    bool checkHamlibVersion(int major, int minor, int revision);
};

#endif // MAINWINDOW_H
