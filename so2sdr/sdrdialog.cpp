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
#include <QComboBox>
#include <QDebug>
#include <QSettings>
#include "defines.h"
#include "sdrdialog.h"
#include "utils.h"

SDRDialog::SDRDialog(QSettings& s,QWidget *parent) : QDialog(parent),settings(s)
{
    setupUi(this);
    DeviceCombo[0]       = SoundCard1ComboBox;
    DeviceCombo[1]       = SoundCard2ComboBox;
    DeviceCombo[2]       = SoundCardDVKComboBox;

    APICombo[0]          = API1ComboBox;
    APICombo[1]          = API2ComboBox;
    APICombo[2]          = APIDVKComboBox;
    Checkbox[0]          = checkBox;
    Checkbox[1]          = checkBox_2;
    Checkbox[2]          = checkBox_3;
    for (int i=0;i<3;i++) {
        DeviceCombo[i]->setEnabled(false);
        APICombo[i]->setEnabled(false);
    }
    DVKRecordingComboBox->setEnabled(false);
    loopCheckBox->setChecked(false);

    OffsetLineEditPtr[0] = OffsetLineEdit;
    OffsetLineEditPtr[1] = Offset2LineEdit;
    OffsetLineEditPtr[0]->setEnabled(false);
    OffsetLineEditPtr[1]->setEnabled(false);

    iconOK               = QIcon("check.png");
    iconNOK              = QIcon("x.png");

    // bit sizes
    BitsCombo[0] = Bits1ComboBox;
    BitsCombo[1] = Bits2ComboBox;
    BitsCombo[0]->setEnabled(false);
    BitsCombo[1]->setEnabled(false);

    for (int i = 0; i < NRIG; i++) {
        BitsCombo[i]->insertItem(0, "16");
        BitsCombo[i]->insertItem(1, "24");
        BitsCombo[i]->insertItem(2, "32");
    }
    // find audio devices
    // have to start Portaudio to get the device list
    Pa_Initialize();
    nAPI            = Pa_GetHostApiCount();
    nApiDevices     = new int[nAPI];
    nApiDeviceNames = new QList<QString>[nAPI];
    deviceOK        = new QList<bool>[nAPI];
    for (int i = 0; i < nAPI; i++) {
        deviceOK[i].clear();
        nApiDevices[i] = 0;
        nApiDeviceNames[i].clear();
        APICombo[0]->insertItem(i, Pa_GetHostApiInfo(i)->name);
        APICombo[1]->insertItem(i, Pa_GetHostApiInfo(i)->name);
        APICombo[2]->insertItem(i, Pa_GetHostApiInfo(i)->name);
    }
    connect(APICombo[0], SIGNAL(currentIndexChanged(int)), this, SLOT(launchUpdateDeviceList0(int)));
    connect(APICombo[1], SIGNAL(currentIndexChanged(int)), this, SLOT(launchUpdateDeviceList1(int)));
    connect(APICombo[2], SIGNAL(currentIndexChanged(int)), this, SLOT(launchUpdateDeviceList2(int)));
    audioDevices.clear();
    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        numDevices = 0;
    }
    const PaDeviceInfo *deviceInfo;
    PaStreamParameters testFormat;
    testFormat.channelCount              = 2;
    testFormat.sampleFormat              = paInt16;
    testFormat.suggestedLatency          = 0;
    testFormat.hostApiSpecificStreamInfo = NULL;
    for (int i = 0; i < numDevices; i++) {
        deviceInfo = Pa_GetDeviceInfo(i);
        int api = deviceInfo->hostApi;
        nApiDevices[api]++;
        audioDevices.append(deviceInfo->name);
        nApiDeviceNames[api].append(deviceInfo->name);

        // check to see if this device will work
        bool ok = true;
        testFormat.device           = i;
        testFormat.suggestedLatency = Pa_GetDeviceInfo(i)->defaultLowInputLatency;

        // check for stereo support
        if (Pa_GetDeviceInfo(i)->maxInputChannels < 2) {
            ok = false;
        }

        // test for support of 96 KHz sampling
        int err = Pa_IsFormatSupported(&testFormat, NULL, 96000);
        if (err != paNoError) ok = false;
        if (ok) {
            DeviceCombo[0]->insertItem(i, iconOK, audioDevices[i]);
            DeviceCombo[1]->insertItem(i, iconOK, audioDevices[i]);
        } else {
            DeviceCombo[0]->insertItem(i, iconNOK, audioDevices[i]);
            DeviceCombo[1]->insertItem(i, iconNOK, audioDevices[i]);
        }
        // for DVK playback need 44.1 Khz stereo
        ok=true;
        err = Pa_IsFormatSupported(&testFormat, NULL, 44100);
        if (err != paNoError) ok = false;
        if (ok) {
            DeviceCombo[2]->insertItem(i, iconOK, audioDevices[i]);
        } else {
            DeviceCombo[2]->insertItem(i, iconNOK, audioDevices[i]);
        }
        // DVK recording input needs 44.1 Khz mono
        if (Pa_GetDeviceInfo(i)->maxInputChannels < 1) {
            ok = false;
        }
        if (ok) {
            DVKRecordingComboBox->insertItem(i,iconOK,audioDevices[i]);
        } else {
            DVKRecordingComboBox->insertItem(i,iconNOK,audioDevices[i]);
        }

        deviceOK[api].append(ok);
    }

    // terminate once we have the list
    Pa_Terminate();
    updateDeviceList(0, 0);
    updateDeviceList(1, 0);
    for (int i = 0; i < NRIG; i++) {
        Format[i].device                    = 0;
        Format[i].suggestedLatency          = 0;
        Format[i].channelCount              = 2;
        Format[i].sampleFormat              = paInt24;
        Format[i].hostApiSpecificStreamInfo = NULL;
    }
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(updateSDR()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejectChanges()));
    updateFromSettings();
}

