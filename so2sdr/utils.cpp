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
#include "hamlib/rig.h"
#include "utils.h"
#include "defines.h"
#include <QString>

/*!
  input validator that converts all input to uppercase
*/
UpperValidator::UpperValidator(QObject *parent) : QValidator(parent) {}

QValidator::State UpperValidator::validate(QString &input, int &pos) const {
  Q_UNUSED(pos)
  input = input.toUpper();
  return (Acceptable);
}

/*!
  input validator for entering 4-digit UTC times
  */
TimeValidator::TimeValidator(QObject *parent) : QValidator(parent) {}

QValidator::State TimeValidator::validate(QString &input, int &pos) const {
  Q_UNUSED(pos)
  bool ok = false;
  int t = input.toInt(&ok, 10);
  if (ok && t >= 0 && t <= 2359) {
    if (input.size() != 4) {
      return (Intermediate);
    } else {
      return (Acceptable);
    }
  }
  return (Invalid);
}

void TimeValidator::fixup(QString &input) const {
  // format as 4 digits with leading zeros
  switch (input.size()) {
  case 0:
    input = "0000";
    break;
  case 1:
    input = "000" + input;
    break;
  case 2:
    input = "00" + input;
    break;
  case 3:
    input = "0" + input;
    break;
  case 4:
    break;
  }
}

/*!
   get band index from frequency
 */
int getBand(double f) {
  int g = qRound(f / 1000000.0);
  switch (g) {
  case 1:
  case 2:
    return (BAND160);
  case 3:
  case 4:
    return (BAND80);
  case 5:
    return (BAND60);
  case 6:
  case 7:
    return (BAND40);
  case 9:
  case 10:
    return (BAND30);
  case 13:
  case 14:
    return (BAND20);
  case 18:
    return (BAND17);
  case 20:
  case 21:
    return (BAND15);
  case 24:
  case 25:
    return (BAND12);
  case 27:
  case 28:
  case 29:
  case 30:
  case 31:
    return (BAND10);
  case 49:
  case 50:
  case 51:
  case 52:
  case 53:
  case 54:
  case 55:
  case 56:
    return (BAND6);
  case 143:
  case 144:
  case 145:
  case 146:
  case 147:
  case 148:
  case 149:
    return (BAND2);
  case 219:
  case 220:
  case 221:
  case 222:
  case 223:
  case 224:
  case 225:
    return (BAND222);
  }
  // handle UHF
  double fmhz = f / 1000000;
  if (fmhz > 419 && fmhz < 451) {
    return (BAND420);
  } else if (fmhz > 901 && fmhz < 929) {
    return (BAND902);
  } else if (fmhz > 1239 && fmhz < 1301) {
    return (BAND1240);
  }
  // 630m, 2200m
  double khz = f / 1000;
  if (khz > 400 && khz < 500) {
    return (BAND630);
  } else if (khz > 100 && khz < 150) {
    return (BAND2200);
  }
  // @todo microwave bands!
  return (BAND_NONE);
}

/*! returns directory where program data is stored
 */
QString dataDirectory() {
  // INSTALL_DIR is usually /usr/local
  return QString(INSTALL_DIR) + QString("/share/so2sdr/");
}

/*! returns directory where user data (station config, hamlib cache,...) are
 * stored
 */
QString userDirectory() { return QDir::homePath() + "/.so2sdr"; }

/*! convert hamlib mode to ModeType
 * @todo this should be depreciated, as it is impossible to tell the modetype
 * from the hamlib mode; currently only used in one place in serial.cpp
 */
ModeTypes getModeType(rmode_t mode) {
  switch (mode) {
  case RIG_MODE_NONE:
    return CWType;
  case RIG_MODE_AM:
    return PhoneType;
  case RIG_MODE_CW:
    return CWType;
  case RIG_MODE_USB:
    return PhoneType;
  case RIG_MODE_LSB:
    return PhoneType;
  case RIG_MODE_RTTY:
    return DigiType;
  case RIG_MODE_FM:
    return PhoneType;
  case RIG_MODE_WFM:
    return PhoneType;
  case RIG_MODE_CWR:
    return CWType;
  case RIG_MODE_RTTYR:
    return DigiType;
  case RIG_MODE_AMS:
    return PhoneType;
  case RIG_MODE_PKTLSB:
    return DigiType;
  case RIG_MODE_PKTUSB:
    return DigiType;
  case RIG_MODE_PKTFM:
    return DigiType;
  case RIG_MODE_ECSSUSB:
    return PhoneType;
  case RIG_MODE_ECSSLSB:
    return PhoneType;
  case RIG_MODE_FAX:
    return DigiType;
  case RIG_MODE_SAM:
    return PhoneType;
  case RIG_MODE_SAL:
    return PhoneType;
  case RIG_MODE_SAH:
    return PhoneType;
  case RIG_MODE_DSB:
    return PhoneType;
  default:
    return CWType;
  }
}

/*! convert ADIF mode to ModeType
 */
ModeTypes getAdifModeType(QByteArray mode) {
  if (mode == "CW") {
    return CWType;
  } else if (mode == "SSB" || mode == "USB" || mode == "LSB" || mode == "AM" ||
             mode == "FM" || mode == "DIGITALVOICE") {
    return PhoneType;
  } else {
    return DigiType;
  }
}

/*! convert hamlib mode to ADIF mode. In general it is not possible to guess the
 * ADIF digital mode name from hamlib, so this must be used with care
 */
QByteArray getAdifMode(rmode_t mode) {
  switch (mode) {
  case RIG_MODE_NONE:
    return "CW";
  case RIG_MODE_AM:
    return "AM";
  case RIG_MODE_CW:
    return "CW";
  case RIG_MODE_USB:
    return "USB";
  case RIG_MODE_LSB:
    return "LSB";
  case RIG_MODE_RTTY:
    return "RTTY";
  case RIG_MODE_FM:
    return "FM";
  case RIG_MODE_WFM:
    return "FM";
  case RIG_MODE_CWR:
    return "CW";
  case RIG_MODE_RTTYR:
    return "RTTY";
  case RIG_MODE_AMS:
    return "AM";
  case RIG_MODE_PKTLSB:
    return "PKT";
  case RIG_MODE_PKTUSB:
    return "PKT";
  case RIG_MODE_PKTFM:
    return "PKT";
  case RIG_MODE_ECSSUSB:
    return "USB";
  case RIG_MODE_ECSSLSB:
    return "USB";
  case RIG_MODE_FAX:
    return "FAX";
  case RIG_MODE_SAM:
    return "AM";
  case RIG_MODE_SAL:
    return "AM";
  case RIG_MODE_SAH:
    return "AM";
  case RIG_MODE_DSB:
    return "AM";
  default:
    return "CW";
  }
}
