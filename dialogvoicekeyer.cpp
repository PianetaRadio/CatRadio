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


#include "dialogvoicekeyer.h"
#include "ui_dialogvoicekeyer.h"
#include "guidata.h"

#include <QFileDialog>
#include <QSettings>

extern voiceKeyerConfig voiceKConf;


DialogVoiceKeyer::DialogVoiceKeyer(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogVoiceKeyer)
{
    ui->setupUi(this);

    ui->lineEdit_voiceK1->setText(voiceKConf.memoryFile[0]);
    ui->lineEdit_voiceK2->setText(voiceKConf.memoryFile[1]);
    ui->lineEdit_voiceK3->setText(voiceKConf.memoryFile[2]);
    ui->lineEdit_voiceK4->setText(voiceKConf.memoryFile[3]);
    ui->lineEdit_voiceK5->setText(voiceKConf.memoryFile[4]);

    audioDevices = new QMediaDevices(this);
    const QList<QAudioDevice> devices = audioDevices->audioOutputs();
    for (const QAudioDevice &deviceInfo : devices)
        ui->comboBox_audioDevice->addItem(deviceInfo.description(), QVariant::fromValue(deviceInfo));
    ui->comboBox_audioDevice->setCurrentText(voiceKConf.audioOutput);

    connect(ui->horizontalSlider_audioLevel, &QAbstractSlider::valueChanged, ui->label_audioLevel, QOverload<int>::of(&QLabel::setNum));
    ui->horizontalSlider_audioLevel->setValue(voiceKConf.audioOutputVolume*10);
}

DialogVoiceKeyer::~DialogVoiceKeyer()
{
    delete ui;
}

void DialogVoiceKeyer::on_pushButton_voiceK1_clicked()
{
    voiceKConf.memoryFile[0] = QFileDialog::getOpenFileName(this, tr("Select file"), "./", tr("Audio file (*.wav *.mp3)"));
    ui->lineEdit_voiceK1->setText(voiceKConf.memoryFile[0]);
}

void DialogVoiceKeyer::on_pushButton_voiceK2_clicked()
{
    voiceKConf.memoryFile[1] = QFileDialog::getOpenFileName(this, tr("Select file"), "./", tr("Audio file (*.wav *.mp3)"));
    ui->lineEdit_voiceK2->setText(voiceKConf.memoryFile[1]);
}


void DialogVoiceKeyer::on_pushButton_voiceK3_clicked()
{
    voiceKConf.memoryFile[2] = QFileDialog::getOpenFileName(this, tr("Select file"), "./", tr("Audio file (*.wav *.mp3)"));
    ui->lineEdit_voiceK3->setText(voiceKConf.memoryFile[2]);
}


void DialogVoiceKeyer::on_pushButton_voiceK4_clicked()
{
    voiceKConf.memoryFile[3] = QFileDialog::getOpenFileName(this, tr("Select file"), "./", tr("Audio file (*.wav *.mp3)"));
    ui->lineEdit_voiceK4->setText(voiceKConf.memoryFile[3]);
}


void DialogVoiceKeyer::on_pushButton_voiceK5_clicked()
{
    voiceKConf.memoryFile[4] = QFileDialog::getOpenFileName(this, tr("Select file"), "./", tr("Audio file (*.wav *.mp3)"));
    ui->lineEdit_voiceK5->setText(voiceKConf.memoryFile[4]);
}


void DialogVoiceKeyer::on_buttonBox_accepted()
{
    voiceKConf.memoryFile[0] = ui->lineEdit_voiceK1->text();
    voiceKConf.memoryFile[1] = ui->lineEdit_voiceK2->text();
    voiceKConf.memoryFile[2] = ui->lineEdit_voiceK3->text();
    voiceKConf.memoryFile[3] = ui->lineEdit_voiceK4->text();
    voiceKConf.memoryFile[4] = ui->lineEdit_voiceK5->text();

    //* Save settings in catradio.ini
    QSettings configFile(QString("catradio.ini"), QSettings::IniFormat);
    configFile.setValue("VoiceKeyer/voiceMemoryFile1", voiceKConf.memoryFile[0]);
    configFile.setValue("VoiceKeyer/voiceMemoryFile2", voiceKConf.memoryFile[1]);
    configFile.setValue("VoiceKeyer/voiceMemoryFile3", voiceKConf.memoryFile[2]);
    configFile.setValue("VoiceKeyer/voiceMemoryFile4", voiceKConf.memoryFile[3]);
    configFile.setValue("VoiceKeyer/voiceMemoryFile5", voiceKConf.memoryFile[4]);

    QAudioDevice audioDevice = ui->comboBox_audioDevice->itemData(ui->comboBox_audioDevice->currentIndex()).value<QAudioDevice>();
    configFile.setValue("VoiceKeyer/audioOutput", QVariant::fromValue(audioDevice.description()));

    voiceKConf.audioOutputVolume = (float)(ui->horizontalSlider_audioLevel->value())/10;
    configFile.setValue("VoiceKeyer/audioOutputVolume", voiceKConf.audioOutputVolume);
}
