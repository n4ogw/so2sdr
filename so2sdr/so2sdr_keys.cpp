 /*! Copyright 2010-2017 R. Torsten Clay N4OGW

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
#include <QKeyEvent>
#include <QSettings>
#include <QSqlTableModel>
#include "so2sdr.h"

/*! event filter handling key presses. This gets installed in
   -main window
   -both call entry windows
   -both exchange entry windows
   -bandmap
   -dupesheet
 */
bool So2sdr::eventFilter(QObject* o, QEvent* e)
{
    if (!uiEnabled) {
        return QObject::eventFilter(o,e);
    }

    // out-of-focus event: refocus line edits
    // on Windows, this eats all mouse presses into the bandmap!
#ifdef Q_OS_LINUX
    if (grabbing && e->type()==QEvent::FocusOut) {
        QFocusEvent* ev = static_cast<QFocusEvent*>(e);
        if (ev->reason()==Qt::MouseFocusReason || ev->reason()==Qt::ActiveWindowFocusReason) {
            setEntryFocus();
            return true;
        }
    }
#endif

    // set r true if this completely handles the key. Otherwise
    // it will be passed on to other widgets
    bool r   = false;

    switch (e->type()) {
    case QEvent::MouseButtonPress:

        // if call line edit clicked in, switch to that radio
        // otherwise focus that entry line
        for (int i=0;i<NRIG;i++) {
            if (lineEditCall[i]->underMouse()) {
                if (activeRadio!=i) {
                    switchRadios(false);
                }
                callFocus[activeRadio]=true;
                setEntryFocus();
                return true;
            }
            if (lineEditExchange[i]->underMouse()) {
                if (activeRadio!=i) {
                    switchRadios(false);
                }
                callFocus[activeRadio]=false;
                setEntryFocus();
                return true;
            }
        }
        // catch other mouse presses and reset focus to line edits
        setEntryFocus();
        break;
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::ShortcutOverride:
    {
        // eat any kind of single alt keypress to prevent menubar from
        // stealing focus (this is a pain!)
        QKeyEvent* kev = static_cast<QKeyEvent*>(e);
        if (kev->key() == Qt::Key_Alt || kev->key() == Qt::Key_Meta) {
            kev->setAccepted(true);
            return(true);
        }
    }
    break;
    default:
        break;
    }

    switch (e->type()) {
    case QEvent::KeyPress:
    {
        QKeyEvent* kev = static_cast<QKeyEvent*>(e);
        Qt::KeyboardModifiers mod = kev->modifiers();
        switch (kev->key()) {
        case Qt::Key_Escape:
            // notes window is active, prevent main window from getting esc
            if (notes) {
                if (notes->isActiveWindow()) {
                    return(r);
                }
            }

            // if other dialogs are active, close them
            if (radios) {
                if (radios->isActiveWindow()) {
                    radios->rejectChanges();
                    return(r);
                }
            }
            if (cabrillo) {
                if (cabrillo->isActiveWindow()) {
                    cabrillo->reject();
                    return(r);
                }
            }
            if (cwMessage) {
                if (cwMessage->isActiveWindow()) {
                    cwMessage->rejectChanges();
                    return(r);
                }
            }
            if (ssbMessage) {
                if (ssbMessage->isActiveWindow()) {
                    ssbMessage->rejectChanges();
                    return(r);
                }
            }
            if (options) {
                if (options->isActiveWindow()) {
                    options->rejectChanges();
                    return(r);
                }
            }
            if (winkeyDialog) {
                if (winkeyDialog->isActiveWindow()) {
                    winkeyDialog->rejectChanges();
                    return(r);
                }
            }
            if (sdr) {
                if (sdr->isActiveWindow()) {
                    sdr->rejectChanges();
                    return(r);
                }
            }
            if (help) {
                if (help->isActiveWindow()) {
                    help->reject();
                    return(r);
                }
            }
            if (progsettings) {
                if (progsettings->isActiveWindow()) {
                    progsettings->rejectChanges();
                    return(r);
                }
            }
            if (station) {
                if (station->isActiveWindow()) {
                    station->rejectChanges();
                    return(r);
                }
            }

            esc(mod);
            r = true;
            break;
        case Qt::Key_QuoteLeft: // left quote ` Toggles audio stereo
            toggleStereo();
            return(true);
            break;
        case Qt::Key_C:     // alt-C : bring up config menu
            if (mod == Qt::AltModifier) {
                menubar->setActiveAction(menubar->actions()[1]);
                return(true);
            }
            break;
        case Qt::Key_D:     // alt-D
            if (mod == Qt::AltModifier) {
                altd();
                r = true;
            }
            break;
        case Qt::Key_E:    // control-E : edit log
            if (mod == Qt::ControlModifier) {
                controlE();
                r=true;
            }
            break;
        case Qt::Key_F:
            // alt-F : bring up file menu
            if (mod == Qt::AltModifier) {
                menubar->setActiveAction(menubar->actions()[0]);
                return(true);
            } else if (mod == Qt::ControlModifier) {
                // ctrl-F : log search
                logSearch();
                return(true);
            }
            break;
        case Qt::Key_H:     // alt-H : bring up Help menu
            if (mod == Qt::AltModifier) {
                menubar->setActiveAction(menubar->actions()[3]);
                return(true);
            }
            break;
        case Qt::Key_M:     // alt-M
            if (mod == Qt::AltModifier) {
                switchMultMode();
                r = true;
            }
            break;
        case Qt::Key_N:     // ctrl-N, alt-N
            if (mod == Qt::ControlModifier) {
                writeNote();
                r = true;
            } else if (mod == Qt::AltModifier) {
                if (csettings->value(c_multimode,c_multimode_def).toBool()) {
                    // switch to next modeType if it is allowed
                    modeTypeShown=contest->nextModeType(modeTypeShown);
                    setSummaryGroupBoxTitle();
                }
                r=true;
            }
            break;
        case Qt::Key_Q:     // alt-Q
            if (mod == Qt::AltModifier) {
                autoCQActivate(autoCQMode ^ 1);
                r = true;
            }
            break;
        case Qt::Key_R:     // alt-R
            if (mod == Qt::AltModifier) {
                if (duelingCQMode) duelingCQModePause = true;
                switchRadios();
                r = true;
            } else if (mod == Qt::ControlModifier) { // switchradios w/o killing cw
                if (duelingCQMode) duelingCQModePause = true;
                switchRadios(false);
                r = true;
            }
            // if radio switched during alt-D process, clear altD unless
            // an alt-d qso is in progress (altDActive==3)
            if (mod == Qt::AltModifier || mod == Qt::ControlModifier) {
                if (altDActive==1 || altDActive==2) {
                    QPalette palette(lineEditCall[altDActiveRadio]->palette());
                    palette.setColor(QPalette::Base, CQ_COLOR);
                    lineEditCall[altDActiveRadio]->setPalette(palette);
                    lineEditCall[altDActiveRadio]->clear();
                    lineEditCall[altDActiveRadio]->setModified(false);
                    lineEditExchange[altDActiveRadio]->setPalette(palette);
                    lineEditExchange[altDActiveRadio]->clear();
                    qso[altDActiveRadio]->call.clear();
                    exchangeSent[altDActiveRadio] = false;
                    qso[altDActiveRadio]->prefill.clear();
                    origCallEntered[altDActiveRadio].clear();
                    labelCountry[altDActiveRadio]->clear();
                    labelBearing[altDActiveRadio]->clear();
                    labelLPBearing[altDActiveRadio]->clear();
                    sunLabelPtr[altDActiveRadio]->clear();
                    clearWorked(altDActiveRadio);
                    cqMode[altDActiveRadio]=true;
                    qso[altDActiveRadio]->dupe = false;
                    altDActive             = 0;
                    So2sdrStatusBar->clearMessage();
		    spotListPopUp[altDActiveRadio]=false;
                    r=true;
                }
            }
            break;
        case Qt::Key_S:
            // alt-S
            if (mod == Qt::AltModifier) {
                launch_WPMDialog();
                r = true;
            }
            if ((mod & Qt::AltModifier) && (mod & Qt::ControlModifier)) {
                screenShot();
                r = true;
            }
            break;
        case Qt::Key_W:     // alt-W : bring up Windows menu
            if (mod == Qt::AltModifier) {
                menubar->setActiveAction(menubar->actions()[2]);
                return(true);
            }
            break;
        case Qt::Key_Tab:
            tab();
            r = true;
            break;
        case Qt::Key_Space:
            if (callFocus[activeRadio]) {
                spaceBar();
                r = true;
            } else if (mod == Qt::AltModifier && altDActive == 2 && activeRadio != altDActiveRadio) {
                spaceAltD();
                r = true;
            }
            break;
        case Qt::Key_Backslash:
            backSlash();
            r = true;
            break;
        case Qt::Key_Up:
            if (mod == Qt::ControlModifier) {   // ctrl-up
                keyCtrlUp();
            } else {
                up();
            }
            r = true;
            break;
        case Qt::Key_Down:
            if (mod == Qt::ControlModifier) {   // ctrl-down
                keyCtrlDn();
            } else {
                down();
            }
            r = true;
            break;
        case Qt::Key_Return:

            // notes window is active- prevent main window from getting enter
            if (notes) {
                if (notes->isActiveWindow()) {
                    return(r);
                }
            }

            // if various dialogs are active, they should get the enter
            if (station) {
                if (station->isActiveWindow()) {
                    station->updateStation();
                    return(r);
                }
            }
            if (winkeyDialog) {
                if (winkeyDialog->isActiveWindow()) {
                    winkeyDialog->updateWinkey();
                    return(r);
                }
            }
            if (radios) {
                if (radios->isActiveWindow()) {
                    radios->updateRadio();
                    return(r);
                }
            }
            if (cwMessage) {
                if (cwMessage->isActiveWindow()) {
                    cwMessage->updateCWMsg();
                    return(r);
                }
            }
            if (ssbMessage) {
                if (ssbMessage->isActiveWindow()) {
                    ssbMessage->updateSSBMsg();
                    return(r);
                }
            }
            if (options) {
                if (options->isActiveWindow()) {
                    options->updateOptions();
                    return(r);
                }
            }
            if (sdr) {
                if (sdr->isActiveWindow()) {
                    sdr->updateSDR();
                    return(r);
                }
            }
            // toggle mode
            if (duelingCQMode) {
                enter(mod);
            } else if (toggleMode || mod == Qt::AltModifier) {
                enter(mod);
            } else if (altDActive) {
                altDEnter(altDActive, mod);
            } else {
                enter(mod);
            }
            r = true;
            break;
        case Qt::Key_PageUp:
            if (mod == Qt::AltModifier) {
                autoCQdelay(true); // increase
            } else {
                launch_speedUp(mod);
            }
            r = true;
            break;
        case Qt::Key_PageDown:
            if (mod == Qt::AltModifier) {
                autoCQdelay(false); // decrease
            } else {
                launch_speedDn(mod);
            }
            r = true;
            break;
        case Qt::Key_F1:
            if (((duelingCQMode && !duelingCQModePause) || (autoCQMode && !autoCQModePause) || (toggleMode && !duelingCQMode)) && !excMode[activeRadio]
                    && mod != Qt::ShiftModifier && mod != Qt::ControlModifier
                    && !(altDActive > 2 && altDActiveRadio == activeRadio)) { // set long CQ MSG, no CW
                sendLongCQ = true;
            } else {
                if (altDActive > 2 && altDActiveRadio == activeRadio) {
                    if (activeTxRadio != activeRadio) {
                        switchTransmit(altDActiveRadio);
                    }
                } else {
                    if ((autoCQMode && !excMode[autoCQRadio]) || (duelingCQMode && !excMode[activeRadio])) {
                        sendLongCQ = true;
                    }
                }
                sendFunc(0, mod);
            }
            r = true;
            break;
        case Qt::Key_F2:
            if (((duelingCQMode && !duelingCQModePause) || (autoCQMode && !autoCQModePause) || (toggleMode && !duelingCQMode)) && !excMode[activeRadio]
                    && mod != Qt::ShiftModifier && mod != Qt::ControlModifier
                    && !(altDActive > 2 && altDActiveRadio == activeRadio)) { // set short CQ MSG, no CW
                sendLongCQ = false;
            } else {
                if (altDActive > 2 && altDActiveRadio == activeRadio) {
                    if (activeTxRadio != activeRadio) {
                        switchTransmit(altDActiveRadio);
                    }
                } else {
                    if ((autoCQMode && !excMode[autoCQRadio]) || (duelingCQMode && !excMode[activeRadio])) {
                        sendLongCQ = false;
                    }
                }
                sendFunc(1, mod);
            }
            r = true;
            break;
        case Qt::Key_F3:
            sendFunc(2, mod);
            r = true;
            break;
        case Qt::Key_F4:
            sendFunc(3, mod);
            r = true;
            break;
        case Qt::Key_F5:
            sendFunc(4, mod);
            r = true;
            break;
        case Qt::Key_F6:
            sendFunc(5, mod);
            r = true;
            break;
        case Qt::Key_F7:
            sendFunc(6, mod);
            r = true;
            break;
        case Qt::Key_F8:
            sendFunc(7, mod);
            r = true;
            break;
        case Qt::Key_F9:
            sendFunc(8, mod);
            r = true;
            break;
        case Qt::Key_F10:
            sendFunc(9, mod);
            r = true;
            break;
        case Qt::Key_F11:
            sendFunc(10, mod);
            r = true;
            break;
        case Qt::Key_F12:
            sendFunc(11, mod);
            r = true;
            break;
        case Qt::Key_Equal:
            markDupe(activeRadio ^ 1);
            r = true;
            break;
        case Qt::Key_Minus:
            if (mod == Qt::ControlModifier) {
                duelingCQActivate(duelingCQMode ^ 1);
            } else if (mod == Qt::AltModifier) {
                autoSendActivate(autoSend ^ 1);
            } else {
                markDupe(activeRadio);
            }
            r = true;
            break;
        case Qt::Key_Backspace:
            if (lineEditCall[activeRadio]->text().isEmpty()) {
                autoSendTrigger = false;
                autoSendPause = false;
                tmpCall.clear();
            }
            break;
        default:
            break;
        }
    }
        break;
    default:
        break;
    }
    return(r);
}

