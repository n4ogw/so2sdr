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
#include <QDebug>
#include <QFont>
#include <QCheckBox>
#include <QErrorMessage>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QWidgetAction>
#include "so2sdr.h"

// //////// Visible dupesheet


void So2sdr::initDupeSheet()
{
    nDupesheet           = 0;
    dupesheetCheckBox[0] = new QCheckBox("Dupesheet 1", menuWindows);
    dupesheetCheckBox[1] = new QCheckBox("Dupesheet 2", menuWindows);
    for (int i = 0; i < NRIG; i++) {
        dupesheetCheckAction[i] = new QWidgetAction(menuWindows);
        dupesheetCheckAction[i]->setDefaultWidget(dupesheetCheckBox[i]);
        menuWindows->addAction(dupesheetCheckAction[i]);
    }
    connect(dupesheetCheckBox[0], SIGNAL(stateChanged(int)), this, SLOT(showDupesheet1(int)));
    connect(dupesheetCheckBox[1], SIGNAL(stateChanged(int)), this, SLOT(showDupesheet2(int)));
    dupeCalls[0]    = new QList<QByteArray>[dsColumns];
    dupeCalls[1]    = new QList<QByteArray>[dsColumns];
    dupeCallsKey[0] = new QList<char>[dsColumns];
    dupeCallsKey[1] = new QList<char>[dsColumns];
    for (int i = 0; i < dsColumns; i++) {
        dupeCalls[0][i].clear();
        dupeCalls[1][i].clear();
        dupeCallsKey[0][i].clear();
        dupeCallsKey[1][i].clear();
    }
}

void So2sdr::showDupesheet1(int checkboxState)
{
    showDupesheet(0, checkboxState);
}

void So2sdr::showDupesheet2(int checkboxState)
{
    showDupesheet(1, checkboxState);
}

void So2sdr::showDupesheet(int nr, int checkboxState)
{
    if (checkboxState == Qt::Unchecked) {
        // need to disconnect, otherwise signal will toggle checkbox again and
        // reopen the dupesheet
        disconnect(dupesheet[nr], SIGNAL(destroyed()), dupesheetCheckBox[nr], SLOT(toggle()));
        dupesheet[nr]->close();
        nDupesheet--;
    } else {
        if (!dupesheet[nr]) {
            dupesheet[nr] = new DupeSheet(this);
            dupesheet[nr]->setWindowIcon(QIcon(dataDirectory + "/icon24x24.png"));
            dupesheet[nr]->installEventFilter(this);
            // restore geometry
            switch (nr) {
            case 0: settings->beginGroup("DupeSheetWindow1"); break;
            case 1: settings->beginGroup("DupeSheetWindow2"); break;
            }
            dupesheet[nr]->resize(settings->value("size", QSize(750, 366)).toSize());
            dupesheet[nr]->move(settings->value("pos", QPoint(400, 400)).toPoint());
            settings->endGroup();
        }
        nDupesheet++;
        dupesheet[nr]->setAttribute(Qt::WA_DeleteOnClose);
        dupesheet[nr]->setFocusPolicy(Qt::NoFocus);
        dupesheet[nr]->Dupes0->setFocusPolicy(Qt::NoFocus);
        dupesheet[nr]->Dupes0->setLineWrapMode(QTextEdit::NoWrap);
        dupesheet[nr]->Dupes0->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        dupesheet[nr]->Dupes1->setFocusPolicy(Qt::NoFocus);
        dupesheet[nr]->Dupes1->setLineWrapMode(QTextEdit::NoWrap);
        dupesheet[nr]->Dupes1->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        dupesheet[nr]->Dupes2->setFocusPolicy(Qt::NoFocus);
        dupesheet[nr]->Dupes2->setLineWrapMode(QTextEdit::NoWrap);
        dupesheet[nr]->Dupes2->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        dupesheet[nr]->Dupes3->setFocusPolicy(Qt::NoFocus);
        dupesheet[nr]->Dupes3->setLineWrapMode(QTextEdit::NoWrap);
        dupesheet[nr]->Dupes3->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        dupesheet[nr]->Dupes4->setFocusPolicy(Qt::NoFocus);
        dupesheet[nr]->Dupes4->setLineWrapMode(QTextEdit::NoWrap);
        dupesheet[nr]->Dupes4->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        dupesheet[nr]->Dupes5->setFocusPolicy(Qt::NoFocus);
        dupesheet[nr]->Dupes5->setLineWrapMode(QTextEdit::NoWrap);
        dupesheet[nr]->Dupes5->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        dupesheet[nr]->Dupes6->setFocusPolicy(Qt::NoFocus);
        dupesheet[nr]->Dupes6->setLineWrapMode(QTextEdit::NoWrap);
        dupesheet[nr]->Dupes6->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        dupesheet[nr]->Dupes7->setFocusPolicy(Qt::NoFocus);
        dupesheet[nr]->Dupes7->setLineWrapMode(QTextEdit::NoWrap);
        dupesheet[nr]->Dupes7->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        dupesheet[nr]->Dupes8->setFocusPolicy(Qt::NoFocus);
        dupesheet[nr]->Dupes8->setLineWrapMode(QTextEdit::NoWrap);
        dupesheet[nr]->Dupes8->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        dupesheet[nr]->Dupes9->setFocusPolicy(Qt::NoFocus);
        dupesheet[nr]->Dupes9->setLineWrapMode(QTextEdit::NoWrap);
        dupesheet[nr]->Dupes9->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        populateDupesheet();
        dupesheet[nr]->show();
        connect(dupesheet[nr], SIGNAL(destroyed()), dupesheetCheckBox[nr], SLOT(toggle()));
    }
}

