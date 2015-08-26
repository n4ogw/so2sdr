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
#ifndef UTILS_H
#define UTILS_H
#include <QString>
#include <QValidator>

/*!
  upper-case validator: converts all input to upper case
  */
class UpperValidator : public QValidator
{
public:
    explicit UpperValidator(QObject *parent = 0);
    QValidator::State validate(QString &input, int &pos) const;
};

/*!
  time validator: requires time in format 0000 - 2359
  */
class TimeValidator : public QValidator
{
public:
    explicit TimeValidator(QObject *parent = 0);
    void fixup ( QString & input ) const;
    QValidator::State validate(QString &input, int &pos) const;
};

int getBand(int f);
QString dataDirectory();
QString userDirectory();

#endif // UTILS_H
