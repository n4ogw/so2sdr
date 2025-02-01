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
#ifndef AFEDRISETUP_H
#define AFEDRISETUP_H

#include "bandoffsetsetup.h"
#include "defines.h"
#include "ui_afedrisetup.h"
#include <QDialog>
#include <QSettings>

class AfedriSetup : public QDialog, public Ui::afedriSetup {
  Q_OBJECT
public:
  explicit AfedriSetup(QSettings &s, uiSize sizes, QWidget *parent = nullptr);
  double offset(int band) const;
  bool invert(int band) const;

signals:
  void afedriError(const QString &);

public slots:
  void setFreq(unsigned int f) { clockFreqLabel->setText(QString::number(f)); }
  void setActualSampleRate(unsigned int f) { realSampleRateLabel->setText(QString::number(f)); }

private slots:
  void updateAfedri();
  void rejectChanges();

private:
  QSettings &settings;
  void updateFromSettings();
  BandOffsetSetup *offsetSetup;
};

#endif // AFEDRISETUP_H