/*!
 set title of group box surrounding qso totals
 */
void So2sdr::setSummaryGroupBoxTitle()
{
    if (csettings->value(c_multimode,c_multimode_def).toBool()) {
        switch (modeTypeShown) {
        case CWType:
            groupBox->setTitle("Summary:CW");
            break;
        case PhoneType:
            groupBox->setTitle("Summary:PHONE");
            break;
        case DigiType:
            groupBox->setTitle("Summary:DIGITAL");
            break;
        }
    } else {
        groupBox->setTitle("Summary:");
    }
}

/*!
Control-E : starts detailed qso edit if a log cell has been selected
*/
void So2sdr::controlE()
{
    QModelIndexList indices=LogTableView->selectionModel()->selectedIndexes();
    if (indices.size()) {
        logdel->startDetailedEdit();
    }
}

/*! Toggles current freq on bandmap as a dupe, placing "*" as callsign
 *
 */
void So2sdr::markDupe(int nrig)
{
    int f = cat[nrig]->getRigFreq();
    if (isaSpot(f, band[nrig])) {
        // remove spot and reset status
        removeSpotFreq(f, band[nrig]);
        callSent[nrig]=false;
        exchangeSent[nrig]=false;
        qso[nrig]->dupe=false;
        qso[nrig]->call.clear();
        qso[nrig]->valid=false;
        lineEditCall[nrig]->clear();
        lineEditExchange[nrig]->clear();
        statusBarDupe=false;
        So2sdrStatusBar->clearMessage();
        validLabel[nrig]->clear();
    } else {
        addSpot("*", cat[nrig]->getRigFreq(), true);
    }
}

/*!
   alt+d : dupe check on other radio

   Mutually exclusive with Sprint Mode
 */
