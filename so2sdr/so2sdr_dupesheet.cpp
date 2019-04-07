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
#include <QDebug>
#include <QSqlQueryModel>
#include "dupesheet.h"
#include "log.h"
#include "so2sdr.h"

// //////// Visible dupesheet

/* the program operates differently depending on whether one or two dupesheets are open:
 *
 *1) If only one dupesheet is open, it follows the active radio. This is the correct behavior
 *for Sprint operating.
 *
 *2) If two dupesheets are open, one shows each radio.
 *
 *If you have two open and then close one, the program switches over to the other mode. Similar behavior
 *if one is open and then a second is opened.
 **/


void So2sdr::initDupeSheet()
{
    connect(dupesheetAction1,SIGNAL(triggered(bool)),this,SLOT(showDupesheet1(bool)));
    connect(dupesheetAction2,SIGNAL(triggered(bool)),this,SLOT(showDupesheet2(bool)));
    for (int nr=0;nr<NRIG;nr++) {
        dupesheet[nr] = new DupeSheet(this);
        dupesheet[nr]->hide();
        dupesheet[nr]->setWindowIcon(QIcon(dataDirectory() + "/icon24x24.png"));
        dupesheet[nr]->installEventFilter(this);

        // restore geometry
        switch (nr) {
        case 0: settings->beginGroup("DupeSheetWindow1"); break;
        case 1: settings->beginGroup("DupeSheetWindow2"); break;
        }
        dupesheet[nr]->resize(settings->value("size", QSize(750, 366)).toSize());
        dupesheet[nr]->move(settings->value("pos", QPoint(400, 400)).toPoint());
        settings->endGroup();
        dupesheet[nr]->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint |
                                      Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
    }
    connect(dupesheet[0],SIGNAL(closed(bool)),dupesheetAction1,SLOT(setChecked(bool)));
    connect(dupesheet[1],SIGNAL(closed(bool)),dupesheetAction2,SLOT(setChecked(bool)));
}

void So2sdr::showDupesheet1(bool checkboxState)
{
    showDupesheet(0, checkboxState);
}

void So2sdr::showDupesheet2(bool checkboxState)
{
    showDupesheet(1, checkboxState);
}

void So2sdr::showDupesheet(int nr, bool checkboxState)
{
    if (!checkboxState) {
        dupesheet[nr]->hide();
        populateDupesheet();
    } else {
        dupesheet[nr]->show();
        populateDupesheet();
        setEntryFocus();
    }
}

int So2sdr::nDupesheet() const
{
    int i=0;
    if (dupesheet[0]->isVisible()) i++;
    if (dupesheet[1]->isVisible()) i++;
    return i;
}


/*! populates dupe sheet. Needs to be called when switching bands
 or first turning on the dupesheet. If band has not changed, will do
 nothing.
 */
void So2sdr::populateDupesheet()
{
    // if only one dupesheet is active, figure out which one it is
    bool oneactive=false;
    int nr=0;
    if (nDupesheet()==1) {
        oneactive=true;
        for (int i=0;i<NRIG;i++) {
            if (!dupesheet[i]) continue;
            if (dupesheet[i]->isVisible()) {
                nr=i;
                break;
            }
        }
    }

    for (int id=0;id<NRIG;id++) {
        if (!dupesheet[id]) continue;
        int ib=id;
        if (oneactive) {
            if (nr!=id) continue;
            ib=activeRadio;
        }
        // abort if not on known band, or if band has not changed
        int b=cat[ib]->band();
        if (b==BAND_NONE || b==dupesheet[id]->band()) continue;

        dupesheet[id]->clear();
        dupesheet[id]->setBand(b);
        QSqlQueryModel m;
        m.setQuery("SELECT * FROM log WHERE valid=1 and BAND=" + QString::number(b), log->dataBase());

        while (m.canFetchMore()) {
            m.fetchMore();
        }
        for (int i = 0; i < m.rowCount(); i++) {
            QByteArray tmp = m.record(i).value("call").toString().toLatin1();
            dupesheet[id]->updateDupesheet(tmp);
        }
        dupesheet[id]->setWindowTitle("Dupesheet " + bandName[b]);
    }
}
