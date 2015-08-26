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
#ifndef AUDIOREADER_PORTAUDIO_H
#define AUDIOREADER_PORTAUDIO_H

#include <portaudio.h>
#include <QSettings>
#include "defines.h"
#include "spectrum.h"
#include "sdrdatasource.h"

/*!
   Low-level audio input using Portaudio
 */
class AudioReaderPortAudio : public SdrDataSource
{
Q_OBJECT

public:
    AudioReaderPortAudio(QString settingsFile,QObject *parent=0);
    ~AudioReaderPortAudio();

protected:
    static int callback(const void *input, void *output, unsigned long frameCount,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags, void *userdata);
public slots:
    void stop();
    void initialize();

private:
    PaStreamParameters inputParameters;
    PaError            err;
    PaStream           *stream;
    unsigned char      *buff;
    unsigned int       bpmax;
    unsigned int       bptr;
    unsigned long      periodSize;

    void emitAudioReady();
    bool checkError(PaError err);
};

#endif // AUDIOREADER_PORT_AUDIO_H
