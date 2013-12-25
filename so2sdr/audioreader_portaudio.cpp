/*! Copyright 2010-2014 R. Torsten Clay N4OGW

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
#include "audioreader_portaudio.h"

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#include "pa_asio.h"
#endif

AudioReaderPortAudio::AudioReaderPortAudio(QObject *parent) : QObject(parent)
{
    bptr        = 0;
    buff        = 0;
    initialized = false;
}

AudioReaderPortAudio::~AudioReaderPortAudio()
{
    delete [] buff;
}

bool AudioReaderPortAudio::checkError(PaError err)
{
    if (err != paNoError) {
        emit(PortAudioError(Pa_GetErrorText(err)));
        return(true);
    } else {
        return(false);
    }
}

/*!
   Start the audio device

 */
bool AudioReaderPortAudio::initialize(const PaStreamParameters &format, sampleSizes s)
{
    initialized     = false;
    inputParameters = format;
    sizes           = s;
    bpmax           = sizes.chunk_size / sizes.advance_size;
    if (buff) {
        delete [] buff;
    }
    buff = new unsigned char[sizes.chunk_size];
    for (unsigned long i = 0; i < sizes.chunk_size; i++) {
        buff[i] = 0;
    }
    stream = NULL;
    err    = Pa_Initialize();
    if (checkError(err)) return(false);

#ifndef Q_OS_LINUX
    // show ASIO control panel if using ASIO
    if (Pa_GetHostApiInfo(Pa_GetDeviceInfo(format.device)->hostApi)->type == paASIO) {
        PaAsio_ShowControlPanel(inputParameters.device, NULL);
    }
#endif

    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        emit(PortAudioError("ERROR: PortAudio found no audio devices"));
        return(false);
    }
    int b;
    switch (inputParameters.sampleFormat) {
    case paInt24:

        // 24 bits packed in 32bit size
        b = 6;
        break;
    case paInt32:

        // 32 bits; 8 bytes/frame
        b = 8;
        break;
    case paInt16:

        // 16 bits; 4 bytes/frame
        b = 4;
        break;
    default:
        b = 4;
    }

    // number of frames to read per scanline of spectrum
    readFrames = sizes.advance_size / b;

    // check we have a stereo device
    if (Pa_GetDeviceInfo(inputParameters.device)->maxInputChannels < 2) {
        emit(PortAudioError("ERROR: audio device does not support stereo"));
        return(false);
    }

    // set suggested latency
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    periodSize                       = 1024;
    err                              = Pa_IsFormatSupported(&inputParameters, NULL, 96000);
    if (checkError(err)) return(false);
    err = Pa_OpenStream(
        &stream,
        &inputParameters, NULL,
        96000,
        periodSize,
        paClipOff | paDitherOff,
        callback,
        this);
    if (checkError(err)) return(false);
    err = Pa_StartStream(stream);
    if (checkError(err)) return(false);
    initialized = true;
    return(true);
}

/*!
   Stop audio input
 */
void AudioReaderPortAudio::stopReader()
{
    if (!initialized) {
        emit(finished());
        return;
    }
    do {
        err = Pa_CloseStream(stream);
#ifdef Q_OS_LINUX
        usleep(100000);
#endif
#ifdef Q_OS_WIN
        Sleep(100);
#endif
        if (err != paNoError) {
            emit(PortAudioError("ERROR: could not close PortAudio stream"));
#ifdef Q_OS_LINUX
            sleep(1);
#endif
#ifdef Q_OS_WIN
            Sleep(1000);
#endif
        }
    } while (err != paNoError);

    do {
        err = Pa_Terminate();
        if (err != paNoError) {
            emit(PortAudioError("ERROR: could not terminate PortAudio"));
#ifdef Q_OS_LINUX
            sleep(1);
#endif
#ifdef Q_OS_WIN
            Sleep(1000);
#endif
        }
    } while (err != paNoError);

    emit(finished());
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
    emit(audioReady(buff, bptr));
}
