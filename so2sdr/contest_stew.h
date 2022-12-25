/*! Copyright 2010-2023 R. Torsten Clay N4OGW

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
  Stew(QSettings &cs, QSettings &ss);
  ~Stew() override;

  void addQso(Qso *qso) override;
  ContestType contestType() const override { return Stew_t; }
  QByteArray prefillExchange(Qso *qso) override;
  void setupContest(QByteArray MultFile[MMAX], const Cty *cty) override;
  bool validateExchange(Qso *qso) override;
  int fieldWidth(int col) const override;
  unsigned int rcvFieldShown() const override;
  int Score() const override;
  bool showQsoPtsField() const override { return true; }
  int numberField() const override;
};

#endif // CONTEST_STEW_H
