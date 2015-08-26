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
#ifndef SOUNDCARDSETUP_H
#define SOUNDCARDSETUP_H

#include "defines.h"
#include <portaudio.h>
#include <QDialog>
#include <QIcon>
#include <QList>
#include <QString>
#include <QSettings>
#include "ui_soundcard.h"

class SoundCardSetup : public QDialog,  public Ui::SoundCardSetup
{
    Q_OBJECT
public:
    explicit SoundCardSetup(QSettings &s,QWidget  *parent=0);
    ~SoundCardSetup();

signals:
    void PortAudioError(const QString &);

private slots:
    void updateSoundCard();
    void rejectChanges();
    void updateDeviceList(int indx);

private:
    QSettings        &settings;
    int                nAPI;
    int                *nApiDevices;
    QIcon              iconNOK;
    QIcon              iconOK;
    QList<bool>        *deviceOK;
    QList<QString>     audioDevices;
    QList<QString>     *nApiDeviceNames;
  //  PaStreamParameters Format;
    void updateFromSettings();
};

#endif // SOUNDCARDSETUP_H
