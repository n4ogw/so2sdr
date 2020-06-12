/*! Copyright 2010-2020 rec. Torsten Clay N4OGW

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
#include <QDateTime>
#include <QDir>
#include <QIntValidator>
#include <QSqlField>
#include "detailededit.h"
#include "utils.h"
#include <QDebug>

/*!
Detailed qso editing dialog
*/
DetailedEdit::DetailedEdit(uiSize sizes,QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    connect(this,SIGNAL(accepted()),this,SLOT(updateRecord()));
    modeComboBox->insertItem(modeComboBox->count(),"NONE",RIG_MODE_NONE);
    modeComboBox->insertItem(modeComboBox->count(),"CW",RIG_MODE_CW);
    modeComboBox->insertItem(modeComboBox->count(),"CWR",RIG_MODE_CWR);
    modeComboBox->insertItem(modeComboBox->count(),"LSB",RIG_MODE_LSB);
    modeComboBox->insertItem(modeComboBox->count(),"USB",RIG_MODE_USB);
    modeComboBox->insertItem(modeComboBox->count(),"RTTY",RIG_MODE_RTTY);
    modeComboBox->insertItem(modeComboBox->count(),"RTTYR",RIG_MODE_RTTYR);
    modeComboBox->insertItem(modeComboBox->count(),"FM",RIG_MODE_FM);
    modeComboBox->insertItem(modeComboBox->count(),"FMN",RIG_MODE_FMN);
    modeComboBox->insertItem(modeComboBox->count(),"AMS",RIG_MODE_AMS);
    modeComboBox->insertItem(modeComboBox->count(),"DSB",RIG_MODE_DSB);
    modeComboBox->insertItem(modeComboBox->count(),"FAX",RIG_MODE_FAX);
    modeComboBox->insertItem(modeComboBox->count(),"SAH",RIG_MODE_SAH);
    modeComboBox->insertItem(modeComboBox->count(),"SAM",RIG_MODE_SAM);
    modeComboBox->insertItem(modeComboBox->count(),"PKTAM",RIG_MODE_PKTAM);
    modeComboBox->insertItem(modeComboBox->count(),"PKTLSB",RIG_MODE_PKTLSB);
    modeComboBox->insertItem(modeComboBox->count(),"PKTUSB",RIG_MODE_PKTUSB);
    modeComboBox->insertItem(modeComboBox->count(),"ECSSLSB",RIG_MODE_ECSSLSB);
    modeComboBox->insertItem(modeComboBox->count(),"ECSSUSB",RIG_MODE_ECSSUSB);

    rec.clear();
    adifLineEdit->setValidator(new UpperValidator(adifLineEdit));
    callLineEdit->setValidator(new UpperValidator(callLineEdit));
    sentExch1LineEdit->setValidator(new UpperValidator(sentExch1LineEdit));
    sentExch2LineEdit->setValidator(new UpperValidator(sentExch2LineEdit));
    sentExch3LineEdit->setValidator(new UpperValidator(sentExch3LineEdit));
    sentExch4LineEdit->setValidator(new UpperValidator(sentExch4LineEdit));
    rcvExch1LineEdit->setValidator(new UpperValidator(rcvExch1LineEdit));
    rcvExch2LineEdit->setValidator(new UpperValidator(rcvExch2LineEdit));
    rcvExch3LineEdit->setValidator(new UpperValidator(rcvExch3LineEdit));
    rcvExch4LineEdit->setValidator(new UpperValidator(rcvExch4LineEdit));
    freqLineEdit->setValidator(new QIntValidator(freqLineEdit));
    callLineEdit->setFocus();
    callLineEdit->setFixedWidth(qRound(sizes.width*12));
    adjustSize();
    setFixedSize(size());
}

/*!
Displays qso in record r
*/
void DetailedEdit::loadRecord(const QSqlRecord &r,int nexchange)
{
    rec=r;
    setWindowTitle("Edit QSO "+rec.value(SQL_COL_NR).toString());
    QDate date=QDate(rec.value(SQL_COL_DATE).toByteArray().right(4).toInt(),
                     rec.value(SQL_COL_DATE).toByteArray().left(2).toInt(),
                     rec.value(SQL_COL_DATE).toByteArray().mid(2,2).toInt());
    QTime time=QTime(rec.value(SQL_COL_TIME).toByteArray().left(2).toInt(),
                     rec.value(SQL_COL_TIME).toByteArray().right(2).toInt());
    modeComboBox->setCurrentIndex(modeComboBox->findData(QVariant(rec.value(SQL_COL_MODE))));
    adifLineEdit->setText(rec.value(SQL_COL_ADIF_MODE).toString());
    dateTimeEdit->setDateTime(QDateTime(date,time));
    dateTimeEdit->setTimeSpec(Qt::UTC);
    callLineEdit->setText(rec.value(SQL_COL_CALL).toString());
    callLineEdit->deselect();
    sentExch1LineEdit->setText(rec.value(SQL_COL_SNT1).toString());
    sentExch2LineEdit->setText(rec.value(SQL_COL_SNT2).toString());
    sentExch3LineEdit->setText(rec.value(SQL_COL_SNT3).toString());
    sentExch4LineEdit->setText(rec.value(SQL_COL_SNT4).toString());
    rcvExch1LineEdit->setText(rec.value(SQL_COL_RCV1).toString());
    rcvExch2LineEdit->setText(rec.value(SQL_COL_RCV2).toString());
    rcvExch3LineEdit->setText(rec.value(SQL_COL_RCV3).toString());
    rcvExch4LineEdit->setText(rec.value(SQL_COL_RCV4).toString());
    freqLineEdit->setText(QString::number(rec.value(SQL_COL_FREQ).toDouble(),'f',0));
    if (nexchange<4) {
        sentExch4LineEdit->hide();
        rcvExch4LineEdit->hide();
    }
    if (nexchange<3) {
        sentExch3LineEdit->hide();
        rcvExch3LineEdit->hide();
    }
    if (nexchange<2) {
        sentExch2LineEdit->hide();
        rcvExch2LineEdit->hide();
    }
}

