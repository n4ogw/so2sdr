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
#ifndef WSJTXCALLDIALOG_H
#define WSJTXCALLDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QSettings>
#include <QSortFilterProxyModel>
#include "defines.h"
#include "qso.h"
#include "udpreader.h"
#include "wsjtxdelegate.h"
#include "ui_wsjtxcalldialog.h"

/*!
   Dialog showing calls and other information from wsjt-x
 */
class WsjtxCallDialog : public QDialog, public Ui::WsjtxCalls
{
Q_OBJECT

public:
    WsjtxCallDialog(QSettings &s,uiSize sizes, int nrig, QWidget *parent = nullptr);
    ~WsjtxCallDialog();
    void decayCalls() { if (reader->isEnabled()) reader->decayCalls();}
    bool isEnabled() const { return reader->isEnabled(); }
    void redupe() { if (reader->isEnabled()) reader->redupe(); }
    void setNrig(int n);
    void setFreq(double f);

public slots:
    void clear() { if (reader->isEnabled()) reader->clearCalls(); }
    void clearWsjtx() { if (reader->isEnabled()) reader->clearWsjtxCalls(); }
    void enable(bool);
    void replay() { if (reader->isEnabled()) reader->replay(); }

signals:
    void wsjtxDialog(bool);
    void dupeCheck(Qso *qso);
    void wsjtxQso(Qso *);

private slots:
    void dialogFinished(int);
    void hideDupesChanged(int);

private:
    int _nrig;
    QSettings &settings;
    QSortFilterProxyModel *proxy;
    UDPReader *reader;
    wsjtxDelegate *delegate;
};

#endif
