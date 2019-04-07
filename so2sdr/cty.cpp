/*! Copyright 2010-2019 R. Torsten Clay N4OGW

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
#include <QDir>
#define _USE_MATH_DEFINES
#include <cmath>
#include <QDate>
#include <QDebug>
#include "hamlib/rotator.h"
#include "cty.h"
#include "utils.h"

/*! After calling constructor, must call initialize(), which reads cty file and sets up
     databases.
 */
Cty::Cty(QSettings& s) : settings(s)
{
    // portable identifiers not giving a new country
    portId.clear();
    portId << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "0";
    portId << "P" << "M" << "QRP" << "AE" << "AG" << "KT" << "R";

    // portable ids for aero/maritime stations, ie qth is R1, R2, or R3
    portIdMM.clear();
    portIdMM << "MM" << "AM";

    // portable ids for mobiles
    portIdMobile.clear();
    portIdMobile << "M";

    // portable ids for roverse
    portIdRover.clear();
    portIdRover << "R";
}

Cty::~Cty()
{
    for (int i = 0; i < countryList.size(); i++)
        delete (countryList[i]);

    for (int i = 0; i < CallE.size(); i++)
        delete (CallE[i]);

    for (int i = 0; i < pfxList.size(); i++)
        delete (pfxList[i]);
}



/*!  check if call is a callsign exception

   returns CTY index, or -1 if callsign not found
   if found, returns cq and itu zones if known (or -1 if not)
 */
int Cty::checkException(QByteArray call, int& zone,QString& sun) const
{
    // start search in middle of list
    int  i1    = 0;
    int  i2    = CallE.size() / 2;
    int  i3    = CallE.size() - 1;
    bool found = false;

    if (call == CallE.at(i3)->call) {
        found = true;
        i2    = i3;
    } else {
        while (true) {
            if (call < CallE.at(i2)->call) {
                i3 = i2;
                i2 = (i3 + i1) / 2;
            } else {
                i1 = i2;
                i2 = (i3 + i1) / 2;
            }
            if (call == CallE.at(i2)->call) {
                found = true;
                break;
            } else if (i1 == i2 || i2 == i3) {
                break;
            }
        }
    }
    if (found) {
        zone = CallE.at(i2)->Zone;
        sun = CallE.at(i2)->sun;
        return(CallE.at(i2)->CtyIndx);
    } else {
        return(-1);
    }
}

/*!
   check prefix in main prefix list

   returns CTY index, or -1 if prefix not found.
   if found,
 */
int Cty::findPfx(QByteArray prefix, int& zone, Cont &continent, bool &o) const
{
    // start search in middle of list
    int  i1    = 0;
    int  i2    = pfxList.size() / 2;
    int  i3    = pfxList.size() - 1;
    bool found = false;

    if (prefix == pfxList[i3]->prefix) {
        found = true;
        i2    = i3;
    } else {
        while (true) {
            if (prefix < pfxList[i2]->prefix) {
                i3 = i2;
                i2 = (i3 + i1) / 2;
            } else {
                i1 = i2;
                i2 = (i3 + i1) / 2;
            }
            if (prefix == pfxList[i2]->prefix) {
                found = true;
                break;
            } else if (i1 == i2 || i2 == i3) {
                break;
            }
        }
    }
    if (found) {
        o         = pfxList[i2]->zoneOverride;
        zone      = pfxList[i2]->Zone;
        continent = countryList[pfxList[i2]->CtyIndx]->Continent;
        return(pfxList[i2]->CtyIndx);
    } else {
        return(-1);
    }
}


/*! prefix identification, to be called from main program
 */
