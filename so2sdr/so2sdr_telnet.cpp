/*! Copyright 2010-2013 R. Torsten Clay N4OGW

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
#include <QFile>
#include <QMutableListIterator>
#include "so2sdr.h"

// /// Telnet/spot database stuff

/*! load spots from a file xxxx.dat
 */
void So2sdr::loadSpots()
{
    // file will be contest_name.dat in same directory as contest cfg file
    directory->setCurrent(contestDirectory);
    QString fname = fileName;
    fname.chop(3);
    QFile   *spotFile = new QFile(fname + "dat");
    if (!spotFile->open(QIODevice::ReadOnly)) {
        delete spotFile;
        return;
    }
    QDataStream s(spotFile);

    // determine how old (in minutes) saved spots are
    QDateTime fileTime;
    s >> fileTime;
    int       mins = fileTime.secsTo(QDateTime::currentDateTimeUtc()) / 60;

    // clear spot list
    for (int b = 0; b < N_BANDS; b++) {
        spotList[b].clear();
    }

    // read data, updating times
    while (!spotFile->atEnd()) {
        int b;
        s >> b;
        if (b >= 0 && b < N_BANDS) {
            BandmapEntry *newSpot = new BandmapEntry();
            s >> newSpot->call;
            s >> newSpot->dupe;
            s >> newSpot->t;
            s >> newSpot->f;
            newSpot->t += mins;
            if (newSpot->t < settings->value(s_sdr_spottime,s_sdr_spottime_def).toInt()) {
                spotList[b].append(*newSpot);
            } else {
                delete newSpot;
            }
        } else {
            // spot file corrupted? ignore rest
            spotFile->close();
            delete spotFile;
            return;
        }
    }
    spotFile->close();
    delete spotFile;
}

/*! save spots to file
 */
void So2sdr::saveSpots()
{
    // file will be contest_name.dat in same directory as contest cfg file
    directory->setCurrent(contestDirectory);
    QString fname = fileName;

    // check to make sure config file is properly defined
    if (fileName.right(4) != ".cfg") return;

    fname.chop(3);
    QFile *spotFile = new QFile(fname + "dat");
    if (!spotFile->open(QIODevice::WriteOnly)) {
        delete spotFile;
        return;
    }
    QDataStream s(spotFile);

    // save current time
    s << QDateTime::currentDateTimeUtc();

    // save spots
    for (int b = 0; b < N_BANDS; b++) {
        for (int i = 0; i < spotList[b].size(); i++) {
            s << (qint32) b;
            s << spotList[b][i].call;
            s << spotList[b][i].dupe;
            s << spotList[b][i].t;
            s << spotList[b][i].f;
        }
    }
    spotFile->close();
    delete spotFile;
}

void So2sdr::showTelnet(int checkboxState)

// show/remove telnet window
{
    if (checkboxState == Qt::Unchecked) {
        if (telnetOn) {
            disconnect(telnet, SIGNAL(done()), telnetCheckBox, SLOT(toggle()));
            telnet->hide();
            telnetOn = false;
        }
    } else {
        if (!telnet) {
            telnet = new Telnet(*settings);
        }
        telnet->show();
        connect(telnet, SIGNAL(done()), telnetCheckBox, SLOT(toggle()));
        connect(telnet, SIGNAL(dxSpot(QByteArray, int)), this, SLOT(addSpot(QByteArray, int)));
        telnetOn = true;
    }
}

/*! add a spot. Checks to see if it is a dupe first
 */
void So2sdr::addSpot(QByteArray call, int f)
{
    if (!settings->value(s_sdr_mark,s_sdr_mark_def).toBool()) return;

    // * is a special case, used to mark freq without callsign
    bool d = true;
    if (call != "*") {
        Qso tmp(1);
        tmp.call = call;
        tmp.freq = f;
        tmp.band = getBand(f);
        d        = mylog->isDupe(&tmp, contest->dupeCheckingByBand(), false);
    }
    addSpot(call, f, d);
}


/*!
   add a callsign spot
 */
