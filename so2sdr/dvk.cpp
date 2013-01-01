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
#include "defines.h"
#include "dvk.h"
#include <portaudio.h>
#include <sndfile.h>
#include <QDir>

DVK::DVK(QSettings *s,QObject *parent) :
    QObject(parent)
{
    settings=s;
}

/*!
 * \brief DVK::sndfile_version
 * \return Returns libsndfile version string
 */
QString DVK::sndfile_version()
{
    char  buffer[128];
    sf_command(NULL, SFC_GET_LIB_VERSION, buffer, sizeof (buffer));
    return QString(buffer);
}

/*!
 * \brief DVK::initializeAudio Initialize Portaudio output
 * \return true on success
 */
void DVK::initializeAudio()
{
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        qDebug("Error starting DVK portaudio");
        return;
    } else {
        qDebug("Started DVK portaudio");
        return;
    }
}

/*!
 * \brief DVK::stopAudio stop Portaudio output
 * \return true on success
 */
void DVK::stopAudio()
{
    Pa_StopStream(stream);
    Pa_Terminate();
}

/*!
 * \brief DVK::loadMessages Reads DVK messages from disk. Format should be 44.1KHz WAV mono.
 * Files are placed in a subdirectory contestname_dvk/. Format of filenames:
 *  call_operator_f1.wav   etc.
 *
 * \param call Station callsign
 * \param op Operator callsign or name
 */
void DVK::loadMessages(QString filename,QString op)
{
    Q_UNUSED(op)
    SNDFILE *sndFile;
    SF_INFO sfInfo;

    // initial test: just load F1 message
    QDir directory;
    QString tmp=filename;
    tmp.chop(4);
    tmp=tmp+"_dvk/";
    directory.setCurrent(tmp);

    // open wav file
    sndFile = sf_open("n4ogw_n4ogw_f1.wav",SFM_READ,&sfInfo);
    if (!sndFile) {
        qDebug("error opening wav file");
    } else {
        qDebug("frames=%d samplerate=%d channels=%d",(int)sfInfo.frames,(int)sfInfo.samplerate,(int)sfInfo.channels);
    }
    if ((int)sfInfo.channels!=1) {
        sf_close(sndFile);
        return;
    }

    // read in WAV
    func[0].snddata=new int[sfInfo.frames];
    unsigned long int n=sf_read_int(sndFile, func[0].snddata, sfInfo.frames);
    func[0].sz=sfInfo.frames;
    qDebug("read %ld frames\n",sfInfo.frames);
    sf_close(sndFile);
}

/*! Callback for writing audio data
 */
int DVK::callback(const void *input, void *output, unsigned long frameCount,
                                   const PaStreamCallbackTimeInfo* timeInfo,
                                   PaStreamCallbackFlags statusFlags, void *userdata)
{
    Q_UNUSED(input);
    Q_UNUSED(timeInfo);
    Q_UNUSED(statusFlags);

    unsigned long int position=static_cast<DVK*>(userdata)->position;
    unsigned long int sz=static_cast<DVK*>(userdata)->sz;
    int *ptr=& static_cast<DVK*>(userdata)->snddata[position];
    int *out=(int *)output;
    unsigned long m=frameCount;

    if ((sz - position) < frameCount) {
      m-=(sz - position);
    }
    // insert padding to get only left or right channel output
    for (unsigned long int i=0;i<m;i++) {
      if (static_cast<DVK*>(userdata)->channel) {
        *out = *ptr;
        out++;
        *out = 0;
        out++;
      } else {
        *out = 0;
        out++;
        *out = *ptr;
        out++;
      }
      ptr++;
    }
    static_cast<DVK*>(userdata)->position += m;
    return paContinue;
}

void DVK::playMessage(int nr,int ch)
{
    qDebug("playing message %d",nr);
    channel=ch;
    position=0;
    sz=func[nr].sz;
    snddata=new int[func[nr].sz];
    for (unsigned long int i=0;i<func[nr].sz;i++) snddata[i]=func[nr].snddata[i];

    PaStreamParameters outputParameters;
    outputParameters.device = Pa_GetDefaultOutputDevice();
    outputParameters.channelCount = 2;
    outputParameters.sampleFormat = paInt32;
    outputParameters.suggestedLatency = 0.1;
    outputParameters.hostApiSpecificStreamInfo = 0;
    PaError error = Pa_OpenStream(&stream,
                                  0,
                                  &outputParameters,
                                  44100,
                                  paFramesPerBufferUnspecified,
                                  paNoFlag,
                                  callback,
                                  this);
    if (error!=paNoError)
    {
        qDebug("error opening output");
        return;
    }
    Pa_StartStream(stream);
    Pa_Sleep(1000);
    Pa_StopStream(stream);
    delete [] snddata;
}
