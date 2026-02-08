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


#ifndef DIALOGNETRIGCTL_H
#define DIALOGNETRIGCTL_H

#include <QDialog>

class netRigCtl;


namespace Ui {
class DialogNetRigctl;
}

class DialogNetRigctl : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNetRigctl(netRigCtl *netrigctlPtr = nullptr, QWidget *parent = nullptr);
    ~DialogNetRigctl();
    void setArguments(QString arguments);

private slots:
    void on_pushButton_start_toggled(bool checked);

    void on_lineEdit_portNumber_editingFinished();

private:
    Ui::DialogNetRigctl *ui;
    netRigCtl *netrigctl;
    QString argumentsString;
};

#endif // DIALOGNETRIGCTL_H
