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
#include <QDir>
#include "notedialog.h"

NoteDialog::NoteDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    connect(NoteButtonBox, SIGNAL(accepted()), this, SLOT(writeNotes()));
}

/*!
   called to open note dialog
 */
void NoteDialog::enterNote(QString fname, QString dir, QString time, bool grab)
{
    show();
    NoteLineEdit->setFocus();
    if (grab) NoteLineEdit->grabKeyboard();
    NoteLineEdit->setText(time + ":");
    noteFile = fname.remove("cfg") + "txt";
    noteDir  = dir;
}

/*!
   append note to file

   @todo File write error not handled
 */
void NoteDialog::writeNotes()
{
    QDir  directory;
    directory.setCurrent(noteDir);
    QFile file(noteFile);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        file.write(NoteLineEdit->text().toAscii().data());
        file.write("\n");
        file.close();
    }
}
