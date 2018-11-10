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
#include <QFile>
#include <QByteArray>
#include <QChar>
#include <QDataStream>
#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QString>
#include <QStringList>
#include <QTime>
#include "log.h"

/*!
   n_exch is number of exchange fields; this can't change during the contest
 */
Log::Log(QSettings &cs,QSettings &s, uiSize sizes,QObject *parent = 0) : QObject(parent),csettings(cs),settings(s)
{
    contest = 0;
    detail = 0;
    model = 0;
    lat=0.0;
    lon=0.0;
    logdel=0;
    origEditRecord.clear();
    db=QSqlDatabase::addDatabase("QSQLITE");
    for (int i=0;i<N_BANDS;i++) qsoCnt[i]=0;
    logSearchFlag=false;
    searchList.clear();
    detail=new DetailedEdit(sizes);
    connect(detail,SIGNAL(editedRecord(QSqlRecord)),this,SLOT(updateRecord(QSqlRecord)));
    detail->hide();
    cty = new Cty(csettings);
}

Log :: ~Log()
{
    closeLogFile();
    if (model) {
        QSqlDatabase::removeDatabase("QSQLITE");
        delete model;
    }
    if (contest) delete contest;
    if (logdel) delete logdel;
    if (detail) delete detail;
    if (cty) delete cty;
}

QString Log::bandLabel(int i) const {return contest->bandLabel(i);}
bool Log::bandLabelEnable(int i) const {return contest->bandLabelEnable(i);}
int Log::columnCount(int col) const {return contest->columnCount(col);}
QSqlDatabase& Log::dataBase() { return db; }
tableModel* Log::mod() { return model;}
int Log::nQso(int band) const { return qsoCnt[band];}
QSqlRecord Log::record(QModelIndex index) { return model->record(index.row());}
logDelegate* Log::delegate() const { return logdel;}
Cty* Log::ctyPtr() const { return cty;}
QString Log::exchangeName(int i) const { return contest->exchangeName(i);}
int Log::fieldWidth(int col) const { return contest->fieldWidth(col);}
bool Log::gridMults() const { return contest->gridMults();}
int Log::highlightBand(int b, ModeTypes modeType) const { return contest->highlightBand(b,modeType);}
int Log::nExch() const { return contest->nExchange();}
bool Log::newCall(QByteArray &s) const { return contest->newCall(s);}
bool Log::hasPrefill() const { return contest->hasPrefill();}
QByteArray Log::prefillExchange(Qso *qso) { return contest->prefillExchange(qso);}
ModeTypes Log::nextModeType(ModeTypes m) const { return contest->nextModeType(m);}
FieldTypes Log::exchType(int i) const { return contest->exchType(i);}
int Log::nMultsBWorked(int ii, int band) const { return contest->nMultsBWorked(ii,band);}
int Log::nMultsColumn(int col,int ii) const { return contest->nMultsColumn(col,ii);}
int Log::score() const { return contest->Score();}
int Log::nMults(int ii) const { return contest->nMults(ii);}
ContestType Log::contestType() const { return contest->contestType();}
QByteArray Log::neededMultName(int ii, int band, int i, bool &needed_band, bool &needed) const
{
    return contest->neededMultName(ii,band,i,needed_band,needed);
}
QByteArray Log::neededMultNameMode(int ii, int band, ModeTypes mode,int i, bool &needed_band, bool &needed) const
{
    return contest->neededMultNameMode(ii,band,mode,i,needed_band,needed);
}
void Log::workedMults(Qso * qso, unsigned int worked[MMAX]) const { contest->workedMults(qso,worked);}
void Log::guessMult(Qso *qso) const {  contest->guessMult(qso);}
bool Log::dupeCheckingByBand() const { return contest->dupeCheckingByBand();}
bool Log::validateExchange(Qso *qso) { return contest->validateExchange(qso);}
unsigned int Log::sntFieldShown() const { return contest->sntFieldShown();}
unsigned int Log::rcvFieldShown() const { return contest->rcvFieldShown();}
QVariant Log::columnName(int c) const { return contest->columnName(c);}
QString Log::mySunTimes() const { return cty->mySunTimes();}
int Log::zoneType() const { return contest->zoneType(); }

/*!
  close sql database
  */
void Log::closeLogFile()
{
    db.close();
}

/*!
   ADIF file export

 */
