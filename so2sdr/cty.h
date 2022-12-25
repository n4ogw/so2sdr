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
#ifndef CTY_H
#define CTY_H
/*! reads CTY country database file, provides prefix/country/zone lookups, beam
   headings

  */
#include "defines.h"
#include "qso.h"
#include <QByteArray>
#include <QFile>
#include <QList>
#include <QSettings>
#include <QString>

class Cty : public QObject {
  Q_OBJECT

public:
  explicit Cty(QSettings &s);
  ~Cty();

  int findPfx(QByteArray prefix, int &zone, Cont &continent, bool &o) const;
  int idPfx(Qso *qso, bool &qsy) const;
  void initialize(double la, double lo, int ZoneType);
  QString mySunTimes() const;
  int nCountries() const;
  QByteArray pfxName(int indx) const;
  void readCtyFile(QByteArray cty_file);

signals:
  void ctyError(const QString &);

private:
  int nARRLCty;
  int nCQCty;
  int usaIndx;
  QList<Country *> countryList;
  QList<CtyCall *> CallE;
  QList<Pfx *> pfxList;
  QList<QByteArray> portId;
  QList<QByteArray> portIdMM;
  QList<QByteArray> portIdMobile;
  QList<QByteArray> portIdRover;
  QSettings &settings;
  QString mySun;
  QList<int> zoneBearing;
  QList<QString> zoneSun;

  int checkException(QByteArray call, int &zone, QString &sun) const;
  int idPfx2(Qso *qso, int sz) const;
  bool isDigit(char c) const;
  void sunTimes(double lat, double lon, QString &suntime);
};
#endif // CTY_H