void SDRDialog::updateFromSettings()
{
    SpotTimeoutLineEdit->setText(settings.value(s_sdr_spottime,s_sdr_spottime_def).toString());
    for (int i = 0; i < NRIG; i++) {
        Checkbox[i]->setChecked(settings.value(s_sdr_enabled[i],s_sdr_enabled_def[i]).toBool());
        OffsetLineEditPtr[i]->setText(settings.value(s_sdr_offset[i],s_sdr_offset_def[i]).toString());
        BitsCombo[i]->setCurrentIndex(settings.value(s_sdr_bits[i],s_sdr_bits_def[i]).toInt());
        int n=settings.value(s_sdr_api[i],0).toInt();
        if (n>nAPI) n=0;
        APICombo[i]->setCurrentIndex(n);
        n=settings.value(s_sdr_device[i],s_sdr_device_def[i]).toInt();
        if (n>(DeviceCombo[i]->maxCount())) n=0;
        DeviceCombo[i]->setCurrentIndex(n);
    }
#ifdef DVK_ENABLE
    Checkbox[2]->setChecked(settings.value(s_dvk_enabled,s_dvk_enabled_def).toBool());
    APICombo[2]->setCurrentIndex(settings.value(s_dvk_api,s_dvk_api_def).toInt());
    DeviceCombo[2]->setCurrentIndex(settings.value(s_dvk_device,s_dvk_device_def).toInt());
    DVKRecordingComboBox->setCurrentIndex(settings.value(s_dvk_rec_device,s_dvk_rec_device_def).toInt());
    loopCheckBox->setChecked(settings.value(s_dvk_loop,s_dvk_loop_def).toBool());
#else
    Checkbox[2]->setEnabled(false);
    APICombo[2]->setEnabled(false);
    DeviceCombo[2]->setEnabled(false);
    DVKRecordingComboBox->setEnabled(false);
#endif
    SpotCallsCheckBox->setChecked(settings.value(s_sdr_mark,s_sdr_mark_def).toBool());
    ChangeRadioClickCheckBox->setChecked(settings.value(s_sdr_changeclick,s_sdr_changeclick_def).toBool());
    lineEditIntegTime->setText(settings.value(s_sdr_cqtime,s_sdr_cqtime_def).toString());
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

void SDRDialog::launchUpdateDeviceList0(int i)
{
    updateDeviceList(0, i);
}

void SDRDialog::launchUpdateDeviceList1(int i)
{
    updateDeviceList(1, i);
}

void SDRDialog::launchUpdateDeviceList2(int i)
{
    updateDeviceList(2, i);
}

void SDRDialog::updateDeviceList(int nr, int indx)
{
    if (nr < 0 || nr >= (NRIG+1)) return;

    DeviceCombo[nr]->clear();
    for (int i = 0; i < nApiDeviceNames[indx].size(); i++) {
        if (deviceOK[indx][i]) {
            DeviceCombo[nr]->insertItem(i, iconOK, nApiDeviceNames[indx].at(i));
        } else {
            DeviceCombo[nr]->insertItem(i, iconNOK, nApiDeviceNames[indx].at(i));
        }
    }
    DeviceCombo[nr]->setCurrentIndex(0);
}

SDRDialog::~SDRDialog()
{
    delete[] nApiDevices;
    delete[] nApiDeviceNames;
    delete[] deviceOK;
}

/*!
 * \brief SDRDialog::format
 * \param nrig either 0 or 1
 * \return Returns PaStreamParameters format for bandscope displays
 */
PaStreamParameters& SDRDialog::format(int nrig)
{
    int nr;
    if (nrig < 0 || nrig >= NRIG) {
        nr = 0;
    } else {
        nr = nrig;
    }
    switch (settings.value(s_sdr_bits[nr],s_sdr_bits_def[nr]).toInt()) {
    case 0:
        Format[nr].sampleFormat = paInt16;
        break;
    case 1:
        Format[nr].sampleFormat = paInt24;
        break;
    case 2:
        Format[nr].sampleFormat = paInt32;
        break;
    }

    // calculate device index
    int indx = 0;
    for (int i = 0; i < settings.value(s_sdr_api[nr],s_sdr_api_def[nr]).toInt(); i++) indx += nApiDevices[i];

    indx += settings.value(s_sdr_device[nr],0).toInt();
    Format[nr].device = indx;
    return(Format[nr]);
}

void SDRDialog::updateSDR()
{
    for (int i = 0; i < NRIG; i++) {
        settings.setValue(s_sdr_bits[i],BitsCombo[i]->currentIndex());
        settings.setValue(s_sdr_offset[i],OffsetLineEditPtr[i]->text().toInt());
        settings.setValue(s_sdr_enabled[i],Checkbox[i]->isChecked());
        settings.setValue(s_sdr_api[i],APICombo[i]->currentIndex());
        settings.setValue(s_sdr_device[i],DeviceCombo[i]->currentIndex());
    }
#ifdef DVK_ENABLE
    settings.setValue(s_dvk_enabled,Checkbox[2]->isChecked());
    settings.setValue(s_dvk_api,APICombo[2]->currentIndex());
    settings.setValue(s_dvk_device,DeviceCombo[2]->currentIndex());
    settings.setValue(s_dvk_rec_device,DVKRecordingComboBox->currentIndex());
    bool updatedvk=false;
    if (settings.value(s_dvk_loop,s_dvk_loop_def).toBool()!=loopCheckBox->isChecked()) updatedvk=true;
    settings.setValue(s_dvk_loop,loopCheckBox->isChecked());

    // calculate device index for DVK audio output device
    int indx = 0;
    for (int i = 0; i < settings.value(s_dvk_api,s_dvk_api_def).toInt(); i++) indx += nApiDevices[i];
    indx += settings.value(s_dvk_device,s_dvk_device_def).toInt();
    settings.setValue(s_dvk_play_devindex,indx);

    // calculate device index for DVK audio input device
    indx = 0;
    for (int i = 0; i < settings.value(s_dvk_api,s_dvk_api_def).toInt(); i++) indx += nApiDevices[i];
    indx += settings.value(s_dvk_rec_device,s_dvk_rec_device_def).toInt();
    settings.setValue(s_dvk_rec_devindex,indx);

#endif
    settings.setValue(s_sdr_cqtime,lineEditIntegTime->text().toInt());
    settings.setValue(s_sdr_spottime,SpotTimeoutLineEdit->text().toInt());
    settings.setValue(s_sdr_mark,SpotCallsCheckBox->isChecked());
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
#ifdef DVK_ENABLE
    if (updatedvk) emit(updateDVK());
#endif
}

void SDRDialog::rejectChanges()
{
    updateFromSettings();
    reject();
}

