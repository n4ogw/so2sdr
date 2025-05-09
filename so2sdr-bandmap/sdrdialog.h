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
#ifndef SDRDIALOG_H
#define SDRDIALOG_H

#include "afedrisetup.h"
#include "defines.h"
#include "networksetup.h"
#include "rtlsetup.h"
#include "soundcardsetup.h"
#include "ui_sdrdialog.h"
#include <QIcon>
#include <QList>
#include <QSettings>
#include <QString>

class SDRDialog : public QDialog, public Ui::SDRDialog {
  Q_OBJECT

public:
  SDRDialog(QSettings &s, QWidget *parent = nullptr);
  ~SDRDialog();
  double offset(int band) const;
  bool invert(int band) const;
  int invertSign(int band) const;
  SoundCardSetup *soundcard;
  AfedriSetup *afedri;
  NetworkSetup *network;
  RtlSetup *rtl;

signals:
  void setupErrors(const QString &);
  void update();
  void restartSdr();

public slots:
  void updateSDR();
  void rejectChanges();

private slots:
  void launchConfigure();
  void setRfAuto(int);

private:
  QSettings &settings;
  void updateFromSettings();
};
#endif
