/*! Copyright 2010-2020 R. Torsten Clay N4OGW

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
#ifndef SSBMESSAGEDIALOG_H
#define SSBMESSAGEDIALOG_H

#include <QButtonGroup>
#include <QPushButton>
#include <QByteArray>
#include <QSettings>
#include <QString>
#include "defines.h"
#include "ui_ssbmessagedialog.h"
#include "utils.h"

class QPushButton;
class QLineEdit;
class QProcess;
class So2sdr;

/*!
   Dialog for entering SSB messages
 */
class SSBMessageDialog : public QDialog, public Ui::SSBMessageDialog
{
Q_OBJECT

public:
    SSBMessageDialog(uiSize sizes, QWidget *parent = 0);
    ~SSBMessageDialog();
    void initialize(QSettings *cs, QSettings *s);
    bool isPlaying() const {return playing; }
    friend class So2sdr;

signals:
    void finished();
    void recordingStatus(bool);
    void sendMsg(QByteArray,bool);
    void setPtt(int nr,int state);

public slots:
    void recMessage(QString);
    void cancelMessage();
    void playMessage(int nrig, QString);
    void updateSSBMsg();
    void rejectChanges();

private slots:
    void recMessage2(int);
    void recMessage3(int);
    void playMessage2(int);
    void playMessage3(int);
    void playButtons(int id);
    void playExcButtons(int id);
    void recButtons(int id);
    void excRecButtons(int id);
    void otherPlayButtons(int id);
    void otherRecButtons(int id);

private:
    // m is an index for the mode: 0=CW, 1=SSB, 2=DIGI can all have different macros
    const static int  m=1;

    bool           playing;
    bool           recording;
    QButtonGroup   excPlayGroup;
    QButtonGroup   playGroup;
    QButtonGroup   otherPlayGroup;
    QButtonGroup   recGroup;
    QButtonGroup   excRecGroup;
    QButtonGroup   otherRecGroup;
    QByteArray     cqF[N_FUNC];
    QByteArray     excF[N_FUNC];
    QByteArray     cqRecF[N_FUNC];
    QByteArray     excRecF[N_FUNC];
    QLineEdit      *excFuncEditPtr[N_FUNC];
    QLineEdit      *excFuncRecEditPtr[N_FUNC];
    QPushButton    *excFuncRecPtr[N_FUNC];
    QPushButton    *excFuncPlayPtr[N_FUNC];
    QLineEdit      *funcEditPtr[N_FUNC];
    QLineEdit      *funcRecEditPtr[N_FUNC];
    QPushButton    *funcRecPtr[N_FUNC];
    QPushButton    *funcPlayPtr[N_FUNC];
    UpperValidator *upperValidate;
    QSettings      *csettings,*settings;
    QString         message;
    QString         cmd;
    int             playMessageRig;
    QProcess        *scriptProcess;
};

#endif
