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
#include <QDialog>
#include <QMessageBox>
#include <QSettings>
#include "defines.h"
#include "stationdialog.h"
#include "hamlib/rotator.h"

/*!
  StationDialog is a dialog for entering station parameters (call, name, ...)

  s is station config file (so2sdr.ini) in QSettings .ini format
 */
StationDialog::StationDialog(QSettings &s, QWidget *parent) : QDialog(parent),settings(s)
{
    setupUi(this);

    // load from settings file
    CallLineEdit->setValidator(new UpperValidator(CallLineEdit));
    CallLineEdit->setText(settings.value(s_call,s_call_def).toString());
    NameLineEdit->setValidator(new UpperValidator(NameLineEdit));
    NameLineEdit->setText(settings.value(s_name,s_name_def).toString());
    StateLineEdit->setValidator(new UpperValidator(StateLineEdit));
    StateLineEdit->setText(settings.value(s_state,s_state_def).toString());
    ARRLSectionLineEdit->setValidator(new UpperValidator(ARRLSectionLineEdit));
    ARRLSectionLineEdit->setText(settings.value(s_section,s_section_def).toString());
    GridLineEdit->setValidator(new UpperValidator(GridLineEdit));
    GridLineEdit->setText(settings.value(s_grid,s_grid_def).toString());
    Lat           = 0.;
    Lon           = 0.;
    locator2longlat(&Lon, &Lat, settings.value(s_grid,s_grid_def).toByteArray().data());
    Lon            *= -1.0;
    CQZoneLineEdit->setText(settings.value(s_cqzone,s_cqzone_def).toString());
    ITUZoneLineEdit->setText(settings.value(s_ituzone,s_ituzone_def).toString());
    connect(station_dialog_buttons, SIGNAL(rejected()), this, SLOT(rejectChanges()));
    connect(station_dialog_buttons, SIGNAL(accepted()), this, SLOT(updateStation()));
    CabrilloAddressEdit->setText(settings.value(s_cab_address,s_cab_address_def).toString());
    CabrilloNameLineEdit->setText(settings.value(s_cab_name,s_cab_name_def).toString());
    CityLineEdit->setText(settings.value(s_cab_city,s_cab_city_def).toString());
    StateLineEdit_2->setText(settings.value(s_cab_state,s_cab_state_def).toString());
    CountryLineEdit->setText(settings.value(s_cab_country,s_cab_country_def).toString());
    PostalCodeLineEdit->setText(settings.value(s_cab_zip,s_cab_zip_def).toString());
}

StationDialog::~StationDialog()
{
}

double StationDialog::lat() const
{
    return(Lat);
}

double StationDialog::lon() const
{
    return(Lon);
}

/*!
   update from form input
 */
void StationDialog::updateStation()
{
    settings.setValue(s_call,CallLineEdit->text());
    settings.setValue(s_section,ARRLSectionLineEdit->text());
    settings.setValue(s_state,StateLineEdit->text());
    settings.setValue(s_grid,GridLineEdit->text());
    locator2longlat(&Lon, &Lat, GridLineEdit->text().toLatin1().data());
    Lon            *= -1.0;
    settings.setValue(s_cqzone,CQZoneLineEdit->text().toInt());
    settings.setValue(s_ituzone,ITUZoneLineEdit->text().toInt());
    settings.setValue(s_name,NameLineEdit->text());
    settings.setValue(s_cab_address,CabrilloAddressEdit->toPlainText());
    settings.setValue(s_cab_name,CabrilloNameLineEdit->text());
    settings.setValue(s_cab_zip,PostalCodeLineEdit->text());
    settings.setValue(s_cab_city,CityLineEdit->text());
    settings.setValue(s_cab_country,CountryLineEdit->text());
    settings.setValue(s_cab_state,StateLineEdit_2->text());
    settings.sync();
    emit(stationUpdate());
    accept();
}

/*! called if dialog rejected */
void StationDialog::rejectChanges()
{
    CabrilloAddressEdit->setText(settings.value(s_cab_address,s_cab_address_def).toString());
    StateLineEdit_2->setText(settings.value(s_cab_state,s_cab_state_def).toString());
    PostalCodeLineEdit->setText(settings.value(s_cab_zip,s_cab_zip_def).toString());
    CountryLineEdit->setText(settings.value(s_cab_country,s_cab_country_def).toString());
    CabrilloNameLineEdit->setText(settings.value(s_cab_name,s_cab_name_def).toString());
    CityLineEdit->setText(settings.value(s_cab_city,s_cab_city_def).toString());
    CallLineEdit->setText(settings.value(s_call,s_call_def).toString());
    StateLineEdit->setText(settings.value(s_state,s_state_def).toString());
    ARRLSectionLineEdit->setText(settings.value(s_section,s_section_def).toString());
    GridLineEdit->setText(settings.value(s_grid,s_grid_def).toString());
    NameLineEdit->setText(settings.value(s_name,s_name_def).toString());
    CQZoneLineEdit->setText(settings.value(s_cqzone,s_cqzone_def).toString());
    ITUZoneLineEdit->setText(settings.value(s_ituzone,s_ituzone_def).toString());
    reject();
}
