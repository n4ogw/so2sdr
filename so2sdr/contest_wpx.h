/*! Copyright 2010-2015 R. Torsten Clay N4OGW

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

class WPX : public Contest {
public:
    WPX();
    ~WPX();
    QString cabrilloName() const
    {
        return("CQ-WPX-CW");
    }
    QVariant columnName(int c) const;
    ContestType contestType() const
    {
        return Wpx_t;
    }
    void setupContest(QByteArray MultFile[MMAX], const Cty * cty);
    bool validateExchange(Qso *qso);
    void addQso(Qso *qso);
    int fieldWidth(int col) const;
    unsigned int rcvFieldShown() const;
    unsigned int sntFieldShown() const;
    int numberField() const;
    void workedMults(QByteArray Call, int mult, unsigned int &worked);
    void wpxPrefix(QByteArray call, QByteArray &pfx);
};

#endif // CONTEST_WPX
