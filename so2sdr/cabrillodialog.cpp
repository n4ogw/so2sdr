/*! Copyright 2010-2023 R. Torsten Clay N4OGW

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
#include "cabrillodialog.h"
#include "defines.h"
#include "utils.h"
#include <QDialog>
#include <QFile>
#include <QFormLayout>
#include <QLabel>
#include <QSettings>
#include <QString>

/*!
  Dialog appearing before writing cabrillo log
  */
CabrilloDialog::CabrilloDialog(QWidget *parent) : QDialog(parent) {
  setupUi(this);
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
  lineEditOperators->setValidator(new UpperValidator(lineEditOperators));
  lineEditOperators->clear();
  lineEditLocation->clear();
  textEditSoapbox->clear();
  sent[0] = lineEditExch1;
  sent[1] = lineEditExch2;
  sent[2] = lineEditExch3;
  sent[3] = lineEditExch4;
  for (int i = 0; i < 4; i++)
    sent[i]->setValidator(new UpperValidator(sent[i]));

  // pointers to objects in form
  // must update if MAX_CAB_FIELDS changes
  cabLabel[0] = labelCabrillo1;
  cabLabel[1] = labelCabrillo2;
  cabLabel[2] = labelCabrillo3;
  cabLabel[3] = labelCabrillo4;
  cabLabel[4] = labelCabrillo5;
  cabLabel[5] = labelCabrillo6;
  cabLabel[6] = labelCabrillo7;
  cabCombo[0] = comboBoxCabrillo1;
  cabCombo[1] = comboBoxCabrillo2;
  cabCombo[2] = comboBoxCabrillo3;
  cabCombo[3] = comboBoxCabrillo4;
  cabCombo[4] = comboBoxCabrillo5;
  cabCombo[5] = comboBoxCabrillo6;
  cabCombo[6] = comboBoxCabrillo7;
  for (int i = 0; i < MAX_CAB_FIELDS; i++) {
    cabCombo[i]->clear();
    cabCombo[i]->setEnabled(false);
    cabLabel[i]->clear();
  }
  comboBoxContestName->clear();
  labelClaimedScore->clear();
}

/*!
  initialize variables from config files

  s1=station config: used for name, address
  s2=contest config; used for cabrillo categories
*/
void CabrilloDialog::initialize(QSettings *s1, QSettings *s2) {
  stnSettings = s1;
  settings = s2;

  lineEditName->setText(
      stnSettings->value(s_cab_name, s_cab_name_def).toString());
  textEditAddress->setText(
      stnSettings->value(s_cab_address, s_cab_address_def).toString());
  lineEditAddressCountry->setText(
      stnSettings->value(s_cab_country, s_cab_country_def).toString());
  lineEditAddressPostalCode->setText(
      stnSettings->value(s_cab_zip, s_cab_zip_def).toString());
  lineEditAddressCity->setText(
      stnSettings->value(s_cab_city, s_cab_city_def).toString());
  lineEditAddressState->setText(
      stnSettings->value(s_cab_state, s_cab_state_def).toString());
  lineEditClub->setText(settings->value(c_cab_club, c_cab_club_def).toString());
  labelCall->setText(stnSettings->value(s_call, s_call_def).toString());

  /* read cabrillo categories for this contest
   ini file format:

   cabrillo/cab1/size = 2
   cabrillo/cab1/1/cabstring
   cabrillo/cab1/2/cabstring

   etc for cabrillo/cab2, cabrillo/cab3, etc

   */
  for (int j = 0; j < MAX_CAB_FIELDS; j++) {
    int sz = settings->beginReadArray(c_cab[j]);
    if (sz) {
      cabCombo[j]->setEnabled(true);
      // first entry is name of group
      settings->setArrayIndex(0);
      cabLabel[j]->setText(settings->value("cabstring").toString());
      // further entries are combobox values
      cabCombo[j]->clear();
      for (int i = 1; i < sz; i++) {
        settings->setArrayIndex(i);
        cabCombo[j]->addItem(settings->value("cabstring").toString());
      }
    }
    settings->endArray();
  }
  int sz = settings->beginReadArray(c_cab_contestname);
  comboBoxContestName->clear();
  for (int i = 0; i < sz; i++) {
    settings->setArrayIndex(i);
    comboBoxContestName->addItem(settings->value("name").toString());
  }
  settings->endArray();

  updateExch();
}

/*!
  update exchange fields
  */
void CabrilloDialog::updateExch() {
  lineEditExch1->setText(
      settings->value(c_sentexch1, c_sentexch1_def).toString());
  lineEditExch2->setText(
      settings->value(c_sentexch2, c_sentexch2_def).toString());
  lineEditExch3->setText(
      settings->value(c_sentexch3, c_sentexch3_def).toString());
  lineEditExch4->setText(
      settings->value(c_sentexch4, c_sentexch4_def).toString());
}