bool Log::exportADIF(QFile *adifFile) const
{
    QSqlQueryModel m;

    m.setQuery("SELECT * FROM log where valid=1", db);
    while (m.canFetchMore()) {
        m.fetchMore();
    }
    if (m.rowCount() == 0) {
        adifFile->close();
        return(false);  // nothing to do
    }

    bool ok = true;
    for (int i = 0; i < m.rowCount(); i++) {
        // do not include invalid qsos
        if (!m.record(i).value(SQL_COL_VALID).toBool()) continue;

        QByteArray tmp;
        QByteArray tmp2 = m.record(i).value(SQL_COL_CALL).toByteArray();

        // call
        tmp = "<CALL:" + QByteArray::number(tmp2.size()) + ">" + tmp2;

        // band
        switch (m.record(i).value(SQL_COL_BAND).toInt()) {
        case BAND160: tmp = tmp + "<BAND:4>160M"; break;
        case BAND80: tmp = tmp + "<BAND:3>80M"; break;
        case BAND60: tmp = tmp + "<BAND:3>60M"; break;
        case BAND40: tmp = tmp + "<BAND:3>40M"; break;
        case BAND30: tmp = tmp + "<BAND:3>30M"; break;
        case BAND20: tmp = tmp + "<BAND:3>20M"; break;
        case BAND17: tmp = tmp + "<BAND:3>17M"; break;
        case BAND15: tmp = tmp + "<BAND:3>15M"; break;
        case BAND12: tmp = tmp + "<BAND:3>12M"; break;
        case BAND10: tmp = tmp + "<BAND:3>10M"; break;
        case BAND6: tmp = tmp + "<BAND:2>6M"; break;
        case BAND2: tmp = tmp + "<BAND:3>2M"; break;
        case BAND222: tmp = tmp + "<BAND:5>1.25M"; break;
        case BAND420: tmp = tmp + "<BAND:4>70CM"; break;
        case BAND902: tmp = tmp + "<BAND:4>33CM"; break;
        case BAND1240: tmp = tmp + "<BAND:4>23CM"; break;
        case BAND630: tmp = tmp + "<BAND:3>630M"; break;
        case BAND2200: tmp = tmp + "<BAND:5>2200M"; break;
        }

        // frequency
        double f = m.record(i).value(SQL_COL_FREQ).toDouble() / 1000000.0;
        tmp2 = QString::number(f, 'f', 4).toLatin1();
        tmp  = tmp + "<FREQ:" + QString::number(tmp2.size()).toLatin1() + ">" + tmp2;

        // date
        // in SQL log, date is of format MMddyyyy; need yyyyMMdd for adif
        tmp2 = m.record(i).value(SQL_COL_DATE).toByteArray().right(4) +
               m.record(i).value(SQL_COL_DATE).toByteArray().left(2)
               + m.record(i).value(SQL_COL_DATE).toByteArray().mid(2, 2);
        tmp = tmp + "<QSO_DATE:8>" + tmp2;

        // time
        tmp = tmp + "<TIME_ON:4>" + m.record(i).value(SQL_COL_TIME).toByteArray();

        int rsti = 0;
        switch (m.record(i).value(SQL_COL_MODE).toInt()) {
        case RIG_MODE_LSB: case RIG_MODE_USB:
            tmp  = tmp + "<MODE:3>SSB";
            rsti = 1;
            break;
        case RIG_MODE_CW: case RIG_MODE_CWR:
            tmp = tmp + "<MODE:2>CW";
            break;
        case RIG_MODE_FM:
            tmp  = tmp + "<MODE:2>FM";
            rsti = 1;
            break;
        case RIG_MODE_AM:
            tmp  = tmp + "<MODE:2>AM";
            rsti = 1;
            break;
        case RIG_MODE_RTTY: case RIG_MODE_RTTYR:
            tmp = tmp + "<MODE:4>RTTY";
            break;
        default:
            tmp = tmp + "<MODE:2>CW";
            break;
        }
        // RS(T). If not in log, use 59/599
        if (rsti == 0) {
            // RST for CW, RTTY
            if (contest->rstField() == -1) {
                tmp = tmp + "<RST_SENT:3>599<RST_RCVD:3>599";
            } else {
                QByteArray tmp3;
                switch (contest->rstField()) {
                case 0:
                    tmp2 = m.record(i).value(SQL_COL_SNT1).toByteArray();
                    tmp3 = m.record(i).value(SQL_COL_RCV1).toByteArray(); break;
                case 1:
                    tmp2 = m.record(i).value(SQL_COL_SNT2).toByteArray();
                    tmp3 = m.record(i).value(SQL_COL_RCV2).toByteArray(); break;
                case 2:
                    tmp2 = m.record(i).value(SQL_COL_SNT3).toByteArray();
                    tmp3 = m.record(i).value(SQL_COL_RCV3).toByteArray(); break;
                case 3:
                    tmp2 = m.record(i).value(SQL_COL_SNT4).toByteArray();
                    tmp3 = m.record(i).value(SQL_COL_RCV4).toByteArray(); break;
                }
                tmp = tmp + "<RST_SENT:3>" + tmp2.left(3) + "<RST_RCVD:3>" + tmp3.left(3);
            }
        } else {
            // RS for voice modes
            if (contest->rstField() == -1) {
                tmp = tmp + "<RST_SENT:2>59<RST_RCVD:2>59";
            } else {
                QByteArray tmp3;
                switch (contest->rstField()) {
                case 0:
                    tmp2 = m.record(i).value(SQL_COL_SNT1).toByteArray();
                    tmp3 = m.record(i).value(SQL_COL_RCV1).toByteArray(); break;
                case 1:
                    tmp2 = m.record(i).value(SQL_COL_SNT2).toByteArray();
                    tmp3 = m.record(i).value(SQL_COL_RCV2).toByteArray(); break;
                case 2:
                    tmp2 = m.record(i).value(SQL_COL_SNT3).toByteArray();
                    tmp3 = m.record(i).value(SQL_COL_RCV3).toByteArray(); break;
                case 3:
                    tmp2 = m.record(i).value(SQL_COL_SNT4).toByteArray();
                    tmp3 = m.record(i).value(SQL_COL_RCV4).toByteArray(); break;
                }
                tmp = tmp + "<RST_SENT:2>" + tmp2.left(2) + "<RST_RCVD:2>" + tmp3.left(2);
            }
        }

        tmp = tmp + "<eor>\n";
        if (adifFile->write(tmp) == -1) {
            ok = false;
        }
    }
    adifFile->close();
    return(ok);
}

/*!
   Cabrillo export

 */
