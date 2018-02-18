/*! Copyright 2010-2018 R. Torsten Clay N4OGW

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
#include <QDialog>
#include <QKeyEvent>
#include "dupesheet.h"

DupeSheet::DupeSheet(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    clear();
    setFocusPolicy(Qt::NoFocus);
    Dupes0->setFocusPolicy(Qt::NoFocus);
    Dupes0->setLineWrapMode(QTextEdit::NoWrap);
    Dupes0->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    Dupes1->setFocusPolicy(Qt::NoFocus);
    Dupes1->setLineWrapMode(QTextEdit::NoWrap);
    Dupes1->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    Dupes2->setFocusPolicy(Qt::NoFocus);
    Dupes2->setLineWrapMode(QTextEdit::NoWrap);
    Dupes2->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    Dupes3->setFocusPolicy(Qt::NoFocus);
    Dupes3->setLineWrapMode(QTextEdit::NoWrap);
    Dupes3->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    Dupes4->setFocusPolicy(Qt::NoFocus);
    Dupes4->setLineWrapMode(QTextEdit::NoWrap);
    Dupes4->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    Dupes5->setFocusPolicy(Qt::NoFocus);
    Dupes5->setLineWrapMode(QTextEdit::NoWrap);
    Dupes5->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    Dupes6->setFocusPolicy(Qt::NoFocus);
    Dupes6->setLineWrapMode(QTextEdit::NoWrap);
    Dupes6->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    Dupes7->setFocusPolicy(Qt::NoFocus);
    Dupes7->setLineWrapMode(QTextEdit::NoWrap);
    Dupes7->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    Dupes8->setFocusPolicy(Qt::NoFocus);
    Dupes8->setLineWrapMode(QTextEdit::NoWrap);
    Dupes8->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    Dupes9->setFocusPolicy(Qt::NoFocus);
    Dupes9->setLineWrapMode(QTextEdit::NoWrap);
    Dupes9->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

DupeSheet::~DupeSheet()
{
}

/*!
   overrides the default key handler so ESC doesn't close dialog
 */
void DupeSheet::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Return:
        break;
    case Qt::Key_Escape:
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void DupeSheet::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    emit(closed(false));
    QDialog::closeEvent(event);
}

void DupeSheet::clear()
{
    for (int i = 0; i < dsColumns; i++) {
        dupeCalls[i].clear();
        dupeCallsKey[i].clear();
    }
    Dupes0->clear();
    Dupes1->clear();
    Dupes2->clear();
    Dupes3->clear();
    Dupes4->clear();
    Dupes5->clear();
    Dupes6->clear();
    Dupes7->clear();
    Dupes8->clear();
    Dupes9->clear();
}

void DupeSheet::updateDupesheet(QByteArray call)
{
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
    for (int i = 0; i < dupeCalls[digit].size(); i++) {
        if (call == dupeCalls[digit].at(i)) {
            found = true;
            break;
        }
    }
    if (!found) {
        int j = dupeCallsKey[digit].size();

        // find place to insert call based on letter after the number
        for (int i = 0; i < dupeCallsKey[digit].size(); i++) {
            if (next < dupeCallsKey[digit].at(i)) {
                j = i;
                break;
            }
        }
        dupeCalls[digit].insert(j, call);
        dupeCallsKey[digit].insert(j, next);

        QByteArray tmp = "";
        for (int i = 0; i < dupeCalls[digit].size(); i++) {
            tmp = tmp + dupeCalls[digit][i];
            if (i != (dupeCalls[digit].size() - 1)) {
                tmp = tmp + "\n";
            }
        }
        switch (digit) {
        case 0: Dupes0->setText(tmp); break;
        case 1: Dupes1->setText(tmp); break;
        case 2: Dupes2->setText(tmp); break;
        case 3: Dupes3->setText(tmp); break;
        case 4: Dupes4->setText(tmp); break;
        case 5: Dupes5->setText(tmp); break;
        case 6: Dupes6->setText(tmp); break;
        case 7: Dupes7->setText(tmp); break;
        case 8: Dupes8->setText(tmp); break;
        case 9: Dupes9->setText(tmp); break;
        }
    }
}

