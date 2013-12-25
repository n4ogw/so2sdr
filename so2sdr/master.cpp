/*! Copyright 2010-2014 R. Torsten Clay N4OGW

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
#include "master.h"
#include <QString>
#include <QIODevice>

/*!
   Must call initialize after constructor before using class
 */
Master::Master()
{
    initialized = false;
    index       = 0;
    CallData    = 0;
    chars       = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/";
    nchars      = chars.size();
    indexSize   = nchars * nchars + 1;
    indexBytes  = indexSize * sizeof(int);
}


Master::~Master()
{
    delete[] index;
    delete[] CallData;
}

/*!
   Reads master.dta file from disk; initialize lookup tables

   Can be called again if file is changed
 */
void Master::initialize(QFile &file)
{
    delete[] index;     // in case re-initialized
    delete[] CallData;
    index    = new int[indexSize];
    fileSize = file.size();
    if (file.read((char *) (&index[0]), indexBytes) != indexBytes) {
        emit(masterError("ERROR: master file has incorrect size"));
        return;
    }

    // some basic checks on the master.dta file index
    if (index[0] != indexBytes || index[indexSize - 1] != fileSize) {
        emit(masterError("ERROR: Invalid master data file: index"));
        return;
    }

    // read callsign data
    CallData = new char[fileSize - indexBytes];
    if (file.read(CallData, fileSize - indexBytes) != (fileSize - indexBytes)) {
        emit(masterError("ERROR: Invalid master data file: calls"));
        return;
    }
    file.close();
    initialized = true;
}

/*!
   Supercheck partial lookup

   -  partial : callsign fragment
   -  CallList : returned bytearray containing possible callsigns
 */
void Master::search(QByteArray partial, QByteArray &CallList)
{
    QByteArray mask = partial;
    CallList = "";
    for (int i = 0; i < partial.size(); i++) {
        if (chars.contains(partial.at(i))) {
            mask[i] = 'A';
        } else if (partial[i] != '?') {
            return;
        }
    }
    int idxpos = mask.indexOf("AA");
    if (idxpos == -1) {
        return;
    }
    int chr1n     = chars.indexOf(partial[idxpos]);
    int chr2n     = chars.indexOf(partial[idxpos + 1]);
    int CallBegin = index[chr1n * chars.size() + chr2n] - indexBytes;
    int CallEnd   = index[chr1n * chars.size() + chr2n + 1] - indexBytes;
    while (CallBegin < CallEnd) {
        QByteArray call(&CallData[CallBegin]);
        if (call.contains(partial)) {
            CallList = CallList + call + " ";
        }
        CallBegin = CallBegin + call.size() + 1;
    }
}