void So2sdr::altd()
{

    // not available with sprint so2r
    if (csettings->value(c_sprintmode,c_sprintmode_def).toBool()) return;

    // wipe alt-d if already active (toggle off)
    // nothing if dueling CQ or Toggle Modes active
    if (altDActive) {
        callFocus[altDActiveRadio] = true;
        lineEditCall[altDActiveRadio]->clear();
        lineEditCall[altDActiveRadio]->setModified(false);
        lineEditExchange[altDActiveRadio]->clear();
        qso[altDActiveRadio]->call.clear();
        exchangeSent[altDActiveRadio] = false;
        qso[altDActiveRadio]->prefill.clear();
        origCallEntered[altDActiveRadio].clear();
        labelCountry[altDActiveRadio]->clear();
        labelBearing[altDActiveRadio]->clear();
        labelLPBearing[altDActiveRadio]->clear();
        sunLabelPtr[altDActiveRadio]->clear();
        clearWorked(altDActiveRadio);
        validLabel[altDActiveRadio]->clear();
        qso[altDActiveRadio]->dupe = false;
        So2sdrStatusBar->clearMessage();
        altDActive = false;
        if (altDOrigMode) {
            spMode(altDActiveRadio);
        } else {
            setCqMode(altDActiveRadio);
        }
        if (altDActiveRadio == activeRadio) {
            switchRadios(false);
        }
       // return;
    }
    if (duelingCQMode || toggleMode) {
        So2sdrStatusBar->showMessage("Disable Dueling CQ or Toggle",2000);
        return;
    }

    // switch radios, but don't switch/stop cw
    callFocus[activeRadio ^ 1] = true;
    altDActive = 1;
    switchRadios(false);
    if (cqMode[activeRadio]) {
        altDOrigMode = 0;
    } else {
        altDOrigMode = 1;
    }
    altDActiveRadio = activeRadio;

    // change color to alert alt-D in progress
    QPalette palette(lineEditCall[activeRadio]->palette());
    palette.setColor(QPalette::Base, ALTD_COLOR);
    lineEditCall[activeRadio]->setPalette(palette);
    lineEditExchange[activeRadio]->setPalette(palette);
}

/*!
   up arrow key
 */
void So2sdr::up()
{
    // if in CQ mode and only call window showing
    if (cqMode[activeRadio] && !excMode[activeRadio]) return;

    callFocus[activeRadio]= !callFocus[activeRadio];
    setEntryFocus();
}

/*! control+up : tune to next higher signal on bandmap

   If activeRadio is in S&P mode, tune that radio. Otherwise tune the 2nd radio
   Tune active radio in Sprint, assumes active radio always S/P
 */
void So2sdr::keyCtrlUp()
{
    int nr = activeRadio;
    if (cqMode[activeRadio] && !csettings->value(c_sprintmode,c_sprintmode_def).toBool()) {
        nr = nr ^ 1;
    }
    if (bandmap->bandmapon(nr)) {
        bandmap->nextFreq(nr,true);
    }
}


/*!
   down arrow key
 */
void So2sdr::down()
{
    // if in CQ mode and only call window showing
    if (cqMode[activeRadio] && !excMode[activeRadio]) {
        // method to get to exch w/o CW
        if (!lineEditCall[activeRadio]->text().simplified().isEmpty()) {
            excMode[activeRadio] = true;
            lineEditExchange[activeRadio]->show();
            prefillExch(activeRadio);
        } else {
            return;
        }
    }
    callFocus[activeRadio]= !callFocus[activeRadio];
    setEntryFocus();
}

/*! control+down : tune to next lower signal on bandmap

   If activeRadio is in S&P mode, tune that radio. Otherwise tune the 2nd radio
   Tune active radio in Sprint, assumes active radio always S/P
 */
void So2sdr::keyCtrlDn()
{
    int nr = activeRadio;
    if (cqMode[activeRadio] && !csettings->value(c_sprintmode,c_sprintmode_def).toBool()) {
        nr = nr ^ 1;
    }

    if (bandmap->bandmapon(nr)) {
        bandmap->nextFreq(nr,false);
    }
}


/*!
   Tab:

   if in CQ mode, switch to S&P mode

   if in S&P mode, switch between fields
 */
void So2sdr::tab()
{
    if (cqMode[activeRadio]) {
        if (autoCQRadio == activeRadio) {
            autoCQActivate(false);
        }
        duelingCQActivate(false);
        spMode(activeRadio);
        if (!lineEditCall[activeRadio]->text().simplified().isEmpty()) prefillExch(activeRadio);
        // redisplay band/mult info for this station
        updateWorkedDisplay(activeRadio,qso[activeRadio]->worked);
        updateWorkedMult(activeRadio);
        callFocus[activeRadio]=true;
    } else {
        callFocus[activeRadio]= !callFocus[activeRadio];
    }
    setEntryFocus();

    if (!callFocus[activeRadio]) {
        if (lineEditExchange[activeRadio]->text().simplified().isEmpty()) {
            lineEditExchange[activeRadio]->clear();
        } else {
            lineEditExchange[activeRadio]->setText(lineEditExchange[activeRadio]->text().simplified() + " ");
        }
    }
}

/*!
   Backslash == quick qsl key
 */
void So2sdr::backSlash()
{
    // this key is operational if:
    // 1) text in call field
    // 2) exchange is validated, is not a dupe
    // 3) CQ mode
    // 4) exchange has been sent
    qso[activeRadio]->exch = lineEditExchange[activeRadio]->text().toLatin1();
    qso[activeRadio]->exch = qso[activeRadio]->exch.trimmed();
    if (!qso[activeRadio]->call.isEmpty() &&
        (qso[activeRadio]->valid=contest->validateExchange(qso[activeRadio])) &&
        !(qso[activeRadio]->dupe && csettings->value(c_dupemode,c_dupemode_def).toInt() == 0) &&
        cqMode[activeRadio] &&
        exchangeSent[activeRadio])
    {
        if (toggleMode) {
            switchTransmit(activeRadio);
        }
        int m=(int)cat[activeTxRadio]->modeType();
        expandMacro(csettings->value(c_qqsl_msg[m],c_qqsl_msg_def[m]).toByteArray());

        // log qso
        clearLogSearch();
        if (!qso[activeRadio]->dupe && qso[activeRadio]->valid)
            nqso[band[activeRadio]]++;

        updateBreakdown();
        updateOffTime();

        if (!cqMode[activeRadio]) {
            // add to bandmap if in S&P mode
            qso[activeRadio]->freq = rigFreq[activeRadio];
            addSpot(qso[activeRadio]->call, qso[activeRadio]->freq, true);
            spotListPopUp[activeRadio] = true;
        } else {
            // remove any spot that is on freq, update other spots if in CQ mode
            qso[activeRadio]->freq = rigFreq[activeRadio];
            removeSpotFreq(qso[activeRadio]->freq, band[activeRadio]);
            updateBandmapDupes(qso[activeRadio]);
        }
        qso[activeRadio]->mode = cat[activeRadio]->mode();
        fillSentExch(activeRadio);
        contest->addQso(qso[activeRadio]);
        qso[activeRadio]->time = QDateTime::currentDateTimeUtc(); // update time just before logging qso
        addQso(qso[activeRadio]);
        updateDupesheet(qso[activeRadio]->call,activeRadio);
        updateMults(activeRadio);
        rateCount[ratePtr]++;
        exchangeSent[activeRadio] = false;
        if (csettings->value(c_sprintmode,c_sprintmode_def).toBool()) {
            sprintMode();
        }
        labelCountry[activeRadio]->clear();
        labelBearing[activeRadio]->clear();
        labelLPBearing[activeRadio]->clear();
        sunLabelPtr[activeRadio]->clear();
        qso[activeRadio]->prefill.clear();
        qso[activeRadio]->zone=0;

        // advance qso numbers
        nrReserved[activeRadio] = 0;
        nrSent++;
        updateNrDisplay();
        cqQsoInProgress[activeRadio] = false;

        // focus call field
        callFocus[activeRadio] = true;
        setEntryFocus();

        // clear exchange field
        lineEditExchange[activeRadio]->clear();
        lineEditExchange[activeRadio]->setModified(false);
        qso[activeRadio]->exch.clear();
        qso[activeRadio]->prefill.clear();
        validLabel[activeRadio]->clear();

        // clear call field
        lineEditCall[activeRadio]->clear();
        lineEditCall[activeRadio]->setModified(false);
        qso[activeRadio]->call.clear();
        qso[activeRadio]->dupe = false;
        qso[activeRadio]->valid = false;
        exchangeSent[activeRadio] = false;
        qso[activeRadio]->prefill.clear();
        origCallEntered[activeRadio].clear();
        clearWorked(activeRadio);
        if (autoSend) {
            autoSendPause = false;
            autoSendTrigger = false;
            tmpCall.clear();
        }

        // exit echange mode
        excMode[activeRadio] = false;
        if (cqMode[activeRadio]) {
            lineEditExchange[activeRadio]->hide();
        }

        // check for new call entered in exchange line
        QByteArray tmp;
        if (contest->newCall(tmp)) {
            lineEditCall[activeRadio]->setText(tmp);
            prefixCheck(activeRadio, tmp);
        }
    }
}

