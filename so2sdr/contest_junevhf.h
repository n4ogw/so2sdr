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
#ifndef CONTEST_JUNEVHF_H
#define CONTEST_JUNEVHF_H

#include "contest.h"
#include "defines.h"

class JuneVHF : public Contest
{
public:
    JuneVHF(QSettings &cs,QSettings &ss);

    QString bandLabel(int i) const;
    bool bandLabelEnable(int i) const;
    int highlightBand(int b, ModeTypes modeType=CWType) const;
    int nMultsColumn(int col,int ii) const;
    void setupContest(QByteArray MultFile[MMAX], const Cty * cty);
    bool validateExchange(Qso *qso);
    QVariant columnName(int c) const;
    ContestType contestType() const { return JuneVHF_t;}
    void addQso(Qso *qso);
    int fieldWidth(int col) const;
    int numberField() const { return -1; }
    unsigned int rcvFieldShown() const { return 1; }
    unsigned int sntFieldShown() const { return 0; }
    bool showQsoPtsField() const { return true;}
};

#endif // CONTEST_JUNEVHF_H
