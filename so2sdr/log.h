/*! Copyright 2010-2016 R. Torsten Clay N4OGW

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
#ifndef LOG_H
#define LOG_H
#include <QFile>
#include <QList>
#include <QDateTime>
#include <QSettings>
#include <QSqlDatabase>
#include "defines.h"
#include "qso.h"
#include "serial.h"

/*!
   class defining log database structure and related functions
 */
class Log : public QObject
{
Q_OBJECT

public:
    Log(QSettings& s,int n_exch, QObject *parent);
    ~Log();

    void closeLogFile();
    bool exportADIF(QFile *) const;
    void exportCabrillo(QFile *,QString call,QString,QString,QString,QString) const;
    bool isDupe(Qso *qso, bool DupeCheckingEveryBand, bool FillWorked) const;
    int lastNr() const;
    QString offTime(int minOffTime, QDateTime start, QDateTime end);
    bool openLogFile(QString fname,bool clear);
    bool qsoPtsField() const;
    void setFieldsShown(const unsigned int snt, const unsigned int rcv);
    void setPrefill(const int indx);
    void setQsoPtsField(bool b);
    void setRstField(int i);
    void setupQsoNumbers(const int n);
    friend class So2sdr;

public slots:
    void mobileDupeCheck(Qso *qso);

private:
    bool         prefill[MAX_EXCH_FIELDS];
    bool         _qsoPtsField;
    int          nExchange;
    int          nrField;
    int          rstField;
    QSettings&   csettings;
    QSqlDatabase db;
    QString      logFileName;
    unsigned int rcvFieldsShown;
    unsigned int sntFieldsShown;
};

#endif