/*!
   space bar
 */
void So2sdr::spaceBar()
{
    if (csettings->value(c_sprintmode,c_sprintmode_def).toBool()) {
        // Sprint. If in CQ mode, space switches to S&P and sends call
        // In S&P mode, focus exch field if in call field
        spaceSprint();
    } else if (!csettings->value(c_sprintmode,c_sprintmode_def).toBool() && altDActive == 2) {
        // in regular SO2R, if alt+D is active, space calls station on other radio
        spaceAltD();
    } else if (!csettings->value(c_sprintmode,c_sprintmode_def).toBool() && !cqMode[activeRadio]) {
        // space in S&P mode. Does dupe check
        spaceSP(activeRadio);
    }
}

/*!
   Space bar in Sprint Mode

   set S&P mode
    send call
 */
void So2sdr::spaceSprint()
{
    // if already in SP mode and in call field, move to exch field, otherwise, no nothing
    if (!cqMode[activeRadio]) {
        if (callFocus[activeRadio] && !lineEditCall[activeRadio]->text().isEmpty()) {
            callFocus[activeRadio] = false;
            setEntryFocus();
            excMode[activeRadio]   = true;
            if (lineEditExchange[activeRadio]->text().simplified().isEmpty()) {
                lineEditExchange[activeRadio]->clear();
            } else {
                lineEditExchange[activeRadio]->setText(lineEditExchange[activeRadio]->text().simplified() + " ");
            }
        }
        return;
    }
    if (callFocus[activeRadio]) {
        spMode(activeRadio);
        lineEditCall[activeRadio]->setModified(false);
        setEntryFocus();
        if (lineEditCall[activeRadio]->text().length() > 3) {
            // leaves exchange empty, even non-dupes.  prefill.clear in log.cpp
            if (!qso[activeRadio]->dupe) {
                callFocus[activeRadio] = false;
                setEntryFocus();
                switch (cat[activeRadio]->modeType()) {
                case CWType:case DigiType:
                    expandMacro(settings->value(s_call,s_call_def).toByteArray());
                    break;
                case PhoneType:
                    expandMacro(csettings->value(c_call_msg,c_call_msg_def).toByteArray());
                    break;
                }
                callSent[activeRadio] = true;
            }
            prefillExch(activeRadio);
            if (lineEditExchange[activeRadio]->text().simplified().isEmpty()) {
                lineEditExchange[activeRadio]->clear();
            } else {
                lineEditExchange[activeRadio]->setText(lineEditExchange[activeRadio]->text().simplified() + " ");
            }
            // redisplay band/mult info for this station
            updateWorkedDisplay(activeRadio,qso[activeRadio]->worked);
            updateWorkedMult(activeRadio);

        } else {
            switch (cat[activeRadio]->modeType()) {
            case CWType:case DigiType:
                expandMacro(settings->value(s_call,s_call_def).toByteArray());
                break;
            case PhoneType:
                expandMacro(csettings->value(c_call_msg,c_call_msg_def).toByteArray());
                break;
            }
            callSent[activeRadio] = true;
        }
    }
}

/*!
   Space bar in S&P mode

   do dupecheck
 */
void So2sdr::spaceSP(int nrig)
{
    if (!dupeCheckDone) {
        qso[nrig]->dupe = mylog->isDupe(qso[nrig], contest->dupeCheckingByBand(), true) &&
                          csettings->value(c_dupemode,c_dupemode_def).toInt() < NO_DUPE_CHECKING;
    }
    // update displays
    updateWorkedDisplay(nrig,qso[nrig]->worked);

    // put call on bandmap
    qso[nrig]->freq = rigFreq[nrig];
    addSpot(qso[nrig]->call, qso[nrig]->freq, qso[nrig]->dupe);
    // if dupe, clear call field
    if (qso[nrig]->dupe) {
        spotListPopUp[nrig]=false;
        lineEditCall[nrig]->clear();
        lineEditCall[nrig]->setModified(false);
        lineEditExchange[nrig]->clear();
    }
}

/*!
   space bar: 2nd radio qso
   switch radios, switch to S&P mode, send call
 */
void So2sdr::spaceAltD()
{
    if (autoCQMode) {
        autoCQModePause = true;
    }
    switchRadios();
    spMode(activeRadio);
    callFocus[activeRadio] = false;
    setEntryFocus();
    switch (cat[activeRadio]->modeType()) {
    case CWType:case DigiType:
        expandMacro(settings->value(s_call,s_call_def).toByteArray());
        break;
    case PhoneType:
        expandMacro(csettings->value(c_call_msg,c_call_msg_def).toByteArray());
        break;
    }
    callSent[activeRadio] = true;
    prefillExch(activeRadio);
    // redisplay band/mult info for this station
    updateWorkedDisplay(activeRadio,qso[activeRadio]->worked);
    updateWorkedMult(activeRadio);
    altDActive = 3;
}


/*!
   Enter key - 2nd radio dupecheck

 */
void So2sdr::altDEnter(int level, Qt::KeyboardModifiers mod)
/*!
   - level==1  : call entered on 2nd radio; do dupecheck
   - level==2  : nothing; state when working qso on radio 2
   - level==3  : log qso, switch radios. Handle with normal enter + switchRadios()
 */
{
    switch (level) {
    case 0:
        enter(mod); // in case called incorrectly
        break;
    case 1:
        // in case user switched to other radio in the middle of alt-D sequence
        if (activeRadio != altDActiveRadio) {
            enter(mod);
            return;
        }
        qso[activeRadio]->freq = rigFreq[activeRadio];
        addSpot(qso[activeRadio]->call, qso[activeRadio]->freq, qso[activeRadio]->dupe);
        if (qso[activeRadio]->dupe || lineEditCall[activeRadio]->text().isEmpty()) {
            // call is a dupe- clear everything
            QPalette palette(lineEditCall[activeRadio]->palette());
            palette.setColor(QPalette::Base, CQ_COLOR);
            lineEditCall[activeRadio]->setPalette(palette);
            lineEditCall[activeRadio]->clear();
            lineEditCall[activeRadio]->setModified(false);
            lineEditExchange[activeRadio]->setPalette(palette);
            lineEditExchange[activeRadio]->clear();
            qso[activeRadio]->call.clear();
            exchangeSent[activeRadio] = false;
            qso[activeRadio]->prefill.clear();
            origCallEntered[activeRadio].clear();
            labelCountry[activeRadio]->clear();
            labelBearing[activeRadio]->clear();
            labelLPBearing[activeRadio]->clear();
            sunLabelPtr[activeRadio]->clear();
            clearWorked(activeRadio);
            qso[activeRadio]->dupe = false;
            altDActive             = 0;
            So2sdrStatusBar->clearMessage();
        } else {
            altDActive = 2; // setting 2 will change spacebar checking rules
        }

        // go back to original radio. If not a dupe, space will
        // call station
        switchRadios(false);
        break;
    case 2:
        enter(mod);
        break;
    case 3:
        enter(mod);
        if (altDActive == 0) {
            // only switch radios if qso is logged, which sets altDActive=0 again
            if (altDOrigMode == 0) {
                // originally in cq mode, return
                setCqMode(activeRadio);
            } else {
                spMode(activeRadio);
            }
            switchRadios(false); // don't kill exchange

            // popup any call entered after "/" on original radio
            QByteArray tmp;
            if (contest->newCall(tmp)) {
                lineEditCall[activeRadio]->setText(tmp);
                prefixCheck(activeRadio, tmp);
                if (autoSend) autoSendPause = true;
            }
        }
        break;
    }
}

