/*! Copyright 2010-2021 R. Torsten Clay N4OGW

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

class ARRL10 : public Contest
{
public:
    ARRL10(QSettings &cs,QSettings &ss);
    ~ARRL10() override;

    void setupContest(QByteArray MultFile[MMAX], const Cty * cty) override;
    bool validateExchange(Qso *qso) override;
    ContestType contestType() const override { return Arrl10_t;}
    void addQso(Qso *qso) override;
    QString bandLabel(int i) const override;
    bool bandLabelEnable(int i) const override;
    int fieldWidth(int col) const override;
    int highlightBand(int b, ModeTypes modeType=CWType) const override;
    int nMultsColumn(int col,int ii) const override;
    int numberField() const override;
    unsigned int rcvFieldShown() const override;
    unsigned int sntFieldShown() const override;
    int Score() const override;
    bool showQsoPtsField() const override { return true;}
    int rstField() const override { return 0;}
};

#endif