int Cty::idPfx(Qso *qso, bool &qsy) const
{
    qsy = false;
    const int sz = qso->call.size();
    if (sz == 0) return(-1);

    // check for "*DUPE*"
    if (qso->call == "*DUPE*") return(-1);

    // Check for mode strings with mode names e.g. "USB"
    // This will optionally be followed by an integer for
    // passband width in Hz of 2-5 digits.
    QRegExp rx("^(CWR|CW|LSB|USB|FM|AM)(\\d{2,5})?(;)?$");

    // is this a qsy frequency?
    bool ok = false;
    // Allow the UI to receive values in kHz down to the Hz i.e. "14250.340"
    // will become 14250340 Hz.  Tested with K3 and Dummy rig models.
    int  n  = (int)(double)(1000 * qso->call.toDouble(&ok));

    // semicolon indicates 2nd-radio qsy
    if (n || ok == true || qso->call.contains(";") || (rx.indexIn(qso->call) == 0)) {
        qso->country = -1;
        qso->country_name.clear();
        qso->PfxName.clear();
        qso->mult[0] = -1;
        qso->mult[1] = -1;
        qsy          = true;
        return(-1);
    }

    // check for portable prefix; there could be more than one / in a call!
    qso->isMM = false;
    qso->isMobile = false;
    qso->isRover = false;
    if (qso->call.contains('/')) {
        // nothing to do, need at least three chars
        if (sz < 3) return(-1);

        // find each piece separated by /
        QList<QByteArray> parts;
        parts.clear();
        int               i1 = 0;
        int               indx;
        do {
            indx = qso->call.indexOf("/", i1);
            if (indx == -1) indx = sz;
            parts.append(qso->call.mid(i1, indx - i1));
            i1 = indx + 1;
        } while (indx != sz);

        // do prefix check on each piece
        int *p   = new int[parts.size()];
        Qso *tmp = new Qso[parts.size()]();
        for (int i = 0; i < parts.size(); i++) {
            tmp[i].call = parts.at(i);
            if (portId.contains(tmp[i].call)) {
                // ignore common portable identifiers (see above list)
                p[i] = -1;
            } else {
                p[i] = idPfx2(&tmp[i], tmp[i].call.size());
            }

            // flag MM stations
            if (portIdMM.contains(tmp[i].call)) {
                qso->isMM = true;
            }
            // flag mobile stations
            if (portIdMobile.contains(tmp[i].call)) {
                qso->isMobile = true;
            }
            // flag rover stations
            if (portIdRover.contains(tmp[i].call)) {
                qso->isRover = true;
            }
        }

        // choose the prefix which is shortest and was identified by idpfx2
        bool ok = false;
        i1 = parts.at(0).size();
        int  ip = p[0];
        indx = 0;
        if (ip != -1) ok = true;
        for (int i = 1; i < parts.size(); i++) {
            if (p[i] != -1 && parts.at(i).size() < i1) {
                indx = i;
                i1   = parts.at(i).size();
                ip   = p[i];
            }
        }
        delete[] p;
        if (ok) {
            qso->country      = ip;
            qso->country_name = tmp[indx].country_name;
            qso->PfxName      = tmp[indx].PfxName;
            qso->zone         = tmp[indx].zone;
            qso->bearing      = tmp[indx].bearing;
            qso->sun          = tmp[indx].sun;
            qso->continent    = tmp[indx].continent;
        }
        if (qso->isMM) {
            qso->country_name.clear();
            qso->PfxName.clear();
            qso->zone = 0;
        }
        delete[] tmp;
        return(ip);
    } else {
        return(idPfx2(qso, sz));
    }
}

/*! identifies prefix in call. Returns -1 if
   can't ID prefix, otherwise index to CTY list.

   call should already be uppercase only, no spaces
 */