/*!
   Enter key processing
   <ul>
   <li>a. is there text in call field  (test().size!=0)(0=no, 1=yes)
   <li>b. exchange validated and not a dupe            (0=no, 1=yes)
   <li>c. CQ or S&P mode                               (0=cq 1=sp)
   <li>d. exchange sent                                (0=no, 1=yes)
   </ul>

   --> [2][2][2][2] matrix of unsigned int (16 states)
     a  b  c  d

   actions:

   <ul>
   <li>a. send Call                     0=1
   <li>b. send CQ exchange              1=2
   <li>c. focus exchange field          2=4
   <li>d. log qso                       3=8
   <li>e. send SP exchange              4=16
   <li>f. focus call field              5=32
   <li>g. clear exchange field          6=64
   <li>h. clear call field              7=128
   <li>i. send F1                       8=256
   <li>j. send QSL_Msg                  9=512
   <li>k. set Exc mode                 10=1024
   <li>l. unset Exc mode               11=2048
   <li>m. set next callsign            12=4096
   <li>n. set initial exchange         13=8192
   <li>o. change radios if 2nd R cq    14=16384
   <li>p. clear 2nd radio cq status    15=32768
   <li>q. send F9 (?)                  16=65536
   </ul>

   states

   unsigned int enterState[2][2][2][2];

   mods:
   mod=Qt::ControlModifier : logs without dupecheck
   mod=Qt::ShiftModifier   : same except no cw

 */