void Log::exportCabrillo(QFile *cbrFile,QString call,QString snt_exch1,QString snt_exch2,QString snt_exch3,QString snt_exch4) const
{
    QSqlQueryModel m;
    m.setQuery("SELECT * FROM log  where valid=1", db);

    while (m.canFetchMore()) {
        m.fetchMore();
    }
    if (m.rowCount() == 0) return;  // nothing to do

    // determine max field widths for sent/received data
    int sfw[MAX_EXCH_FIELDS], rfw[MAX_EXCH_FIELDS];
    for (int i = 0; i < MAX_EXCH_FIELDS; i++) {
        sfw[i] = 0.;
        rfw[i] = 0;
    }
    QByteArray snt[MAX_EXCH_FIELDS];
    for (int i = 0; i < m.rowCount(); i++) {
        // for sent exchange, if log field is empty use config file value
        snt[0]=m.record(i).value(SQL_COL_SNT1).toByteArray();
        if (snt[0].isEmpty()) snt[0]=snt_exch1.toLatin1();
        int j = snt[0].size();
        if (j > sfw[0]) sfw[0] = j;

        snt[1]=m.record(i).value(SQL_COL_SNT2).toByteArray();
        if (snt[1].isEmpty()) snt[1]=snt_exch2.toLatin1();
        j = snt[1].size();
        if (j > sfw[1]) sfw[1] = j;

        snt[2]=m.record(i).value(SQL_COL_SNT3).toByteArray();
        if (snt[2].isEmpty()) snt[2]=snt_exch3.toLatin1();
        j = snt[2].size();
        if (j > sfw[2]) sfw[2] = j;

        snt[3]=m.record(i).value(SQL_COL_SNT4).toByteArray();
        if (snt[3].isEmpty()) snt[3]=snt_exch4.toLatin1();
        j = snt[3].size();
        if (j > sfw[3]) sfw[3] = j;

        j = m.record(i).value(SQL_COL_RCV1).toByteArray().size();
        if (j > rfw[0]) rfw[0] = j;
        j = m.record(i).value(SQL_COL_RCV2).toByteArray().size();
        if (j > rfw[1]) rfw[1] = j;
        j = m.record(i).value(SQL_COL_RCV3).toByteArray().size();
        if (j > rfw[2]) rfw[2] = j;
        j = m.record(i).value(SQL_COL_RCV4).toByteArray().size();
    }

    for (int i = 0; i < m.rowCount(); i++) {
        // skip qsos marked as invalid
        if (!m.record(i).value(SQL_COL_VALID).toBool()) continue;

        // for sent exchange, if log field is empty use config file value
        snt[0]=m.record(i).value(SQL_COL_SNT1).toByteArray();
        if (snt[0].isEmpty()) snt[0]=snt_exch1.toLatin1();

        snt[1]=m.record(i).value(SQL_COL_SNT2).toByteArray();
        if (snt[1].isEmpty()) snt[1]=snt_exch2.toLatin1();

        snt[2]=m.record(i).value(SQL_COL_SNT3).toByteArray();
        if (snt[2].isEmpty()) snt[2]=snt_exch3.toLatin1();

        snt[3]=m.record(i).value(SQL_COL_SNT4).toByteArray();
        if (snt[3].isEmpty()) snt[3]=snt_exch4.toLatin1();

        QString tmp;
        tmp = "QSO: ";
        // for VHF+ only band is given in freq column
        int khz = qRound(m.record(i).value(SQL_COL_FREQ).toDouble() / 1000.0);
        QString tmp2;
        if (khz>30000) {
            tmp2 = QString::number(khz/1000);
        } else {
            tmp2 = QString::number(khz);
        }
        for (int j = 0; j < (5 - tmp2.size()); j++) {
            tmp = tmp + " ";
        }
        tmp = tmp + tmp2;
        switch (m.record(i).value(SQL_COL_MODE).toInt()) {
        case RIG_MODE_CW:
        case RIG_MODE_CWR:
            tmp = tmp + " CW ";
            break;
        case RIG_MODE_USB:
        case RIG_MODE_LSB:
        case RIG_MODE_AM:
        case RIG_MODE_FM:
            tmp = tmp + " PH ";
            break;
        case RIG_MODE_RTTY:
        case RIG_MODE_RTTYR:
            tmp = tmp + " RY ";
            break;
        default:
            tmp = tmp + " CW ";
            break;
        }

        // in SQL log, date is of format MMddyyyy
        tmp2 = m.record(i).value(SQL_COL_DATE).toByteArray();
        tmp  = tmp + tmp2.right(4) + "-" + tmp2.left(2) + "-" + tmp2.mid(2, 2);
        tmp  = tmp + " " + m.record(i).value(SQL_COL_TIME).toByteArray();
        tmp  = tmp + " " + call;
        int n = 11 - call.size();
        for (int j = 0; j < n; j++) tmp.append(" ");
        for (int j = 0; j < contest->nExchange(); j++) {
            QByteArray s;
            s.clear();
            switch (j) {
            case 0:
                s=snt[0];
                s = s.leftJustified(sfw[0], ' ');
                break;
            case 1:
                s=snt[1];
                s = s.leftJustified(sfw[1], ' ');
                break;
            case 2:
                s=snt[2];
                s = s.leftJustified(sfw[2], ' ');
                break;
            case 3:
                s=snt[3];
                s = s.leftJustified(sfw[3], ' ');
                break;
            }
            tmp = tmp + s + " ";
        }
        tmp = tmp + m.record(i).value(SQL_COL_CALL).toByteArray();
        n   = 11 - m.record(i).value(SQL_COL_CALL).toByteArray().size();
        for (int j = 0; j < n; j++) tmp.append(" ");
        for (int j = 0; j < contest->nExchange(); j++) {
            switch (j) {
            case 0:
                tmp = tmp + m.record(i).value(SQL_COL_RCV1).toByteArray().leftJustified(rfw[0], ' '); break;
            case 1:
                tmp = tmp + m.record(i).value(SQL_COL_RCV2).toByteArray().leftJustified(rfw[1], ' '); break;
            case 2:
                tmp = tmp + m.record(i).value(SQL_COL_RCV3).toByteArray().leftJustified(rfw[2], ' '); break;
            case 3:
                tmp = tmp + m.record(i).value(SQL_COL_RCV4).toByteArray().leftJustified(rfw[3], ' '); break;
            }
            tmp = tmp + " ";
        }
        tmp = tmp + "\n";
        cbrFile->write(tmp.toLatin1());
    }
    cbrFile->write("END-OF-LOG:\n");
    cbrFile->close();
}

/*!
   Dupe checking subroutine.

   - qso->worked returns with bits set according to bands worked
   - DupeCheckingEveryBand controls whether calls can be worked on every band or only once.
*/
bool Log::isDupe(Qso *qso, bool DupeCheckingEveryBand, bool FillWorked) const
{
    // if called with no call, abort
    if (qso->call.isEmpty()) return false;

    // only keep dupes on known bands
    if (qso->band==BAND_NONE) return false;

    bool dupe = false;
    qso->worked = 0;
    qso->prefill.clear();
    QSqlQueryModel m;

    // call can only be worked once on any band
    if (!DupeCheckingEveryBand) {
        if (!settings.value(c_multimode,c_multimode_def).toBool()) {
            m.setQuery("SELECT * FROM log WHERE valid=1 and CALL LIKE '" + qso->call + "'", db);
        } else {
            // multimode: if voice mode, check for dupe with LSB, USB, or FM
            // @todo this would be much simpler if modeType was stored in log, but this requires changing the so2sdr log
            // format
            if (qso->modeType==PhoneType) {
                m.setQuery("SELECT * FROM log WHERE valid=1 and CALL LIKE '" + qso->call + "' and  \
                       (MODE="+QByteArray::number(RIG_MODE_AM)+" OR MODE="+QByteArray::number(RIG_MODE_LSB)+" OR MODE="+ \
                        QByteArray::number(RIG_MODE_USB)+")'",db);
           } else if (qso->modeType==CWType) {
                m.setQuery("SELECT * FROM log WHERE valid=1 and CALL LIKE '" + qso->call + "' and  \
                       (MODE="+QByteArray::number(RIG_MODE_CW)+" OR MODE="+QByteArray::number(RIG_MODE_CWR)+")'",db);
            }

        }

        while (m.canFetchMore()) {
            m.fetchMore();
        }
        if (m.rowCount()) {
            dupe = true;
            if (FillWorked) {
                // mult not needed on any band
                qso->worked = 63;
            }
        }
    } else {
        // if mobile station, check for mobile dupe option. In this
        // case, count dupe only if exchange is identical
        QString query="SELECT * FROM log WHERE valid=1 and call like '" + qso->call + "' AND band=" + QString::number(qso->band);

        if ((qso->isMobile || qso->isRover) && csettings.value(c_mobile_dupes,c_mobile_dupes_def).toBool()) {
            QString exch=qso->rcv_exch[csettings.value(c_mobile_dupes_col,c_mobile_dupes_col_def).toInt()-1];
            // if exchange not entered, can't determine dupe status yet
            if (exch.isEmpty()) {
                return(false);
            }
            query=query+ " AND ";
            switch (csettings.value(c_mobile_dupes_col,c_mobile_dupes_col_def).toInt()) {
            case 1:
                query=query+"rcv1 like '"+exch+"'";
                break;
            case 2:
                query=query+"rcv2 like '"+exch+"'";
                break;
            case 3:
                query=query+"rcv3 like '"+exch+"'";
                break;
            case 4:
                query=query+"rcv4 like '"+exch+"'";
                break;
            default:
                return(false);
            }
        }
        m.setQuery(query, db);
        m.query().exec();
        while (m.canFetchMore()) {
            m.fetchMore();
        }
        if (m.rowCount() > 1) {  // it's a dupe if more than one matching qso found
            dupe=true;
        }
        if (FillWorked) {
            m.setQuery("SELECT * FROM log WHERE valid=1 and CALL LIKE '" + qso->call + "'", db);
            m.query().exec();
            while (m.canFetchMore()) {
                m.fetchMore();
            }
            for (int i = 0; i < m.rowCount(); i++) {
                qso->worked += bits[m.record(i).value(SQL_COL_BAND).toInt()];
            }
        }
    }
    // if a dupe, set zero pts
    if (dupe) qso->pts = 0;
    return(dupe);
}


