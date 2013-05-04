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
#ifndef DVK_H
#define DVK_H
#include <QMutex>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QTimer>
#include <portaudio.h>

struct DVKMessage
{
    int *snddata;
    unsigned long int sz;
};

// DVK message slot numbers
// 0 -12 F1-F12 CQ
// 13-24 F1-F12 Exc
// 25-36 F1-F12 Ctrl
// 37-48 F1-F12 Shift
#define DVK_MAX_MSG 48

class DVK : public QObject
{
    Q_OBJECT
public:
    bool audioRunning();
    bool messagePlaying();
    explicit DVK(QSettings& s,QObject *parent = 0);
    ~DVK();
    void loadMessages(QString filename,QString op);
    QString sndfile_version();
protected:
    static int loopCallback(const void *input, void *output, unsigned long frameCount,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags, void *userdata);
    static int writeCallback(const void *input, void *output, unsigned long frameCount,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags, void *userdata);
    static int recordCallback(const void *input, void *output, unsigned long frameCount,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags, void *userdata);
signals:
    void messageDone();
public slots:
    void cancelMessage();
    void initializeAudio();
    void loopAudio();
    void playMessage(int nr,int ch);
    void recordMessage(int nr);
    void setLiveChannel(int ch);
    void stopAudio();
    void stopLoopAudio();
private slots:
    void saveMessage();
    void clearTimer();

private:
    bool busy_;
    bool audioRunning_;
    bool loopback_;
    bool restartLoopback_;
    bool messagePlaying_;
    bool messageRecording_;
    int channel;
    int liveChannel;
    int msgNr;
    unsigned long int position;
    unsigned long int sz;
    DVKMessage msg[DVK_MAX_MSG];
    PaStream  *stream;
    PaStream *loopStream;
    QMutex mutex;
    QSettings& settings;
    QTimer *timer;

    void emitMessageDone();
};

#endif // DVK_H
