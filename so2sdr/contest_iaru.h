/*! Copyright 2010-2018 R. Torsten Clay N4OGW

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
#ifndef CONTEST_IARU_H
#define CONTEST_IARU_H

#include "contest.h"

class IARU : public Contest {
public:
    IARU(QSettings &cs,QSettings &ss);
    ~IARU();
    void addQso(Qso *qso);
    ContestType contestType() const { return Iaru_t;}
    int fieldWidth(int col) const;
    int nMultsWorked() const;
    int numberField() const;
    unsigned int rcvFieldShown() const;
    void setupContest(QByteArray MultFile[MMAX], const Cty * cty);
    unsigned int sntFieldShown() const;
    bool validateExchange(Qso *qso);
    bool showQsoPtsField() const { return true;}
    int rstField() const { return 0;}
};

#endif