void So2sdr::addSpot(QByteArray call, int f, bool d)
{
    if (!settings->value(s_sdr_mark,s_sdr_mark_def).toBool()) return;

    BandmapEntry spot;
    spot.call = call;
    spot.f    = f;
    spot.t    = 0;
    spot.dupe = d;

    int b = getBand(f);
    if (b >= 0 && b < N_BANDS) {
        bool dupe  = false;
        int  idupe = spotList[b].size();

        // does this spot duplicate another on the same band with same call,
        // or match the freq of another spot
        // if so, the old one will be replaced
        if (call != "*") {
            // first check to see if this call is already in list anywhere on the band
            for (int i = 0; i < spotList[b].size(); i++) {
                if (spotList[b].at(i).call == call) {
                    dupe  = true;
                    idupe = i;
                    break;
                }
            }
            // no callsign match, then
            // check to see if there is a spot close to the same frequency
            if (idupe==spotList[b].size()) {
                for (int i = 0; i < spotList[b].size(); i++) {
                    if (abs(spotList[b].at(i).f - f) < SIG_MIN_SPOT_DIFF) {
                        dupe  = true;
                        idupe = i;
                        break;
                    }
                }
            }
        } else {
            // for non-call spots, just check frequency
            for (int i = 0; i < spotList[b].size(); i++) {
                if (abs(spotList[b].at(i).f - f) < SIG_MIN_SPOT_DIFF) {
                    dupe  = true;
                    idupe = i;
                    break;
                }
            }
        }

        if (dupe) {
            // replace previous spot, reset timer
            spotList[b][idupe].call = call;
            spotList[b][idupe].f    = f;
            spotList[b][idupe].t    = 0;
            spotList[b][idupe].dupe = d;
        } else {
            spotList[b].append(spot);
        }
        // check frequency against peak-detected signals
        if (bandmapOn[0]) {
            spotList[b][idupe].f=bandmap[0]->closestFreq(spotList[b][idupe].f);
        }
        if (bandmapOn[1]) {
            spotList[b][idupe].f=bandmap[1]->closestFreq(spotList[b][idupe].f);
        }
    }
}

/*!
   checks spot list to see if freq is close to a spot. If so, pop up call to window

   spotListPopUp prevents the same spot from being popped up multiple times unless
   the frequency has changed.

   turned off if spotCalls=false
 */
void So2sdr::checkSpot(int nr)
{
    if (!settings->value(s_sdr_mark,s_sdr_mark_def).toBool() || altDActive == 1 || altDActive == 3) return;
    static int lastFreq[2] = { 0, 0 };

    // initialize last freq
    if (lastFreq[nr] == 0) {
        lastFreq[nr] = cat->getRigFreq(nr);
        return;
    }
    int f = cat->getRigFreq(nr);

    // if just had a band change, don't do anything
    if (getBand(f) != getBand(lastFreq[nr])) {
        lastFreq[nr] = f;
        return;
    }

    // check for alt-D active state. If alt-D is active and vfo on 2nd radio has moved,
    // clear alt-D status
    if (altDActive == 2 && nr == (activeRadio ^ 1)) {
        if (abs(f - lastFreq[nr]) > SIG_MIN_FREQ_DIFF) {
            QPalette palette(lineEditCall[nr]->palette());
            palette.setColor(QPalette::Base, CQ_COLOR);
            lineEditCall[nr]->setPalette(palette);
            lineEditCall[nr]->clear();
            qso[nr]->call.clear();
            lineEditCall[nr]->setModified(false);
            lineEditExchange[nr]->setPalette(palette);
            lineEditExchange[nr]->clear();
            qso[nr]->call.clear();
            exchangeSent[nr] = false;
            qso[nr]->prefill.clear();
            origCallEntered[nr].clear();
            labelCountry[nr]->clear();
            labelBearing[nr]->clear();
            sunLabelPtr[nr]->clear();
            validLabel[nr]->clear();
            statusBarDupe = false;
            So2sdrStatusBar->clearMessage();
            clearWorked(nr);
            altDActive   = 0;
            lastFreq[nr] = f;
            return;
        }
    }
    if (f != lastFreq[nr]) spotListPopUp[nr] = false;

    if (spotListPopUp[nr]) {
        lastFreq[nr] = f;
        return;
    }

    bool found = false;
    for (int i = 0; i < spotList[band[nr]].size(); i++) {
        if (abs(f - spotList[band[nr]].at(i).f) < SIG_MIN_FREQ_DIFF) {
            if (spotList[band[nr]].at(i).call != "*") {
                lineEditCall[nr]->setText(spotList[band[nr]].at(i).call);
            } else {
                lineEditCall[nr]->setText("*DUPE*");

                // set highlight so any typing overwrites "*DUPE*"
                lineEditCall[nr]->setCursorPosition(0);
                lineEditCall[nr]->setSelection(0, 6);
            }
            found = true;
            break;
        }
    }
    if (found) {
        prefixCheck(nr, lineEditCall[nr]->text());
        spotListPopUp[nr] = true;
    } else if (!found && abs(lastFreq[nr] - f) > SIG_MIN_FREQ_DIFF) {
        lineEditCall[nr]->clear();
        qso[nr]->call.clear();
        lineEditExchange[nr]->clear();
        labelCountry[nr]->clear();
        labelBearing[nr]->clear();
        sunLabelPtr[nr]->clear();
        validLabel[nr]->clear();
        spotListPopUp[nr] = false;
        statusBarDupe     = false;
        So2sdrStatusBar->clearMessage();
        clearWorked(nr);
    }
    lastFreq[nr] = f;
}

