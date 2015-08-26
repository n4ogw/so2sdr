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
#include "soundcardsetup.h"

/*!
 * \brief SoundCardSetup::SoundCardSetup
 *    Dialog to configure soundcard-based SDR
 * \param s
 *    Global QSettings object
 * \param parent
 */
SoundCardSetup::SoundCardSetup(QSettings &s,QWidget *parent) : QDialog(parent),settings(s)
{
    setupUi(this);
    iconOK               = QIcon("check.png");
    iconNOK              = QIcon("x.png");
    BitsComboBox->insertItem(0,"32",Qt::DisplayRole);
    BitsComboBox->insertItem(0,"24",Qt::DisplayRole);
    BitsComboBox->insertItem(0,"16",Qt::DisplayRole);

    // find audio devices; start Portaudio to get the device list
    Pa_Initialize();
    nAPI            = Pa_GetHostApiCount();
    nApiDevices     = new int[nAPI+1];
    nApiDeviceNames = new QList<QString>[nAPI+1];
    deviceOK        = new QList<bool>[nAPI+1];
    for (int i = 0; i < nAPI; i++) {
        deviceOK[i].clear();
        nApiDevices[i] = 0;
        nApiDeviceNames[i].clear();
        APIComboBox->insertItem(i, Pa_GetHostApiInfo(i)->name,Qt::DisplayRole);
    }

    connect(APIComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateDeviceList(int)));

    audioDevices.clear();
    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        numDevices = 0;
    }
    PaStreamParameters testFormat;
    testFormat.channelCount              = 2;
    testFormat.sampleFormat              = paInt16;
    testFormat.suggestedLatency          = 0;
    testFormat.hostApiSpecificStreamInfo = NULL;
    for (int i = 0; i < numDevices; i++) {
        const PaDeviceInfo *deviceInfo;
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
        if (Pa_GetDeviceInfo(i)->maxInputChannels != 2) {
            ok = false;
        }

        // test for support of chosen sample rate
        PaError err = Pa_IsFormatSupported(&testFormat, NULL, settings.value(s_sdr_sound_sample_freq,
                                                                             s_sdr_sound_sample_freq_def).toInt());
        if (err != paNoError) ok = false;
        if (ok) {
            SoundCardComboBox->insertItem(i, iconOK, audioDevices[i],Qt::DisplayRole);
        } else {
            SoundCardComboBox->insertItem(i, iconNOK, audioDevices[i],Qt::DisplayRole);
        }
        deviceOK[api].append(ok);
    }
    // terminate once we have the list
    PaError err=Pa_Terminate();
    if (err != paNoError) {
        emit(PortAudioError("ERROR: couldn't terminate test portaudio"));
    }
    updateDeviceList(0);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(updateSoundCard()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejectChanges()));
    updateFromSettings();
}

SoundCardSetup::~SoundCardSetup()
{
    delete[] nApiDevices;
    delete[] nApiDeviceNames;
    delete[] deviceOK;
}

/*!
 * \brief SoundCardSetup::rejectChanges
 *   called when "cancel" is clicked. Resets widgets to values in settings.
 */
void SoundCardSetup::rejectChanges()
{
    updateFromSettings();
    reject();
}

/*!
 * \brief SoundCardSetup::updateSoundCard
 *   Update settings object from widgets in dialog
 */
void SoundCardSetup::updateSoundCard()
{
    switch (SampleRateComboBox->currentIndex()) {
    case 0:
        settings.setValue(s_sdr_sound_sample_freq,48000);
        break;
    case 1:
        settings.setValue(s_sdr_sound_sample_freq,96000);
        break;
    case 2:
        settings.setValue(s_sdr_sound_sample_freq,192000);
        break;
    }
    switch (fftComboBox->currentIndex()) {
    case 0:
        settings.setValue(s_sdr_sound_speed,1);
        break;
    case 1:
        settings.setValue(s_sdr_sound_speed,2);
        break;
    case 2:
        settings.setValue(s_sdr_sound_speed,4);
        break;
    }
    settings.setValue(s_sdr_bits,BitsComboBox->currentIndex());
    settings.setValue(s_sdr_offset_soundcard,OffsetLineEdit->text().toInt());
    settings.setValue(s_sdr_api,APIComboBox->currentIndex());
    settings.setValue(s_sdr_device,SoundCardComboBox->currentIndex());

    // calculate overall device index- can be different from s_sdr_device if there
    // are multiple API's
    int indx = 0;
    for (int i = 0; i < settings.value(s_sdr_api,s_sdr_api_def).toInt(); i++) indx += nApiDevices[i];
    indx += settings.value(s_sdr_device,0).toInt();
    settings.setValue(s_sdr_deviceindx,indx);
    settings.setValue(s_sdr_swap_soundcard,checkBoxSwap->isChecked());
    settings.setValue(s_sdr_iqcorrect,checkBoxIq->isChecked());
    settings.setValue(s_sdr_iqdata,checkBoxIQData->isChecked());
}

/*!
 * \brief SoundCardSetup::updateFromSettings
 *   update widgets from settings object
 */
void SoundCardSetup::updateFromSettings()
{
    switch (settings.value(s_sdr_sound_speed,s_sdr_sound_speed_def).toInt()) {
    case 1:
        fftComboBox->setCurrentIndex(0);
        break;
    case 2:
        fftComboBox->setCurrentIndex(1);
        break;
    case 4:
        fftComboBox->setCurrentIndex(2);
        break;
    default:
        fftComboBox->setCurrentIndex(0);

    }
    switch (settings.value(s_sdr_sound_sample_freq,s_sdr_sound_sample_freq_def).toInt()) {
    case 48000:
        SampleRateComboBox->setCurrentIndex(0);
        break;
    case 96000:
        SampleRateComboBox->setCurrentIndex(1);
        break;
    case 192000:
        SampleRateComboBox->setCurrentIndex(2);
        break;
    default:
        SampleRateComboBox->setCurrentIndex(1);
    }
    BitsComboBox->setCurrentIndex(settings.value(s_sdr_bits,s_sdr_bits_def).toInt());
    int n=settings.value(s_sdr_api,s_sdr_api_def).toInt();
    APIComboBox->setCurrentIndex(n);
    n=settings.value(s_sdr_device,s_sdr_device_def).toInt();
    SoundCardComboBox->setCurrentIndex(n);
    checkBoxSwap->setChecked(settings.value(s_sdr_swap_soundcard,s_sdr_swap_soundcard_def).toBool());
    checkBoxIq->setChecked(settings.value(s_sdr_iqcorrect,s_sdr_iqcorrect_def).toBool());
    checkBoxIQData->setChecked(settings.value(s_sdr_iqdata,s_sdr_iqdata_def).toBool());
    OffsetLineEdit->setText(settings.value(s_sdr_offset_soundcard,s_sdr_offset_soundcard_def).toString());
}

void SoundCardSetup::updateDeviceList(int indx)
{
    SoundCardComboBox->clear();
    for (int i = 0; i < nApiDeviceNames[indx].size(); i++) {
        if (deviceOK[indx][i]) {
            SoundCardComboBox->insertItem(i, iconOK, nApiDeviceNames[indx].at(i),Qt::DisplayRole);
        } else {
            SoundCardComboBox->insertItem(i, iconNOK, nApiDeviceNames[indx].at(i),Qt::DisplayRole);
        }
    }
    SoundCardComboBox->setCurrentIndex(0);
}
