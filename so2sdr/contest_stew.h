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
#ifndef CONTEST_STEW_H
#define CONTEST_STEW_H

#include "contest.h"

class Stew : public Contest {
public:
    Stew();
    ~Stew();
    void addQso(Qso *qso);
    QString cabrilloName() const
    {
        return("STEW-PERRY");
    }
    QByteArray prefillExchange(int cntry, int zone);
    void setupContest(QByteArray MultFile[MMAX], const Cty * cty);
    bool validateExchange(Qso *qso);
    int fieldWidth(int col) const;
    unsigned int rcvFieldShown() const;
    int Score() const;
    unsigned int sntFieldShown() const;
    int numberField() const;
};

#endif // CONTEST_STEW_H
