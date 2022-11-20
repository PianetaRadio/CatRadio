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


#ifndef DIALOGCOMMAND_H
#define DIALOGCOMMAND_H

#include <QDialog>

namespace Ui {
class DialogCommand;
}

class DialogCommand : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCommand(QWidget *parent = nullptr);
    ~DialogCommand();

private slots:
    void on_pushButton_close_clicked();

    void on_pushButton_send_clicked();

private:
    Ui::DialogCommand *ui;
};

#endif // DIALOGCOMMAND_H