/*! remove a spot
 */
void So2sdr::removeSpot(QByteArray call, int band)
{
    if (!settings->value(s_sdr_mark,s_sdr_mark_def).toBool()) return;

    int indx = -1;
    for (int i = 0; i < spotList[band].size(); i++) {
        if (spotList[band][i].call == call) {
            indx = i;
            break;
        }
    }
    if (indx != -1) spotList[band].removeAt(indx);
}

/*! remove a spot at freq f
 */
void So2sdr::removeSpotFreq(int f, int band)
{
    if (!settings->value(s_sdr_mark,s_sdr_mark_def).toBool()) return;

    int indx = -1;
    for (int i = 0; i < spotList[band].size(); i++) {
        if (abs(spotList[band][i].f - f) < SIG_MIN_FREQ_DIFF) {
            indx = i;
            break;
        }
    }
    if (indx != -1) spotList[band].removeAt(indx);
}

/*! returns true if there is a call spotted at this freq
 */
bool So2sdr::isaSpot(int f, int band)
{
    for (int i = 0; i < spotList[band].size(); i++) {
        if (abs(spotList[band][i].f - f) < SIG_MIN_FREQ_DIFF) {
            return(true);
        }
    }
    return(false);
}

/*!
   updates the dupe flags in the bandmap list based on whether this
   qso is a dupe or not
 */
void So2sdr::updateBandmapDupes(const Qso *qso)
{
    if (!settings->value(s_sdr_mark,s_sdr_mark_def).toBool()) return;

    for (int i = 0; i < spotList[qso->band].size(); i++) {
        if (spotList[qso->band].at(i).call == qso->call) {
            spotList[qso->band][i].dupe = qso->dupe;
            spotList[qso->band][i].f    = qso->freq;
        }
    }
}

/*! remove old band spots
 */
void So2sdr::decaySpots()
{
    if (!settings->value(s_sdr_mark,s_sdr_mark_def).toBool()) return;

    for (int i = 0; i < N_BANDS; i++) {
        QMutableListIterator<BandmapEntry> j(spotList[i]);
        while (j.hasNext()) {
            BandmapEntry tmp = j.next();
            tmp.t++;
            if (tmp.t > settings->value(s_sdr_spottime,s_sdr_spottime_def).toInt()) {
                j.remove();
            } else {
                j.setValue(tmp);
            }
        }
    }
}
