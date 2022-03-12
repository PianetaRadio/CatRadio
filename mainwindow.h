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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#define RELEASE_DATE __DATE__
#define VERSION_MAJ 1
#define VERSION_MIN 0
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
    void on_rigDaemonResultReady();    //Slot for rigDaemon resultReady
    void on_vfoDisplayValueChanged(int value); //Slot for vfoDisplay valueChanged

signals:

private slots:
    void on_pushButton_Connect_toggled(bool checked);
    void on_pushButton_Power_toggled(bool checked);
    void on_pushButton_PTT_toggled(bool checked);
    void on_pushButton_Split_toggled(bool checked);
    void on_pushButton_AB_clicked();
    void on_pushButton_AeqB_clicked();

    void on_dial_valueChanged(int value);

    void on_action_AboutQT_triggered();
    void on_action_AboutHamLib_triggered();



    void on_comboBox_Mode_activated(int index);

    void on_pushButton_Fast_toggled(bool checked);

    void on_horizontalSlider_RFpower_valueChanged(int value);

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

    void on_horizontalSlider_RFgain_valueChanged(int value);

    void on_spinBox_NR_valueChanged(int arg1);

    void on_horizontalSlider_IFshift_valueChanged(int value);

    void on_pushButton_Band2_clicked();

    void on_pushButton_Band70_clicked();

    void on_pushButton_BandGen_clicked();

    void on_action_Setup_triggered();

private:
    Ui::MainWindow *ui;
    QTimer *timer;

    void guiInit();
    void setSubMeter();
};

#endif // MAINWINDOW_H