int Cty::idPfx2(Qso *qso, int sz) const
{
    // is it an exception call?
    int indx = checkException(qso->call, qso->zone,qso->sun);
    if (indx != -1) {
        qso->country      = indx;
        qso->country_name = countryList[indx]->name;
        qso->PfxName      = countryList[indx]->MainPfx;
        qso->bearing      = zoneBearing.at(qso->zone);
        qso->continent    = countryList[indx]->Continent;
        return(indx);
    }

    // check main pfx list
    bool       over = false;
    QByteArray pfx4;
    if (sz > 3) {
        pfx4 = qso->call.mid(0, 4);
        indx = findPfx(pfx4, qso->zone, qso->continent, over);
    }
    QByteArray pfx1;
    QByteArray pfx2;
    QByteArray pfx3;
    if (indx == -1) {
        if (sz > 2) {
            pfx3 = qso->call.mid(0, 3);
            indx = findPfx(pfx3, qso->zone, qso->continent, over);
        }
        if (indx == -1) {
            if (sz > 1) {
                pfx2 = qso->call.mid(0, 2);
                indx = findPfx(pfx2, qso->zone, qso->continent, over);
            }
            if (indx == -1) {
                pfx1 = qso->call.mid(0, 1);
                indx = findPfx(pfx1, qso->zone, qso->continent, over);
            }
        }
    }
    bool found=false;
    if (indx == -1) {
        qso->bearing      = 0;
        qso->zone         = 0;
        qso->country_name = "Unknown";
        qso->PfxName.clear();
    } else {
        qso->country_name = countryList[indx]->name;
        qso->continent    = countryList[indx]->Continent;

        // check for multiple zones in country
        if (!over && countryList[indx]->multipleZones) {
            // the following may not work for all calls, but
            // it works for the current CTY list, where only BY,K,UA9,VE,VK have
            // multiple zones
            //
            // find position of number in callsign
            int i = 0;
            int j;
            while ((i < sz) && !isDigit(qso->call.at(i))) {
                i++;
            }

            if (i < sz) {
                pfx1 = qso->call.mid(i, 1);  // check both number and number+letter exceptions
                pfx2 = qso->call.mid(i, 2);

                for (j = 0; j < countryList.at(indx)->zonePfx.size(); j++) {
                    if (pfx1 == countryList.at(indx)->zonePfx.at(j) ||
                        pfx2 == countryList.at(indx)->zonePfx.at(j)) break;
                }
                if (j != countryList.at(indx)->zonePfx.size()) {
                    qso->zone = countryList.at(indx)->zones[j];
                    found=true;
                }
            }
        }
        if (qso->zone == 0) {
            // fall back to usual zone
            qso->zone = countryList[indx]->Zone;
        }
        if (found) {
            qso->bearing      = zoneBearing.at(qso->zone);
            qso->sun          = zoneSun.at(qso->zone);
        } else {
            qso->bearing      = countryList[indx]->bearing;
            qso->sun          = countryList[indx]->sun;
        }
    }
    qso->country = indx;
    qso->PfxName = pfxName(indx);

    // exceptions: KG4 only for KG4AA-KG4ZZ calls
    if (qso->PfxName == "KG4" && qso->call.size() != 5) {
        Qso tmpqso;
        tmpqso.call       = "W1AW"; // just to get the country ID for USA
        indx              = idPfx2(&tmpqso, 4);
        qso->PfxName      = tmpqso.PfxName;
        qso->bearing      = tmpqso.bearing;
        qso->country_name = tmpqso.country_name;
        qso->country      = tmpqso.country;
        qso->mult_name    = tmpqso.mult_name;
        qso->sun          = tmpqso.sun;
        qso->zone         = tmpqso.zone;
    }
    return(indx);
}



/*! parses .cty file, makes country list

   -  lat=latitude of station +=N
   -  lon=longitude of station +=W
   -  ZoneType: 0: read CQ zones/countries  1: read ITU zones/ARRL countries

 */
