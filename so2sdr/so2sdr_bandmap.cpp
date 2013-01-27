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
#include <QErrorMessage>
#include "so2sdr.h"
#include <portaudio.h>

// //// Bandmap stuff

/*!
   update borderless option for bandmap. Useful because without borders, clicking on the bandmap
   won't take keyboard focus away from the main window
 */
void So2sdr::windowBorders(bool t)
{
    for (int i = 0; i < 2; i++) {
        if (bandmapOn[i]) {
            if (t) {
                bandmap[i]->setWindowFlags(Qt::X11BypassWindowManagerHint);
                bandmap[i]->show();
            } else {
                bandmap[i]->setWindowFlags(Qt::Widget);
                bandmap[i]->show();
            }
        }
    }
}

/*!
   called when bandmap clicked; argument hz is offset from freq of radio nr
 */
void So2sdr::mouseQSYevent(int nr, int hz)
{
    int tmp = rigFreq[nr] + hz; // freq=rig freq + offset
    qsyEvent(nr, tmp);
}

/*!
   qsy radio nr to freq f; change radio focus if option enabled
 */
void So2sdr::qsyEvent(int nr, int f)
{
    qsy(nr, f, true);

    // if option enabled, change to this radio and focus callsign
    if (sdr->ChangeRadioClickCheckBox->isChecked()) {
        if (nr != activeRadio) switchRadios(true);
        lineEditCall[activeRadio]->setFocus();
        if (grab) {
            lineEditCall[activeRadio]->grabKeyboard();
        }
        grabWidget = lineEditCall[activeRadio];
    }
}

void So2sdr::showBandmap1(int checkboxState)
{
    showBandmap(0, checkboxState);
}

void So2sdr::showBandmap2(int checkboxState)
{
    showBandmap(1, checkboxState);
}

/*! set transmit status for bandmap nr
 */
void So2sdr::setBandmapTxStatus(bool b, int nr)
{
    if (bandmapOn[nr]) {
        bandmap[nr]->setMark(b);
    }
}

/*!
   show/remove bandmap windows
 */
void So2sdr::showBandmap(int nr, int checkboxState)
{
    if (checkboxState == Qt::Unchecked) {
        if (bandmapOn[nr]) {
            disconnect(bandmap[nr], SIGNAL(done()), bandmapCheckBox[nr], SLOT(toggle()));
            disconnect(winkey, SIGNAL(markSignals(bool, int)), this, SLOT(setBandmapTxStatus(bool, int)));
            bandmap[nr]->close();
            bandmapOn[nr] = false;
        }
    } else {
        if (!bandmap[nr]) {
            bandmap[nr] = new Bandmap(*settings);
            bandmap[nr]->setWindowIcon(QIcon(dataDirectory + "/icon24x24.png"));
            bandmap[nr]->installEventFilter(this);
            bandmap[nr]->setAttribute(Qt::WA_ShowWithoutActivating);
            connect(bandmap[nr], SIGNAL(bandmapError(const QString &)), errorBox, SLOT(showMessage(const QString &)));
            connect(bandmap[nr], SIGNAL(deleteCallFreq(int, int)), this, SLOT(removeSpotFreq(int, int)));
            connect(bandmap[nr], SIGNAL(qsy(int, int)), this, SLOT(qsyEvent(int, int)));
            connect(bandmap[nr],SIGNAL(updateParams()),sdr,SLOT(updateSDR()));
            connect(winkey, SIGNAL(markSignals(bool, int)), this, SLOT(setBandmapTxStatus(bool, int)));
            connect(bandmap[nr], SIGNAL(findCQMessage(QString)), this, SLOT(showMessage(QString)));
        }
        bandmap[nr]->initialize(userDirectory, nr, sdr->format(nr));
        bandmap[nr]->setWindowTitle("Bandmap:" + bandName[band[nr]] + "m");
        if (windowBorderCheckBox->isChecked()) {
            bandmap[nr]->setWindowFlags(Qt::X11BypassWindowManagerHint);
        }

        // invert spectrum if needed
        bandmap[nr]->setInvert(bandInvert[nr][band[nr]] ^ (cat->mode(nr) == RIG_MODE_CWR));
        connect(bandmap[nr], SIGNAL(done()), bandmapCheckBox[nr], SLOT(toggle()));
        connect(bandmap[nr], SIGNAL(mouseQSY(int, int)), this, SLOT(mouseQSYevent(int, int)));
        bandmap[nr]->setFreq(rigFreq[nr], band[nr], spotList[band[nr]]);
        if (bandmap[nr]->start()) {
            bandmap[nr]->show();
            bandmapOn[nr] = true;
        } else {
            bandmapOn[nr] = false;
            bandmapCheckBox[nr]->setCheckState(Qt::Unchecked);
        }
        bandmap[nr]->setDefaultCenter();
    }

    // return focus
    regrab();
}
