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
#ifndef AUDIOREADER_PORTAUDIO_H
#define AUDIOREADER_PORTAUDIO_H

#include "sdrdatasource.h"
#include <QSettings>
#include <portaudio.h>

/*!
   Low-level audio input using Portaudio
 */
class AudioReaderPortAudio : public SdrDataSource {
  Q_OBJECT

public:
  AudioReaderPortAudio(QString settingsFile, QObject *parent = nullptr);
  ~AudioReaderPortAudio();
  unsigned int sampleRate() const;

protected:
  static int callback(const void *input, void *output, unsigned long frameCount,
                      const PaStreamCallbackTimeInfo *timeInfo,
                      PaStreamCallbackFlags statusFlags, void *userdata);
public slots:
  void stop();
  void initialize();
  void setRfFreq(double f) { Q_UNUSED(f) }

private:
  PaStreamParameters inputParameters;
  PaError err;
  PaStream *stream;
  unsigned char *buff;
  unsigned char *ptr;
  unsigned int bpmax;
  unsigned int bptr;
  unsigned int iptr;
  unsigned long periodSize;
  int frameSize;

  void emitAudioReady();
  bool checkError(PaError err);
  void stopAudioreader();
};

#endif // AUDIOREADER_PORT_AUDIO_H