void So2sdr::enter(Qt::KeyboardModifiers mod)
{
    // enable/disable if alt-Enter, unless dueling CQ (dependent on toggle)
    if (mod == Qt::AltModifier && !duelingCQMode) {
        toggleMode ^= 1;
        if (toggleMode) {
            autoCQActivate(false);
            autoSendStatus->hide();
            toggleStatus->setText("<font color=#0000FF>TOGGLE</font>");
        } else {
            toggleStatus->clear();
            autoSendStatus->show();
            if (activeRadio != activeTxRadio) {
                switchRadios(false);  // move focus to where last transmit
            }
            return;
        }
    }
    if (toggleMode) {
        switchTransmit(activeRadio);
    }

    int                 i1;
    int                 i2;
    int                 i3;
    int                 i4;
    static bool         first = true;
    static unsigned int enterState[2][2][2][3];
    if (first) {
        enterState[0][0][0][0] = 256 + 32 + 32768;
        enterState[0][0][1][0] = 1 + 32;
        enterState[0][1][0][0] = 0;
        enterState[0][1][1][0] = 1;
        enterState[1][0][0][0] = 2 + 4 + 1024 + 8192 + 16384;
        enterState[1][0][1][0] = 1 + 1024 + 8192 + 4;
        enterState[1][1][0][0] = 2 + 4 + 1024;
        enterState[0][0][0][1] = 0;
        enterState[0][0][1][1] = 0;
        enterState[0][1][0][1] = 0;
        enterState[0][1][1][1] = 0;
        enterState[1][0][0][1] = 4 + 65536;
        enterState[1][0][1][1] = 0;
        enterState[1][1][0][1] = 8 + 32 + 64 + 128 + 512 + 2048 + 4096;
        enterState[1][1][1][1] = 8 + 32 + 64 + 128 + 2048;
        first                  = false;
    }
    if (callSent[activeRadio]) {
        if (settings->value(s_settings_exchangelogs,s_settings_exchangelogs_def).toBool()) {
            enterState[1][1][1][0] = 8 + 16 + 32 + 64 + 128 + 2048;
        } else {
            enterState[1][1][1][0] = 16;
        }
    } else {
        enterState[1][1][1][0] = 1 + 1024 + 8192 + 4;
    }

    // is it an entered frequency or mode?
    if (!qso[activeRadio]->call.isEmpty()) {
        i1 = 1;
        if (qso[activeRadio]->country == -1) {
            if (enterFreqOrMode()) {
                // clear any garbage that showed up in supercheck partial
                if (csettings->value(c_mastermode,c_mastermode_def).toBool()) {
                    MasterTextEdit->clear();
                }
                duelingCQActivate(false);
                return; // qsy successful
            }
        }
    } else {
        i1 = 0;
    }

    if (cqMode[activeRadio]) {
        // CQ mode
        i3 = 0;
        // if WORK_DUPES is set, don't use dupe status of qso
        if (csettings->value(c_dupemode,c_dupemode_def).toInt()==WORK_DUPES) {
            if (qso[activeRadio]->valid) {
                i2=1;
            } else {
                i2=0;
            }
        } else {
            if (qso[activeRadio]->valid && !qso[activeRadio]->dupe) {
                i2=1;
            } else {
                i2=0;
            }
        }
    } else {
        // S&P mode
        i3 = 1;
        // always check dupe status in S&P. NO_DUPE_CHECKING setting is
        // enforced in so2sdr.cpp where qso[activeRadio]->dupe is set
        if (qso[activeRadio]->valid && !qso[activeRadio]->dupe) {
            i2=1;
        } else {
            i2=0;
        }
    }

    // ctrl+Enter logs without dupecheck or exchange validation
    if (mod == Qt::ControlModifier) {
        i2 = 1;
        // so only one enter press will log
        exchangeSent[activeRadio] = true;
        // set qso number in case exchange never sent
        if (nrReserved[activeRadio]==0) {
            if (nrSent!=nrReserved[activeRadio ^ 1]) {
                nrReserved[activeRadio]=nrSent;
            } else {
                nrReserved[activeRadio]=nrSent+1;
            }
        }
    }
    if (exchangeSent[activeRadio]) {
        i4=1;
    } else {
        i4=0;
    }
    // change radios
    /*!
       @todo make actions below separate functions; some are duplicated elsewhere (backslash,...)
     */
    if (enterState[i1][i2][i3][i4] & 16384) {
        if (activeR2CQ) {
            lineEditCall[activeRadio]->clear();
            qso[activeRadio]->prefill.clear();
            origCallEntered[activeRadio].clear();
            switchRadios();

            // remove any band spot that is present
            removeSpotFreq(cat[activeRadio]->getRigFreq(), band[activeRadio]);

            // copy call to other radio
            qso[activeRadio]->call = qso[activeRadio ^ 1]->call;
            lineEditCall[activeRadio]->setText(qso[activeRadio]->call);
            qso[activeRadio ^ 1]->call.clear();
            updateNrDisplay();

            // redo prefix check on other radio. This will fill in freq, band, etc.
            prefixCheck(activeRadio, qso[activeRadio]->call);

            clearWorked(activeRadio ^ 1);
            clearR2CQ(activeRadio);

            cqMode[activeRadio]       = true;
            excMode[activeRadio]      = false;
            exchangeSent[activeRadio] = false;
            autoSendPause = true;
        }
    }

    // send callsign
    if (enterState[i1][i2][i3][i4] & 1) {
        if (mod != Qt::ShiftModifier &&
            !qso[activeRadio]->dupe) {
            // send AGN if call already sent and exchange invalid
            if (callSent[activeRadio] && lineEditExchange[activeRadio]->hasFocus()) {
                switch (cat[activeTxRadio]->modeType()) {
                case CWType:case DigiType:
                    expandMacro(cwMessage->cqShiftF[1].toUpper());
                    break;
                case PhoneType:
                    break;
                }
            } else {
                switch (cat[activeTxRadio]->modeType()) {
                case CWType:case DigiType:
                    expandMacro(settings->value(s_call,s_call_def).toByteArray());
                    break;
                case PhoneType:
                    expandMacro(csettings->value(c_call_msg,c_call_msg_def).toByteArray());
                    break;
                }
                callSent[activeRadio] = true;
            }
        }
    }

    // send CQ exchange
    if (enterState[i1][i2][i3][i4] & 2) {
        int m=(int)cat[activeTxRadio]->modeType();
        if (lineEditCall[activeRadio]->text().contains("?") && (m == CWType || m == DigiType)) {
            expandMacro("{CALL_ENTERED}");
            if (toggleMode) { // steer focus to opposite radio
                switchRadios(false);
            }
            return; // prevent further processing
        } else {
            if (autoSend && !autoSendPause &&
                    (settings->value(s_settings_autosend_mode,s_settings_autosend_mode_def).toInt() == 0
                       || csettings->value(c_sprintmode,c_sprintmode_def).toBool() )
                ) { // semi-auto
                autoSendTrigger = true;
            } else {
                // check to see if this qso number already taken
                if (nrSent == nrReserved[activeRadio ^ 1]) {
                    nrReserved[activeRadio] = nrSent + 1;
                } else {
                    nrReserved[activeRadio] = nrSent;
                }
                if (mod != Qt::ShiftModifier) {
                    if (qso[activeRadio]->dupe && csettings->value(c_dupemode,c_dupemode_def).toInt() == STRICT_DUPES) {
                        expandMacro(csettings->value(c_dupe_msg[m],c_dupe_msg_def[m]).toByteArray());
                    } else {
                        expandMacro(csettings->value(c_cq_exc[m],c_cq_exc_def[m]).toByteArray());
                    }
                }
                exchangeSent[activeRadio]    = true;
                callSent[activeRadio]        = true; // set this true as well in case mode switched to S&P
                origCallEntered[activeRadio] = qso[activeRadio]->call;
                updateNrDisplay();
                cqQsoInProgress[activeRadio] = true;
            }
        }
    }

    // set exc mode
    if (enterState[i1][i2][i3][i4] & 1024) {
        excMode[activeRadio] = true;
        lineEditExchange[activeRadio]->show();
    }

    // set initial exchange
    if (enterState[i1][i2][i3][i4] & 8192) {
        prefillExch(activeRadio);
    }

    // focus exchange
    if ((enterState[i1][i2][i3][i4] & 4) && (!autoSendTrigger || autoSendPause) 
        && (!qso[activeRadio]->dupe || csettings->value(c_dupemode,c_dupemode_def).toInt() != STRICT_DUPES) 
    ) {
        callFocus[activeRadio] = false;
        setEntryFocus();
        if (lineEditExchange[activeRadio]->text().simplified().isEmpty()) {
            lineEditExchange[activeRadio]->clear();
        } else {
            if (contest->contestType()!=Sweepstakes_t) {
                lineEditExchange[activeRadio]->setText(lineEditExchange[activeRadio]->text().simplified() + " ");
            }
        }
    }

    // send SP exch
    if (enterState[i1][i2][i3][i4] & 16) {
        // check to see if this qso number already taken
        if (nrSent == nrReserved[activeRadio ^ 1]) {
            nrReserved[activeRadio] = nrSent + 1;
        } else {
            nrReserved[activeRadio] = nrSent;
        }
        updateNrDisplay();
        if (mod != Qt::ShiftModifier) {
            int m=(int)cat[activeTxRadio]->modeType();
            expandMacro(csettings->value(c_sp_exc[m],c_sp_exc_def[m]).toByteArray());
        }
        exchangeSent[activeRadio] = true;
    }

    // log qso
    if (enterState[i1][i2][i3][i4] & 8) {
        clearLogSearch();
        fillSentExch(activeRadio);
        qso[activeRadio]->mode = cat[activeRadio]->mode();
        contest->addQso(qso[activeRadio]);
        qso[activeRadio]->time = QDateTime::currentDateTimeUtc(); // update time just before logging qso
        addQso(qso[activeRadio]);
        if (!qso[activeRadio]->dupe && qso[activeRadio]->valid) {
            nqso[band[activeRadio]]++;
        }
        updateBreakdown();
        updateOffTime();
        qso[activeRadio]->dupe = true;
        if (!cqMode[activeRadio]) {
            // add to bandmap if in S&P mode
            qso[activeRadio]->freq = rigFreq[activeRadio];
            addSpot(qso[activeRadio]->call, qso[activeRadio]->freq, true);
            spotListPopUp[activeRadio]=true; // this prevents the call from immediately popping up
        } else {
            // remove any spot on this freq, update other spots if in CQ mode
            qso[activeRadio]->freq = rigFreq[activeRadio];
            removeSpotFreq(qso[activeRadio]->freq, band[activeRadio]);
            updateBandmapDupes(qso[activeRadio]);
        }
        updateDupesheet(qso[activeRadio]->call,activeRadio);
        updateMults(activeRadio);
        rateCount[ratePtr]++;
        exchangeSent[activeRadio] = false;
        if (csettings->value(c_sprintmode,c_sprintmode_def).toBool()) {
            sprintMode();
        }
        labelCountry[activeRadio]->clear();
        labelBearing[activeRadio]->clear();
        labelLPBearing[activeRadio]->clear();
        sunLabelPtr[activeRadio]->clear();
        qso[activeRadio]->prefill.clear();
        qso[activeRadio]->zone=0;

        // advance qso numbers
        nrReserved[activeRadio] = 0;
        nrSent++;
        updateNrDisplay();
        cqQsoInProgress[activeRadio] = false;
        if (altDActive && altDActiveRadio == activeRadio) {
            // if doing an alt-D SO2R qso, reset counter
            altDActive = 0;
        }
        qso[activeRadio]->dupe = false;
        qso[activeRadio]->valid = false;
        callSent[activeRadio]  = false;
        if (mod != Qt::ShiftModifier) { // don't unpause autoCQ or dueling CQ if silently logged
            if (autoCQMode) {
                autoCQModePause = false;
                autoCQModeWait = true;
            }
            if (duelingCQMode) {
                duelingCQWait = true; // prevent premature dueling CQ takeover until QSL message is complete
                duelingCQModePause = false;
            }
        }
    }

    // focus call field
    if (enterState[i1][i2][i3][i4] & 32 || lineEditCall[activeRadio]->text().contains("?")) {
        callFocus[activeRadio] = true;
        setEntryFocus();
    }

    // clear exch field
    if (enterState[i1][i2][i3][i4] & 64) {
        lineEditExchange[activeRadio]->clear();
        lineEditExchange[activeRadio]->setModified(false);
        qso[activeRadio]->exch.clear();
        qso[activeRadio]->prefill.clear();
        validLabel[activeRadio]->clear();
    }

    // send F1 message
    if (enterState[i1][i2][i3][i4] & 256) {
        if (mod != Qt::ShiftModifier) {
            if (activeR2CQ && csettings->value(c_sprintmode,c_sprintmode_def).toBool()) { // sprint 2nd radio CQ
                    expandMacro("{R2CQ}" + cwMessage->cqF[0]);
            } else {
                switch (cat[activeTxRadio]->modeType()) {
                case CWType:case DigiType:
                    if ((sendLongCQ && (toggleMode || autoCQMode)) || !(toggleMode || autoCQMode)) {
                        expandMacro(cwMessage->cqF[0]);
                    } else {
                        expandMacro(cwMessage->cqF[1]);
                    }
                    break;
                case PhoneType:
                    if ((sendLongCQ && (toggleMode || autoCQMode)) || !(toggleMode || autoCQMode)) {
                        expandMacro(ssbMessage->cqF[0]);
                    } else {
                        expandMacro(ssbMessage->cqF[1]);
                    }
                    break;
                }

            }
            exchangeSent[activeRadio] = false;
            if (autoCQMode) {
                autoCQModePause = false;
                autoCQModeWait = true;
            }
            if (duelingCQMode) {
                duelingCQModePause = false;
                duelingCQWait = true;
            }
        }
    }

    // send QSL msg
    if (enterState[i1][i2][i3][i4] & 512) {
        // see if call was corrected
        // if control+Enter, do not send any CW here.
        int m=(int)cat[activeTxRadio]->modeType();
        if (qso[activeRadio]->call != origCallEntered[activeRadio]) {
            if (mod != Qt::ShiftModifier) expandMacro(csettings->value(c_qsl_msg_updated[m],c_qsl_msg_updated_def[m]).toByteArray());
        } else {
            if (mod != Qt::ShiftModifier) expandMacro(csettings->value(c_qsl_msg[m],c_qsl_msg_def[m]).toByteArray());
        }
        exchangeSent[activeRadio] = false;
    }

    // clear call field
    if (enterState[i1][i2][i3][i4] & 128) {
        lineEditCall[activeRadio]->clear();
        lineEditCall[activeRadio]->setModified(false);
        qso[activeRadio]->call.clear();
        exchangeSent[activeRadio] = false;
        qso[activeRadio]->prefill.clear();
        origCallEntered[activeRadio].clear();
        clearWorked(activeRadio);
        statusBarDupe         = false;
        callSent[activeRadio] = false;
        if (autoSend) {
            autoSendPause = false;
            autoSendTrigger = false;
            tmpCall.clear();
        }
    }

    // exit exc mode
    if (enterState[i1][i2][i3][i4] & 2048) {
        excMode[activeRadio] = false;
        if (cqMode[activeRadio]) {
            lineEditExchange[activeRadio]->hide();
        }
    }

    // clear radio 2 cq
    if (enterState[i1][i2][i3][i4] & 32768) {
        if (!csettings->value(c_sprintmode,c_sprintmode_def).toBool()) { // not sprint
            clearR2CQ(activeRadio ^ 1);
        }
    }

    // check for new call entered in exchange line
    if (enterState[i1][i2][i3][i4] & 4096) {
        QByteArray tmp;
        if (contest->newCall(tmp)) {
            lineEditCall[activeRadio]->setText(tmp);
            prefixCheck(activeRadio, tmp);
        }
    }

    // send shift-F2 (AGN?) message
    if (enterState[i1][i2][i3][i4] & 65536) {
        switch (cat[activeTxRadio]->modeType()) {
        case CWType:case DigiType:
            expandMacro(cwMessage->cqShiftF[1].toUpper());
            break;
        case PhoneType:
            break;
        }
    }

    if (toggleMode) { // steer focus to opposite radio
        switchRadios(false);
    }
}

