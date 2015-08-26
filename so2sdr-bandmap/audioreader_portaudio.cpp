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
#include <math.h>
#include <QDebug>
#include <QThread>
#include "audioreader_portaudio.h"
#include "utils.h"

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif


AudioReaderPortAudio::AudioReaderPortAudio(QString settingsFile, QObject *parent):SdrDataSource(settingsFile,parent)
{
    bptr        = 0;
    buff        = 0;
    stream      = NULL;
    bpmax       = 0;
    periodSize  = 0;
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
    initialized     = false;
    bpmax           = sizes.chunk_size / sizes.advance_size;
    inputParameters.device=settings->value(s_sdr_deviceindx,s_sdr_deviceindx_def).toInt();
    switch (settings->value(s_sdr_bits,s_sdr_bits_def).toInt()) {
    case 0:
        inputParameters.sampleFormat = paInt16;
        break;
    case 1:
        inputParameters.sampleFormat = paInt24;
        break;
    case 2:
        inputParameters.sampleFormat = paInt32;
        break;
    }
    inputParameters.channelCount=2;
    inputParameters.hostApiSpecificStreamInfo=NULL;

    if (buff) {
        delete [] buff;
    }
    buff = new unsigned char[sizes.chunk_size];
    for (unsigned long i = 0; i < sizes.chunk_size; i++) {
        buff[i] = 0;
    }
    stream = NULL;
    err    = Pa_Initialize();
    if (checkError(err)) {
        stop();
        return;
    }
    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        emit(error("ERROR: PortAudio found no audio devices"));
        return;
    }
    if (Pa_GetDeviceInfo(inputParameters.device)==NULL) {
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

    err = Pa_IsFormatSupported(&inputParameters, NULL,
                               settings->value(s_sdr_sound_sample_freq,s_sdr_sound_sample_freq_def).toInt());
    if (checkError(err)) {
        stop();
        return;
    }
    err = Pa_OpenStream(
                &stream,
                &inputParameters, NULL,
                settings->value(s_sdr_sound_sample_freq,s_sdr_sound_sample_freq_def).toInt(),
                periodSize,
                paClipOff | paDitherOff,
                callback,
                this);
    if (checkError(err)) {
        stop();
        return;
    }
    err = Pa_StartStream(stream);
    if (checkError(err)) {
        stop();
        return;
    }
    mutex.lock();
    initialized = true;
    running = true;
    mutex.unlock();
}

/*!
   Stop audio input
 */
void AudioReaderPortAudio::stop()
{
    if (!running || Pa_IsStreamStopped(stream)) {
        return;
    }
    do {
        err = Pa_CloseStream(stream);
        delay(100);
        if (err != paNoError) {
            emit(error("ERROR: could not close PortAudio stream"));
            delay(1000);
        }
    } while (err != paNoError);

    do {
        err = Pa_Terminate();
        if (err != paNoError) {
            emit(error("ERROR: could not terminate PortAudio"));
            delay(1000);
        }
    } while (err != paNoError);
    running = false;
    emit(stopped());
}

/*! Callback for reading audio data
 */
int AudioReaderPortAudio::callback(const void *input, void *output, unsigned long frameCount,
                                   const PaStreamCallbackTimeInfo* timeInfo,
                                   PaStreamCallbackFlags statusFlags, void *userdata)
{
    Q_UNUSED(output);
    Q_UNUSED(frameCount);
    Q_UNUSED(timeInfo);
    Q_UNUSED(statusFlags);
    int           sz       = static_cast<AudioReaderPortAudio*>(userdata)->sizes.advance_size;
    int           bp       = static_cast<AudioReaderPortAudio*>(userdata)->bptr;
    unsigned char *ptr_in  = (unsigned char *) input;
    unsigned char *ptr_out = static_cast<AudioReaderPortAudio*>(userdata)->buff;

    // copy data into circular buffer
    for (int i = 0; i < sz; i++) {
        ptr_out[bp * sz + i] = *ptr_in++;
    }
    static_cast<AudioReaderPortAudio*>(userdata)->emitAudioReady();
    return(paContinue);
}

/*!
   called by callback when done reading data
 */
void AudioReaderPortAudio::emitAudioReady()
{
    bptr++;
    bptr = bptr % bpmax;
    emit(ready(buff, bptr));
}