/*!
   qso number sent for last qso in log
 */
int Log::lastNr() const
{
    QSqlQueryModel m;
    m.setQuery("SELECT * FROM log where valid=1", db);

    while (m.canFetchMore()) {
        m.fetchMore();
    }
    if (m.rowCount()) {
        QByteArray snt[MAX_EXCH_FIELDS];
        snt[0] = m.record(m.rowCount() - 1).value(SQL_COL_SNT1).toByteArray();
        snt[1] = m.record(m.rowCount() - 1).value(SQL_COL_SNT2).toByteArray();
        snt[2] = m.record(m.rowCount() - 1).value(SQL_COL_SNT3).toByteArray();
        snt[3] = m.record(m.rowCount() - 1).value(SQL_COL_SNT4).toByteArray();
        bool ok = false;
        int  nr = snt[contest->numberField()].toInt(&ok, 10);
        if (!ok) nr = 0;
        return(nr);
    } else {
        return(0);
    }
}

/*!
slot for mobile dupe checking. Gets called by exchange validator if
exchange if ok. In this case, modify qso dupes status based on whether this
is a new mult or not
*/
void Log::mobileDupeCheck(Qso *qso)
{
    qso->dupe=isDupe(qso,true,false);
}

/*!
   open log file on disk. If already exists, open as append.

   s is pointer to contest settings file
 */
bool Log::openLogFile(QString fname,bool clear)
{
    Q_UNUSED(clear);
    if (fname.isEmpty())
        return(false);

    fname = fname.remove(".cfg") + ".log";
    db.setDatabaseName(fname);
    if (!db.open()) {
        return(false);
    }
    QSqlQuery query(db);

    if (!query.exec("create table if not exists log (nr int primary key,time text,freq double,call text,band int,date text,mode int,snt1 text,snt2 text,snt3 text,snt4 text,rcv1 text,rcv2 text,rcv3 text,rcv4 text,pts int,valid boolean)")) {
        return(false);
    }
    if (model) delete model;
    model = new tableModel(this,db);
    connect(model, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SIGNAL(dataChanged(QModelIndex, QModelIndex)));
    connect(model,SIGNAL(beforeUpdate(int,QSqlRecord&)),this,SLOT(finishEdit(int,QSqlRecord&)));
    model->setTable("log");
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->select();
    while (model->canFetchMore()) {
        model->fetchMore();
    }
    return true;
}

void Log::finishEdit(int row, QSqlRecord &r)
{
    Q_UNUSED(row)
    emit(logEditDone(origEditRecord,r));
}

/*! show the qso points column on screen

   in contests where every qso is worth the same number of points, don't show it
 */
bool Log::showQsoPtsField() const
{
    return(contest->showQsoPtsField());
}

/*!
 * \brief offTime Calculates the number of minutes of off-time taken since start.
 * \param minOffTime Minimum offtime length in minutes.
 * \param start Start of contest. QSO's must be after this time to be valid.
 * \param end End of contest. QSO's must be before this time to be valid.
 * \return string giving off time
 */
QString Log::offTime(int minOffTime,QDateTime start,QDateTime end)
{
    QSqlQueryModel m;
    m.setQuery("SELECT * FROM log where valid=1", db);

    while (m.canFetchMore()) {
        m.fetchMore();
    }
    if (m.rowCount() == 0) {
        return "Off 00:00/00:00";  // nothing to do
    }
    // make sure times are rounded to minutes
    start=start.addSecs(-start.time().second());
    end=end.addSecs(-end.time().second()+60); // need to add 1 minute since end is time of last possible qso

    int totOffTime=0;
    QDateTime lastQsoTime=start;
    for (int i = 0; i < m.rowCount(); i++) {
        int yr=m.record(i).value(SQL_COL_DATE).toByteArray().right(4).toInt();
        int mon=m.record(i).value(SQL_COL_DATE).toByteArray().left(2).toInt();
        int d=m.record(i).value(SQL_COL_DATE).toByteArray().mid(2,2).toInt();
        int hr=m.record(i).value(SQL_COL_TIME).toByteArray().left(2).toInt();
        int min=m.record(i).value(SQL_COL_TIME).toByteArray().right(2).toInt();
        QDateTime qsoTime=QDateTime(QDate(yr,mon,d),QTime(hr,min),Qt::UTC);

        if (qsoTime<start || qsoTime>end) continue; // qso not during contest

        // calculate time difference from last qso
        int diff=lastQsoTime.secsTo(qsoTime);
        if (diff<0) continue; // probably means log is out of order, this will fail!

        diff/=60;
        diff--;
        if (diff>=minOffTime) {
            totOffTime+=diff;
        }
        lastQsoTime=qsoTime;
    }
    // add any additional off time taken up to current time
    int extra=0;
    if (lastQsoTime<end) {
        QDateTime current=QDateTime::currentDateTime();
        if (end<current) {
            current=end;
        }
        // instead of current time, want only current minute
        current=current.addSecs(-current.time().second());
        extra=lastQsoTime.secsTo(current);
        extra/=60;
        extra--;
        if (extra<0) extra=0;
        if (extra>=minOffTime) totOffTime+=extra;
    }

    if (totOffTime>=6039) {
        return "Off 00:00/99:99";
    } else {
        int ehr=extra/60;
        int emin=extra-ehr*60;
        int hr=totOffTime/60;
        int min=totOffTime-hr*60;
        QString tmp="Off "+QString("%1").arg(QString::number(ehr), 2, QChar('0'))+
                ":"+QString("%1").arg(QString::number(emin), 2, QChar('0'));
        tmp=tmp+"/";
        tmp=tmp+QString("%1").arg(QString::number(hr), 2, QChar('0'))+
                ":"+QString("%1").arg(QString::number(min), 2, QChar('0'));
        return tmp;
    }
}

