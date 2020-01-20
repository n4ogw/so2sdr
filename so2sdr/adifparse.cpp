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
#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QTime>
#include "adifparse.h"
#include "hamlib/rig.h"
#include "utils.h"

ADIFParse::ADIFParse(QObject *parent) : QObject(parent)
{

}


/*! parse an ADIF record
 *
 * the first record found in data is returned as a SQL log record
 * Designed for WSJTX ADIF ouput, may not work well on more general
 *   ADIF sources.
 *  @todo validate each field and return a bool
 */
bool ADIFParse::parse(QByteArray data, Qso *qso)
{
    const QByteArray token_names[] = { "CALL",
                                       "GRIDSQUARE",
                                       "MODE",
                                       "QSO_DATE_OFF",
                                       "TIME_OFF",
                                       "BAND",
                                       "FREQ"};
    const int n_token_names = 7;

    // only common and WSJTX modes will be matched
    const QByteArray mode_name[] = { "AM",
                                     "CW",
                                     "FM",
                                     "SSB",
                                     "USB",
                                     "LSB",
                                     "FT8",
                                     "ISCAT",
                                     "JT4",
                                     "JT9",
                                     "JT65",
                                     "MSK144",
                                     "QRA64",
                                     "RTTY"};
    const rmode_t modes[] = { RIG_MODE_AM,
                              RIG_MODE_CW,
                              RIG_MODE_FM,
                              RIG_MODE_USB,
                              RIG_MODE_USB,
                              RIG_MODE_LSB,
                              RIG_MODE_RTTY,
                              RIG_MODE_RTTY,
                              RIG_MODE_RTTY,
                              RIG_MODE_RTTY,
                              RIG_MODE_RTTY,
                              RIG_MODE_RTTY,
                              RIG_MODE_RTTY,
                              RIG_MODE_RTTY};
    const int n_mode_names=14;

    // bands are in the same order as in defines.h
    const QByteArray band_name[]= { "160M",
                                    "80M",
                                    "40M",
                                    "20M",
                                    "15M",
                                    "10M",
                                    "60M",
                                    "30M",
                                    "17M",
                                    "12M",
                                    "6M",
                                    "2M",
                                    "1.25M",
                                    "70CM",
                                    "33CM",
                                    "23CM"};

    qso->clear();
    data=data.toUpper();
    int end=data.indexOf("<EOR>");
    int start=data.indexOf("<EOH>");
    if (start==-1) {
        start=0;
    } else {
        start+=5;
    }
    data=data.mid(start,end-start);
    QList<QByteArray>elements=data.split('<');

    QDateTime time;
    time.setTimeSpec(Qt::UTC);
    QByteArray key,val;
    for (int i=0;i<elements.size();i++) {
        parseBit(elements.at(i),key,val);
        for (int j = 0; j < n_token_names; j++) {
            if (key == token_names[j]) {
                switch (j) {
                case 0:  // CALL
                    qso->call=val;
                    break;
                case 1: // GRID
                    qso->exch=val;
                    qso->rcv_exch[0]=val;
                    break;
                case 2: // MODE
                {
                    for (int k=0;k<n_mode_names;k++) {
                        if (val==mode_name[k]) {
                            qso->mode=modes[k];
                            qso->modeType=getModeType(modes[k]);
                            break;
                        }
                    }
                    break;
                }
                case 3: // DATE
                {
                    int y=val.mid(0,4).toInt();
                    int m=val.mid(4,2).toInt();
                    int d=val.mid(6,2).toInt();
                    time.setDate(QDate(y, m, d));
                    break;
                }
                case 4: // TIME
                {
                    int h=val.mid(0,2).toInt();
                    int m=val.mid(2,2).toInt();
                    int s=0;
                    if (val.size()==6) {
                        s=val.mid(4,2).toInt();
                    }
                    time.setTime(QTime(h,m,s));
                    break;
                }
                case 5: // BAND
                {
                    for (int k=0;k<N_BANDS;k++) {
                        if (val==band_name[k]) {
                            qso->band=k;
                            break;
                        }
                    }
                    break;
                }
                case 6: // FREQ
                    qso->freq=val.toDouble()*1000000;
                    break;
                }
            }
        }
    }
    qso->time=time;
    return true;
}

/*!
 * \brief ADIFParse::parseBit
 * \param in
 * \param out
 */
void ADIFParse::parseBit(QByteArray in,QByteArray &key,QByteArray &val)
{
    if (in.size()<5) {
        key.clear();
        val.clear();
        return;
    }
    int i1=in.indexOf(":");
    key=in.mid(0,i1);
    int i2=in.indexOf(">");
    bool ok=false;
    int n=in.mid(i1+1,i2-i1-1).toInt(&ok);
    if (!ok) {
        key.clear();
        val.clear();
        return;
    }
    val=in.mid(i2+1,n);
}