void So2sdr::updateDupesheet(QByteArray call)
{
    if (nDupesheet == 0) return;

    // find first digit in callsign
    int idigit = 0;
    for (int i = 0; i < call.size(); i++) {
        if (call.at(i) > 47 && call.at(i) < 58) {
            idigit = i;
            break;
        }
    }
    int  digit = call.mid(idigit, 1).toInt();
    char next;
    if ((idigit + 1) < call.size()) {
        next = call.mid(idigit + 1, 1).at(0);
    } else {
        next = 'A';
    }

    // is call already in list?
    bool found = false;
    for (int i = 0; i < dupeCalls[0][digit].size(); i++) {
        if (call == dupeCalls[0][digit].at(i)) {
            found = true;
            break;
        }
    }
    if (!found) {
        int j = dupeCallsKey[0][digit].size();

        // find place to insert call based on letter after the number
        for (int i = 0; i < dupeCallsKey[0][digit].size(); i++) {
            if (next < dupeCallsKey[0][digit].at(i)) {
                j = i;
                break;
            }
        }
        dupeCalls[0][digit].insert(j, call);
        dupeCallsKey[0][digit].insert(j, next);

        QByteArray tmp = "";
        for (int i = 0; i < dupeCalls[0][digit].size(); i++) {
            tmp = tmp + dupeCalls[0][digit][i];
            if (i != (dupeCalls[0][digit].size() - 1)) {
                tmp = tmp + "\n";
            }
        }
        switch (digit) {
        case 0: dupesheet[0]->Dupes0->setText(tmp); break;
        case 1: dupesheet[0]->Dupes1->setText(tmp); break;
        case 2: dupesheet[0]->Dupes2->setText(tmp); break;
        case 3: dupesheet[0]->Dupes3->setText(tmp); break;
        case 4: dupesheet[0]->Dupes4->setText(tmp); break;
        case 5: dupesheet[0]->Dupes5->setText(tmp); break;
        case 6: dupesheet[0]->Dupes6->setText(tmp); break;
        case 7: dupesheet[0]->Dupes7->setText(tmp); break;
        case 8: dupesheet[0]->Dupes8->setText(tmp); break;
        case 9: dupesheet[0]->Dupes9->setText(tmp); break;
        }
    }
}

void So2sdr::populateDupesheet()

// populates dupes sheet. Needs to be called when switching bands
// or first turning on the dupesheet
{
    dupesheet[0]->Dupes0->clear();
    dupesheet[0]->Dupes1->clear();
    dupesheet[0]->Dupes2->clear();
    dupesheet[0]->Dupes3->clear();
    dupesheet[0]->Dupes4->clear();
    dupesheet[0]->Dupes5->clear();
    dupesheet[0]->Dupes6->clear();
    dupesheet[0]->Dupes7->clear();
    dupesheet[0]->Dupes8->clear();
    dupesheet[0]->Dupes9->clear();
    for (int i = 0; i < dsColumns; i++) {
        dupeCalls[0][i].clear();
        dupeCallsKey[0][i].clear();
    }
    QSqlQueryModel m;
    m.setQuery("SELECT * FROM log WHERE BAND=" + QString::number(band[activeRadio]), *mylog->db);
    while (m.canFetchMore()) {
        m.fetchMore();
    }
    for (int i = 0; i < m.rowCount(); i++) {
        QByteArray tmp = m.record(i).value("call").toString().toAscii();
        updateDupesheet(tmp);
    }
    dupesheet[0]->setWindowTitle("Dupesheet " + bandName[band[activeRadio]]);
}
