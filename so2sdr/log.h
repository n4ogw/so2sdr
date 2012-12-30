/*! Copyright 2010-2013 R. Torsten Clay N4OGW

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

// column numbers in SQL log
const int SQL_COL_NR    =  0;    // ID number (SQL primary key)
const int SQL_COL_TIME  =  1;    // time HHMM  (string)
const int SQL_COL_FREQ  =  2;    // freq in Hz (int)
const int SQL_COL_CALL  =  3;    // call (string)
const int SQL_COL_BAND  =  4;    // band (int)
const int SQL_COL_DATE  =  5;    // date MMddyyyy (string)
const int SQL_COL_MODE  =  6;    // mode (int)
const int SQL_COL_SNT1  =  7;    // sent exchange field 1 (string)
const int SQL_COL_SNT2  =  8;    // sent exchange field 2 (string)
const int SQL_COL_SNT3  =  9;    // sent exchange field 3 (string)
const int SQL_COL_SNT4  =  10;   // sent exchange field 4 (string)
const int SQL_COL_RCV1  =  11;   // rcv exchange field 1 (string)
const int SQL_COL_RCV2  =  12;   // rcv exchange field 2 (string)
const int SQL_COL_RCV3  =  13;   // rcv exchange field 3 (string)
const int SQL_COL_RCV4  =  14;   // rcv exchange field 4 (string)
const int SQL_COL_PTS   =  15;   // qso points (int)
const int SQL_COL_VALID =  16;   // valid flag (int) if 0, qso not exported to cabrillo
const int SQL_N_COL     =  17;   // total number of columns

/*!
   class defining log database structure and related functions
 */
class log : public QObject
{
Q_OBJECT

public:
    log(int n_exch, QObject *parent);
    ~log();

    void closeLogFile();
    bool exportADIF(QFile *) const;
    void exportCabrillo(QFile *,QString call,QString,QString,QString,QString) const;
    bool isDupe(Qso *qso, bool DupeCheckingEveryBand, bool FillWorked) const;
    int lastNr() const;
    void offTime(QString &str,int minOffTime,QDateTime start,QDateTime end);
    bool openLogFile(QString fname,bool clear,QSettings *s);
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
    bool         *prefill;
    bool         _qsoPtsField;
    int          nExchange;
    int          nrField;
    int          rstField;
    QSettings    *csettings;
    QSqlDatabase *db;
    QString      logFileName;
    unsigned int rcvFieldsShown;
    unsigned int sntFieldsShown;
};

#endif
