/*! Copyright 2010-2012 rec. Torsten Clay N4OGW

   This file is part of so2sdrec.

    so2sdr is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    so2sdr is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with so2sdrec.  If not, see <http://www.gnu.org/licenses/>.

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
DetailedEdit::DetailedEdit(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    connect(this,SIGNAL(accepted()),this,SLOT(updateRecord()));
    for (int i=0;i<nModes;i++) {
        modeComboBox->insertItem(i,modes[i]);
    }
    rec.clear();
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
    modeComboBox->setCurrentIndex(rec.value(SQL_COL_MODE).toInt());
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
    freqLineEdit->setText(rec.value(SQL_COL_FREQ).toString());
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
    rec.setValue(SQL_COL_SNT1,QVariant(sentExch1LineEdit->text()));
    rec.setValue(SQL_COL_SNT2,QVariant(sentExch2LineEdit->text()));
    rec.setValue(SQL_COL_SNT3,QVariant(sentExch3LineEdit->text()));
    rec.setValue(SQL_COL_SNT4,QVariant(sentExch4LineEdit->text()));
    rec.setValue(SQL_COL_RCV1,QVariant(rcvExch1LineEdit->text()));
    rec.setValue(SQL_COL_RCV2,QVariant(rcvExch2LineEdit->text()));
    rec.setValue(SQL_COL_RCV3,QVariant(rcvExch3LineEdit->text()));
    rec.setValue(SQL_COL_RCV4,QVariant(rcvExch4LineEdit->text()));
    rec.setValue(SQL_COL_FREQ,QVariant(freqLineEdit->text().toInt()));
    rec.setValue(SQL_COL_MODE,QVariant(modeComboBox->currentIndex()));
    rec.setValue(SQL_COL_DATE,QVariant(dateTimeEdit->date().toString("MMddyyyy")));
    rec.setValue(SQL_COL_TIME,QVariant(dateTimeEdit->time().toString("hhmm")));
    rec.setValue(SQL_COL_CALL,QVariant(callLineEdit->text()));
    emit(editedRecord(rec));
}
