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


#ifndef DIALOGCONFIG_H
#define DIALOGCONFIG_H

#include <QDialog>

namespace Ui {
class DialogConfig;
}

class DialogConfig : public QDialog
{
    Q_OBJECT

public:
    explicit DialogConfig(QWidget *parent = nullptr);
    ~DialogConfig();

private slots:
    void on_buttonBox_accepted();

    void on_checkBox_netRigctl_toggled(bool checked);

    void on_comboBox_rigModel_currentIndexChanged(int index);

    void on_comboBox_comPort_currentIndexChanged(int index);

private:
    Ui::DialogConfig *ui;

    int findRigModel(QString rigModel);  //Find hamlib rig model from the rig selected in the comboBox_rigModel
    void setDialogSerialConfig(int dataBits, int parity, int stopBits, int handshake);    //Set the serial port configuration on the dialog
    void setRigSerialConfigFromDialog();    //Set the rigConf serial config from dialog serial settings
};

int printRigList(const struct rig_caps *rigCaps, void *data);   //Retrives rig list from Hamlib and write to file
bool createRigFile();    //Create the rig list file

#endif // DIALOGCONFIG_H
