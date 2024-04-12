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


#ifndef RIGCOMMAND_H
#define RIGCOMMAND_H

#endif // RIGCOMMAND_H

#include <QString>

#include <rig.h>


void set_band (int band);
void quick_split ();
void send_cw_mem (int memory);
agc_level_e levelagcvalue (int agcValue);
agc_level_e levelagcstr (QString agcString);
value_t valueagclevel (agc_level_e agcLevel);
ant_t antstr (QString antString);
unsigned long long levelmeterstr (QString meterString); //Convert Submeter combo box string into Hamlib RIG_LEVEL constant
