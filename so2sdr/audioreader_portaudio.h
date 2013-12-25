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
#ifndef AUDIOREADER_PORTAUDIO_H
#define AUDIOREADER_PORTAUDIO_H

#include <portaudio.h>
#include <QObject>
#include "defines.h"
#include "spectrum.h"

/*!
   Low-level audio input using Portaudio
 */
class AudioReaderPortAudio : public QObject
{
Q_OBJECT

public:
    AudioReaderPortAudio(QObject *parent = 0);
    ~AudioReaderPortAudio();
    bool initialize(const PaStreamParameters &format, sampleSizes s);

public slots:
    void stopReader();

protected:
    static int callback(const void *input, void *output, unsigned long frameCount,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags, void *userdata);

signals:
    void PortAudioError(const QString &);
    void audioReady(unsigned char *, unsigned char);
    void finished();

private:
    bool               initialized;
    bool               running;
    PaError            err;
    PaStreamParameters inputParameters;
    PaStream           *stream;
    sampleSizes        sizes;
    unsigned char      *buff;
    unsigned int       bpmax;
    unsigned int       bptr;
    unsigned long      periodSize;
    unsigned long      readFrames;

    void emitAudioReady();
    bool checkError(PaError err);
};


#endif // AUDIOREADER_PORT_AUDIO_H
