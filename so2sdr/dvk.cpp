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

/*!
  DVK support. Provides the following functionality

  -record messages
  -play back stored WAV files
  -in live mode, loop audio from input to output

  Uses portaudio and libsndfile for audio I/O
*/
DVK::DVK(QSettings &s, QObject *parent) : QObject(parent),settings(s)
{
    connect(this,SIGNAL(messageDone()),this,SLOT(cancelMessage()));
    timer=new QTimer(this); // see clearTimer below
    timer->setSingleShot(true);
    connect(timer,SIGNAL(timeout()),this,SLOT(clearTimer()));
    audioRunning_=false;
    messagePlaying_=false;
    messageRecording_=false;
    loopback_=false;
    restartLoopback_=false;
    busy_=false;
    channel=0;
    liveChannel=0;
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

/*!
 * \brief DVK::setLiveChannel
 * \param ch : 0 or 1
 * sets the radio number for output during live audio
 */
void DVK::setLiveChannel(int ch)
{
    liveChannel=ch;
}

/*!
 * \brief DVK::audioRunning
 * \return state of DVK Portaudio
 */
bool DVK::audioRunning()
{
    return audioRunning_;
}

/*!
 * \brief DVK::messagePlaying
 * \return message playing state
 */
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
    // open streams
    // there are 3 streams: play, record, and loopback
    //
    PaStreamParameters inputParameters;
    inputParameters.device=settings.value(s_dvk_rec_devindex,s_dvk_rec_devindex_def).toInt();
    inputParameters.channelCount = 1;
    inputParameters.sampleFormat = paInt32;
    inputParameters.suggestedLatency = 0.1;
    inputParameters.hostApiSpecificStreamInfo = 0;

    PaStreamParameters outputParameters;
    outputParameters.device=settings.value(s_dvk_play_devindex,s_dvk_play_devindex_def).toInt();
    outputParameters.channelCount = 2;
    outputParameters.sampleFormat = paInt32;
    outputParameters.suggestedLatency = 0.1;
    outputParameters.hostApiSpecificStreamInfo = 0;

    PaError error = Pa_OpenStream(&loopStream,
                                  &inputParameters,
                                  &outputParameters,
                                  44100,
                                  paFramesPerBufferUnspecified,
                                  paNoFlag,
                                  loopCallback,
                                  this);
    Pa_StopStream(loopStream);
    if (error!=paNoError) {
        qDebug("Pa_OpenStream error=%d loopStream",(int)error);
    }

    error = Pa_OpenStream(&playStream,
                                  NULL,
                                  &outputParameters,
                                  44100,
                                  paFramesPerBufferUnspecified,
                                  paNoFlag,
                                  writeCallback,
                                  this);
    Pa_StopStream(playStream);
    if (error!=paNoError) {
        qDebug("Pa_OpenStream error=%d playStream",(int)error);
    }

    error = Pa_OpenStream(&recStream,
                                  &inputParameters,
                                  NULL,
                                  44100,
                                  paFramesPerBufferUnspecified,
                                  paNoFlag,
                                  recordCallback,
                                  this);
    Pa_StopStream(recStream);
    if (error!=paNoError) {
        qDebug("Pa_OpenStream error=%d recStream",(int)error);
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
        if (Pa_IsStreamActive(&playStream)==1)
        {
            err=Pa_CloseStream(&playStream);
        }
        if (Pa_IsStreamActive(&recStream)==1)
        {
            err=Pa_CloseStream(&recStream);
        }
        if (Pa_IsStreamActive(&loopStream)==1)
        {
            err=Pa_CloseStream(&loopStream);
        }
        Pa_Sleep(100);
        err=Pa_Terminate();
        Pa_Sleep(100);
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
    /*
    if ((int)sfInfo.channels!=1) {
        sf_close(sndFile);
        return;
    }*/

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

    unsigned long int pos=static_cast<DVK*>(userdata)->position;
    int nr=static_cast<DVK*>(userdata)->msgNr;
    int *ptr=& static_cast<DVK*>(userdata)->msg[nr].snddata[pos];
    int *inp=(int *)input;

    // terminate if over the max length or signal given to stop
    if (!(static_cast<DVK*>(userdata)->messageRecording_) || (pos + frameCount)>= (44100*DVK_MAX_LEN) ) {
        static_cast<DVK*>(userdata)->msg[nr].sz=pos+frameCount;
        static_cast<DVK*>(userdata)->saveMessage();
        static_cast<DVK*>(userdata)->emitMessageDone();
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

/*! Callback for live audio: loops audio input to output
 */
int DVK::loopCallback(const void *input, void *output, unsigned long frameCount,
                                   const PaStreamCallbackTimeInfo* timeInfo,
                                   PaStreamCallbackFlags statusFlags, void *userdata)
{
    Q_UNUSED(timeInfo);
    Q_UNUSED(statusFlags);

    int *inp=(int *)input;
    int *out=(int *)output;

    // copy audio data to output
    for (unsigned long int i=0;i<frameCount;i++){
        if (static_cast<DVK*>(userdata)->liveChannel) {
          *out = *inp;
          out++;
          *out = 0;
          out++;
        } else {
          *out = 0;
          out++;
          *out = *inp;
          out++;
        }
        inp++;
    }
    frameCount=0;

    // check for exit
    if (!static_cast<DVK*>(userdata)->loopback_) {
        qDebug("loop callback got stop");
        return paComplete;
    }

    return paContinue;
}


/*!
 * \brief DVK::emitMessageDone
 * send signal that current message has finished playing
 */
void DVK::emitMessageDone()
{
    emit(ptt(channel,0));
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
        emit(ptt(channel,0));
        Pa_AbortStream(playStream);
    }
    mutex.unlock();
    // restart live audio
    if (restartLoopback_) {
        Pa_StartStream(loopStream);
        restartLoopback_=false;
    }
}

/*!
 * \brief DVK::clearTimer
 * reset busy timer. Timer is used to prevent triggering messages too rapidly. Only allow
 * a new message to be played after DVK_BUSY_TIMER ms.
 */
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
    qDebug("play message nr=%d size=%lu",nr,msg[nr].sz);
    if (busy_) return; // prevents from being called too often
    if (msg[nr].sz==0) return; // skip empty message

    // if currently playing a message, abort it
    cancelMessage();
    mutex.lock();
    messagePlaying_=true;
    mutex.unlock();

    // if currently in live audio mode, stop it
    if (Pa_IsStreamActive(loopStream)==1) {
        Pa_AbortStream(loopStream);
        restartLoopback_=true;
    }
    busy_=true;
    timer->start(DVK_BUSY_TIMER);
    msgNr=nr;
    channel=ch;
    position=0;
    sz=msg[nr].sz;

    Pa_StopStream(playStream);
    while (Pa_IsStreamActive(playStream)==1) {
        Pa_AbortStream(playStream);
        Pa_Sleep(10);
    }
    emit(ptt(channel,1));
    PaError error=Pa_StartStream(playStream);

    if (error!=paNoError)
    {
        qDebug("error %d starting output stream",(int)error);
        Pa_AbortStream(playStream);
        mutex.lock();
        messagePlaying_=false;
        mutex.unlock();
    }
}

/*!
 * \brief DVK::saveMessage Called when recording is finished
 */
void DVK::saveMessage()
{
    qDebug("SAVE message");
}

/*!
 * \brief DVK::recordMessage a DVK message
 * \param nr message number
 * if called a second time, stops recording
  */
void DVK::recordMessage(int nr)
{
    // cancel any message currently playing
    if (audioRunning_) {
        cancelMessage();
    }
    // stop live audio mode

    if (Pa_IsStreamActive(loopStream)==1) {
        Pa_AbortStream(loopStream);
        restartLoopback_=true;
    }

    // if recording a message, end that recording
    if (messageRecording_) {
        messageRecording_=false;
        return;
    } else {
        messageRecording_=true;
    }

    // clear out message
    // allocate enough memory for up to 20 seconds of recording
    msgNr=nr;
    if (msg[nr].sz!=0) {
        msg[nr].sz=0;
        delete [] msg[nr].snddata;
    }
    msg[nr].snddata=new int[44100*DVK_MAX_LEN];

    position=0;
    Pa_StopStream(recStream);
    while (Pa_IsStreamActive(recStream)==1) {
        Pa_AbortStream(recStream);
        Pa_Sleep(10);
    }
    PaError error = Pa_StartStream(recStream);

    if (error!=paNoError)
    {
        qDebug("error %d starting recording stream",(int)error);
        Pa_AbortStream(recStream);
    }
}

/*!
 * \brief DVK::loopAudio starts live audio mode
 *  loops audio input to audio output
  */
void DVK::loopAudio()
{
    // cancel any message currently playing
    if (audioRunning_) {
        cancelMessage();
    }
    Pa_StopStream(loopStream);
    while (Pa_IsStreamActive(loopStream)==1) {
        Pa_AbortStream(loopStream);
        Pa_Sleep(10);
    }
    PaError error=Pa_StartStream(loopStream);

    if (error!=paNoError)
    {
        qDebug("error %d starting loop stream",(int)error);
        Pa_AbortStream(loopStream);
        return;
    }

    mutex.lock();
    loopback_=true;
    mutex.unlock();

}

/*!
 * \brief DVK::stopLoopAudio
 * stop loopback mode if it is active
 */
void DVK::stopLoopAudio()
{
    mutex.lock();
    loopback_=false;
    mutex.unlock();
}
