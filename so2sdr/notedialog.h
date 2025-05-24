/*! Copyright 2010-2025 R. Torsten Clay N4OGW

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
#ifndef NOTEDIALOG_H
#define NOTEDIALOG_H

#include "ui_notedialog.h"
#include <QString>

/*!
   Dialog to enter log notes
 */
class NoteDialog : public QDialog, public Ui::NoteDialog {
  Q_OBJECT

public:
  explicit NoteDialog(QWidget *parent = nullptr);
  void enterNote(QString fname, const QString &dir, const QString &time,
                 bool grab = false);

private slots:
  void writeNotes();

private:
  QString noteDir;
  QString noteFile;
};

#endif
