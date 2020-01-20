/*! Copyright 2010-2020 R. Torsten Clay N4OGW

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
#ifndef CONTEST_CWOPS_H
#define CONTEST_CWOPS_H

#include "contest.h"

class Cwops : public Contest {
public:
    Cwops(QSettings &cs,QSettings &ss);
    ~Cwops();
    void addQso(Qso *qso);
    ContestType contestType() const { return Cwops_t;}
    int fieldWidth(int col) const;
    int numberField() const;
    void setupContest(QByteArray MultFile[2], const Cty * cty);
    unsigned int rcvFieldShown() const;
    unsigned int sntFieldShown() const;
    bool validateExchange(Qso *qso);
    bool showQsoPtsField() const { return false;}
};

#endif
