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
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutableListIterator>
#include "so2sdr.h"

// /// Telnet/spot database stuff

/*! load spots from a file xxxx.dat
 */
void So2sdr::loadSpots()
{
    // clear spot list
    for (int b = 0; b < N_BANDS; b++) {
        spotList[b].clear();
    }

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

    // read data, deleting old spots
    qint64 currentTime=QDateTime::currentMSecsSinceEpoch();
    while (!spotFile->atEnd()) {
        int b;
        s >> b;
        if (b >= 0 && b < N_BANDS) {
            BandmapEntry *newSpot = new BandmapEntry();
            s >> newSpot->call;
            s >> newSpot->dupe;
            s >> newSpot->createdTime;
            s >> newSpot->f;
            if ((newSpot->createdTime+settings->value(s_sdr_spottime,s_sdr_spottime_def).toInt()*1000) > currentTime) {
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

    // save spots
    for (int b = 0; b < N_BANDS; b++) {
        for (int i = 0; i < spotList[b].size(); i++) {
            s << (qint32) b;
            s << spotList[b][i].call;
            s << spotList[b][i].dupe;
            s << spotList[b][i].createdTime;
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
        setEntryFocus();
    }
}

/*! add a spot. Checks to see if it is a dupe first
 */
void So2sdr::addSpot(int nr,QByteArray call, int f)
{
    // * is a special case, used to mark freq without callsign
    bool d = true;
    if (call != "*") {
        Qso tmp(1);
        tmp.call = call;
        tmp.freq = f;
        tmp.band = getBand(f);
        d        = mylog->isDupe(&tmp, contest->dupeCheckingByBand(), false);
    }
    addSpot(nr,call, f, d);
}


/*!
   add a callsign spot
 */
void So2sdr::addSpot(int nr,QByteArray call, int f, bool d)
{
    BandmapEntry spot;
    spot.call = call;
    spot.f    = f;
    qint64 t = QDateTime::currentMSecsSinceEpoch();
    spot.createdTime = t;
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
            if (bandmap->bandmapon(nr) && b==getBand(cat->getRigFreq(nr))) {
                bandmap->removeSpot(nr,spotList[b][idupe]);
            }
            spotList[b][idupe].call = call;
            spotList[b][idupe].f    = f;
            spotList[b][idupe].createdTime = t;
            spotList[b][idupe].dupe = d;
            if (bandmap->bandmapon(nr) && b==getBand(cat->getRigFreq(nr))) {
                bandmap->addSpot(nr,spotList[b][idupe]);
            }
        } else {
            spotList[b].append(spot);
            if (bandmap->bandmapon(nr) && b==getBand(cat->getRigFreq(nr))) {
                bandmap->addSpot(nr,spot);
            }
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
    static int lastFreq[2] = { 0, 0 };

    // initialize last freq
    if (lastFreq[nr] == 0) {
        lastFreq[nr] = cat->getRigFreq(nr);
        return;
    }
    int f = cat->getRigFreq(nr);

    // freq changed, so recheck spot list
    if (f != lastFreq[nr]) spotListPopUp[nr] = false;

    // currently have a call from list shown. Don't do anything
    if (spotListPopUp[nr]) {
        lastFreq[nr] = f;
        return;
    }

    // search list of spots for one matching current freq
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
    } else if (abs(lastFreq[nr] - f) > SIG_MIN_FREQ_DIFF && contest) {
        if (winkey->isSending() && nr == activeTxRadio)
        {
            winkey->cancelcw();
        }
        if (autoCQMode && nr == autoCQRadio) {
            autoCQActivate(false);
        }
        if (duelingCQMode) {
            duelingCQActivate(false);
        }
        if (activeR2CQ && nr == activeTxRadio) {
            activeR2CQ = false;
            setCqMode(nr);
        }
        if (altDActive && nr == altDActiveRadio) {
            altDActive = false;
            if (nr == activeRadio) {
                switchRadios(false);
            }
            if (altDOrigMode) {
                spMode(nr);
            } else {
                setCqMode(nr);
            }
        }
        if (toggleMode) {
            toggleMode = false;
            toggleStatus->clear();
            if (nr == activeRadio) {
                switchRadios(false);
            }
        }
        if (!cqMode[nr] && csettings->value(c_sprintmode,c_sprintmode_def).toBool()) {
            setCqMode(nr);
        }
        qso[nr]->call.clear();
        qso[nr]->valid=false;
        lineEditCall[nr]->clear();
        lineEditCall[nr]->setModified(false);
        lineEditCall[nr]->setCursorPosition(0);
        lineEditExchange[nr]->clear();
        origCallEntered[nr].clear();
        clearWorked(nr);
        labelCountry[nr]->clear();
        labelBearing[nr]->clear();
        labelLPBearing[nr]->clear();
        sunLabelPtr[nr]->clear();
        So2sdrStatusBar->clearMessage();
        updateNrDisplay();
        callSent[nr] = false;
        if (settings->value(s_settings_qsyfocus,s_settings_qsyfocus_def).toBool()) {
            if ( lineEditCall[nr ^ 1]->text().simplified().isEmpty() && lineEditExchange[nr ^ 1]->text().simplified().isEmpty()
                 && nr != activeRadio && !activeR2CQ && !winkey->isSending()) {
                if (csettings->value(c_sprintmode,c_sprintmode_def).toBool()) {
                    if (cqMode[nr ^ 1]) {
                        switchRadios();
                        callFocus[nr] = true;
                    }
                } else {
                    switchRadios();
                    callFocus[nr] = true;
                }
                if (altDActive && nr != altDActiveRadio && lineEditCall[altDActiveRadio]->text().simplified().isEmpty()) {
                    altDActive = false;
                    if (altDOrigMode && !csettings->value(c_sprintmode,c_sprintmode_def).toBool()) {
                        spMode(altDActiveRadio);
                    } else {
                        setCqMode(altDActiveRadio);
                    }
                }
            }
        }
    }
    lastFreq[nr] = f;
}

/*! remove a spot
 */
void So2sdr::removeSpot(QByteArray call, int band)
{
    int indx = -1;
    for (int i = 0; i < spotList[band].size(); i++) {
        if (spotList[band][i].call == call) {
            indx = i;
            break;
        }
    }
    if (indx != -1) {
        for (int i=0;i<NRIG;i++) {
            if (bandmap->bandmapon(i) && bandmap->currentBand(i)==band) {
                bandmap->removeSpot(i,spotList[band].at(indx));
            }
        }
        spotList[band].removeAt(indx);
    }
}

/*! remove a spot at freq f
 */
void So2sdr::removeSpotFreq(int f, int band)
{
    int indx = -1;
    for (int i = 0; i < spotList[band].size(); i++) {
        if (abs(spotList[band][i].f - f) < SIG_MIN_FREQ_DIFF) {
            indx = i;
            break;
        }
    }
    if (indx != -1) {
        for (int i=0;i<NRIG;i++) {
            if (bandmap->bandmapon(i) && bandmap->currentBand(i)==band) {
                bandmap->removeSpot(i,spotList[band].at(indx));
            }
        }
        spotList[band].removeAt(indx);
    }
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
    for (int i = 0; i < spotList[qso->band].size(); i++) {
        if (spotList[qso->band].at(i).call == qso->call) {
            for (int j=0;j<NRIG;j++) {
                if (bandmap->bandmapon(j) && bandmap->currentBand(j)==qso->band) {
                    bandmap->removeSpot(j,spotList[qso->band].at(i));
                }
            }
            spotList[qso->band][i].dupe = qso->dupe;
            spotList[qso->band][i].f    = qso->freq;
            for (int j=0;j<NRIG;j++) {
                if (bandmap->bandmapon(j) && bandmap->currentBand(j)==qso->band) {
                    bandmap->addSpot(j,spotList[qso->band].at(i));
                }
            }
        }
    }
}

/*! remove old band spots
 */
void So2sdr::decaySpots()
{
    qint64 currentTime=QDateTime::currentMSecsSinceEpoch();
    for (int i = 0; i < N_BANDS; i++) {
        QMutableListIterator<BandmapEntry> j(spotList[i]);
        while (j.hasNext()) {
            BandmapEntry tmp = j.next();
            if ((tmp.createdTime+settings->value(s_sdr_spottime,s_sdr_spottime_def).toInt()*1000)<currentTime) {
                for (int k=0;k<NRIG;k++) {
                    if (bandmap->bandmapon(k) && bandmap->currentBand(k)==i)
                    {
                        bandmap->removeSpot(k,tmp);
                    }
                }
                j.remove();
            }
        }
    }
}

/* the follow slots get called when the bandmap TCP
 * connection succeeds. sendCalls then send the list
 * of bandmap spots to the bandmap through BandmapInterface */

void So2sdr::sendCalls1(bool b)
{
    if (b) sendCalls(0);
}

void So2sdr::sendCalls2(bool b)
{
    if (b) sendCalls(1);
}

void So2sdr::sendCalls(int nr)
{
    int b=getBand(cat->getRigFreq(nr));
    bandmap->syncCalls(nr,spotList[b]);
}
