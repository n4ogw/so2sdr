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
#ifndef BANDMAP_H
#define BANDMAP_H
#include "ui_bandmap.h"
#include "glbandmap.h"
#include "audioreader_portaudio.h"
#include "defines.h"
#include "bandmapentry.h"
#include "qso.h"
#include "spectrum.h"
#include "iqbalance.h"
#include <QByteArray>
#include <QPixmap>
#include <QSettings>
#include <QStatusBar>
#include <QString>
#include <QThread>
#include <QWidget>

class QAction;

/*!
   Visible bandmap window
 */
class Bandmap : public QWidget, public Ui::Bandmap
{
Q_OBJECT

public:
    Bandmap(QSettings& s,QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~Bandmap();
    void calc();
    void closeIQ();
    int closestFreq(int) const;
    void initialize(QString dir, int nr,const PaStreamParameters &format);
    bool invert() const;
    int nextFreq(bool) const;
    void setAddOffset(int);
    void setDefaultCenter();
    void setFreq(int i, int b, const QList<BandmapEntry>& map);
    void setInvert(bool t);
    void setIQ(bool, bool);
    bool start();
    void stop();

signals:
    void updateParams();
    void bandmapError(const QString &);
    void deleteCallFreq(int, int);
    void done();
    void findCQMessage(QString);
    void mouseClick();
    void mouseQSY(int, int);
    void qsy(int, int);
    void startReading();

public slots:
    void getCQFreq();
    void setMark(bool);

protected:
    void closeEvent(QCloseEvent *event);
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void resizeEvent(QResizeEvent * event);

private slots:
    void deleteCallMouse();
    void emitParams();
    void setScaleX1();
    void setScaleX2();
    void showIQData();

private:
    AudioReaderPortAudio *audioReader;
    bool                 initialized;
    bool                 _invert;
    bool                 stopping;
    double               pix_per_hz;
    double               pix_per_khz;
    int                  addOffset;
    int                  band;
    int                  centerFreq;
    int                  cornerx;
    int                  cornery;
    int                  dragPos;
    int                  endFreqs[2];
    int                  freqMax;
    int                  freqMin;
    int                  mouse_y;
    int                  nrig;
    int                  vfoPos;
    IQBalance            *iqDialog;
    QAction              *deleteAct;
    QAction              *iqShowData;
    QAction              *scaleX1;
    QAction              *scaleX2;
    QPixmap              callPixmap;
    QPixmap              freqPixmap;
    QStatusBar           *statusBar;
    QString              userDirectory;
    QThread              audioThread;
    QThread              spectrumThread;
    sampleSizes          sizes;
    Spectrum             *spectrumProcessor;
    unsigned long        chunk_size;
    QSettings&            settings;

    void makeCall(const QList<BandmapEntry>& map);
    void makeFreqScale();
    void makeFreqScaleAbsolute();
    void qsyToNearest();
};

#endif
