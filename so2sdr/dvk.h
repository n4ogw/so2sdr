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

#include <QObject>
#include <QSettings>
#include <QString>
#include <portaudio.h>

struct DVKMessage
{
    int *snddata;
    unsigned long int sz;
};

class DVK : public QObject
{
    Q_OBJECT
public:
    explicit DVK(QSettings *s,QObject *parent = 0);
    void loadMessages(QString filename,QString op);
    QString sndfile_version();
protected:
    static int callback(const void *input, void *output, unsigned long frameCount,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags, void *userdata);
signals:
    
public slots:
    void initializeAudio();
    void playMessage(int nr,int ch);
    void stopAudio();


private:
    int channel;
    int *snddata;
    unsigned long int position;
    unsigned long int sz;
    DVKMessage func[12];
    PaStream  *stream;
    QSettings *settings;
};

#endif // DVK_H