void So2sdr::prefillExch(int nr)
{
    if (!qso[nr]->prefill.isEmpty()) {
        // prefill from log
        lineEditExchange[nr]->setText(qso[nr]->prefill.trimmed());
        // in ARRL Sweepstakes, put cursor in correct position for qso #
        if (contest->contestType()==Sweepstakes_t) {
            lineEditExchange[nr]->setCursorPosition(0);
        }
    } else if (contest->hasPrefill() && !contest->prefillExchange(qso[nr]).isEmpty()) {
        lineEditExchange[nr]->setText(contest->prefillExchange(qso[nr]));
    }
    exchCheck(nr,lineEditExchange[nr]->text());
}

/*!
   ESC key

   <ol>
   <li> if sending CW, stop CW and do nothing else
   <li> otherwise effect depends on
   </ol>

   <ol>
   <li> is there text in call field  (size!=0)    (0=no, 1=yes)
   <li> is there text in exchange field (size!=0) (0=no, 1=yes)
   <li> focus in Call or Exchange field              (0=call 1=exch)
   <li> CQ or S&P mode                               (0=cq 1=sp)
   </ol>

   --> [2][2][2][2] matrix of chars (16 states)
     a  b  c  d

   actions:
   <ol>
   <li> clear call field                0=1  (also unsets isModified)
   <li> clear exchange field            1=2  (also unsets isModified)
   <li> exit S&P mode                   2=4
   <li> return focus to call field      3=8
   <li> return focus to exch field      4=16
   <li> exit Exc mode                   5=32
   <li> set ExchangeSent false          6=64
   <li> reset winkeyDialog output port        7=128
   <li> clear alt-D calls               8=256
   <li> clear radio2 cq status          9=512
   <li> reset alt-D without deleting call 10=1024
   <li> clear log search                11=2048
   <li> pause autoCQ                    12=4096
   </ol>
 */
void So2sdr::esc(Qt::KeyboardModifiers mod)
{
    int                 i1;
    int                 i2;
    int                 i3;
    int                 i4;
    static unsigned int escState[2][2][2][2];
    unsigned int        x;
    static bool         first     = true;

    ModeTypes mode=cat[activeTxRadio]->modeType();

    // define states
    if (first) {
        // RTC: 128 not needed here (?), leads to multiple cancelCw being called
        escState[0][0][0][0] = 1 + 2 + 8 + 32 + 64 + 256 + 512 + 2048;//+128
        escState[1][0][0][0] = 1 + 2 + 8 + 32 + 64 + 1024 + 2048;
        escState[0][1][0][0] = 1 + 2 + 8 + 32 + 64 + 1024;
        escState[1][1][0][0] = 1 + 2 + 8 + 32 + 64 + 1024;
        escState[0][0][1][0] = 8;
        escState[1][0][1][0] = 1 + 8 + 32 + 64;
        escState[0][1][1][0] = 1 + 2 + 8 + 32 + 64;
        escState[1][1][1][0] = 2;
        escState[0][0][0][1] = 1 + 4 + 64 + 2048;
        escState[1][0][0][1] = 1 + 2 + 64 + 1024 + 2048;
        escState[0][1][0][1] = 1 + 2 + 8 + 64; // 32
        escState[1][1][0][1] = 1 + 2 + 8 + 64 + 256;// 1024;
        escState[0][0][1][1] = 4 + 8 + 64;
        escState[1][0][1][1] = 1 + 8 + 64 + 1024;
        escState[0][1][1][1] = 2 + 8 + 64 + 1024;
        escState[1][1][1][1] = 2 + 1024;
        first                = false;
    }
    qso[activeRadio]->exch = lineEditExchange[activeRadio]->text().toLatin1();
    qso[activeRadio]->exch = qso[activeRadio]->exch.trimmed();

    // if sending CW or audio, stop
    if (mod != Qt::AltModifier) {
        switch (mode) {
        case CWType:
        {
            if (winkey->isSending()) {
                winkey->cancelcw();
                // de-activate dueling-CQ
                if (duelingCQMode) duelingCQModePause = true;
                // de-activate auto-CQ if call field empty on auto-CQ radio
                if (autoCQMode &&
                        (
                            (!altDActive && lineEditCall[activeRadio]->text().isEmpty())
                            || (altDActive && lineEditCall[altDActiveRadio ^ 1]->text().isEmpty())
                            )
                        ) {
                    autoCQModePause = true;
                }
                if (toggleMode && !duelingCQMode) toggleStatus->setText("<font color=#0000FF>TOGGLE</font>");
                if (autoSend && !lineEditCall[activeRadio]->text().isEmpty()) {
                    autoSendPause = true;
                }
                return;
            }
            break;
        }
        case PhoneType:
        {
            if (csettings->value(c_ssb_cancel,c_ssb_cancel_def).toByteArray().isEmpty()) {
                ssbMessage->cancelMessage();
            } else {
                expandMacro(csettings->value(c_ssb_cancel,c_ssb_cancel_def).toByteArray());
            }
            break;
        }
        case DigiType:
            break;
        }
    }
    // ESC needs to return focus to call field if it
    // screwed up by the mouse
    if (callFocus[activeRadio] && !lineEditCall[activeRadio]->hasFocus()) {
        setEntryFocus();
    }

    QByteArray call = lineEditCall[activeRadio]->text().toLatin1().toUpper();
    if (!call.isEmpty()) {
        i1 = 1;
    } else {
        i1 = 0;
    }
    if (!qso[activeRadio]->exch.simplified().isEmpty()) {
        i2 = 1;
    } else {
        i2 = 0;
    }
    i3 = callFocus[activeRadio] ^ 1;
    i4 = cqMode[activeRadio] ^ 1;
    x  = escState[i1][i2][i3][i4];

    // reset alt-d state
    if (x & 1024) {
        if (altDActive && altDActive != 2 && activeRadio == altDActiveRadio) {
            // set state ready for space to call station on 2nd radio
            altDActive = 2;

            // hide exchange line
            lineEditExchange[activeRadio]->hide();
            validLabel[activeRadio]->clear();

            // switch back to radio 1
            callFocus[altDActiveRadio] = true;
            switchRadios(false);
            callSent[altDActiveRadio] = false;
            return;

        } else if (altDActive == 2 && activeRadio == altDActiveRadio) {
            QPalette palette(lineEditCall[altDActiveRadio]->palette());
            palette.setColor(QPalette::Base, CQ_COLOR);
            lineEditCall[altDActiveRadio]->setPalette(palette);
            lineEditCall[altDActiveRadio]->clear();
            lineEditExchange[altDActiveRadio]->setPalette(palette);
            lineEditExchange[altDActiveRadio]->clear();
            labelCountry[altDActiveRadio]->clear();
            labelBearing[altDActiveRadio]->clear();
            labelLPBearing[altDActiveRadio]->clear();
            sunLabelPtr[altDActiveRadio]->clear();
            clearWorked(altDActiveRadio);
            qso[altDActiveRadio]->call.clear();
            altDActive = 0;
            callSent[altDActiveRadio] = false;
            spotListPopUp[altDActiveRadio]=false;
        }
    }

    // clear call field
    if (x & 1) {
        lineEditCall[activeRadio]->clear();
        lineEditCall[activeRadio]->setModified(false);
        labelCountry[activeRadio]->clear();
        labelBearing[activeRadio]->clear();
        labelLPBearing[activeRadio]->clear();
        sunLabelPtr[activeRadio]->clear();
        lineEditCall[activeRadio]->setCursorPosition(0);
        qso[activeRadio]->call.clear();
        qso[activeRadio]->valid=false;
        So2sdrStatusBar->clearMessage();
        clearWorked(activeRadio);
        origCallEntered[activeRadio].clear();
        nrReserved[activeRadio] = 0;
        updateNrDisplay();
        MasterTextEdit->clear();
        statusBarDupe         = false;
        callSent[activeRadio] = false;
        if (autoSend) {
            autoSendPause = false;
            autoSendTrigger = false;
            tmpCall.clear();
        }
        for (int ii = 0; ii < MMAX; ii++) {
            qso[activeRadio]->mult[ii]=-1;
        }
        qso[activeRadio]->worked=0;
    }

    // clear exchange field
    if (x & 2) {
        lineEditExchange[activeRadio]->clear();
        lineEditExchange[activeRadio]->setModified(false);
        lineEditExchange[activeRadio]->setCursorPosition(0);
        qso[activeRadio]->prefill.clear();
        qso[activeRadio]->exch.clear();
        qso[activeRadio]->valid=false;
        nrReserved[activeRadio] = 0;
        validLabel[activeRadio]->clear();
    }

    // exit S&P, set CQ
    if (x & 4) {
        setCqMode(activeRadio);
        callSent[activeRadio] = false;
    }

    // return focus to call
    if (x & 8) {
        callFocus[activeRadio] = true;
        setEntryFocus();
    }

    // return focus to exch
    if (x & 16) {
        callFocus[activeRadio] = false;
        setEntryFocus();
    }

    // exit exchange mode
    if (x & 32) {
        excMode[activeRadio] = false;
        lineEditExchange[activeRadio]->hide();
        cqQsoInProgress[activeRadio] = false;

        // callFocus[activeRadio]=false;
    }

    // exch sent=false
    if (x & 64) {
        exchangeSent[activeRadio] = false;
    }

    // reset winkey output port // transmit focus
    if (x & 128) {
        switchTransmit(activeRadio);
    }

    // clear radio2 cq status
    if (x & 512) {
        clearR2CQ(activeRadio ^ 1);
    }

    // clear alt-d state
    if ((x & 256) && !(autoCQMode && !autoCQModePause)) {
        if (altDActive) {
            QPalette palette(lineEditCall[altDActiveRadio]->palette());
            palette.setColor(QPalette::Base, CQ_COLOR);
            lineEditCall[altDActiveRadio]->setPalette(palette);
            lineEditCall[altDActiveRadio]->clear();
            lineEditExchange[altDActiveRadio]->setPalette(palette);
            lineEditExchange[altDActiveRadio]->clear();
            labelCountry[altDActiveRadio]->clear();
            labelBearing[altDActiveRadio]->clear();
            labelLPBearing[altDActiveRadio]->clear();
            sunLabelPtr[altDActiveRadio]->clear();
            clearWorked(altDActiveRadio);
            qso[altDActiveRadio]->call.clear();
            if (activeRadio == altDActiveRadio) {
                switchRadios();
            }
            altDActive = 0;
            if (altDOrigMode == 0) {
                setCqMode(altDActiveRadio);
            } else {
                spMode(altDActiveRadio);
            }
            callSent[altDActiveRadio] = false;
            spotListPopUp[altDActiveRadio]=false;
        }
    }
    // clear log search
    if (x & 2048) {
        clearLogSearch();
        LogTableView->clearSelection();
    }

    // pause autoCQ or dueling CQ unconditionally
    if (autoCQMode) autoCQModePause = true;
    if (duelingCQMode) duelingCQModePause = true;

}

