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
#ifndef SO2SDR_BANDMAP_H
#define SO2SDR_BANDMAP_H

#include <QAction>
#include <QApplication>
#include <QByteArray>
#include <QCheckBox>
#include <QCloseEvent>
#include <QColor>
#include <QDateTime>
#include <QDir>
#include <QErrorMessage>
#include <QLabel>
#include <QList>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPixmap>
#include <QResizeEvent>
#include <QSlider>
#include <QString>
#include <QStringList>
#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>
#include <QtGlobal>
#include <QThread>
#include <QTimer>
#include <QTimerEvent>
#include <QXmlStreamReader>
#include <QWidget>

#include "ui_bandmap.h"
#include "bandmapentry.h"
#include "defines.h"
#include "sdrdatasource.h"
#include "iqbalance.h"
#include "spectrum.h"
#include "sdrdialog.h"
#include "call.h"
#include "utils.h"
#include "helpdialog.h"

class So2sdrBandmap : public QMainWindow, public Ui::Bandmap
{
Q_OBJECT

public:
    So2sdrBandmap(QStringList args, QWidget *parent = 0);
    ~So2sdrBandmap();
    bool so2sdrBandmapOk() const;
    void calc();
    void closeIQ();
    int closestFreq(int) const;
    void initPointers();
    void initVariables();
    bool invert() const;
    int nextFreq(bool) const;
    void setAddOffset(int);
    void setDefaultCenter();
    void setInvert(bool t);
    void setIQ(bool, bool);

public slots:
    void quit();
    void restartSdr();

protected:
    void closeEvent(QCloseEvent *event);
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void resizeEvent(QResizeEvent * event);
    void timerEvent(QTimerEvent *event);

private slots:
    void deleteCallMouse();
    void emitParams();
    void showHelp();
    void setShowToolbar(bool);
    void setScaleX1();
    void setScaleX2();
    void setSdrType();
    void showIQData();
    void start();
    void stop();
    void readData();
    void startConnection();
    void udpRead();
    void updateLevel(int);
    void mouseQSYDelta(int);
    void findQsy(int);
    void disconnectSignals();

private:
    QList<Call>          callList;
    QByteArray           bandMapName;
    SdrDataSource        *sdrSource;
    QString              bandName;
    bool                 firstTime;
    bool                 initialized;
    bool                 _invert;
    bool                 tx;
    double               pix_per_hz;
    double               pix_per_khz;
    int                  addOffset;
    int                  band;
    int                  centerFreq;
    int                  dragPos;
    int                  endFreqs[2];
    int                  freqMax;
    int                  freqMin;
    int                  mouse_y;
    int                  vfoPos;
    int                  toolBarHeight;
    int                  timerId[N_BANDMAP_TIMERS];
    char                 cmd;
    char                 cmdLen;
    IQBalance            *iqDialog;
    QAction              *showToolBar;
    QAction              *deleteAct;
    QAction              *iqShowData;
    QAction              *scaleX1;
    QAction              *scaleX2;
    QLabel               txLabel;
    QPixmap              callPixmap;
    QPixmap              freqPixmap;
    QString              settingsFile;
    QThread              sdrThread;
    sampleSizes          sizes;
    Spectrum             *spectrumProcessor;
    QErrorMessage        errorBox;
    QSettings            *settings;
    SDRDialog            *sdrSetup;
    QDir                 directory;
    QCheckBox            checkBoxMark;
    QSlider              slider;
    QString              ipAddress;
    int                  flow;
    int                  fhigh;
    QTcpSocket           *socket;
    QTcpServer           server;
    QUdpSocket           socketUdp,socketUdpN1MM;
    QXmlStreamReader     xmlReader;
    HelpDialog           *help;

    void addCall(QByteArray);
    bool checkUserDirectory();
    void deleteCall(QByteArray);
    void makeCall();
    void makeFreqScaleAbsolute();
    void qsyToNearest();
    int  getBand(unsigned int f);
    void qsyNext(bool up);
    void setBandName(int b);
    void startTimers();
    void stopTimers();
    void xmlParseN1MM();
    void writeUdpXML(int freq,QByteArray call,bool del);
};


#endif // SO2SDR_BANDMAP_H
