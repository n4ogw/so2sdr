/*! Copyright 2010-2021 R. Torsten Clay N4OGW

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
#include "bandmapinterface.h"
#include "log.h"
#include "so2sdr.h"
#include "ssbmessagedialog.h"
#include "telnet.h"
#include "winkey.h"

// Telnet/spot database stuff

/*! load spots from a file xxxx.dat
 */
void So2sdr::loadSpots()
{
    // clear spot list
    for (int b = 0; b < N_BANDS; b++) {
        spotList[b].clear();
    }

    // file will be contest_name.dat in same directory as contest cfg file
    QDir::setCurrent(contestDirectory);
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
    QDir::setCurrent(contestDirectory);
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

void So2sdr::showTelnet(bool checkboxState)

// show/remove telnet window
{
    if (!checkboxState) {
        if (telnetOn) {
           // disconnect(telnet, SIGNAL(done()), telnetAction, SLOT(toggle()));
            telnet->hide();
            telnetOn = false;
        }
    } else {
        if (!telnet) {
            telnet = new Telnet(*settings);
            connect(telnet, SIGNAL(done(bool)), telnetAction, SLOT(setChecked(bool)));
            connect(telnet, SIGNAL(dxSpot(QByteArray, double)), this, SLOT(addSpot(QByteArray, double)));
        }
        telnet->show();
        telnetOn = true;
        setEntryFocus(activeRadio);
    }
}

/*! add a spot. Checks to see if it is a dupe first
 */
void So2sdr::addSpot(QByteArray call, double f)
{
    qDebug("addSpot <%s>",call.data());
    // * is a special case, used to mark freq without callsign
    bool d = true;
    if (call != "*") {
        Qso tmp(1);
        tmp.call = call;
        tmp.freq = f;
        tmp.band = getBand(f);
        if (tmp.band==BAND_NONE) return;
        log->isDupe(&tmp, log->dupeCheckingByBand(), false);
        d = tmp.dupe;
    }
    addSpot(call, f, d);
}


/*!
   add a callsign spot. d is dupe status
 */
void So2sdr::addSpot(QByteArray call, double f, bool d)
{
    if (call.isEmpty()) return;
    int b = getBand(f);
    if (b==BAND_NONE) return;
    BandmapEntry spot;
    spot.call = call;
    spot.f    = f;
    qint64 t = QDateTime::currentMSecsSinceEpoch();
    spot.createdTime = t;
    spot.dupe = d;
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
                    if (qAbs(spotList[b].at(i).f - f) < SIG_MIN_SPOT_DIFF) {
                        dupe  = true;
                        idupe = i;
                        break;
                    }
                }
            }
        } else {
            // for non-call spots, just check frequency
            for (int i = 0; i < spotList[b].size(); i++) {
                if (qAbs(spotList[b].at(i).f - f) < SIG_MIN_SPOT_DIFF) {
                    dupe  = true;
                    idupe = i;
                    break;
                }
            }
        }

        if (dupe) {
            // replace previous spot, reset timer
            for (int nr=0;nr<NRIG;nr++) {
                if (bandmap->bandmapon(nr) && b==getBand(cat[nr]->getRigFreq())) {
                    bandmap->removeSpot(nr,spotList[b][idupe]);
                }
            }
            spotList[b][idupe].call = call;
            spotList[b][idupe].f    = f;
            spotList[b][idupe].createdTime = t;
            spotList[b][idupe].dupe = d;
            for (int nr=0;nr<NRIG;nr++) {
                if (bandmap->bandmapon(nr) && b==getBand(cat[nr]->getRigFreq())) {
                    bandmap->addSpot(nr,spotList[b][idupe]);
                }
            }
        } else {
            spotList[b].append(spot);
            for (int nr=0;nr<NRIG;nr++) {
                if (bandmap->bandmapon(nr) && b==getBand(cat[nr]->getRigFreq())) {
                    bandmap->addSpot(nr,spot);
                }
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
    static double lastFreq[NRIG] = { 0, 0 };

    double f = cat[nr]->getRigFreq();
    if (cat[nr]->band()==BAND_NONE) return;

    // freq changed, so recheck spot list
    if (qAbs(f - lastFreq[nr]) > 0.0 ) spotListPopUp[nr] = false;

    // currently have a call from list shown. Don't do anything
    if (spotListPopUp[nr]) {
        lastFreq[nr] = f;
        return;
    }
    // search list of spots for one matching current freq
    bool found = false;
    for (int i = 0; i < spotList[cat[nr]->band()].size(); i++) {
        if (qAbs(f - spotList[cat[nr]->band()].at(i).f) < SIG_MIN_FREQ_DIFF) {
            if (spotList[cat[nr]->band()].at(i).call != "*") {
                lineEditCall[nr]->setText(spotList[cat[nr]->band()].at(i).call);
            } else {
                lineEditCall[nr]->setText("*DUPE*");
                setDupeColor(nr,true);

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
    } else if (qAbs(lastFreq[nr] - f) > SIG_MIN_FREQ_DIFF && log) {
        if (cat[nr]->modeType()==CWType && winkey->isSending() && nr == activeTxRadio)
        {
            winkey->cancelcw();
        }
        if (cat[nr]->modeType()==PhoneType && ssbMessage->isPlaying() && nr == activeTxRadio)
        {
            ssbMessage->cancelMessage();
        }
        if (autoCQMode && nr == autoCQRadio) {
            autoCQActivate(false);
        }
        if (duelingCQMode) {
            duelingCQActivate(false);
        }
        if (activeR2CQ && nr == (activeRadio ^ 1)) {
            clearR2CQ(nr);
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
        qso[nr]->dupe=false;
        qso[nr]->prefill.clear();
        qso[nr]->exch.clear();
        qso[nr]->worked=0;
        for (int ii = 0; ii < MMAX; ii++) {
            qso[nr]->mult[ii]=-1;
        }
        callFocus[nr] = true;
        setEntryFocus(activeRadio);
        MasterTextEdit->clear();
        statusBarDupe = false;
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
        exchangeSent[nr] = false;
        validLabel[nr]->clear();
        setDupeColor(nr,false);
        if (settings->value(s_settings_qsyfocus,s_settings_qsyfocus_def).toBool()) {
            if ( lineEditCall[nr ^ 1]->text().simplified().isEmpty() && lineEditExchange[nr ^ 1]->text().simplified().isEmpty()
                 && nr != activeRadio && !activeR2CQ && !winkey->isSending() && !ssbMessage->isPlaying()) {
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

/*! remove a spot by callsign
 */
void So2sdr::removeSpot(QByteArray call, int band)
{
    if (band==BAND_NONE) return;

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

/*! remove all spots within SIG_MIN_FREQ_DIFF of freq f
 */
void So2sdr::removeSpotFreq(double f, int band)
{
    if (band==BAND_NONE) return;

    int indx = -1;
    for (int i = 0; i < spotList[band].size(); i++) {
        if (qAbs(spotList[band][i].f - f) < SIG_MIN_FREQ_DIFF) {
            indx = i;
            break;
        }
    }
    if (indx != -1) {
        for (int i=0;i<NRIG;i++) {
            if (bandmap->bandmapon(i) && bandmap->currentBand(i)==band) {
                bandmap->removeSpotFreq(i,spotList[band].at(indx));
            }
        }
        spotList[band].removeAt(indx);
    }
}

/*! returns true if there is a call spotted at this freq
 */
bool So2sdr::isaSpot(double f, int band)
{
    if (band==BAND_NONE) return false;

    for (int i = 0; i < spotList[band].size(); i++) {
        if (qAbs(spotList[band][i].f - f) < SIG_MIN_FREQ_DIFF) {
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
    if (qso->band==BAND_NONE) return;

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
    int b=getBand(cat[nr]->getRigFreq());
    if (b!=BAND_NONE) bandmap->syncCalls(nr,spotList[b]);
}