/*!
   Function keys: choose either regular or Exchange/S&P
 */
void So2sdr::sendFunc(int i,Qt::KeyboardModifiers mod)
{
    // clear dupe message if present
    if (statusBarDupe) {
        So2sdrStatusBar->clearMessage();
    }

    if (toggleMode) {
        switchTransmit(activeRadio); // steer transmit to focused radio
    }
    if (autoCQMode && !excMode[autoCQRadio] && autoCQRadio == activeRadio){
        if (i == 0 || i == 1) {
            if (altDActive < 3) { // F1/F2 CQ unpauses AutoCQ
                autoCQModePause = false;
                autoCQModeWait = true;
            }
        } else {
            autoCQModePause = true;
        }
    }

    if (duelingCQMode && !excMode[activeRadio]) {
        if (i == 0 || i == 1) {
            duelingCQModePause = false; // F1/F2 CQ unpauses dueling CQ
        } else {
            duelingCQModePause = true;
        }
        duelingCQWait = true; // prevent switching hysteresis
    }


    ModeTypes mode=cat[activeRadio]->modeType();
    int m=(int)mode;

    // in phone check for both Control+Shift to record a message
    if (mode==PhoneType) {
        if ((mod & Qt::ShiftModifier) && (mod & Qt::ControlModifier)) {
            if (cqMode[activeRadio]) {
                expandMacro(ssbMessage->cqRecF[i].toUpper());
            } else {
                expandMacro(ssbMessage->excRecF[i].toUpper());
            }
            return;
        }
    }

    switch (mod) {
    case Qt::NoModifier:
        if (i == 1 && csettings->value(c_sprintmode,c_sprintmode_def).toBool() && cqQsoInProgress[activeRadio]) {
            /*!
               sprint: if CQ qso is in progress, continue to send CQ Exch when F2
               pressed. Must be cleaner way to handle this?
             */
            switch (mode) {
            case CWType:case DigiType:
                expandMacro(csettings->value(c_cq_exc[m],c_cq_exc_def[m]).toByteArray());
                break;
            case PhoneType:
                expandMacro(csettings->value(c_cq_exc[m],c_cq_exc_def[m]).toByteArray());
                break;
            }

        } else if (cqMode[activeRadio] && !excMode[activeRadio]) {
            /*!
               reset 2nd radio color change if needed
             */
            clearR2CQ(activeRadio ^ 1);
            switch (mode) {
            case CWType:case DigiType:
                expandMacro(cwMessage->cqF[i].toUpper());
                break;
            case PhoneType:
                expandMacro(ssbMessage->cqF[i].toUpper());
                break;
            }
        } else {
            /*!
               assume exchange is in F2; sending exchange will reserve this
               qso number
             */
            if (i == 1 && !cqMode[activeRadio] && !exchangeSent[activeRadio]) {
                exchangeSent[activeRadio] = true;
                if (nrSent != nrReserved[activeRadio ^ 1]) {
                    nrReserved[activeRadio] = nrSent;
                } else {
                    nrReserved[activeRadio] = nrSent + 1;
                }
                updateNrDisplay();
            }
            /*!
               assume callsign in in F1; set flag to keep enter status correct
             */
            if (i == 0 && !cqMode[activeRadio]) {
                callSent[activeRadio] = true;
            }
            switch (mode) {
            case CWType:case DigiType:
                expandMacro(cwMessage->excF[i].toUpper());
                break;
            case PhoneType:
                expandMacro(ssbMessage->excF[i].toUpper());
                break;
            }
        }
        break;
    case Qt::ControlModifier:
        switch (mode) {
        case CWType:case DigiType:
            expandMacro(cwMessage->cqCtrlF[i].toUpper());
            break;
        case PhoneType:
            // no Ctrl-func macros for SSB
            break;
        }
        break;
    case Qt::ShiftModifier:
        switch (mode) {
        case CWType:case DigiType:
            expandMacro(cwMessage->cqShiftF[i].toUpper());
            break;
        case PhoneType:
            // no Shift-func macros for SSB
            break;
        }
        break;
    }
    if (toggleMode) { // steer focus to opposite radio
        switchRadios(false);
    }
}