void Cty::initialize(double la, double lo, int ZoneType)
{
/*! CTY file format

   Column   Length      Description

   -    1           26  country Name
   -    27          5   CQ Zone
   -    32          5   ITU Zone
   -    37          5   2-letter continent abbreviation
   -    42          9   Latitude in degrees, + for North
   -    51          10  Longitude in degrees, + for West
   -    61          9   Local time offset from GMT
   -    70          6   Primary DXCC Prefix (A "*" preceding this prefix indicates
                        that the country is on the DARC WAEDC list, and counts in
                        CQ-sponsored contests, but not ARRL-sponsored contests).
 */
    double mylat=la;
    double mylon=lo;

    QFile file(userDirectory()+"/"+settings.value(c_cty,c_cty_def).toString());
    int   indx;

    // sunrise/sunset times for station
    sunTimes(mylat, mylon, mySun);

    // sunrise/sunset times and bearings for CQ or ITU zones. These are used for
    // certain countries that span many zones
    QString zoneFileName;
    switch (ZoneType) {
    case 0:zoneFileName=dataDirectory()+"/cq_zone_latlong.dat";break;
    case 1:zoneFileName=dataDirectory()+"/itu_zone_latlong.dat";break;
    }
    QFile file2(zoneFileName);
    if (file2.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // add blank for 0, zones start with 1
        zoneBearing.append(0);
        zoneSun.append("");
        while (!file2.atEnd()) {
            QString buffer;
            buffer=file2.readLine(80);
            if (buffer.contains('#')) continue; // #=comment line

            buffer=buffer.trimmed();
            QStringList field = buffer.split(" ", QString::SkipEmptyParts);
            double lat=field.at(1).toDouble();
            double lon=field.at(2).toDouble();
            QString sunTime;
            QString set;
            sunTimes(lat, -lon, sunTime);
            zoneSun.append(sunTime);

            double dist;
            double head;
            qrb(mylon * -1.0, mylat, lon , lat, &dist, &head);
            zoneBearing.append(qRound(head));
        }
        file2.close();
    } else {
        // file missing, just make these blank
        int nz=40;
        if (ZoneType==1) nz=80;
        for (int i=0;i<nz;i++) {
            zoneBearing.append(0);
            zoneSun.append("ERR");
        }
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString tmp = "ERROR: can't open file " + userDirectory() + "/"+settings.value(c_cty,c_cty_def).toString();
        emit(ctyError(tmp));
        return;
    }
    nARRLCty = 0;
    nCQCty   = 0;
    indx     = 0;

    while (!file.atEnd()) {
        QByteArray buffer, tmp, tmp2, buffer_a, buffer_e;
        int        i0, i1, i2, i3, i4, cqz, ituz;

        // get one record
        // all records are at least one line
        buffer = file.readLine();
        tmp    = file.readLine();
        buffer_e.clear();

        // zone exceptions for main pfx
        while (tmp.contains("#")) {
            buffer_e.append(tmp);
            while (!tmp.contains(";")) {
                tmp = file.readLine();
                buffer_e.append(tmp);
            }
            tmp = file.readLine();
        }

        // alias pfx; may include zone exceptions as well
        buffer_a = tmp;
        while (!tmp.contains(";")) {
            tmp = file.readLine();
            buffer_a.append(tmp);
        }
        buffer   = buffer.trimmed();
        buffer_a = buffer_a.trimmed();
        buffer_e = buffer_e.trimmed();

        // now process parts
        // main line
        Country* new_country = new Country;
        new_country->multipleZones = false;
        new_country->indx          = indx;

        i1                = buffer.indexOf(":", 0);
        new_country->name = buffer.mid(0, i1);

        // insert some common abbrieviations (I=Island,...)
        // to save screen space
        if (new_country->name.contains("Island")) {
            new_country->name.replace("Island", "I");
        }
        i2 = buffer.indexOf(":", i1 + 1);
        int cqzone = buffer.mid(i1 + 1, i2 - i1 - 1).toInt();

        i1 = buffer.indexOf(":", i2 + 1);
        int ituzone = buffer.mid(i2 + 1, i1 - i2 - 1).toInt();
        if (ZoneType) {
            new_country->Zone = ituzone;
        } else {
            new_country->Zone = cqzone;
        }
        i2  = buffer.indexOf(":", i1 + 1);
        tmp = buffer.mid(i1 + 1, i2 - i1 - 1);

        tmp = tmp.trimmed();
        if (tmp == "NA") {
            new_country->Continent = NA;
        } else if (tmp == "SA") {
            new_country->Continent = SA;
        } else if (tmp == "EU") {
            new_country->Continent = EU;
        } else if (tmp == "AF") {
            new_country->Continent = AF;
        } else if (tmp == "AS") {
            new_country->Continent = AS;
        } else if (tmp == "OC") {
            new_country->Continent = OC;
        }
        i1 = buffer.indexOf(":", i2 + 1);
        double lat = buffer.mid(i2 + 1, i1 - i2 - 1).trimmed().toFloat();

        i2 = buffer.indexOf(":", i1 + 1);
        double lon = buffer.mid(i1 + 1, i2 - i1 - 1).trimmed().toFloat();

        double dist;
        double head;
        qrb(mylon * -1.0, mylat, lon * -1.0, lat, &dist, &head);
        new_country->bearing = qRound(head); // round azimuth to nearest degree
        sunTimes(lat, lon, new_country->sun);

        i1                   = buffer.indexOf(":", i2 + 1);
        new_country->delta_t = buffer.mid(i2 + 1, i1 - i2 - 1).trimmed().toFloat();

        i2                   = buffer.indexOf(":", i1 + 1);
        new_country->MainPfx = buffer.mid(i1 + 1, i2 - i1 - 1);
        new_country->MainPfx = new_country->MainPfx.trimmed();

        if (new_country->MainPfx.contains("*")) {
            if (ZoneType == 0) {
                // CQ country
                new_country->MainPfx.replace("*", "");
                nCQCty++;
            } else {
                // only keeping ARRL countries, skip this one
                delete new_country;
                continue;
            }
        } else {
            nARRLCty++;
        }

        // country has multiple zones?
        if (buffer_e != "") {
            new_country->multipleZones = true;
            QList<QByteArray> tmpl = buffer_e.split(';');
            tmpl.removeLast();  // for some reason get 1 too many here
            for (int i = 0; i < tmpl.size(); i++) {
                i1 = tmpl[i].indexOf("#");
                i2 = tmpl[i].indexOf(":");
                QByteArray pf = tmpl[i].mid(i1 + 1, i2 - i1 - 1);
                pf = pf.trimmed();

                // remove any final digit in the prefix
                if (isDigit(pf.at(pf.size() - 1))) {
                    pf.chop(1);
                }

                // remove initial pfx and final ;
                tmpl[i] = tmpl[i].mid(i2 + 1, tmpl[i].size() - 2);
                QList<QByteArray> tmpl2 = tmpl[i].split(',');  // separate
                for (int j = 0; j < tmpl2.size(); j++) {
                    tmpl2[j] = tmpl2[j].trimmed();
                    tmpl2[j].replace(pf, "");

                    // does it have a ITU zone exception?
                    ituz = 0;
                    if (tmpl2[j].contains("[")) {
                        i3 = tmpl2[j].indexOf("[");
                        i4 = tmpl2[j].indexOf("]");
                        if (i3 != -1 && i4 != -1) {
                            tmp2     = tmpl2[j].mid(i3 + 1, i4 - i3 - 1);
                            ituz     = tmp2.toInt();
                            tmp2     = tmpl2[j].mid(i3, i4 - i3 + 1);
                            tmpl2[j] = tmpl2[j].replace(tmp2, "");
                        }
                    }
                    cqz = 0;

                    // does it have a CQ zone exception?
                    if (tmpl2[j].contains("(")) {
                        i3 = tmpl2[j].indexOf("(");
                        i4 = tmpl2[j].indexOf(")");
                        if (i3 != -1 && i4 != -1) {
                            tmp2     = tmpl2[j].mid(i3 + 1, i4 - i3 - 1);
                            cqz      = tmp2.toInt();
                            tmp2     = tmpl2[j].mid(i3, i4 - i3 + 1);
                            tmpl2[j] = tmpl2[j].replace(tmp2, "");
                        }
                    }
                    new_country->zonePfx.append(tmpl2[j]);
                    if (ZoneType) {
                        if (ituz) {
                            new_country->zones.append(ituz);
                        } else {
                            new_country->zones.append(ituzone);
                        }
                    } else {
                        if (cqz) {
                            new_country->zones.append(cqz);
                        } else {
                            new_country->zones.append(cqzone);
                        }
                    }
                }
            }
        }

        // alias prefixes, may include zone exceptions
        i1 = 0;
        i2 = buffer_a.indexOf(",", i1 + 1);
        if (i2 == -1) i2 = buffer_a.size() - 1;
        int nalias = 0;
        while (i1 < (buffer_a.size() - 1)) {
            tmp = buffer_a.mid(i1, i2 - i1);
            tmp = tmp.trimmed();
            if (tmp.startsWith("=")) {
                i0 = 1; // call exception
            } else {
                i0 = 0; // alias prefix
                nalias++;
            }
            ituz = -1; // -1 is marker to use regular zone in pfx list
            cqz  = -1;

            // does it have a ITU zone exception?
            if (tmp.contains("[")) {
                i3 = tmp.indexOf("[");
                i4 = tmp.indexOf("]");
                if (i3 != -1 && i4 != -1) {
                    tmp2 = tmp.mid(i3 + 1, i4 - i3 - 1);
                    ituz = tmp2.toInt();
                    tmp2 = tmp.mid(i3, i4 - i3 + 1);
                    tmp  = tmp.replace(tmp2, "");
                }
            }

            // does it have a CQ zone exception?
            if (tmp.contains("(")) {
                i3 = tmp.indexOf("(");
                i4 = tmp.indexOf(")");
                if (i3 != -1 && i4 != -1) {
                    tmp2 = tmp.mid(i3 + 1, i4 - i3 - 1);
                    cqz  = tmp2.toInt();
                    tmp2 = tmp.mid(i3, i4 - i3 + 1);
                    tmp  = tmp.replace(tmp2, "");
                }
            }

            if (!i0) {
                // new alias prefix
                Pfx* new_pfx = new Pfx;
                new_pfx->CtyIndx      = indx;
                new_pfx->zoneOverride = true;
                if (ZoneType) {
                    new_pfx->Zone = ituz;
                } else {
                    new_pfx->Zone = cqz;
                }
                if (new_pfx->Zone == -1) {
                    new_pfx->Zone         = new_country->Zone;
                    new_pfx->zoneOverride = false;
                }

                new_pfx->prefix = tmp;
                pfxList.append(new_pfx);
            } else {
                // new exception call
                CtyCall* new_call = new CtyCall;
                new_call->call = tmp.right(tmp.size() - 1);
                if (ZoneType) {
                    if (ituz != -1) {
                        new_call->Zone = ituz;
                    } else {
                        new_call->Zone = new_country->Zone;
                    }
                } else {
                    if (cqz != -1) {
                        new_call->Zone = cqz;
                    } else {
                        new_call->Zone = new_country->Zone;
                    }
                }
                if (cqz == -1 && ituz == -1) {
                    // it NEITHER zone was defined, add call to exception list
                    // (with default zones set above). This is mostly for weird/unexpected calls
                    new_call->CtyIndx = indx;
                    new_call->sun=new_country->sun;
                    CallE.append(new_call);
                } else if ((!ZoneType && cqz == -1) || (ZoneType && ituz == -1)) {
                    // the exception doesn't have ONE of the zones defined, remove it
                    // (some entries in cty file only have CQ or ITU zones defined, but
                    // not both. In IARU contest for example, do not want CQ zone exceptions
                    // to be kept
                    delete new_call;
                } else {
                    new_call->CtyIndx = indx;
                    new_call->sun=zoneSun.at(new_call->Zone);
                    CallE.append(new_call);
                }
            }

            i1 = i2 + 1;
            i2 = buffer_a.indexOf(",", i1 + 1);
            if (i2 == -1) {
                i2 = buffer_a.size() - 1;
            }
        }
        countryList.append(new_country);

        // special case: some countries (4U1U, ...) have no alias prefixes listed, only exception calls.
        // make sure to add to main pfx list!
        if (nalias == 0) {
            Pfx* new_pfx = new Pfx;
            new_pfx->zoneOverride = false;
            new_pfx->CtyIndx      = indx;
            if (ZoneType) {
                new_pfx->Zone = ituz;
            } else {
                new_pfx->Zone = cqz;
            }
            if (new_pfx->Zone == -1) {
                new_pfx->Zone = new_country->Zone;
            }
            new_pfx->prefix = new_country->MainPfx;
            pfxList.append(new_pfx);
        }

        indx++;
    }

    /*!
       @todo define < operator so qsort can be used sort prefix and call exception lists
     */

    // prefix
    for (int i = 1; i < pfxList.size(); i++) {
        QByteArray tmp = pfxList[i]->prefix;
        int        i0  = pfxList[i]->Zone;
        int        i1  = pfxList[i]->CtyIndx;
        bool       i2  = pfxList[i]->zoneOverride;
        int        j   = i - 1;
        while (j >= 0 && pfxList[j]->prefix > tmp) {
            pfxList[j + 1]->prefix       = pfxList[j]->prefix;
            pfxList[j + 1]->Zone         = pfxList[j]->Zone;
            pfxList[j + 1]->CtyIndx      = pfxList[j]->CtyIndx;
            pfxList[j + 1]->zoneOverride = pfxList[j]->zoneOverride;
            j--;
        }
        pfxList[j + 1]->prefix       = tmp;
        pfxList[j + 1]->Zone         = i0;
        pfxList[j + 1]->CtyIndx      = i1;
        pfxList[j + 1]->zoneOverride = i2;
    }

    // exception
    for (int i = 1; i < CallE.size(); i++) {
        QByteArray tmp = CallE[i]->call;
        QString suntmp= CallE[i]->sun;
        int        i0  = CallE[i]->Zone;
        int        i1  = CallE[i]->CtyIndx;
        int        j   = i - 1;
        while (j >= 0 && CallE[j]->call > tmp) {
            CallE[j + 1]->call    = CallE[j]->call;
            CallE[j + 1]->sun    = CallE[j]->sun;
            CallE[j + 1]->Zone    = CallE[j]->Zone;
            CallE[j + 1]->CtyIndx = CallE[j]->CtyIndx;
            j--;
        }
        CallE[j + 1]->call    = tmp;
        CallE[j + 1]->sun    = suntmp;
        CallE[j + 1]->Zone    = i0;
        CallE[j + 1]->CtyIndx = i1;
    }

    // save index for US
    Qso  tmpqso;
    tmpqso.call = "W1AW";
    bool b;
    usaIndx = idPfx(&tmpqso, b);
}

