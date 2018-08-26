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
#ifndef SO2R_H
#define SO2R_H

#include <QObject>
#include <QSettings>
#include <QString>
#include "defines.h"
#include "otrsp.h"
#include "microham.h"
#include "linux_pp.h"
#include "so2rdialog.h"

class So2r : public QObject
{
    Q_OBJECT
public:
    explicit So2r(QSettings &s,uiSize sz,QObject *parent = 0);
    ~So2r();
    void sendMicrohamCommand(QByteArray c);
    void sendOtrspCommand(QByteArray c,int nr);
    bool stereoActive() const;
    void switchAudio(int r);
    void switchTransmit(int r);
    void toggleStereo(int activeRadio);
    int transmitRadio() const;
    void updateIndicators(int activeRadio);

signals:
    void setRX1(const QString &);
    void setRX2(const QString &);
    void setTX1(const QString &);
    void setTX2(const QString &);
    void error(const QString &);
    void So2rDialogAccepted();
    void So2rDialogRejected();

public slots:
    void setPtt(int nr,int state);
    void showDialog();

private:
    bool stereo;
    int txRadio;
    QSettings &settings;
    MicroHam *microham;
    OTRSP    *otrsp[2];
    ParallelPort *pport;
    const QString redLED    = "QLabel { background-color : red; border-radius: 4px; }";
    const QString greenLED  = "QLabel { background-color : green; border-radius: 4px; }";
    const QString clearLED  = "QLabel { background-color : none; border-radius: 4px; }";
    So2rDialog   *so2rDialog;
    uiSize sizes;
};

#endif // SO2R_H