/*!
   add a new qso to the log
*/
void Log::addQso(Qso *qso)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO log (nr,time,freq,call,band,date,mode,snt1,snt2,snt3,snt4,rcv1,rcv2,rcv3,rcv4,pts,valid)"
                    "VALUES (:nr,:time,:freq,:call,:band,:date,:mode,:snt1,:snt2,:snt3,:snt4,:rcv1,:rcv2,:rcv3,:rcv4,:pts,:valid)");
    query.bindValue(":nr",model->rowCount()+1);
    query.bindValue(":time",qso->time.toUTC().toString("hhmm"));
    query.bindValue(":freq",qso->freq);
    query.bindValue(":call",qso->call);
    query.bindValue(":band",qso->band);
    query.bindValue(":date",qso->time.toUTC().toString("MMddyyyy"));
    query.bindValue(":mode",qso->mode);
    if (contest->nExchange()>0) {
        query.bindValue(":snt1",qso->snt_exch[0]);
        query.bindValue(":rcv1",qso->rcv_exch[0]);
    } else {
        query.bindValue(":snt1",QVariant(QVariant::String));
        query.bindValue(":rcv1",QVariant(QVariant::String));
    }
    if (contest->nExchange()>1) {
        query.bindValue(":snt2",qso->snt_exch[1]);
        query.bindValue(":rcv2",qso->rcv_exch[1]);
    } else {
        query.bindValue(":snt2",QVariant(QVariant::String));
        query.bindValue(":rcv2",QVariant(QVariant::String));
    }
    if (contest->nExchange()>2) {
        query.bindValue(":snt3",qso->snt_exch[2]);
        query.bindValue(":rcv3",qso->rcv_exch[2]);
    } else {
        query.bindValue(":snt3",QVariant(QVariant::String));
        query.bindValue(":rcv3",QVariant(QVariant::String));
    }
    if (contest->nExchange()>3) {
        query.bindValue(":snt4",qso->snt_exch[3]);
        query.bindValue(":rcv4",qso->rcv_exch[3]);
    } else {
        query.bindValue(":snt4",QVariant(QVariant::String));
        query.bindValue(":rcv4",QVariant(QVariant::String));
    }
    query.bindValue(":pts",qso->pts);
    query.bindValue(":valid",qso->valid);
    query.exec();
    model->select();
    while (model->canFetchMore()) {
        model->fetchMore();
    }
    if (csettings.value(c_historyupdate,c_historyupdate_def).toBool()) {
        emit(addQsoHistory(qso));
    }
    contest->addQso(qso);
    if (!qso->dupe && qso->valid) qsoCnt[qso->band]++;
}

