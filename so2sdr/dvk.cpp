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
#include <QDebug>
#include <QDir>
#include <QThread>

DVK::DVK(QSettings &s, QObject *parent) : QObject(parent),settings(s)
{
    connect(this,SIGNAL(messageDone()),this,SLOT(cancelMessage()));
    timer=new QTimer(this);
    timer->setSingleShot(true);
    connect(timer,SIGNAL(timeout()),this,SLOT(clearTimer()));
    audioRunning_=false;
    messagePlaying_=false;
    busy_=false;
    for (int i=0;i<DVK_MAX_MSG;i++) {
        msg[i].sz=0;
        msg[i].snddata=0;
    }
}

DVK::~DVK()
{
    delete(timer);
    for (int i=0;i<DVK_MAX_MSG;i++) {
        if (msg[i].snddata) {
            delete [] msg[i].snddata;
        }
    }
}

bool DVK::audioRunning()
{
    return audioRunning_;
}

bool DVK::messagePlaying()
{
    bool b;
    mutex.lock();
    b=messagePlaying_;
    mutex.unlock();
    return b;
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
 *
 */
void DVK::initializeAudio()
{
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        qDebug("Error starting DVK portaudio");
    } else {
        qDebug("Started DVK portaudio");
        audioRunning_=true;
    }
}

/*!
 * \brief DVK::stopAudio stop Portaudio output
 *
 */
void DVK::stopAudio()
{
    if (audioRunning_) {
        PaError err;
        if (Pa_IsStreamActive(&stream))
        {
            err=Pa_CloseStream(&stream);
        }
#ifdef Q_OS_LINUX
        usleep(100000);
#endif
#ifdef Q_OS_WIN
        Sleep(100);
#endif
        err=Pa_Terminate();
#ifdef Q_OS_LINUX
        usleep(100000);
#endif
#ifdef Q_OS_WIN
        Sleep(100);
#endif

        audioRunning_=false;
    }
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
    msg[0].snddata=new int[sfInfo.frames];
    unsigned long int n=sf_read_int(sndFile, msg[0].snddata, sfInfo.frames);
    msg[0].sz=sfInfo.frames;
    qDebug("read %ld frames\n",sfInfo.frames);
    sf_close(sndFile);
}

/*! Callback for writing audio data
 */
int DVK::writeCallback(const void *input, void *output, unsigned long frameCount,
                                   const PaStreamCallbackTimeInfo* timeInfo,
                                   PaStreamCallbackFlags statusFlags, void *userdata)
{
    Q_UNUSED(input);
    Q_UNUSED(timeInfo);
    Q_UNUSED(statusFlags);

    unsigned long int position=static_cast<DVK*>(userdata)->position;
    unsigned long int sz=static_cast<DVK*>(userdata)->sz;
    int nr=static_cast<DVK*>(userdata)->msgNr;
    int *ptr=& static_cast<DVK*>(userdata)->msg[nr].snddata[position];
    int *out=(int *)output;
    unsigned long m=frameCount;

    if ((sz - position) < frameCount) {
      m=(sz - position);
    }

    // insert zero padding to get only left or right channel output
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
    if (static_cast<DVK*>(userdata)->position==sz) {
        // message finished
        static_cast<DVK*>(userdata)->emitMessageDone();
        static_cast<DVK*>(userdata)->mutex.lock();
        static_cast<DVK*>(userdata)->messagePlaying_=false;
        static_cast<DVK*>(userdata)->mutex.unlock();
        return paComplete;
    }
    //static_cast<DVK*>(userdata)->messagePlaying_=true;
    return paContinue;
}

/*! Callback for recording an audio message
 */
int DVK::recordCallback(const void *input, void *output, unsigned long frameCount,
                                   const PaStreamCallbackTimeInfo* timeInfo,
                                   PaStreamCallbackFlags statusFlags, void *userdata)
{
    Q_UNUSED(output);
    Q_UNUSED(timeInfo);
    Q_UNUSED(statusFlags);

    int pos=static_cast<DVK*>(userdata)->position;
    int nr=static_cast<DVK*>(userdata)->msgNr;
    int *ptr=& static_cast<DVK*>(userdata)->msg[nr].snddata[pos];
    int *inp=(int *)input;

    // terminate after 5 seconds
    if ((pos + frameCount)>= (44100*5) ) {
        static_cast<DVK*>(userdata)->saveMessage();
        return paComplete;
    }

    // copy audio data to buffer
    for (unsigned long int i=0;i<frameCount;i++){
        *ptr= *inp;
        ptr++;
        inp++;
    }
    static_cast<DVK*>(userdata)->position +=frameCount;

    return paContinue;
}


void DVK::emitMessageDone()
{
    emit(messageDone());
}

/*!
 * \brief DVK::cancelMessage cancel any message currently being played
 */
void DVK::cancelMessage()
{
    mutex.lock();
    if (messagePlaying_) {
        messagePlaying_=false;
        Pa_AbortStream(stream);
    }
    mutex.unlock();
}

void DVK::clearTimer()
{
    busy_=false;
}

/*!
 * \brief DVK::playMessage Plays a DVK message
 * \param nr message number
 * \param ch stereo channel 0=right 1=left
 */
void DVK::playMessage(int nr,int ch)
{
    if (busy_) return; // prevents from being called too often
    if (msg[nr].sz==0) return; // skip empty message

    // if currently playing a message, abort it
    cancelMessage();

    mutex.lock();
    messagePlaying_=true;
    mutex.unlock();

    busy_=true;
    timer->start(200);
    msgNr=nr;
    channel=ch;
    position=0;
    sz=msg[nr].sz;

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
                                  writeCallback,
                                  this);
    if (error!=paNoError)
    {
        qDebug("error opening output");
        Pa_AbortStream(stream);
        mutex.lock();
        messagePlaying_=false;
        mutex.unlock();
        return;
    }
    Pa_StartStream(stream);
}

/*!
 * \brief DVK::saveMessage Called when recording is finished
 */
void DVK::saveMessage()
{

}

/*!
 * \brief DVK::recordMessage a DVK message
 * \param nr message number
  */
void DVK::recordMessage(int nr)
{
    // cancel any message currently playing
    if (audioRunning_) {
        cancelMessage();
    }
    // clear out message
    msgNr=nr;
    if (msg[nr].sz!=0) {
        msg[nr].sz=0;
        delete [] msg[nr].snddata;
    }
    msg[nr].snddata=new int[44100*5];

    // allocate enough memory for 5 seconds of recording
    position=0;


    PaStreamParameters inputParameters;
    inputParameters.device = Pa_GetDefaultInputDevice();
    inputParameters.channelCount = 1;
    inputParameters.sampleFormat = paInt32;
    inputParameters.suggestedLatency = 0.1;
    inputParameters.hostApiSpecificStreamInfo = 0;
    PaError error = Pa_OpenStream(&stream,
                                  &inputParameters,
                                  0,
                                  44100,
                                  paFramesPerBufferUnspecified,
                                  paNoFlag,
                                  recordCallback,
                                  this);
    if (error!=paNoError)
    {
        qDebug("error opening input");
        return;
    }
    Pa_StartStream(stream);
}
