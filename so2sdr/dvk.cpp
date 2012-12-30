/*! Copyright 2010-2013 R. Torsten Clay N4OGW

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
#include "dvk.h"
#include <sndfile.h>

DVK::DVK(QObject *parent) :
    QObject(parent)
{
}

QString DVK::sndfile_version()
{
    char  buffer[128];
    sf_command(NULL, SFC_GET_LIB_VERSION, buffer, sizeof (buffer));
    return QString(buffer);
}
