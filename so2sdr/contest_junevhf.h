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
#ifndef CONTEST_JUNEVHF_H
#define CONTEST_JUNEVHF_H

#include "contest.h"
#include "defines.h"

class JuneVHF : public Contest
{
public:
    JuneVHF(QSettings &cs,QSettings &ss);
    ~JuneVHF() override;

    QString bandLabel(int i) const override;
    bool bandLabelEnable(int i) const override;
    int highlightBand(int b, ModeTypes modeType=CWType) const override;
    int nMultsColumn(int col,int ii) const override;
    void setupContest(QByteArray MultFile[MMAX], const Cty * cty) override;
    bool validateExchange(Qso *qso) override;
    QVariant columnName(int c) const override;
    ContestType contestType() const  override{ return JuneVHF_t;}
    void addQso(Qso *qso) override;
    int fieldWidth(int col) const override;
    int numberField() const override { return -1; }
    unsigned int rcvFieldShown() const override { return 1; }
    bool showQsoPtsField() const override { return true;}
};

#endif // CONTEST_JUNEVHF_H