/*! import a Cabrillo log
   note: the number of exchange fields must be correct for this contest, otherwise
    bad things happen.
*/
void Log::importCabrillo(QString cabFile)
{
    int n=0;
    for (int i=0;i<N_BANDS;i++) n+=qsoCnt[i];
    if (n) {
        emit(errorMessage("ERROR: log must be empty to import cabrillo"));
        return;
    }

    // open the file
    QFile file(cabFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    // count number of lines in file to get estimate of time needed to read
    int maxLines = 0;
    while (!file.atEnd()) {
        QString buffer;
        buffer = file.readLine();
        maxLines++;
    }
    emit(progressMax(maxLines));
    file.close();
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QDataStream s(&file);
    int         cnt = 0;
    for (int i = 0; i < N_BANDS; i++) qsoCnt[i] = 0;
    Qso qso (contest->nExchange());
    contest->zeroScore();
    model->database().transaction();

    QString buffer;
    QStringList field;
    QSqlRecord  newqso;
    newqso.append(QSqlField("nr", QVariant::Int));
    newqso.append(QSqlField("time", QVariant::String));
    newqso.append(QSqlField("freq", QVariant::Double));
    newqso.append(QSqlField("call", QVariant::String));
    newqso.append(QSqlField("band", QVariant::Int));
    newqso.append(QSqlField("date", QVariant::String));
    newqso.append(QSqlField("mode", QVariant::Int));
    newqso.append(QSqlField("snt1", QVariant::String));
    newqso.append(QSqlField("snt2", QVariant::String));
    newqso.append(QSqlField("snt3", QVariant::String));
    newqso.append(QSqlField("snt4", QVariant::String));
    newqso.append(QSqlField("rcv1", QVariant::String));
    newqso.append(QSqlField("rcv2", QVariant::String));
    newqso.append(QSqlField("rcv3", QVariant::String));
    newqso.append(QSqlField("rcv4", QVariant::String));
    newqso.append(QSqlField("pts", QVariant::Int));
    newqso.append(QSqlField("valid", QVariant::Int));
    for (int i = 0; i < SQL_N_COL; i++) newqso.setGenerated(i, true);

    while (!file.atEnd()) {
        buffer = file.readLine();
        buffer = buffer.trimmed();
        if (!buffer.contains("QSO:") && !buffer.contains("qso:")) {
            continue;  // ignore header data
        }
        field = buffer.split(" ", QString::SkipEmptyParts);
        int         nf    = field.size();

        // Field1 = frequency in KHz
        double f = field.at(1).toDouble() * 1000;
        newqso.setValue(SQL_COL_FREQ, QVariant(f));

        int b = getBand(f);
        newqso.setValue(SQL_COL_BAND, QVariant(b));
        qso.band = b;

        // Field2 = mode
        int m;
        if (field.at(2).toUpper() == "CW") {
            m = RIG_MODE_CW;
        } else if (field.at(2).toUpper() == "RY") {
            m = RIG_MODE_RTTY;
        } else if (field.at(2).toUpper() == "PH") {
            // Cabrillo doesn't store LSB/USB?
            if (f < 14000000) {
                m = RIG_MODE_LSB;
            } else {
                m = RIG_MODE_USB;
            }
        } else if (field.at(2).toUpper() == "FM") {
            m = RIG_MODE_FM;
        } else {
            m = RIG_MODE_CW;
        }
        qso.mode = (rmode_t) m;
        qso.modeType=getModeType(qso.mode);
        newqso.setValue(SQL_COL_MODE, QVariant(m));
        cnt++;
        newqso.setValue(SQL_COL_NR, QVariant(cnt));

        // Field3 = date
        QDateTime time;
        time.setTimeSpec(Qt::UTC);
        int       y = field.at(3).mid(0, 4).toInt();
        m = field[3].mid(5, 2).toInt();
        int       d = field.at(3).mid(8, 2).toInt();
        time.setDate(QDate(y, m, d));
        newqso.setValue(SQL_COL_DATE, QVariant(time.toString("MMddyyyy").toLatin1()));

        // Field4=time
        newqso.setValue(SQL_COL_TIME, QVariant(field.at(4)));

        // Field5=station call. ignore this

        // Field6+
        // next fields are sent exchange
        int i, j;
        for (i = 6, j = 0; i < (6 + contest->nExchange()); i++, j++) {
            switch (j) {
            case 0:
                newqso.setValue(SQL_COL_SNT1, QVariant(field.at(i).toUpper()));
                qso.snt_exch[0]=field.at(i).toLatin1().toUpper();
                break;
            case 1:
                newqso.setValue(SQL_COL_SNT2, QVariant(field.at(i).toUpper()));
                qso.snt_exch[1]=field.at(i).toLatin1().toUpper();
                break;
            case 2:
                newqso.setValue(SQL_COL_SNT3, QVariant(field.at(i).toUpper()));
                qso.snt_exch[2]=field.at(i).toLatin1().toUpper();
                break;
            case 3:
                newqso.setValue(SQL_COL_SNT4, QVariant(field.at(i).toUpper()));
                qso.snt_exch[3]=field.at(i).toLatin1().toUpper();
                break;
            }
        }

        // next field=call worked
        newqso.setValue(SQL_COL_CALL, QVariant(field.at(6 + contest->nExchange()).toUpper()));
        qso.call = field.at(6 + contest->nExchange()).toLatin1().toUpper();
        bool bb;
        qso.country = idPfx(&qso, bb);

        // next received report
        qso.exch.clear();
        for (i = 7 + contest->nExchange(), j = 0; i < (7 + 2 * contest->nExchange()); i++, j++) {
            // some fields may be empty (flaw in Cabrillo spec?)
            if (i >= nf) {
                continue;
            }
            switch (j) {
                case 0:
                newqso.setValue(SQL_COL_RCV1, QVariant(field.at(i).toUpper()));
                qso.exch = qso.exch + field.at(i).toLatin1().toUpper();
                qso.rcv_exch[0]=field.at(i).toLatin1().toUpper();
                break;
            case 1:
                newqso.setValue(SQL_COL_RCV2, QVariant(field.at(i).toUpper()));
                qso.exch = qso.exch + " " + field.at(i).toLatin1().toUpper();
                qso.rcv_exch[1]=field.at(i).toLatin1().toUpper();
                break;
            case 2:
                newqso.setValue(SQL_COL_RCV3, QVariant(field.at(i).toUpper()));
                qso.exch = qso.exch + " " + field.at(i).toLatin1().toUpper();
                qso.rcv_exch[2]=field.at(i).toLatin1().toUpper();
                break;
            case 3:
                newqso.setValue(SQL_COL_RCV4, QVariant(field.at(i).toUpper()));
                qso.exch = qso.exch + " " + field.at(i).toLatin1().toUpper();
                qso.rcv_exch[3]=field.at(i).toLatin1().toUpper();
                break;
            }
        }
        newqso.setValue(SQL_COL_PTS, QVariant(qso.pts));
        newqso.setValue(SQL_COL_VALID, QVariant(true)); // set to valid
        contest->addQso(&qso);
        model->insertRecord(-1, newqso);
        emit(progressCnt(cnt));
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }
    model->submitAll();
    model->database().commit();
    rescore();

    while (model->canFetchMore()) {
        model->fetchMore();
    }
    emit(progressCnt(maxLines));
}

int Log::rowCount() const
{
    return model->rowCount();
}

/*! log search
 * returns false if no matches found, otherwise true and
 * list of rows in the log
 */
bool Log::logSearch(QByteArray searchFrag)
{
    if (searchFrag.size()<2) return false;
    model->setFilter("CALL LIKE '%"+searchFrag+"%'");
    model->select();
    while (model->canFetchMore()) {
        model->fetchMore();
    }
    if (model->rowCount()==0) {
        model->setFilter("");
        model->select();
        while (model->canFetchMore())
            model->fetchMore();
        return false;
    } else {
        // save a list of rows found by the search
        searchList.clear();
        for (int i=0;i<model->rowCount();i++) {
            searchList.append(model->record(i).value(SQL_COL_NR).toInt()-1);
        }
    }
    return true;
}

void Log::logSearchClear()
{
    model->setFilter("");
    model->select();
    while (model->canFetchMore())
        model->fetchMore();
}


/*!
   rescore and redupe

 */
void Log::rescore()
{
    Qso tmpqso(contest->nExchange());
    QSqlQueryModel m;
    m.setQuery("SELECT * FROM log", db);
    while (m.canFetchMore()) {
        m.fetchMore();
    }
    bool b;
    QList<QByteArray> dupes[N_BANDS];
    contest->zeroScore();
    for (int i = 0; i < N_BANDS; i++) {
        qsoCnt[i] = 0;
        dupes[i].clear();
    }
    for (int i = 0; i < m.rowCount(); i++) {
        tmpqso.call    = m.record(i).value("call").toString().toLatin1();
        // run prefix check on call: need to check for /MM, etc
        tmpqso.country = cty->idPfx(&tmpqso, b);
        tmpqso.exch.clear();
        QByteArray tmp[4];
        tmp[0] = m.record(i).value("rcv1").toString().toLatin1();
        tmp[1] = m.record(i).value("rcv2").toString().toLatin1();
        tmp[2] = m.record(i).value("rcv3").toString().toLatin1();
        tmp[3] = m.record(i).value("rcv4").toString().toLatin1();
        tmpqso.nr=m.record(i).value("nr").toInt();

        for (int j = 0; j < contest->nExchange(); j++) {
            tmpqso.exch = tmpqso.exch + tmp[j] + " ";
        }
        tmpqso.mode = (rmode_t) m.record(i).value("mode").toInt();
        tmpqso.modeType = getModeType(tmpqso.mode);
        tmpqso.band = m.record(i).value("band").toInt();
        tmpqso.pts  = m.record(i).value("pts").toInt();

        // valid can be changed to ways:
        // 1) when user unchecks checkbox
        // 2) if program can't parse the exchange
        //
        // first check for user changing check status
        bool userValid=m.record(i).value("valid").toBool();
        tmpqso.valid=userValid;

        // dupe check
        // qsos marked invalid are excluded from log and dupe check
        tmpqso.dupe = false;
        if (tmpqso.valid) {
            if (csettings.value(c_dupemode,c_dupemode_def).toInt()!=NO_DUPE_CHECKING) {
                // can work station on other bands, just check this one
                QByteArray check=tmpqso.call;
                // multi-mode contest: append a mode index to the call
                if (csettings.value(c_multimode,c_multimode_def).toBool()) {
                    check=check+QByteArray::number((int)tmpqso.modeType);
                }
                if (contest->dupeCheckingByBand()) {
                    if (dupes[tmpqso.band].contains(check)) {
                        tmpqso.dupe = true;
                    }
                } else {
                    // qsos count only once on any band
                    for (int j = 0; j < N_BANDS; j++) {
                        if (dupes[j].contains(check)) {
                            tmpqso.dupe = true;
                        }
                    }
                }
                dupes[tmpqso.band].append(check);
            }
        } else {
            tmpqso.pts=0;
            tmpqso.mult[0]=-1;
            tmpqso.mult[1]=-1;
            tmpqso.newmult[0]=-1;
            tmpqso.newmult[1]=-1;
        }
        // next check exchange
        // in the case of mobiles, this might change dupe status!
        bool exchValid=contest->validateExchange(&tmpqso);
        tmpqso.valid=userValid & exchValid;

        if (!tmpqso.dupe && tmpqso.valid) qsoCnt[tmpqso.band]++;
        if (!tmpqso.valid || tmpqso.dupe) {
            tmpqso.pts=0;
            tmpqso.mult[0]=-1;
            tmpqso.mult[1]=-1;
            tmpqso.newmult[0]=-1;
            tmpqso.newmult[1]=-1;
        }
        contest->addQso(&tmpqso);
    }
    while (model->canFetchMore()) {
        model->fetchMore();
    }
}

void Log::updateRecord(QSqlRecord r)
{
    if (!model->setRecord(r.value(SQL_COL_NR).toInt()-1,r)) {
        qDebug("Log::setRecord failed");
    }
    model->submitAll();
    while (model->canFetchMore()) {
        model->fetchMore();
    }
    emit(update());
}

void Log::updateHistory()
{
    QSqlQueryModel log;
    QString query_log = "SELECT call,rcv1,rcv2,rcv3,rcv4 from log where valid=1";

    log.setQuery(query_log, db);
    while (log.canFetchMore()) log.fetchMore();
    emit(progressMax(log.rowCount()));
    Qso tmpqso(contest->nExchange());
    for (int i = 0; i < contest->nExchange(); i++) {
        tmpqso.setExchangeType(i, contest->exchType(i));
    }
    for (int row = 0; row < log.rowCount(); row++) {
        tmpqso.call=log.record(row).value("Call").toString().toLatin1();
        for (int i = 0; i < contest->nExchange(); i++) {
            tmpqso.rcv_exch[i]=log.record(row).value(i+1).toString().toLatin1();
        }
        emit(addQsoHistory(&tmpqso));
        emit(progressCnt(row));
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }
    emit(progressCnt(log.rowCount()));
}

void Log::searchPartial(Qso *qso, QByteArray part, QList<QByteArray>& calls, QList<unsigned int>& worked,
                        QList<int>& mult1, QList<int>& mult2)
{
    logSearchClear();

    // query for partial fragment
    QSqlQueryModel m;
    m.setQuery("SELECT * FROM log WHERE VALID=1 AND (CALL LIKE'%" + part + "%' )", db);

    while (m.canFetchMore()) {
        m.fetchMore();
    }
    for (int i = 0; i < m.rowCount(); i++) {
        // if multi-mode contest, check for matching mode
      //  if (csettings.value(c_multimode,c_multimode_def).toBool()) {
       //     if (getModeType((rmode_t)m.record(i).value(SQL_COL_MODE).toInt())!=qso->modeType) {
       //         continue;
       //     }
      //  }
        QByteArray tmp = m.record(i).value(SQL_COL_CALL).toString().toLatin1();

        // special case: ARRL 10M contest: use band slots 4 and 5 for CW/SSB
        // @todo a better way to handle this?
        int ib=m.record(i).value(SQL_COL_BAND).toInt();
        if (contest->contestType()==Arrl10_t) {
            if (getModeType((rmode_t)m.record(i).value(SQL_COL_MODE).toInt())==CWType) ib=4;
            else ib=5;
        }
        // see if this call is already in list
        int j=calls.indexOf(tmp);
        if (j != -1) {
            // add this band

            //worked[j] = worked[j] | bits[m.record(i).value(SQL_COL_BAND).toInt()];
            worked[j] = worked[j] | bits[ib];
        } else {
            // new call fragment, or same call and different mode
            // insert call so list is sorted
            int isrt = 0;
            for (int k = 0; k < calls.size(); k++) {
                if (tmp < calls.at(k)) break;
                isrt++;
            }
            calls.insert(isrt, tmp);
            unsigned int w = bits[ib];
            worked.insert(isrt, w);
            int row = m.record(i).value(SQL_COL_NR).toInt() - 1;
            mult1.insert(isrt, contest->mult(row, 0));
            mult2.insert(isrt, contest->mult(row, 1));
        }
    }

    // if call matches, prefill most recent exchange and mult information from log
    for (int i = m.rowCount() - 1; i >= 0; i--) {
        QByteArray tmp = m.record(i).value(SQL_COL_CALL).toString().toLatin1();
        if (tmp == qso->call) {
            qso->prefill.clear();
            int row = m.record(i).value(SQL_COL_NR).toInt() - 1;
            qso->mult[0] = contest->mult(row, 0);
            qso->mult[1] = contest->mult(row, 1);
            // for contests with RS(T) (always assumed to be first exchange element), make sure
            // filled RS(T) is appropriate for the mode
            if (contest->exchType(0)==RST) {
                switch (qso->modeType) {
                case CWType:case DigiType:
                    qso->prefill="599 ";
                    break;
                case PhoneType:
                    qso->prefill="59 ";
                    break;
                }
                qso->prefill = qso->prefill +
                        m.record(i).value(SQL_COL_RCV2).toString().toLatin1() + " " +
                        m.record(i).value(SQL_COL_RCV3).toString().toLatin1() + " " +
                        m.record(i).value(SQL_COL_RCV4).toString().toLatin1();
            } else {
                qso->prefill = m.record(i).value(SQL_COL_RCV1).toString().toLatin1() + " " +
                        m.record(i).value(SQL_COL_RCV2).toString().toLatin1() + " " +
                        m.record(i).value(SQL_COL_RCV3).toString().toLatin1() + " " +
                        m.record(i).value(SQL_COL_RCV4).toString().toLatin1();
            }
            break;
        }
    }
}

/*!
 * This gets called at the start of a double-click QSO edit
 */
void Log::startQsoEditRow(QModelIndex index)
{
    QSqlRecord rec = model->record(index.row());
    origEditRecord = rec;
}

/*!
  This gets called before a detailed QSO edit (Ctrl-E)
*/
void Log::startDetailedQsoEditRow(QModelIndex index)
{
    detail->loadRecord(model->record(index.row()),contest->nExchange());
    detail->show();
    detail->callLineEdit->setFocus();
    detail->callLineEdit->deselect();
    emit(ungrab());
}

void Log::setOrigRecord(QModelIndex index)
{
    origEditRecord=model->record(index.row());
}

void Log::startDetailedEdit()
{
    logdel->startDetailedEdit();
}

int Log::idPfx(Qso *qso, bool &qsy) const
 {
     int pp=cty->idPfx(qso,qsy);
     if (csettings.value(c_contestname,c_contestname_def).toString().toUpper()=="WPX") {
         ((WPX*) contest)->wpxPrefix(qso->call, qso->mult_name);
         qso->isamult[0]=true;
         contest->multIndx(qso);
     }
      if (csettings.value(c_contestname,c_contestname_def).toString().toUpper()=="ARRLJUNE") {
          qso->isamult[0]=true;
          contest->multIndx(qso);
      }
     return pp;
 }

 void Log::setLatLon(double la, double lo)
 {
     lat=la;
     lon=lo;
 }

 void Log::setZoneType(int z)
 {
     contest->setZoneType(z);
 }

 /*!
    Select contest
  */
 void Log::selectContest()
 {
     QByteArray name=csettings.value(c_contestname,c_contestname_def).toString().toUpper().toLatin1();
     QString snt_exch[MAX_EXCH_FIELDS];
     for (int i=0;i<MAX_EXCH_FIELDS;i++) {
         snt_exch[i].clear();
     }
     if (name == "ARRLDX") {
         // from US/VE
         contest = new ARRLDX(true,csettings,settings);
         snt_exch[0] = "RST";
         snt_exch[1]=settings.value(s_state,s_state_def).toString();
     }
     if (name == "ARRLDX-DX") {
         // from DX
         contest = new ARRLDX(false,csettings,settings);
         snt_exch[0] = "RST";
     }
     if (name == "ARRL10") {
         contest = new ARRL10(csettings,settings);
         snt_exch[0] = "RST";
         snt_exch[1]=settings.value(s_state,s_state_def).toString();
     }
     if (name == "ARRL160") {
         contest = new ARRL160(true,csettings,settings);
         snt_exch[0] = "RST";
         snt_exch[1]=settings.value(s_section,s_section_def).toString();
     }
     if (name == "ARRL160-DX") {
         contest = new ARRL160(false,csettings,settings);
         snt_exch[0] = "RST";
     }
     if (name == "ARRLJUNE") {
         contest = new JuneVHF(csettings,settings);
         snt_exch[0] = settings.value(s_grid,s_grid_def).toString();
     }
     if (name == "CQP-CA") {
         contest = new CQP(csettings,settings);
         static_cast<CQP*>(contest)->setWithinState(true);
         snt_exch[0]="#";
     }
     if (name == "CQP") {
         contest = new CQP(csettings,settings);
         static_cast<CQP*>(contest)->setWithinState(false);
         snt_exch[0]="#";
         snt_exch[1]=settings.value(s_state,s_state_def).toString();
     }
     if (name == "CQ160") {
         contest = new CQ160(csettings,settings);
         snt_exch[0] = "RST";
         snt_exch[1]=settings.value(s_state,s_state_def).toString();
     }
     if (name == "CQWW") {
         contest = new CQWW(csettings,settings);
         snt_exch[0] = "RST";
         snt_exch[1]=settings.value(s_cqzone,s_cqzone_def).toString();
     }
     if (name == "CWOPS") {
         contest = new Cwops(csettings,settings);
         snt_exch[0]=settings.value(s_name,s_name_def).toString();
     }
     if (name == "DXPED") {
         contest = new Dxped(csettings,settings);
         snt_exch[0] = "RST";
     }
     if (name == "FD") {
         contest = new FD(csettings,settings);
         snt_exch[1]=settings.value(s_section,s_section_def).toString();
     }
     if (name == "IARU") {
         contest = new IARU(csettings,settings);
         snt_exch[0] = "RST";
         snt_exch[1]=settings.value(s_ituzone,s_ituzone_def).toString();
     }
     if (name == "KQP-KS") {
         contest = new KQP(csettings,settings);
         static_cast<KQP*>(contest)->setWithinState(true);
         snt_exch[0]="RST";
     }
     if (name == "KQP") {
         contest = new KQP(csettings,settings);
         static_cast<KQP*>(contest)->setWithinState(false);
         snt_exch[0]="RST";
         snt_exch[1]=settings.value(s_state,s_state_def).toString();
     }
     if (name == "MSQP-MS") {
         contest = new MSQP(csettings,settings);
         static_cast<MSQP*>(contest)->setWithinState(true);
         snt_exch[0]="RST";
     }
     if (name == "MSQP") {
         contest = new MSQP(csettings,settings);
         static_cast<MSQP*>(contest)->setWithinState(false);
         snt_exch[0]="RST";
         snt_exch[1]=settings.value(s_state,s_state_def).toString();
     }
     if (name == "NAQP") {
         contest = new Naqp(csettings,settings);
         snt_exch[0]=settings.value(s_name,s_name_def).toString();
         snt_exch[1]=settings.value(s_state,s_state_def).toString();
     }
     if (name == "SPRINT") {
         contest = new Sprint(csettings,settings);
         snt_exch[0]="#";
         snt_exch[1]=settings.value(s_name,s_name_def).toString();
         snt_exch[2]=settings.value(s_state,s_state_def).toString();
     }
     if (name == "STEW") {
         contest = new Stew(csettings,settings);
         snt_exch[0]=settings.value(s_grid,s_grid_def).toString();
     }
     if (name == "SWEEPSTAKES") {
         contest = new Sweepstakes(csettings,settings);
         snt_exch[0]="#";
         snt_exch[3]=settings.value(s_section,s_section_def).toString();
     }
     if (name == "WPX") {
         contest = new WPX(csettings,settings);
         snt_exch[0] = "RST";
         snt_exch[1] = "#";
     }
     if (name == "PAQP-PA") {
         contest = new PAQP(csettings,settings);
         static_cast<PAQP*>(contest)->setWithinState(true);
         snt_exch[0]="#";
     }
     if (name == "PAQP") {
         contest = new PAQP(csettings,settings);
         static_cast<PAQP*>(contest)->setWithinState(false);
         snt_exch[0]="#";
         snt_exch[1]=settings.value(s_state,s_state_def).toString();
     }
     if (contest) {
         int sz=csettings.beginReadArray(c_qso_type1);
         for (int i=0;i<sz;i++) {
             csettings.setArrayIndex(i);
             QByteArray tmp=csettings.value("pfx","").toByteArray();
             contest->addQsoType(tmp,0);
         }
         csettings.endArray();
         sz=csettings.beginReadArray(c_qso_type2);
         for (int i=0;i<sz;i++) {
             csettings.setArrayIndex(i);
             QByteArray tmp=csettings.value("pfx","").toByteArray();
             contest->addQsoType(tmp,1);
         }
         csettings.endArray();
         contest->setContestName(name);
         logdel=new logDelegate(this,*contest,&logSearchFlag,&searchList);
         connect(logdel,SIGNAL(setOrigRecord(QModelIndex)),this,SLOT(setOrigRecord(QModelIndex)));
         connect(logdel,SIGNAL(startLogEdit()),this,SIGNAL(startLogEdit()));
         connect(logdel,SIGNAL(editLogRow(QModelIndex)),this,SLOT(startQsoEditRow(QModelIndex)));
         connect(logdel,SIGNAL(editLogRowDetail(QModelIndex)),this,SLOT(startDetailedQsoEditRow(QModelIndex)));
         connect(logdel,SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),this,SIGNAL(update()));
         connect(contest,SIGNAL(mobileDupeCheck(Qso*)),this,SLOT(mobileDupeCheck(Qso*)));
         connect(contest,SIGNAL(clearDupe()),this,SIGNAL(clearDupe()));
         cty->initialize(lat,lon,contest->zoneType());
         contest->initialize(cty);
         Qso  tmp(2);
         tmp.call = settings.value(s_call,s_call_def).toString().toLatin1();
         bool b;
         contest->setCountry(idPfx(&tmp, b));
         contest->setContinent(tmp.continent);
     }
 }

 /*!
  * \brief Log::initializeContest
  * further initialization of contests. This must be called after the Log constructor, so
  * that signal/slot connections are in place
  */
 void Log::initializeContest()
 {
     QByteArray name=csettings.value(c_contestname,c_contestname_def).toString().toUpper().toLatin1();
     if (name == "CWOPS" || name == "WPX") {
         emit(multByBandEnabled(false));
     }
     if (zoneType() == 0) {
         setMyZone(settings.value(s_cqzone,s_cqzone_def).toInt());
     } else {
         setMyZone(settings.value(s_ituzone,s_ituzone_def).toInt());
     }
 }

 void Log::setMyZone(int zone)
 {
     contest->setMyZone(zone);
 }

 void Log::setCountry(int c)
 {
     contest->setCountry(c);
 }

 void Log::setContinent(Cont c)
 {
     contest->setContinent(c);
 }
