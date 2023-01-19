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
#ifndef UTILS_H
#define UTILS_H

#include "hamlib/rig.h"
#include "defines.h"
#include <QString>
#include <QValidator>

/*!
  upper-case validator: converts all input to upper case
  */
class UpperValidator : public QValidator {
public:
  explicit UpperValidator(QObject *parent = nullptr);
  QValidator::State validate(QString &input, int &pos) const;
};

/*!
  time validator: requires time in format 0000 - 2359
  */
class TimeValidator : public QValidator {
public:
  explicit TimeValidator(QObject *parent = nullptr);
  void fixup(QString &input) const;
  QValidator::State validate(QString &input, int &pos) const;
};

int getBand(double f);
QString dataDirectory();
QString userDirectory();
QByteArray getAdifMode(rmode_t mode);
ModeTypes getAdifModeType(const QByteArray &mode);
ModeTypes getModeType(rmode_t mode);

#endif // UTILS_H
