/*! Copyright 2010-2015 R. Torsten Clay N4OGW

   This file is part of so2sdr.

    so2sdr is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    so2sdr is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with so2sdr.  If not, see <http://www.gnu.org/licenses/>.

 */
#ifndef BANDMAPTCP_H
#define BANDMAPTCP_H

// bandmap TCP commands
#define BANDMAP_CMD_SET_FREQ  0x66
#define BANDMAP_CMD_QUIT      0x71
#define BANDMAP_CMD_RX        0x72
#define BANDMAP_CMD_TX        0x74
#define BANDMAP_CMD_TERMINATE 0x0a
#define BANDMAP_CMD_FIND_FREQ 0x67
#define BANDMAP_CMD_SET_LOWER_FREQ 0x6C
#define BANDMAP_CMD_SET_UPPER_FREQ 0x75
#define BANDMAP_CMD_SET_ADD_OFFSET 0x6f
#define BANDMAP_CMD_ADD_CALL   0x61
#define BANDMAP_CMD_DELETE_CALL 0x64
#define BANDMAP_CMD_SET_INVERT 0x69
#define BANDMAP_CMD_CLEAR      0x78
#define BANDMAP_CMD_QSY_UP     0x55
#define BANDMAP_CMD_QSY_DOWN   0x44

#endif // BANDMAPTCP_H
