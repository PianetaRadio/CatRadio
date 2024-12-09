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


#ifndef DIALOGVOICEKEYER_H
#define DIALOGVOICEKEYER_H

#include <QDialog>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #include <QMediaDevices>
    #include <QAudioDevice>
#else
    #include <QAudioDeviceInfo>
#endif

namespace Ui {
class DialogVoiceKeyer;
}

class DialogVoiceKeyer : public QDialog
{
    Q_OBJECT

public:
    explicit DialogVoiceKeyer(QWidget *parent = nullptr);
    ~DialogVoiceKeyer();

private slots:
    void on_pushButton_voiceK1_clicked();

    void on_buttonBox_accepted();

    void on_pushButton_voiceK2_clicked();

    void on_pushButton_voiceK3_clicked();

    void on_pushButton_voiceK4_clicked();

    void on_pushButton_voiceK5_clicked();

private:
    Ui::DialogVoiceKeyer *ui;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QMediaDevices *audioDevices;
#endif
};

#endif // DIALOGVOICEKEYER_H
