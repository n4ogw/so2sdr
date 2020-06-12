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
#ifndef CONTEST_WPX_H
#define CONTEST_WPX_H

#include "contest.h"

class WPX : public Contest
{
public:
    WPX(QSettings &cs,QSettings &ss);
    ~WPX() override;

    QVariant columnName(int c) const override;
    ContestType contestType() const override { return Wpx_t;}
    void setupContest(QByteArray MultFile[MMAX], const Cty * cty) override;
    bool validateExchange(Qso *qso) override;
    void addQso(Qso *qso) override;
    int fieldWidth(int col) const override;
    unsigned int rcvFieldShown() const override;
    unsigned int sntFieldShown() const override;
    int numberField() const override;
    bool showQsoPtsField() const override { return true;}
    int rstField() const override { return 0;}
    void workedMults(QByteArray Call, int mult, unsigned int &worked);
    void wpxPrefix(QByteArray call, QByteArray &pfx);
};

#endif // CONTEST_WPX
