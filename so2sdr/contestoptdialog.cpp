/*! Copyright 2010-2012 R. Torsten Clay N4OGW

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
#include <QSettings>
#include <QString>
#include "contestoptdialog.h"

ContestOptionsDialog::ContestOptionsDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    connect(this, SIGNAL(rejected()), this, SLOT(rejectChanges()));
    connect(this, SIGNAL(accepted()), this, SLOT(updateOptions()));
    sent[0]=lineEditExch1;
    sent[1]=lineEditExch2;
    sent[2]=lineEditExch3;
    sent[3]=lineEditExch4;
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
    MasterCheckBox->setChecked(settings->value(c_mastermode,c_mastermode_def).toBool());
    ShowModeCheckBox->setChecked(settings->value(c_showmode,c_showmode_def).toBool());
    ShowMultsCheckBox->setChecked(settings->value(c_showmults,c_showmults_def).toBool());
    SprintCheckBox->setChecked(settings->value(c_sprintmode,c_sprintmode_def).toBool());
    MultsByBandCheckBox->setChecked(settings->value(c_multsband,c_multsband_def).toBool());
    dupesComboBox->setCurrentIndex(settings->value(c_dupemode,c_dupemode_def).toInt());
    lineEditExch1->setText(settings->value(c_sentexch1,c_sentexch1_def).toString());
    lineEditExch2->setText(settings->value(c_sentexch2,c_sentexch2_def).toString());
    lineEditExch3->setText(settings->value(c_sentexch3,c_sentexch3_def).toString());
    lineEditExch4->setText(settings->value(c_sentexch4,c_sentexch4_def).toString());
}

/*! update options when dialog accepted
 */
void ContestOptionsDialog::updateOptions()
{
    settings->setValue(c_masterfile,MasterLineEdit->text());
    settings->setValue(c_mastermode,MasterCheckBox->isChecked());
    settings->setValue(c_sprintmode,SprintCheckBox->isChecked());
    settings->setValue(c_showmode,ShowModeCheckBox->isChecked());

    // need to rescore log if dupe mode or mults view change
    int oldDupeMode=settings->value(c_dupemode,c_dupemode_def).toInt();
    int newDupeMode=dupesComboBox->currentIndex();
    settings->setValue(c_dupemode,newDupeMode);

    bool oldShowMults=settings->value(c_showmults,c_showmults_def).toBool();
    bool newShowMults=ShowMultsCheckBox->isChecked();
    settings->setValue(c_showmults,ShowMultsCheckBox->isChecked());

    bool oldMultsBand=settings->value(c_multsband,c_multsband_def).toBool();
    bool newMultsBand=MultsByBandCheckBox->isChecked();
    settings->setValue(c_multsband,MultsByBandCheckBox->isChecked());

    if ((newDupeMode!=oldDupeMode && (oldDupeMode==NO_DUPE_CHECKING || newDupeMode==NO_DUPE_CHECKING))
            || oldShowMults!=newShowMults
            || oldMultsBand!=newMultsBand) emit(rescore());

    settings->setValue(c_sentexch1,lineEditExch1->text());
    settings->setValue(c_sentexch2,lineEditExch2->text());
    settings->setValue(c_sentexch3,lineEditExch3->text());
    settings->setValue(c_sentexch4,lineEditExch4->text());
    settings->sync();
  }

/*! reject changes; cancel
 */
void ContestOptionsDialog::rejectChanges()
{
    setOptions();
}
