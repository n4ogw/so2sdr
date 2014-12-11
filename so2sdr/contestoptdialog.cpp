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
#include <QDateTime>
#include <QSettings>
#include <QString>
#include "contestoptdialog.h"

ContestOptionsDialog::ContestOptionsDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    connect(contest_dialog_buttons, SIGNAL(rejected()), this, SLOT(rejectChanges()));
    connect(contest_dialog_buttons, SIGNAL(accepted()), this, SLOT(updateOptions()));
    sent[0]=lineEditExch1;
    sent[1]=lineEditExch2;
    sent[2]=lineEditExch3;
    sent[3]=lineEditExch4;
    offValidator=new QIntValidator(this);
    offValidator->setBottom(1);
    offMinimumLineEdit->setValidator(offValidator);
}

ContestOptionsDialog::~ContestOptionsDialog()
{
    delete offValidator;
}

/*!
  load options from contest file
  */
void ContestOptionsDialog::initialize(QSettings *s)
{
    settings=s;
    setOptions();
}

void ContestOptionsDialog::setOptions()
{
    MasterLineEdit->setText(settings->value(c_masterfile,c_masterfile_def).toString());
    HistoryLineEdit->setText(settings->value(c_historyfile,c_historyfile_def).toString());
    MultiModeCheckBox->setChecked(settings->value(c_multimode,c_multimode_def).toBool());
    MasterCheckBox->setChecked(settings->value(c_mastermode,c_mastermode_def).toBool());
    HistoryCheckBox->setChecked(settings->value(c_historymode,c_historymode_def).toBool());
    HistoryUpdateCheckBox->setChecked(settings->value(c_historyupdate,c_historyupdate_def).toBool());
    ShowModeCheckBox->setChecked(settings->value(c_showmode,c_showmode_def).toBool());
    ShowMultsCheckBox->setChecked(settings->value(c_showmults,c_showmults_def).toBool());
    SprintCheckBox->setChecked(settings->value(c_sprintmode,c_sprintmode_def).toBool());
    MultsByBandCheckBox->setChecked(settings->value(c_multsband,c_multsband_def).toBool());
    dupesComboBox->setCurrentIndex(settings->value(c_dupemode,c_dupemode_def).toInt());
    lineEditExch1->setText(settings->value(c_sentexch1,c_sentexch1_def).toString());
    lineEditExch2->setText(settings->value(c_sentexch2,c_sentexch2_def).toString());
    lineEditExch3->setText(settings->value(c_sentexch3,c_sentexch3_def).toString());
    lineEditExch4->setText(settings->value(c_sentexch4,c_sentexch4_def).toString());
    offMinimumLineEdit->setText(settings->value(c_off_time_min,c_off_time_min_def).toString());
    startDateTimeEdit->setDateTime(settings->value(c_off_time_start,c_off_time_start_def).toDateTime());
    endDateTimeEdit->setDateTime(settings->value(c_off_time_end,c_off_time_end_def).toDateTime());
    OffTimeCheckBox->setChecked(settings->value(c_off_time_enable,c_off_time_enable_def).toBool());
}

/*! update options when dialog accepted
 */
void ContestOptionsDialog::updateOptions()
{
    // need to rescore log if dupe mode, multimode, or mults view change
    settings->setValue(c_masterfile,MasterLineEdit->text());
    settings->setValue(c_historyfile,HistoryLineEdit->text());

    bool oldMultiMode=settings->value(c_multimode,c_multimode_def).toBool();
    bool newMultiMode=MultiModeCheckBox->isChecked();
    settings->setValue(c_multimode,newMultiMode);
    if (oldMultiMode!=newMultiMode) emit(multiModeChanged());

    settings->setValue(c_mastermode,MasterCheckBox->isChecked());
    if (HistoryLineEdit->text().isEmpty()) {
        settings->setValue(c_historymode,false);
        settings->setValue(c_historyupdate,false);
        HistoryCheckBox->setChecked(false);
        HistoryUpdateCheckBox->setChecked(false);
    } else {
        settings->setValue(c_historymode,HistoryCheckBox->isChecked());
        settings->setValue(c_historyupdate,HistoryUpdateCheckBox->isChecked());
    }
    settings->setValue(c_sprintmode,SprintCheckBox->isChecked());
    settings->setValue(c_showmode,ShowModeCheckBox->isChecked());

    int oldDupeMode=settings->value(c_dupemode,c_dupemode_def).toInt();
    int newDupeMode=dupesComboBox->currentIndex();
    settings->setValue(c_dupemode,newDupeMode);

    bool oldShowMults=settings->value(c_showmults,c_showmults_def).toBool();
    bool newShowMults=ShowMultsCheckBox->isChecked();
    settings->setValue(c_showmults,newShowMults);

    bool oldMultsBand=settings->value(c_multsband,c_multsband_def).toBool();
    bool newMultsBand=MultsByBandCheckBox->isChecked();
    settings->setValue(c_multsband,newMultsBand);

    if ((newDupeMode!=oldDupeMode && (oldDupeMode==NO_DUPE_CHECKING || newDupeMode==NO_DUPE_CHECKING))
            || oldShowMults!=newShowMults || oldMultiMode!=newMultiMode
            || oldMultsBand!=newMultsBand) emit(rescore());

    settings->setValue(c_sentexch1,lineEditExch1->text());
    settings->setValue(c_sentexch2,lineEditExch2->text());
    settings->setValue(c_sentexch3,lineEditExch3->text());
    settings->setValue(c_sentexch4,lineEditExch4->text());

    // off time calculation
    bool oldOffTimeEnabled=settings->value(c_off_time_enable,c_off_time_enable_def).toBool();
    bool newOffTimeEnabled=OffTimeCheckBox->isChecked();
    settings->setValue(c_off_time_enable,newOffTimeEnabled);

    int oldMin=settings->value(c_off_time_min,c_off_time_min_def).toInt();
    int newMin=offMinimumLineEdit->text().toInt();
    settings->setValue(c_off_time_min,newMin);

    QDateTime oldStartDate=settings->value(c_off_time_start,c_off_time_start_def).toDateTime();
    QDateTime newStartDate=startDateTimeEdit->dateTime();
    settings->setValue(c_off_time_start,newStartDate);

    QDateTime oldEndDate=settings->value(c_off_time_end,c_off_time_end_def).toDateTime();
    QDateTime newEndDate=endDateTimeEdit->dateTime();
    settings->setValue(c_off_time_end,newEndDate);

    if (oldOffTimeEnabled!=newOffTimeEnabled ||
            oldMin!=newMin || oldStartDate!=newStartDate || oldEndDate!=newEndDate) emit(updateOffTime());

    settings->sync();
    accept();
}

/*! reject changes; cancel
 */
void ContestOptionsDialog::rejectChanges()
{
    setOptions();
    reject();
}
