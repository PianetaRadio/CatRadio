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


#ifndef DIALOGCWKEYER_H
#define DIALOGCWKEYER_H

#include <QDialog>

#include "winkeyer.h"


namespace Ui {
class DialogCWKeyer;
}

class DialogCWKeyer : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCWKeyer(WinKeyer *winkeyer, QWidget *parent = nullptr);
    ~DialogCWKeyer();

private slots:
    void on_pushButton_Connect_toggled(bool checked);
    void on_buttonBox_accepted();

private:
    Ui::DialogCWKeyer *ui;

    WinKeyer *wk;
};

#endif // DIALOGCWKEYER_H
