/*! Copyright 2010-2023 R. Torsten Clay N4OGW

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
#include "centergriddialog.h"
#include <QFont>
#include <QFontMetricsF>

CenterGridDialog::CenterGridDialog(QWidget *parent) : QDialog(parent) {
  setupUi(this);
  connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(emitgrid()));

  QFont font12("sans", 12);
  QFontMetricsF fm12(font12);
  label->setFixedHeight(qRound(fm12.height()));
  lineEdit->setFixedWidth(qRound(fm12.width("0") * 50));
  lineEdit->setFixedHeight(qRound(fm12.height()));
  adjustSize();
  setFixedSize(size());
  show();
}

void CenterGridDialog::setText(QByteArray s) { lineEdit->setText(s); }

void CenterGridDialog::emitgrid() {
  QByteArray text = lineEdit->text().toLatin1();
  emit grid(text);
  close();
}
