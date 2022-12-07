/*! Copyright 2010-2022 R. Torsten Clay N4OGW

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

#include "audioreader_portaudio.h"

AudioReaderPortAudio::AudioReaderPortAudio(QString settingsFile, QObject *parent):SdrDataSource(settingsFile,parent)
{
    bptr        = 0;
    buff        = nullptr;
    stream      = nullptr;
    ptr         = nullptr;
    bpmax       = 0;
    iptr        = 0;
    periodSize  = 0;
    stopFlag    = false;
    frameSize   = 4;
}

AudioReaderPortAudio::~AudioReaderPortAudio()
{
    if (buff) {
        delete [] buff;
    }
}

bool AudioReaderPortAudio::checkError(PaError err)
{
    if (err != paNoError) {
        emit(error(Pa_GetErrorText(err)));
        return(true);
    } else {
        return(false);
    }
}

/*!
   Start the audio device

 */
void AudioReaderPortAudio::initialize()
{
    stopFlag=false;
    if (stream != nullptr) {
        Pa_StopStream(stream);
        Pa_Terminate();
    }
    bpmax           = sizes.chunk_size / sizes.advance_size;
    inputParameters.device=settings->value(s_sdr_deviceindx,s_sdr_deviceindx_def).toInt();
    switch (settings->value(s_sdr_bits,s_sdr_bits_def).toInt()) {
    case 0:
        inputParameters.sampleFormat = paInt16;
        frameSize = 4;
        break;
    case 1:
        inputParameters.sampleFormat = paInt24;
        frameSize = 6;
        break;
    case 2:
        inputParameters.sampleFormat = paInt32;
        frameSize = 8;
        break;
    }
    inputParameters.channelCount=2;
    inputParameters.hostApiSpecificStreamInfo=nullptr;
    if (buff) {
        delete [] buff;
    }
    buff = new unsigned char[sizes.chunk_size];
    for (unsigned long i = 0; i < sizes.chunk_size; i++) {
        buff[i] = 0;
    }
    ptr = buff;
    bptr = 0;
    iptr = 0;
    stream = nullptr;
    err    = Pa_Initialize();
    if (checkError(err)) {
        qDebug("Pa_Initialize failed");
        stop();
        return;
    }
    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        emit(error("ERROR: PortAudio found no audio devices"));
        return;
    }
    if (Pa_GetDeviceInfo(inputParameters.device)==nullptr) {
        emit(error("ERROR: getdeviceinfo returned null pointer"));
        return;
    }
    if (Pa_GetDeviceInfo(inputParameters.device)->maxInputChannels < 2) {
        emit(error("ERROR: audio device does not support stereo"));
        return;
    }
    // set suggested latency
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    periodSize=settings->value(s_sdr_fft,s_sdr_fft_def).toInt()/4;
    err = Pa_IsFormatSupported(&inputParameters, nullptr,
                               settings->value(s_sdr_sound_sample_freq,s_sdr_sound_sample_freq_def).toInt());
    if (checkError(err)) {
        qDebug("Portaudio format not supported");
        stop();
        return;
    }
    err = Pa_OpenStream(
                &stream,
                &inputParameters, nullptr,
                settings->value(s_sdr_sound_sample_freq,s_sdr_sound_sample_freq_def).toInt(),
                periodSize,
                paClipOff | paDitherOff,
                callback,
                this);
    if (checkError(err)) {
        qDebug("Pa_OpenStream failed");
        stop();
        return;
    }
    err = Pa_StartStream(stream);
    if (checkError(err)) {
        qDebug("Pa_StartStream failed");
        stop();
        return;
    }

    running = true;
}

/*!
   Sset flag to stop audio input.
 */
void AudioReaderPortAudio::stop()
{
    stopFlag=true;
}

void AudioReaderPortAudio::stopAudioreader()
{
    stopFlag=false;
    running=false;
    emit(stopped());
}

/*! Callback for reading audio data
 */
int AudioReaderPortAudio::callback(const void *input, void *output, unsigned long frameCount,
                                   const PaStreamCallbackTimeInfo* timeInfo,
                                   PaStreamCallbackFlags statusFlags, void *userdata)
{
    Q_UNUSED(output)
    Q_UNUSED(timeInfo)
    Q_UNUSED(statusFlags)

    unsigned long sz       = static_cast<AudioReaderPortAudio*>(userdata)->sizes.advance_size;
    unsigned char *ptr_in  = (unsigned char *) input;
    unsigned char *ptr_out = static_cast<AudioReaderPortAudio*>(userdata)->ptr;

    // copy data into circular buffer
    for (unsigned long i = 0; i < frameCount*static_cast<AudioReaderPortAudio*>(userdata)->frameSize; i++) {
        *ptr_out = *ptr_in;
        ptr_in++;
        ptr_out++;
    }
    static_cast<AudioReaderPortAudio*>(userdata)->ptr += frameCount*static_cast<AudioReaderPortAudio*>(userdata)->frameSize;
    static_cast<AudioReaderPortAudio*>(userdata)->iptr += frameCount*static_cast<AudioReaderPortAudio*>(userdata)->frameSize;

    // make buffer circular
    if (static_cast<AudioReaderPortAudio*>(userdata)->iptr == static_cast<AudioReaderPortAudio*>(userdata)->sizes.chunk_size) {
        static_cast<AudioReaderPortAudio*>(userdata)->iptr = 0;
        static_cast<AudioReaderPortAudio*>(userdata)->ptr = static_cast<AudioReaderPortAudio*>(userdata)->buff;
    }
    if (!(static_cast<AudioReaderPortAudio*>(userdata)->iptr % sz)) {
        static_cast<AudioReaderPortAudio*>(userdata)->emitAudioReady();
    }
    // check for stop
    if (static_cast<AudioReaderPortAudio*>(userdata)->stopFlag) {
        static_cast<AudioReaderPortAudio*>(userdata)->stopAudioreader();
        return(paAbort);
    } else {
        return(paContinue);
    }
}

/*!
   called by callback when done reading data
 */
void AudioReaderPortAudio::emitAudioReady()
{
    emit(ready(buff, bptr));
    bptr++;
    bptr = bptr % bpmax;
}

