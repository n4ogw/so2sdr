/*! Copyright 2010-2012 R. Torsten Clay N4OGW

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

// /// Key handling routines

/*! event filter handling key presses. This gets installed in
   -main window
   -both call entry windows
   -both exchange entry windows
   -bandmap
   -dupesheet
 */
bool So2sdr::eventFilter(QObject*, QEvent* e)
{
    if (!uiEnabled) return(false);

    // set r true if this completely handles the key. Otherwise
    // it will be passed on to other widgets
    bool r   = false;
    int  mod = 0;

    switch (e->type()) {
    case QEvent::MouseButtonPress:

        // if call line edit clicked in, switch to that radio
        if (lineEditCall[0]->underMouse() && activeRadio == 1) {
            switchRadios(false);
            return(true);
        }
        if (lineEditCall[1]->underMouse() && activeRadio == 0) {
            switchRadios(false);
            return(true);
        }

        // if exchange line edit clicked, switch to that radio
        // and focus exchange
        if (lineEditExchange[0]->underMouse() && activeRadio == 1) {
            switchRadios(false);
            lineEditExchange[0]->setFocus();
            return(true);
        }
        if (lineEditExchange[1]->underMouse() && activeRadio == 0) {
            switchRadios(false);
            lineEditExchange[1]->setFocus();
            return(true);
        }
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

        int      m = kev->modifiers();
        if (m == Qt::ControlModifier) {
            mod = 1;
        } else if (m == Qt::ShiftModifier) {
            mod = 2;
        } else if (m == Qt::AltModifier) {
            mod = 3;
        }
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
            if (station) {
                if (station->isActiveWindow()) {
                    station->rejectChanges();
                    return(r);
                }
            }

            esc();
            r = true;
            break;
        case Qt::Key_C:     // alt-C : bring up config menu
            if (mod == 3) {
                menubar->setActiveAction(menubar->actions()[1]);
                return(true);
            }
            break;
        case Qt::Key_D:     // alt-D
            if (mod == 3) {
                altd();
                r = true;
            }
            break;
        case Qt::Key_F:
            // alt-F : bring up file menu
            if (mod == 3) {
                menubar->setActiveAction(menubar->actions()[0]);
                return(true);
            } else if (mod == 1) {
                // ctrl-F : log search
                logSearch();
                return(true);
            }
            break;
        case Qt::Key_H:     // alt-H : bring up Help menu
            if (mod == 3) {
                menubar->setActiveAction(menubar->actions()[3]);
                return(true);
            }
            break;
        case Qt::Key_M:     // alt-M
            if (mod == 3) {
                switchMultMode();
                r = true;
            }
            break;
        case Qt::Key_N:     // ctrl-N
            if (mod == 1) {
                writeNote();
                r = true;
            }
            break;
        case Qt::Key_R:     // alt-R
            if (mod == 3) {
                switchRadios();
                r = true;
            }
            break;
        case Qt::Key_S:     // alt-S
            if (mod == 3) {
                launch_WPMDialog();
                r = true;
            }
            break;
        case Qt::Key_W:     // alt-W : bring up Windows menu
            if (mod == 3) {
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
            }
            break;
        case Qt::Key_Backslash:
            backSlash();
            r = true;
            break;
        case Qt::Key_Up:
            if (mod == 1) {   // ctrl-up
                keyCtrlUp();
            } else {
                up();
            }
            r = true;
            break;
        case Qt::Key_Down:
            if (mod == 1) {   // ctrl-down
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
            if (altDActive && !csettings->value(c_sprintmode,c_sprintmode_def).toBool()) {
                altDEnter(altDActive, mod);
            } else {
                enter(mod);
            }
            r = true;
            break;
        case Qt::Key_PageUp:
            launch_speedUp(mod);
            r = true;
            break;
        case Qt::Key_PageDown:
            launch_speedDn(mod);
            r = true;
            break;
        case Qt::Key_F1:
            sendFunc(0, mod);
            r = true;
            break;
        case Qt::Key_F2:
            sendFunc(1, mod);
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
            markDupe(activeRadio);
            r = true;
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

/*! Toggles current freq on bandmap as a dupe, placing "*" as callsign

   radio2=true : marks on inactive radio bandmap
        =false: marks on active radio bandmap

 */
void So2sdr::markDupe(int nrig)
{
    int f = cat->getRigFreq(nrig);
    if (isaSpot(f, band[nrig])) {
        removeSpotFreq(f, band[nrig]);
    } else {
        addSpot("*", cat->getRigFreq(nrig), true);
    }
}

/*!
   alt+d : dupe check on other radio

   Mutually exclusive with Sprint Mode
 */
void So2sdr::altd()
{
    // nothing if sequence already active; not available with sprint so2r
    if (altDActive || csettings->value(c_sprintmode,c_sprintmode_def).toBool()) return;

    // switch radios, but don't switch/stop cw
    switchRadios(false);
    altDActive = 1;
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

    if (callFocus[activeRadio]) {
        callFocus[activeRadio] = false;
        lineEditExchange[activeRadio]->setFocus();
        if (grab) {
            lineEditExchange[activeRadio]->grabKeyboard();
            grabWidget = lineEditExchange[activeRadio];
        }
    } else {
        callFocus[activeRadio] = true;
        lineEditCall[activeRadio]->setFocus();
        if (grab) {
            lineEditCall[activeRadio]->grabKeyboard();
            grabWidget = lineEditCall[activeRadio];
        }
    }
}

/*! control+pageup : tune to next higher signal on bandmap

   If activeRadio is in S&P mode, tune that radio. Otherwise tune the 2nd radio
 */
void So2sdr::keyCtrlUp()
{
    int nr = activeRadio;
    if (cqMode[activeRadio]) {
        nr = nr ^ 1;
    }
    if (bandmapOn[nr]) {
        int f = bandmap[nr]->nextFreq(true);
        if (f) qsy(nr, f, true);
    }
}


/*!
   down arrow key
 */
void So2sdr::down()
{
    // if in CQ mode and only call window showing
    if (cqMode[activeRadio] && !excMode[activeRadio]) return;

    if (callFocus[activeRadio]) {
        callFocus[activeRadio] = false;
        lineEditExchange[activeRadio]->setFocus();
        if (grab) {
            lineEditExchange[activeRadio]->grabKeyboard();
        }
        grabWidget = lineEditExchange[activeRadio];
    }   else    {
        callFocus[activeRadio] = true;
        lineEditCall[activeRadio]->setFocus();
        if (grab) {
            lineEditCall[activeRadio]->grabKeyboard();
        }
        grabWidget = lineEditCall[activeRadio];
    }
}

/*! control+pagedown : tune to next lower signal on bandmap

   If activeRadio is in S&P mode, tune that radio. Otherwise tune the 2nd radio
 */
void So2sdr::keyCtrlDn()
{
    int nr = activeRadio;
    if (cqMode[activeRadio]) {
        nr = nr ^ 1;
    }
    if (bandmapOn[nr]) {
        int f = bandmap[nr]->nextFreq(false);
        if (f) qsy(nr, f, true);
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
        spMode(activeRadio);
    } else {
        if (callFocus[activeRadio]) {
            callFocus[activeRadio] = false;
            lineEditExchange[activeRadio]->setFocus();
            if (grab) {
                lineEditExchange[activeRadio]->grabKeyboard();
            }
            grabWidget = lineEditExchange[activeRadio];
        } else {
            callFocus[activeRadio] = true;
            lineEditCall[activeRadio]->setFocus();
            if (grab) {
                lineEditCall[activeRadio]->grabKeyboard();
            }
            grabWidget = lineEditCall[activeRadio];
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
    qso[activeRadio]->exch = lineEditExchange[activeRadio]->text().toAscii();
    qso[activeRadio]->exch = qso[activeRadio]->exch.trimmed();
    if (!qso[activeRadio]->call.isEmpty() &&
        (qso[activeRadio]->valid=contest->validateExchange(qso[activeRadio])) &&
        !(qso[activeRadio]->dupe && csettings->value(c_dupemode,c_dupemode_def).toInt() == 0) &&
        cqMode[activeRadio] &&
        exchangeSent[activeRadio])
    {
        expandMacro(csettings->value(c_qqsl_msg,c_qqsl_msg_def).toByteArray());

        // log qso
        if (!qso[activeRadio]->dupe && qso[activeRadio]->valid)
            nqso[band[activeRadio]]++;

        updateBreakdown();

        if (!cqMode[activeRadio]) {
            // add to bandmap if in S&P mode
            qso[activeRadio]->freq = rigFreq[activeRadio];
            addSpot(qso[activeRadio]->call, qso[activeRadio]->freq, true);
        } else {
            // remove any spot that is on freq, update other spots if in CQ mode
            qso[activeRadio]->freq = rigFreq[activeRadio];
            removeSpotFreq(qso[activeRadio]->freq, band[activeRadio]);
            updateBandmapDupes(qso[activeRadio]);
        }
        qso[activeRadio]->mode = cat->mode(activeRadio);
        fillSentExch(activeRadio);
        contest->addQso(qso[activeRadio]);
        qso[activeRadio]->time = QDateTime::currentDateTimeUtc(); // update time just before logging qso
        addQso(qso[activeRadio]);
        updateDupesheet(qso[activeRadio]->call);

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

        // advance qso numbers
        nrReserved[activeRadio] = 0;
        nrSent++;
        updateNrDisplay();
        cqQsoInProgress[activeRadio] = false;

        // focus call field
        lineEditCall[activeRadio]->setFocus();
        if (grab) {
            lineEditCall[activeRadio]->grabKeyboard();
        }
        grabWidget             = lineEditCall[activeRadio];
        callFocus[activeRadio] = true;

        // clear exchange field
        lineEditExchange[activeRadio]->clear();
        lineEditExchange[activeRadio]->setModified(false);
        qso[activeRadio]->exch.clear();
        qso[activeRadio]->prefill.clear();

        // clear call field
        lineEditCall[activeRadio]->clear();
        lineEditCall[activeRadio]->setModified(false);
        qso[activeRadio]->call.clear();
        exchangeSent[activeRadio] = false;
        qso[activeRadio]->prefill.clear();
        origCallEntered[activeRadio].clear();
        clearWorked(activeRadio);

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
    if (csettings->value(c_sprintmode,c_sprintmode_def).toBool() && cqMode[activeRadio]) {
        // Sprint. If in CQ mode, space switches to S&P and sends call
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
    // if already in SP mode, do nothing
    if (cqMode[activeRadio] == false) return;
    if (callFocus[activeRadio]) {
        spMode(activeRadio);
        lineEditCall[activeRadio]->setModified(false);
        lineEditCall[activeRadio]->setFocus();
        if (grab) {
            lineEditCall[activeRadio]->grabKeyboard();
        }
        grabWidget = lineEditCall[activeRadio];
        clearR2CQ(activeRadio ^ 1);
        if (!lineEditCall[activeRadio]->text().isEmpty()) {
            if (!dupeCheckDone) {
                qso[activeRadio]->dupe = mylog->isDupe(qso[activeRadio], contest->dupeCheckingByBand(), true) &&
                                         csettings->value(c_dupemode,c_dupemode_def).toInt() < NO_DUPE_CHECKING;
            }

            // update displays
            updateWorkedDisplay(activeRadio,qso[activeRadio]->worked);
            if (!qso[activeRadio]->dupe) {
                prefillExch(activeRadio);
                expandMacro(settings->value(s_call,s_call_def).toByteArray());
                callSent[activeRadio] = true;
                lineEditExchange[activeRadio]->setFocus();
                if (grab) {
                    lineEditExchange[activeRadio]->grabKeyboard();
                }
                grabWidget             = lineEditExchange[activeRadio];
                callFocus[activeRadio] = false;
                excMode[activeRadio]   = true;
            }
        } else {
            expandMacro(settings->value(s_call,s_call_def).toByteArray());
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
    switchRadios();
    spMode(activeRadio);
    lineEditExchange[activeRadio]->setFocus();
    callFocus[activeRadio] = false;
    if (grab) {
        lineEditExchange[activeRadio]->grabKeyboard();
    }
    grabWidget = lineEditExchange[activeRadio];
    expandMacro(settings->value(s_call,s_call_def).toByteArray());
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
void So2sdr::altDEnter(int level, int mod)
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
        qso[activeRadio]->freq = rigFreq[activeRadio];
        addSpot(qso[activeRadio]->call, qso[activeRadio]->freq, qso[activeRadio]->dupe);
        if (qso[activeRadio]->dupe) {
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
            switchRadios(true);

            // popup any call entered after "/" on original radio
            QByteArray tmp;
            if (contest->newCall(tmp)) {
                lineEditCall[activeRadio]->setText(tmp);
                prefixCheck(activeRadio, tmp);
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
   <li>d. exchange sent
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
   </ul>

   states

   unsigned int enterState[2][2][2][2];

   mods:
   mod=1 (control) : logs without dupecheck
   mod=2 (shift)   : same except no cw

 */
void So2sdr::enter(int mod)
{
    keyInProgress=true;
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
        enterState[1][0][1][0] = 4 + 1 + 1024 + 8192;
        enterState[1][1][0][0] = 2 + 4 + 1024;
        enterState[1][1][1][0] = 16;

        enterState[0][0][0][1] = 0;
        enterState[0][0][1][1] = 0;
        enterState[0][1][0][1] = 0;
        enterState[0][1][1][1] = 0;
        enterState[1][0][0][1] = 0;
        enterState[1][0][1][1] = 0;
        enterState[1][1][0][1] = 8 + 32 + 64 + 128 + 512 + 2048 + 4096;
        enterState[1][1][1][1] = 8 + 32 + 64 + 128 + 2048;
        first                  = false;
    }

    // is it an entered frequency or mode?
    if (!qso[activeRadio]->call.isEmpty()) {
        i1 = 1;
        if (qso[activeRadio]->country == -1) {
            // Depend on enterFreq() to return false to process mode entry
            if (enterFreq() || enterMode()) {
                // clear any garbage that showed up in supercheck partial
                // from entering freq
                if (csettings->value(c_mastermode,c_mastermode_def).toBool()) {
                    MasterTextEdit->clear();
                }
                keyInProgress=false;
                return; // qsy successful
            }
        }
    } else {
        i1 = 0;
    }
    qso[activeRadio]->exch = lineEditExchange[activeRadio]->text().toAscii();
    qso[activeRadio]->exch = qso[activeRadio]->exch.trimmed();

    qso[activeRadio]->valid=contest->validateExchange(qso[activeRadio]);
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

    if (!cqMode[activeRadio] && !callSent[activeRadio]) {
        i2 = 0;
    }

    // ctrl+Enter logs without dupecheck or exchange validation
    if (mod == 1) {
        i2 = 1;
        // so only one enter press will log
        exchangeSent[activeRadio] = true;
    }

    i4 = exchangeSent[activeRadio];
    if (i4 > 2) i4 = 0;

    // change radios
    /*!
       @todo made actions below separate functions; some are duplicated elsewhere (backslash,...)
     */
    if (enterState[i1][i2][i3][i4] & 16384) {
        if (activeR2CQ) {
            lineEditCall[activeRadio]->clear();
            qso[activeRadio]->prefill.clear();
            origCallEntered[activeRadio].clear();
            switchRadios();

            // remove any band spot that is present
            removeSpotFreq(cat->getRigFreq(activeRadio), band[activeRadio]);

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
        }
    }

    // send callsign
    if (enterState[i1][i2][i3][i4] & 1) {
        if (mod != 2 &&
            !qso[activeRadio]->dupe) {
            expandMacro(settings->value(s_call,s_call_def).toByteArray());
            callSent[activeRadio] = true;
        }
    }

    // send CQ exchange
    if (enterState[i1][i2][i3][i4] & 2) {
        // check to see if this qso number already taken
        if (nrSent == nrReserved[activeRadio ^ 1]) {
            nrReserved[activeRadio] = nrSent + 1;
        } else {
            nrReserved[activeRadio] = nrSent;
        }
        if (mod != 2) {
            if (qso[activeRadio]->dupe && csettings->value(c_dupemode,c_dupemode_def).toInt() == STRICT_DUPES) {
                expandMacro(csettings->value(c_dupe_msg,c_dupe_msg_def).toByteArray());
            } else {
                expandMacro(csettings->value(c_cq_exc,c_cq_exc_def).toByteArray());
            }
        }
        exchangeSent[activeRadio]    = true;
        callSent[activeRadio]        = true; // set this true as well in case mode switched to S&P
        origCallEntered[activeRadio] = qso[activeRadio]->call;
        updateNrDisplay();
        cqQsoInProgress[activeRadio] = true;
    }

    // focus exchange
    if ((enterState[i1][i2][i3][i4] & 4) &&
        !qso[activeRadio]->dupe) {
        lineEditExchange[activeRadio]->setFocus();
        if (grab) {
            lineEditExchange[activeRadio]->grabKeyboard();
        }
        grabWidget             = lineEditExchange[activeRadio];
        callFocus[activeRadio] = false;
    }

    // log qso
    if (enterState[i1][i2][i3][i4] & 8) {
        fillSentExch(activeRadio);
        qso[activeRadio]->mode = cat->mode(activeRadio);
        contest->addQso(qso[activeRadio]);
        qso[activeRadio]->time = QDateTime::currentDateTimeUtc(); // update time just before logging qso
        addQso(qso[activeRadio]);
        if (!qso[activeRadio]->dupe && qso[activeRadio]->valid) {
            nqso[band[activeRadio]]++;
        }
        updateBreakdown();
        qso[activeRadio]->dupe = true;
        if (!cqMode[activeRadio]) {
            // add to bandmap if in S&P mode
            qso[activeRadio]->freq = rigFreq[activeRadio];
            addSpot(qso[activeRadio]->call, qso[activeRadio]->freq, true);
        } else {
            // remove any spot on this freq, update other spots if in CQ mode
            qso[activeRadio]->freq = rigFreq[activeRadio];
            removeSpotFreq(qso[activeRadio]->freq, band[activeRadio]);
            updateBandmapDupes(qso[activeRadio]);
        }
        updateDupesheet(qso[activeRadio]->call);
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
        callSent[activeRadio]  = false;
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
        if (mod != 2) expandMacro(csettings->value(c_sp_exc,c_sp_exc_def).toByteArray());
        exchangeSent[activeRadio] = true;
    }

    // focus call field
    if (enterState[i1][i2][i3][i4] & 32) {
        lineEditCall[activeRadio]->setFocus();
        if (grab) {
            lineEditCall[activeRadio]->grabKeyboard();
        }
        grabWidget             = lineEditCall[activeRadio];
        callFocus[activeRadio] = true;
    }

    // clear exch field
    if (enterState[i1][i2][i3][i4] & 64) {
        lineEditExchange[activeRadio]->clear();
        lineEditExchange[activeRadio]->setModified(false);
        qso[activeRadio]->exch.clear();
        qso[activeRadio]->prefill.clear();
    }

    // send F1 message
    if (enterState[i1][i2][i3][i4] & 256) {
        if (mod != 2) expandMacro(cwMessage->cqF[0]);
        exchangeSent[activeRadio] = false;
    }

    // send QSL msg
    if (enterState[i1][i2][i3][i4] & 512) {
        // see if call was corrected
        // if control+Enter, do not send any CW here.
        if (qso[activeRadio]->call != origCallEntered[activeRadio]) {
            if (mod != 2) expandMacro(csettings->value(c_qsl_msg_updated,c_qsl_msg_updated_def).toByteArray());
        } else {
            if (mod != 2) expandMacro(csettings->value(c_qsl_msg,c_qsl_msg_def).toByteArray());
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
    }

    // set exc mode
    if (enterState[i1][i2][i3][i4] & 1024) {
        excMode[activeRadio] = true;
        lineEditExchange[activeRadio]->show();
    }

    // exit exc mode
    if (enterState[i1][i2][i3][i4] & 2048) {
        excMode[activeRadio] = false;
        if (cqMode[activeRadio]) {
            lineEditExchange[activeRadio]->hide();
        }
    }

    // set initial exchange
    if (enterState[i1][i2][i3][i4] & 8192) {
        prefillExch(activeRadio);
    }

    // clear radio 2 cq
    if (enterState[i1][i2][i3][i4] & 32768) {
        clearR2CQ(activeRadio ^ 1);
    }

    // check for new call entered in exchange line
    if (enterState[i1][i2][i3][i4] & 4096) {
        QByteArray tmp;
        if (contest->newCall(tmp)) {
            lineEditCall[activeRadio]->setText(tmp);
            prefixCheck(activeRadio, tmp);
        }
    }
    keyInProgress=false;
}

void So2sdr::prefillExch(int nr)
{
    if (!qso[nr]->prefill.isEmpty()) {
        // prefill from log
        lineEditExchange[nr]->setText(qso[nr]->prefill);
    } else if (contest->hasPrefill() && !contest->prefillExchange(qso[nr]).isEmpty()) {
        lineEditExchange[nr]->setText(contest->prefillExchange(qso[nr]));
    }  else {
        // no initial exchange, just clear everything
        // lineEditExchange[nr]->clear();
        // qso[nr]->exch.clear();
        // lineEditExchange[nr]->setModified(false);
    }
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
   </ol>
 */
void So2sdr::esc()
{
    int                 i1;
    int                 i2;
    int                 i3;
    int                 i4;
    static unsigned int escState[2][2][2][2];
    unsigned int        x;
    static bool         first     = true;
    bool                altdClear = true;

	keyInProgress=true;
    // define states
    if (first) {
        escState[0][0][0][0] = 128 + 256 + 512 + 2048;
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
        escState[1][1][0][1] = 1 + 2 + 8 + 64 + 1024;
        escState[0][0][1][1] = 4 + 8 + 64;
        escState[1][0][1][1] = 1 + 8 + 64 + 1024;
        escState[0][1][1][1] = 2 + 8 + 64 + 1024;
        escState[1][1][1][1] = 2 + 1024;
        first                = false;
    }
    qso[activeRadio]->exch = lineEditExchange[activeRadio]->text().toAscii();
    qso[activeRadio]->exch = qso[activeRadio]->exch.trimmed();

    if (winkey->isSending()) {
        // if sending cw, stop
        winkey->cancelcw();
		keyInProgress=false;
        return;
    }

    // ESC needs to return focus to call field if it
    // screwed up by the mouse
    if (callFocus[activeRadio] && !lineEditCall[activeRadio]->hasFocus()) {
        lineEditCall[activeRadio]->setFocus();
        if (grab) {
            lineEditCall[activeRadio]->grabKeyboard();
        }
        grabWidget = lineEditCall[activeRadio];
    }

    QByteArray call = lineEditCall[activeRadio]->text().toAscii().toUpper();
    if (!call.isEmpty()) {
        i1 = 1;
    } else {
        i1 = 0;
    }
    if (!qso[activeRadio]->exch.isEmpty()) { // && !qso[activeRadio]->dupe) {
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

            // switch back to radio 1
            callFocus[altDActiveRadio] = true;
            switchRadios();
            callSent[altDActiveRadio] = false;

            // do not clear call/exchange field in this case
            altdClear = false;
        }
    }

    // clear call field
    if ((x & 1) && altdClear) {
        lineEditCall[activeRadio]->clear();
        lineEditCall[activeRadio]->setModified(false);
        labelCountry[activeRadio]->clear();
        labelBearing[activeRadio]->clear();
        labelLPBearing[activeRadio]->clear();
        sunLabelPtr[activeRadio]->clear();
        lineEditCall[activeRadio]->setCursorPosition(0);
        qso[activeRadio]->call.clear();
        So2sdrStatusBar->clearMessage();
        clearWorked(activeRadio);
        origCallEntered[activeRadio].clear();
        nrReserved[activeRadio] = 0;
        updateNrDisplay();
        MasterTextEdit->clear();
        statusBarDupe         = false;
        callSent[activeRadio] = false;
    }

    // clear exchange field
    if ((x & 2) && altdClear) {
        lineEditExchange[activeRadio]->clear();
        lineEditExchange[activeRadio]->setModified(false);
        lineEditExchange[activeRadio]->setCursorPosition(0);
        qso[activeRadio]->prefill.clear();
        qso[activeRadio]->exch.clear();
        nrReserved[activeRadio] = 0;
    }

    // exit S&P, set CQ
    if (x & 4) {
        setCqMode(activeRadio);
        callSent[activeRadio] = false;
    }

    // return focus to call
    if (x & 8) {
        lineEditCall[activeRadio]->setFocus();
        if (grab) {
            lineEditCall[activeRadio]->grabKeyboard();
        }
        grabWidget             = lineEditCall[activeRadio];
        callFocus[activeRadio] = true;
    }

    // return focus to exch
    if (x & 16) {
        lineEditExchange[activeRadio]->setFocus();
        if (grab) {
            lineEditExchange[activeRadio]->grabKeyboard();
        }
        grabWidget             = lineEditExchange[activeRadio];
        callFocus[activeRadio] = false;
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

    // reset winkey output port
    if (x & 128) {
        winkey->setSpeed(activeRadio);
        winkey->setOutput(activeRadio);
    }

    // clear radio2 cq status
    if (x & 512) {
        clearR2CQ(activeRadio ^ 1);
    }

    // clear alt-d state
    if (x & 256) {
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
        }
    }
    // clear log search
    if (x & 2048) {
        clearLogSearch();
    }
	keyInProgress=false;
}

/*!
   Function keys: choose either regular or Exchange/S&P
 */
void So2sdr::sendFunc(int i, int mod)
{
    // clear dupe message if present
    if (statusBarDupe) {
        So2sdrStatusBar->clearMessage();
    }
    switch (mod) {
    case 0:
        if (i == 1 && csettings->value(c_sprintmode,c_sprintmode_def).toBool() && cqQsoInProgress[activeRadio]) {
            /*!
               sprint: if CQ qso is in progress, continue to send CQ Exch when F2
               pressed. Must be cleaner way to handle this?
             */
            expandMacro(csettings->value(c_cq_exc,c_cq_exc_def).toByteArray());
        } else if (cqMode[activeRadio] && !excMode[activeRadio]) {
            /*!
               reset 2nd radio color change if needed
             */
            clearR2CQ(activeRadio ^ 1);
            expandMacro(cwMessage->cqF[i].toUpper());
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
            expandMacro(cwMessage->excF[i].toUpper());
        }
        break;
    case 1:
        expandMacro(cwMessage->cqCtrlF[i].toUpper());
        break;
    case 2:
        expandMacro(cwMessage->cqShiftF[i].toUpper());
        break;
    }
}
