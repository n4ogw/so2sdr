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
#include <QComboBox>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QSettings>
#include "defines.h"
#include "sdrdialog.h"
#include "utils.h"

SDRDialog::SDRDialog(QSettings& s,QWidget *parent) : QDialog(parent),settings(s)
{
    setupUi(this);
    pathLabel[0] = labelExe1;
    pathLabel[1] = labelExe2;
    ipPtr[0]           = lineEditIP1;
    ipPtr[1]           = lineEditIP2;
    portPtr[0]         = lineEditPort1;
    portPtr[1]         = lineEditPort2;
    configLabel[0]       = labelConfig1;
    configLabel[1]       = labelConfig2;
    connect(buttonExe1,SIGNAL(clicked()),this,SLOT(findExeFile1()));
    connect(buttonExe2,SIGNAL(clicked()),this,SLOT(findExeFile2()));
    connect(buttonConfig1,SIGNAL(clicked()),this,SLOT(findConfig1()));
    connect(buttonConfig2,SIGNAL(clicked()),this,SLOT(findConfig2()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(updateSDR()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejectChanges()));
    updateFromSettings();
}

void SDRDialog::fileGetter(QString msg,QString path,QString files,QString key,QLabel *label)
{
//    QString fileName = QFileDialog::getOpenFileName(this,msg, path,files);
    QString fileName = QFileDialog::getSaveFileName(this,msg, path,files,0,QFileDialog::DontConfirmOverwrite);
    if (fileName.isNull()) {
        return;
    }
    settings.setValue(key,fileName);
    label->setText(shortName(fileName));
}

QString SDRDialog::shortName(QString s)
{
    QString str=s;
    int len=s.length();
    if (len>25) {
        str.remove(0,len-25);
        str="... " + str;
    }
    return str;
}

void SDRDialog::findConfig1()
{
    fileGetter("So2sdr-bandmap config file",userDirectory()+"/so2sdr-bandmap1.ini",
            "ini files (*.ini)",s_sdr_config[0],labelConfig1);
}

void SDRDialog::findConfig2()
{
    fileGetter("So2sdr-bandmap config file",userDirectory()+"/so2sdr-bandmap2.ini",
            "ini files (*.ini)",s_sdr_config[1],labelConfig2);
}

void SDRDialog::findExeFile1()
{
#ifdef Q_OS_LINUX
    fileGetter("So2sdr-bandmap executable",QCoreApplication::applicationDirPath(),
               "so2sdr-bandmap",s_sdr_path[0],labelExe1);
#endif
#ifdef Q_OS_WIN
    fileGetter("So2sdr-bandmap executable",QCoreApplication::applicationDirPath(),
               "so2sdr-bandmap.exe",s_sdr_path[0],labelExe1);
#endif
}

void SDRDialog::findExeFile2()
{
#ifdef Q_OS_LINUX
    fileGetter("So2sdr-bandmap executable",QCoreApplication::applicationDirPath(),
               "so2sdr-bandmap",s_sdr_path[1],labelExe2);
#endif
#ifdef Q_OS_WIN
    fileGetter("So2sdr-bandmap executable",QCoreApplication::applicationDirPath(),
               "so2sdr-bandmap.exe",s_sdr_path[1],labelExe2);
#endif
}

void SDRDialog::updateFromSettings()
{
    SpotTimeoutLineEdit->setText(settings.value(s_sdr_spottime,s_sdr_spottime_def).toString());
    lineEditUDP->setText(settings.value(s_sdr_udp,s_sdr_udp_def).toString());
    for (int i = 0; i < NRIG; i++) {
#ifdef Q_OS_LINUX
        pathLabel[i]->setText(settings.value(s_sdr_path[i],QCoreApplication::applicationDirPath()+"so2sdr-bandmap").toString());
#endif
#ifdef Q_OS_WIN
        pathLabel[i]->setText(settings.value(s_sdr_path[i],QCoreApplication::applicationDirPath()+"so2sdr-bandmap.exe").toString());
#endif
        configLabel[i]->setText(shortName(settings.value(s_sdr_config[i],s_sdr_config_def[i]).toString()));
        ipPtr[i]->setText(settings.value(s_sdr_ip[i],s_sdr_ip_def[i]).toString());
        portPtr[i]->setText(settings.value(s_sdr_port[i],s_sdr_port_def[i]).toString());
    }
    ChangeRadioClickCheckBox->setChecked(settings.value(s_sdr_changeclick,s_sdr_changeclick_def).toBool());
    lineEdit160low->setText(settings.value(s_sdr_cqlimit_low[0],cqlimit_default_low[0]).toString());
    lineEdit160high->setText(settings.value(s_sdr_cqlimit_high[0],cqlimit_default_high[0]).toString());
    lineEdit80low->setText(settings.value(s_sdr_cqlimit_low[1],cqlimit_default_low[1]).toString());
    lineEdit80high->setText(settings.value(s_sdr_cqlimit_high[1],cqlimit_default_high[1]).toString());
    lineEdit40low->setText(settings.value(s_sdr_cqlimit_low[2],cqlimit_default_low[2]).toString());
    lineEdit40high->setText(settings.value(s_sdr_cqlimit_high[2],cqlimit_default_high[2]).toString());
    lineEdit20low->setText(settings.value(s_sdr_cqlimit_low[3],cqlimit_default_low[3]).toString());
    lineEdit20high->setText(settings.value(s_sdr_cqlimit_high[3],cqlimit_default_high[3]).toString());
    lineEdit15low->setText(settings.value(s_sdr_cqlimit_low[4],cqlimit_default_low[4]).toString());
    lineEdit15high->setText(settings.value(s_sdr_cqlimit_high[4],cqlimit_default_high[4]).toString());
    lineEdit10low->setText(settings.value(s_sdr_cqlimit_low[5],cqlimit_default_low[5]).toString());
    lineEdit10high->setText(settings.value(s_sdr_cqlimit_high[5],cqlimit_default_high[5]).toString());
}

SDRDialog::~SDRDialog()
{
}

void SDRDialog::updateSDR()
{
    for (int i = 0; i < NRIG; i++) {
        settings.setValue(s_sdr_ip[i],ipPtr[i]->text());
        settings.setValue(s_sdr_port[i],portPtr[i]->text().toInt());
    }
    settings.setValue(s_sdr_udp,lineEditUDP->text().toInt());
    settings.setValue(s_sdr_spottime,SpotTimeoutLineEdit->text().toInt());
    settings.setValue(s_sdr_changeclick,ChangeRadioClickCheckBox->isChecked());
    settings.setValue(s_sdr_cqlimit_low[0],lineEdit160low->text().toInt());
    settings.setValue(s_sdr_cqlimit_high[0],lineEdit160high->text().toInt());
    settings.setValue(s_sdr_cqlimit_low[1],lineEdit80low->text().toInt());
    settings.setValue(s_sdr_cqlimit_high[1],lineEdit80high->text().toInt());
    settings.setValue(s_sdr_cqlimit_low[2],lineEdit40low->text().toInt());
    settings.setValue(s_sdr_cqlimit_high[2],lineEdit40high->text().toInt());
    settings.setValue(s_sdr_cqlimit_low[3],lineEdit20low->text().toInt());
    settings.setValue(s_sdr_cqlimit_high[3],lineEdit20high->text().toInt());
    settings.setValue(s_sdr_cqlimit_low[4],lineEdit15low->text().toInt());
    settings.setValue(s_sdr_cqlimit_high[4],lineEdit15high->text().toInt());
    settings.setValue(s_sdr_cqlimit_low[5],lineEdit10low->text().toInt());
    settings.setValue(s_sdr_cqlimit_high[5],lineEdit10high->text().toInt());
    settings.sync();
    emit(updateCQLimits());
}

void SDRDialog::rejectChanges()
{
    updateFromSettings();
    reject();
}