/*!
  write cabrillo header information

  also update some settings
  */
void CabrilloDialog::writeHeader(QFile *cbrFile, int score) {
  QString tmpstr;
  cbrFile->write(
      "START-OF-LOG: " +
      settings->value(c_cab_version, c_cab_version_def).toByteArray() + "\n");
  tmpstr = "CREATED-BY: SO2SDR version " + Version + "\n";
  cbrFile->write(tmpstr.toLatin1().data());
  cbrFile->write("CALLSIGN: " + labelCall->text().toLatin1() + "\n");
  cbrFile->write("CONTEST: " + comboBoxContestName->currentText().toLatin1() +
                 "\n");
  if (!lineEditLocation->text().isEmpty()) {
    cbrFile->write("LOCATION: " + lineEditLocation->text().toLatin1() + "\n");
  }
  tmpstr = "CLAIMED-SCORE: " + QString::number(score) + "\n";
  cbrFile->write(tmpstr.toLatin1().data());
  if (comboBoxCabrillo1->isEnabled() &&
      comboBoxCabrillo1->currentText() != "NONE") {
    tmpstr =
        labelCabrillo1->text() + ": " + comboBoxCabrillo1->currentText() + "\n";
    cbrFile->write(tmpstr.toLatin1());
  }
  if (comboBoxCabrillo2->isEnabled() &&
      comboBoxCabrillo2->currentText() != "NONE") {
    tmpstr =
        labelCabrillo2->text() + ": " + comboBoxCabrillo2->currentText() + "\n";
    cbrFile->write(tmpstr.toLatin1());
  }
  if (comboBoxCabrillo3->isEnabled() &&
      comboBoxCabrillo3->currentText() != "NONE") {
    tmpstr =
        labelCabrillo3->text() + ": " + comboBoxCabrillo3->currentText() + "\n";
    cbrFile->write(tmpstr.toLatin1());
  }
  if (comboBoxCabrillo4->isEnabled() &&
      comboBoxCabrillo4->currentText() != "NONE") {
    tmpstr =
        labelCabrillo4->text() + ": " + comboBoxCabrillo4->currentText() + "\n";
    cbrFile->write(tmpstr.toLatin1());
  }
  if (comboBoxCabrillo5->isEnabled() &&
      comboBoxCabrillo5->currentText() != "NONE") {
    tmpstr =
        labelCabrillo5->text() + ": " + comboBoxCabrillo5->currentText() + "\n";
    cbrFile->write(tmpstr.toLatin1());
  }
  if (comboBoxCabrillo6->isEnabled() &&
      comboBoxCabrillo6->currentText() != "NONE") {
    tmpstr =
        labelCabrillo6->text() + ": " + comboBoxCabrillo6->currentText() + "\n";
    cbrFile->write(tmpstr.toLatin1());
  }
  if (comboBoxCabrillo7->isEnabled() &&
      comboBoxCabrillo7->currentText() != "NONE") {
    tmpstr =
        labelCabrillo7->text() + ": " + comboBoxCabrillo7->currentText() + "\n";
    cbrFile->write(tmpstr.toLatin1());
  }
  if (!lineEditOperators->text().isEmpty()) {
    cbrFile->write("OPERATORS: " + lineEditOperators->text().toLatin1() + "\n");
  }
  if (!lineEditName->text().isEmpty()) {
    cbrFile->write("NAME: " + lineEditName->text().toLatin1() + "\n");
  }
  QString tmp = textEditAddress->toPlainText();
  QStringList tmppart = tmp.split("\n");
  for (int i = 0; i < tmppart.size(); i++) {
    if (!tmppart[i].isEmpty()) {
      cbrFile->write("ADDRESS: " + tmppart[i].toLatin1() + "\n");
    }
  }
  cbrFile->write("ADDRESS-CITY: " + lineEditAddressCity->text().toLatin1() +
                 "\n");
  cbrFile->write("ADDRESS-STATE-PROVINCE: " +
                 lineEditAddressState->text().toLatin1() + "\n");
  cbrFile->write("ADDRESS-POSTALCODE: " +
                 lineEditAddressPostalCode->text().toLatin1() + "\n");
  cbrFile->write(
      "ADDRESS-COUNTRY: " + lineEditAddressCountry->text().toLatin1() + "\n");
  if (!lineEditClub->text().isEmpty()) {
    cbrFile->write("CLUB: " + lineEditClub->text().toLatin1() + "\n");
    settings->setValue(c_cab_club, lineEditClub->text());
  }
  if (!textEditSoapbox->toPlainText().isEmpty()) {
    tmp = textEditSoapbox->toPlainText();
    tmppart = tmp.split("\n");
    for (int i = 0; i < tmppart.size(); i++) {
      if (!tmppart[i].isEmpty()) {
        cbrFile->write("SOAPBOX: " + tmppart[i].toLatin1() + "\n");
      }
    }
  }
}
