/*! Copyright 2010-2024 R. Torsten Clay N4OGW

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
#include "newcontestdialog.h"
#include <QFile>
#include <QFileDialog>

NewDialog::NewDialog(QWidget *parent) : QDialog(parent) {
  setupUi(this);
  configFiles.clear();
}

QString NewDialog::selectedContest() {
  return (configFiles.at(NewContestComboBox->currentIndex()));
}

bool NewDialog::readContestList(QString fileName) {
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    emit newContestError("Can't open contest list file at " + fileName);
    return (false);
  }
  while (!file.atEnd()) {
    QByteArray buffer = file.readLine();
    int i = buffer.indexOf(",");
    QByteArray name = buffer.mid(0, i).trimmed();
    NewContestComboBox->addItem(name);
    configFiles.append(buffer.right(buffer.size() - i - 1).trimmed());
  }
  file.close();
  return (true);
}
