/*! Copyright 2010-2019 R. Torsten Clay N4OGW

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
#ifndef CONTEST_ARRL10_H
#define CONTEST_ARRL10_H

#include "cty.h"
#include "contest.h"

class ARRL10 : public Contest {
// Q_OBJECT

public:
    ARRL10(QSettings &cs,QSettings &ss);
    ~ARRL10();
    void setupContest(QByteArray MultFile[MMAX], const Cty * cty);
    bool validateExchange(Qso *qso);
    ContestType contestType() const { return Arrl10_t;}
    void addQso(Qso *qso);
    QString bandLabel(int i) const;
    bool bandLabelEnable(int i) const;
    int fieldWidth(int col) const;
    int highlightBand(int b, ModeTypes modeType=CWType) const;
    int nMultsColumn(int col,int ii) const;
    int numberField() const;
    unsigned int rcvFieldShown() const;
    unsigned int sntFieldShown() const;
    int Score() const;
    bool showQsoPtsField() const { return true;}
    int rstField() const { return 0;}
};

#endif