/*!
   returns true if 'c' is a numeric digit (ASCII 48..57)
 */
bool Cty::isDigit(char c) const
{
    if (c > 47 && c < 58) return(true);
    else return(false);
}

/*!
   local sunrise/set times for current date (utc)

 */
QString Cty::mySunTimes() const
{
    return mySun;
}

/*!
   number of countries in database
 */
int Cty::nCountries() const
{
    return(countryList.size());
}

/*!
   return main prefix given country index
 */
QByteArray Cty::pfxName(int indx) const
{
    if (indx != -1) {
        return(countryList[indx]->MainPfx);
    } else {
        // unknown prefix
        return("");
    }
}

/*!
   Calculate sunrise/set times

   see http://williams.best.vwh.net/sunrise_sunset_algorithm.htm
 */
void Cty::sunTimes(double lat, double lon, QString &sunTime)
{
    /*!
       angle from zenith for sunrise/set
     */
    const double zenith = 90.83;
    int          day    = QDate::currentDate().day();
    int          month  = QDate::currentDate().month();
    int          year   = QDate::currentDate().year();
    double       n1     = floor(275.0 * month / 9.0);
    double       n2     = floor((month + 9.0) / 12.0);
    double       n3     = (1.0 + floor((year - 4.0 * floor(year / 4.0) + 2.0) / 3.0));
    double       n      = n1 - (n2 * n3) + day - 30.0;

    // note published algorith has longitude sign reversed
    // from my convention
    double lngHour = -lon / 15.0;

    // sun rise
    double t = n + ((6.0 - lngHour) / 24.0);
    double m = (0.9856 * t) - 3.289;
    double l = m + (1.916 * sin(m * M_PI / 180.0)) + (0.02 * sin(2.0 * m * M_PI / 180.0)) + 282.634;
    if (l < 0.0) l += 360.0;
    if (l > 360.0) l -= 360.0;
    double ra = atan(0.91764 * tan(l * M_PI / 180.0)) * 180.0 / M_PI;
    if (ra < 0.0) ra += 360.0;
    if (ra > 360.0) ra -= 360.0;
    double lq = (floor(l / 90.0)) * 90.0;
    double rq = (floor(ra / 90.0)) * 90.0;
    ra += (lq - rq);
    ra /= 15.0;
    double sd = 0.39782 * sin(l * M_PI / 180.0);
    double cd = cos(asin(sd));
    double ch = (cos(zenith * M_PI / 180.0) - (sd * sin(lat * M_PI / 180.0))) / (cd * cos(lat * M_PI / 180.0));
    if (ch > 1.0) {
        // sun never rises on this day
        sunTime="DARK";
        return;

    } else {
        double h = 360.0 - acos(ch) * 180 / M_PI;
        h /= 15.0;
        t  = h + ra - (0.06571 * t) - 6.622;
        double ut = t - lngHour;
        if (ut < 0.0) ut += 24.0;
        if (ut > 24.0) ut -= 24.0;
        int hr = (int) floor(ut);          // hours
        int rt = (int) ((ut - hr) * 60.0); // minutes
        rt += hr * 100;
        if (rt<10) {
            sunTime = "000" + QString::number(rt);
        } else if (rt < 60) {
            sunTime = "00" + QString::number(rt);
        } else if (rt < 1000) {
            sunTime = "0" + QString::number(rt);
        } else {
            sunTime = QString::number(rt);
        }
    }

    // sun set
    t = n + ((18.0 - lngHour) / 24.0);
    m = (0.9856 * t) - 3.289;
    l = m + (1.916 * sin(m * M_PI / 180.0)) + (0.02 * sin(2.0 * m * M_PI / 180.0)) + 282.634;
    if (l < 0.0) l += 360.0;
    if (l > 360.0) l -= 360.0;
    ra = atan(0.91764 * tan(l * M_PI / 180.0)) * 180.0 / M_PI;
    if (ra < 0.0) ra += 360.0;
    if (ra > 360.0) ra -= 360.0;
    lq  = (floor(l / 90.0)) * 90.0;
    rq  = (floor(ra / 90.0)) * 90.0;
    ra += (lq - rq);
    ra /= 15.0;
    sd  = 0.39782 * sin(l * M_PI / 180.0);
    cd  = cos(asin(sd));
    ch  = (cos(zenith * M_PI / 180.0) - (sd * sin(lat * M_PI / 180.0))) / (cd * cos(lat * M_PI / 180.0));
    if (ch < -1.0) {
        // sun never sets on this day
        sunTime = "DAY";
        return;
    } else {
        double h = acos(ch) * 180 / M_PI;
        h /= 15.0;
        t  = h + ra - (0.06571 * t) - 6.622;
        double ut = t - lngHour;
        if (ut < 0.0) ut += 24.0;
        if (ut > 24.0) ut -= 24.0;
        int hr = (int) floor(ut);          // hours
        int st = (int) ((ut - hr) * 60.0); // minutes
        st += hr * 100;
        if (st<10) {
            sunTime = sunTime+ ":000" + QString::number(st);
        } else if (st < 60) {
            sunTime = sunTime+ ":00" + QString::number(st);
        } else if (st < 1000) {
            sunTime = sunTime+ ":0" + QString::number(st);
        } else {
            sunTime = sunTime+":"+QString::number(st);
        }
    }

}
