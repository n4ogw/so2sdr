/*! Copyright 2010-2022 R. Torsten Clay N4OGW

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
#ifndef CONTEST_WWDIGI_H
#define CONTEST_WWDIGI_H

#include "contest.h"
#include "defines.h"

class WWDigi : public Contest
{
public:
    WWDigi(QSettings &cs,QSettings &ss);
    ~WWDigi() override;

    void setupContest(QByteArray MultFile[MMAX], const Cty * cty) override;
    bool validateExchange(Qso *qso) override;
    QVariant columnName(int c) const override;
    ContestType contestType() const  override{ return WWDigi_t;}
    void addQso(Qso *qso) override;
    int fieldWidth(int col) const override;
    int numberField() const override { return -1; }
    unsigned int rcvFieldShown() const override { return 1; }
    bool showQsoPtsField() const override { return true;}
};

#endif // CONTEST_WWDIGI_H