/*!
 update record from dialog values
 */
void DetailedEdit::updateRecord()
{
    for (int i=0;i<SQL_N_COL;i++) rec.setGenerated(i,false);
    if (sentExch1LineEdit->text()!=rec.value(SQL_COL_SNT1).toString()) {
        rec.setValue(SQL_COL_SNT1,QVariant(sentExch1LineEdit->text()));
        rec.setGenerated(SQL_COL_SNT1,true);
    }
    if (sentExch1LineEdit->text()!=rec.value(SQL_COL_SNT2).toString()) {
        rec.setValue(SQL_COL_SNT2,QVariant(sentExch2LineEdit->text()));
        rec.setGenerated(SQL_COL_SNT2,true);
    }
    if (sentExch1LineEdit->text()!=rec.value(SQL_COL_SNT3).toString()) {
        rec.setValue(SQL_COL_SNT3,QVariant(sentExch3LineEdit->text()));
        rec.setGenerated(SQL_COL_SNT3,true);
    }
    if (sentExch1LineEdit->text()!=rec.value(SQL_COL_SNT4).toString()) {
        rec.setValue(SQL_COL_SNT4,QVariant(sentExch4LineEdit->text()));
        rec.setGenerated(SQL_COL_SNT4,true);
    }
    if (rcvExch1LineEdit->text()!=rec.value(SQL_COL_RCV1).toString()) {
        rec.setValue(SQL_COL_RCV1,QVariant(rcvExch1LineEdit->text()));
        rec.setGenerated(SQL_COL_RCV1,true);
    }
    if (rcvExch2LineEdit->text()!=rec.value(SQL_COL_RCV2).toString()) {
        rec.setValue(SQL_COL_RCV2,QVariant(rcvExch2LineEdit->text()));
        rec.setGenerated(SQL_COL_RCV2,true);
    }
    if (rcvExch3LineEdit->text()!=rec.value(SQL_COL_RCV3).toString()) {
        rec.setValue(SQL_COL_RCV3,QVariant(rcvExch3LineEdit->text()));
        rec.setGenerated(SQL_COL_RCV3,true);
    }
    if (rcvExch4LineEdit->text()!=rec.value(SQL_COL_RCV4).toString()) {
        rec.setValue(SQL_COL_RCV4,QVariant(rcvExch4LineEdit->text()));
        rec.setGenerated(SQL_COL_RCV4,true);
    }
    if (freqLineEdit->text().toInt()!=rec.value(SQL_COL_FREQ).toInt()) {
        rec.setValue(SQL_COL_FREQ,QVariant(freqLineEdit->text().toInt()));
        rec.setGenerated(SQL_COL_FREQ,true);
    }
    if (modeComboBox->currentIndex()!=rec.value(SQL_COL_MODE).toInt()) {
        rec.setValue(SQL_COL_MODE,QVariant(modeComboBox->currentData()));
        rec.setGenerated(SQL_COL_MODE,true);
    }
    if (adifLineEdit->text()!=rec.value(SQL_COL_ADIF_MODE).toString()) {
        rec.setValue(SQL_COL_ADIF_MODE,QVariant(adifLineEdit->text()));
        rec.setGenerated(SQL_COL_ADIF_MODE,true);
    }
    if (dateTimeEdit->date().toString("MMddyyyy")!=rec.value(SQL_COL_DATE).toString()) {
        rec.setValue(SQL_COL_DATE,QVariant(dateTimeEdit->date().toString("MMddyyyy")));
        rec.setGenerated(SQL_COL_DATE,true);
    }
    if (dateTimeEdit->time().toString("hhmm")!=rec.value(SQL_COL_TIME).toString()) {
        rec.setValue(SQL_COL_TIME,QVariant(dateTimeEdit->time().toString("hhmm")));
        rec.setGenerated(SQL_COL_TIME,true);
    }
    if (callLineEdit->text()!=rec.value(SQL_COL_CALL).toString()) {
        rec.setValue(SQL_COL_CALL,QVariant(callLineEdit->text()));
        rec.setGenerated(SQL_COL_CALL,true);
    }
    emit(editedRecord(rec));
}
