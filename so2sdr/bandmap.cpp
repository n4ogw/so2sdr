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
#include <QDebug>
#include <QDir>
#include <QMenu>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QSettings>
#include "bandmap.h"

Bandmap::Bandmap(QSettings& s,QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f), settings(s)
{
    setupUi(this);
#ifdef Q_OS_LINUX
    userDirectory = QDir::homePath() + "/.so2sdr";
#endif
#ifdef Q_OS_WIN
    userDirectory = QDir::homePath() + "/so2sdr";
#endif
    initialized = false;
    txLabel->clear();
    setFocusPolicy(Qt::NoFocus);
    display->setFocusPolicy(Qt::NoFocus);
    CallLabel->setFocusPolicy(Qt::NoFocus);
    FreqLabel->setFocusPolicy(Qt::NoFocus);

    freqPixmap      = QPixmap(FreqLabel->width(), MAX_H);
    callPixmap      = QPixmap(CallLabel->width(), MAX_H);
    vfoPos          = display->height() / 2;
    dragPos         = vfoPos;
    display->vfoPos = vfoPos;
    display->cornery = MAX_H / 2 - vfoPos;

    connect(display, SIGNAL(GLBandmapMouseQSY(int, int)), this, SIGNAL(mouseQSY(int, int)));
    connect(display, SIGNAL(mouseClick()),this,SIGNAL(mouseClick()));
    nrig        = 0;
    centerFreq  = 0;
    endFreqs[0] = 0;
    endFreqs[1] = 0;
    addOffset=0;
    audioReader = new AudioReaderPortAudio();
    audioReader->moveToThread(&audioThread);
    spectrumProcessor = new Spectrum(s);
    spectrumProcessor->moveToThread(&spectrumThread);

    _invert   = false;
    iqDialog  = 0;
    band      = -1;
    iqDialog  = new IQBalance(this, Qt::Window);
    iqDialog->clearPlots();

    for (int i = 0; i < MAX_H; i++) {
        display->cmap[i] = false;
    }
    scaleX1   = new QAction("Zoom x&1", this);
    scaleX2   = new QAction("Zoom x&2", this);
    deleteAct = new QAction("&Delete Call", this);
    checkBoxMark->setChecked(true);
    iqShowData = new QAction("IQ Balance", this);
    connect(iqShowData, SIGNAL(triggered()), this, SLOT(showIQData()));
    connect(checkBoxMark, SIGNAL(clicked()), this, SLOT(emitParams()));
    connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteCallMouse()));
    checkBoxIq->setChecked(true);
    checkBoxIQData->setChecked(true);
    scaleX1->setCheckable(true);
    scaleX2->setCheckable(true);
    scaleX2->setChecked(false);
    scaleX1->setChecked(true);
    connect(scaleX1, SIGNAL(triggered()), this, SLOT(setScaleX1()));
    connect(scaleX2, SIGNAL(triggered()), this, SLOT(setScaleX2()));

    connect(audioReader, SIGNAL(PortAudioError(QString)), this, SIGNAL(bandmapError(QString)));
    connect(spectrumProcessor, SIGNAL(qsy(int, int)), this, SIGNAL(qsy(int, int)));
    connect(spectrumProcessor, SIGNAL(findCQMessage(QString)), this, SIGNAL(findCQMessage(QString)));

    connect(checkBoxClick, SIGNAL(clicked()), this, SLOT(emitParams()));

    connect(checkBoxIq, SIGNAL(clicked(bool)), iqDialog->balanceCheckBox, SLOT(setChecked(bool)));
    connect(checkBoxIq, SIGNAL(clicked()), this, SLOT(emitParams()));

    connect(checkBoxIQData, SIGNAL(clicked(bool)), iqDialog->dataCheckBox, SLOT(setChecked(bool)));
    connect(checkBoxIQData, SIGNAL(clicked()), this, SLOT(emitParams()));

    connect(iqDialog->balanceCheckBox, SIGNAL(toggled(bool)), checkBoxIq, SLOT(setChecked(bool)));
    connect(iqDialog->balanceCheckBox, SIGNAL(clicked()), this, SLOT(emitParams()));

    connect(iqDialog->dataCheckBox, SIGNAL(toggled(bool)), checkBoxIQData, SLOT(setChecked(bool)));
    connect(iqDialog->dataCheckBox, SIGNAL(clicked()), this, SLOT(emitParams()));

    connect(spectrumProcessor, SIGNAL(clearPlot()), iqDialog, SLOT(clearPlots()));
    connect(spectrumProcessor, SIGNAL(gainPoint(int, double)), iqDialog, SLOT(plotGainPoint(int, double)));
    connect(spectrumProcessor, SIGNAL(phasePoint(int, double)), iqDialog, SLOT(plotPhasePoint(int, double)));
    connect(spectrumProcessor, SIGNAL(gainScale(double, double)), iqDialog, SLOT(setGainScale(double, double)));
    connect(spectrumProcessor, SIGNAL(phaseScale(double, double)), iqDialog, SLOT(setPhaseScale(double, double)));
    connect(spectrumProcessor, SIGNAL(plotGainFunc(double, double, double, double)), iqDialog,
            SLOT(plotGainFunc(double, double, double, double)));
    connect(spectrumProcessor, SIGNAL(plotPhaseFunc(double, double, double, double)), iqDialog,
            SLOT(plotPhaseFunc(double, double, double, double)));
    connect(iqDialog, SIGNAL(closed(bool)), spectrumProcessor, SLOT(setPlotPoints(bool)));
    connect(iqDialog, SIGNAL(restart()), spectrumProcessor, SLOT(clearIQ()));
    connect(slider, SIGNAL(sliderMoved(int)), spectrumProcessor, SLOT(setPeakDetectScale(int)));
}

Bandmap::~Bandmap()
{
    delete audioReader;
    delete spectrumProcessor;
    delete deleteAct;
    iqDialog->close();
    delete iqDialog;
    delete iqShowData;
    delete scaleX1;
    delete scaleX2;
}

/*! called from So2sdr periodically to update IQ error fit
 */
void Bandmap::calc()
{
    spectrumProcessor->setCalcError();
}

/*! reimplemented close event. Closes IQ plot if it is
   open
 */
void Bandmap::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    closeIQ();
    stop();
    // save geometry
    QString tmp="BandmapWindow"+QString::number(nrig+1);
    settings.beginGroup(tmp);
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();
}

/*! close IQ dialog
 */
void Bandmap::closeIQ()
{
    iqDialog->hide();
}

/*!
   Delete call via mouse
 */
void Bandmap::deleteCallMouse()
{
    int f = centerFreq + (int) (1000.0 * SAMPLE_FREQ / (double) sizes.spec_length *
                                settings.value(s_sdr_scale[nrig],s_sdr_scale_def[nrig]).toInt() * (vfoPos - mouse_y));
    emit(deleteCallFreq(f, band));
}

/*! this gets called when a parameter stored in the main program needs to
   be changed */
void Bandmap::emitParams()
{
    emit(updateParams());
    settings.setValue(s_sdr_iqcorrect[nrig],checkBoxIq->isChecked());
    settings.setValue(s_sdr_iqdata[nrig],checkBoxIQData->isChecked());
    settings.setValue(s_sdr_click[nrig],checkBoxClick->isChecked());
    settings.setValue(s_sdr_peakdetect[nrig],checkBoxMark->isChecked());
    settings.sync();
}

/*! get CQ frequency ranked ncq. Spectrum.cpp will find this
   freq and issue a QSY signal */
void Bandmap::getCQFreq()
{
    spectrumProcessor->startFindCQ();
}

/*!
   Initialize

   nr : radio 0 or 1
   dev : audio device
   format : audio format
 */
void Bandmap::initialize(QString dir, int nr,const PaStreamParameters &format)
{
    if (!dir.isEmpty()) userDirectory = dir;
    nrig = nr;

    // restore window geometry
    QString tmp="BandmapWindow"+QString::number(nrig+1);
    settings.beginGroup(tmp);
    resize(settings.value("size", QSize(400, 594)).toSize());
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    settings.endGroup();

    // other settings
    checkBoxClick->setChecked(settings.value(s_sdr_click[nrig],s_sdr_click_def[nrig]).toBool());
    checkBoxIq->setChecked(settings.value(s_sdr_iqcorrect[nrig],s_sdr_iqcorrect_def[nrig]).toBool());
    checkBoxIQData->setChecked(settings.value(s_sdr_iqdata[nrig],s_sdr_iqdata_def[nrig]).toBool());
    iqDialog->dataCheckBox->setChecked(settings.value(s_sdr_iqdata[nrig],s_sdr_iqdata_def[nrig]).toBool());
    iqDialog->balanceCheckBox->setChecked(settings.value(s_sdr_iqcorrect[nrig],s_sdr_iqcorrect_def[nrig]).toBool());
    checkBoxMark->setChecked(settings.value(s_sdr_peakdetect[nrig],s_sdr_peakdetect_def[nrig]).toBool());

    sizes.sample_length = 4096;
    switch (format.sampleFormat) {
    case paInt16:
        sizes.chunk_size   = sizes.sample_length * 2 * 2;
        sizes.advance_size = 1024 * 2 * 2;
        break;
    case paInt24:
        sizes.chunk_size   = sizes.sample_length * 2 * 3;
        sizes.advance_size = 1024 * 2 * 3;
        break;
    case paInt32:
        sizes.chunk_size   = sizes.sample_length * 2 * 4;
        sizes.advance_size = 1024 * 2 * 4;
        break;
    }
    sizes.spec_length    = sizes.sample_length;
    sizes.display_length = MAX_H;

    int scale= settings.value(s_sdr_scale[nrig],s_sdr_scale_def[nrig]).toInt();
    display->setScale(scale);
    pix_per_khz = sizes.spec_length * scale / (double) SAMPLE_FREQ;
    pix_per_hz  = sizes.spec_length * scale / (double) (SAMPLE_FREQ * 1000.0);
    spectrumProcessor->initialize(sizes, format.sampleFormat, nr, userDirectory);

    setScaleX1();
    display->initialize(nrig, sizes);
    display->setScale(1);

    // data from audioReader connected to spectrumProcessor
    spectrumProcessor->connect(audioReader, SIGNAL(audioReady(unsigned char *, unsigned char)),
                               SLOT(processData(unsigned char *, unsigned char)));

    audioReader->initialize(format, sizes);

    // connect output of spectrum processing to display
    connect(spectrumProcessor, SIGNAL(spectrumReady(unsigned char*, unsigned char)), display,
            SLOT(plotSpectrum(unsigned char*, unsigned char)));

    initialized = true;
}

/*! returns current invert setting
 */
bool Bandmap::invert() const
{
    return(_invert);
}

/*!
   make pixmap with bandmap calls
 */
void Bandmap::makeCall(const QList<BandmapEntry>& map)
{
    QPainter p(&callPixmap);
    p.fillRect(callPixmap.rect(), Qt::lightGray);

    p.setPen(Qt::black);
    QFont font;
    font.setPixelSize(BANDMAP_FONT_PIX_SIZE);
    p.setFont(font);
    for (int i = 0; i < MAX_H; i++) {
        display->cmap[i] = false;
    }

    // draw callsigns
    // cmap[i] is true if the call near i is a dupe. Used to color signal on bandmap
    //
    int scale= settings.value(s_sdr_scale[nrig],s_sdr_scale_def[nrig]).toInt();
    display->setMark(true);
    double fm = centerFreq - MAX_H / 2 * SAMPLE_FREQ * 1000 / (scale * sizes.spec_length);
    for (int i = 0; i < map.size(); i++) {
        if (map.at(i).f < fm || map.at(i).f > freqMax) continue;
        int pix_offset = (map.at(i).f - fm) * pix_per_hz;
        int y          = MAX_H - pix_offset;

        int l1 = y - BANDMAP_FONT_PIX_SIZE / 2;
        if (l1 < 0) l1 = 0;
        int l2 = y + BANDMAP_FONT_PIX_SIZE / 2;
        if (l2 >= MAX_H) l2 = MAX_H - 1;

        if (map.at(i).dupe) {
            // make line wider when scale is *2
            int l1 = y - 1 - 2 * scale;
            if (l1 < 0) l1 = 0;
            int l2 = y + 2 + 2 * scale;
            if (l2 >= MAX_H) l2 = MAX_H - 1;
            for (int j = l1; j < l2; j++) display->cmap[j] = true;
            p.setPen(ALTD_COLOR);
        } else {
            p.setPen(Qt::black);
        }
        pix_offset = (map.at(i).f - freqMin) * pix_per_hz;
        y          = MAX_H - pix_offset;
        // BANDMAP_FONT_PIX_SIZE/3 is fudge factor to center callsign
        p.drawText(BANDMAP_CALL_X, y+ BANDMAP_FONT_PIX_SIZE / 3, map.at(i).call);
    }
    p.setPen(Qt::black);

    // draw symbol for each signal
    if (settings.value(s_sdr_peakdetect[nrig],s_sdr_peakdetect_def[nrig]).toBool()) {
        p.setBrush(Qt::SolidPattern);
        Signal *sigs = &spectrumProcessor->sigList[0];
        for (int i = 0; i < SIG_MAX; i++) {
            if (sigs[i].active) {
                if (sigs[i].f<freqMin || sigs[i].f>freqMax) continue;
                int pix_offset = (sigs[i].f - freqMin) * pix_per_hz + SIG_SYMBOL_RAD / 2;
                int y          = MAX_H - pix_offset;
                if (y < 0 || y >= MAX_H) {
                    continue;
                }
                p.drawEllipse(SIG_SYMBOL_X, y, SIG_SYMBOL_RAD, SIG_SYMBOL_RAD);
            }
        }
    }
}

/*!
   make frequency scale for band map (relative KHz markers)

 */
void Bandmap::makeFreqScale()
{
    QPainter p(&freqPixmap);
    p.fillRect(freqPixmap.rect(), Qt::lightGray);

    // make ticks every KHz/4
    pix_per_khz = sizes.spec_length *  settings.value(s_sdr_scale[nrig],s_sdr_scale_def[nrig]).toInt() / (double) SAMPLE_FREQ;
    double y = 0.;
    int    i = 0;
    p.setPen(Qt::black);
    QFont  font;
    font.setPixelSize(BANDMAP_FONT_PIX_SIZE);
    int    j = 0;
    while (i < MAX_H / 2) {
        y += pix_per_khz;
        j++;
        i = (int) y;
        p.drawLine(0, MAX_H / 2 + i, 7, MAX_H / 2 + i);
        p.drawLine(0, MAX_H / 2 - i, 7, MAX_H / 2 - i);
        p.drawText(8, MAX_H / 2 + i + BANDMAP_FONT_PIX_SIZE / 2, QString::number(j));
        p.drawText(8, MAX_H / 2 - i + BANDMAP_FONT_PIX_SIZE / 2, QString::number(j));
    }
    p.setPen(qRgb(255, 0, 0));
    p.drawLine(0, MAX_H / 2 + 1, width(), MAX_H / 2 + 1);
    p.drawLine(0, MAX_H / 2, width(), MAX_H / 2);
    p.drawLine(0, MAX_H / 2 - 1, width(), MAX_H / 2 - 1);
}

/*!
   draw frequency scale pixmap with actual KHz
 */
void Bandmap::makeFreqScaleAbsolute()
{
    int dy = (height() - 20) / 2 - vfoPos;
    int scale= settings.value(s_sdr_scale[nrig],s_sdr_scale_def[nrig]).toInt();
    freqMin = centerFreq - (MAX_H / 2 + dy) * SAMPLE_FREQ * 1000 / (scale * sizes.spec_length);
    int      bottom_start      = (freqMin / 1000 + 1) * 1000;
    int      bottom_pix_offset = (bottom_start - freqMin) * sizes.spec_length * scale / (SAMPLE_FREQ * 1000);
    int      j                 = (bottom_start / 1000) % 1000;
    int      i, i0 = MAX_H - bottom_pix_offset;
    freqMax = freqMin + (MAX_H * SAMPLE_FREQ * 1000 / (double) (scale * sizes.spec_length));
    QPainter p(&freqPixmap);
    p.fillRect(freqPixmap.rect(), Qt::lightGray);
    p.setPen(Qt::black);
    QFont   font;
    font.setPixelSize(BANDMAP_FONT_PIX_SIZE);
    QString tmp;
    i = i0;
    int     k = 1;
    while (i > 0) {
        p.drawLine(0, i, 5, i);
        if (j < 10) {
            tmp = "00" + QString::number(j);
        } else if (j < 100) {
            tmp = "0" + QString::number(j);
        } else {
            tmp = QString::number(j);
        }
        p.drawText(6, i + BANDMAP_FONT_PIX_SIZE / 2, tmp);
        j++;
        j = j % 1000;
        i = i0 - (int) (k * pix_per_khz) - 2;
        k++;
    }

    // draw red line at center freq
    p.setPen(qRgb(255, 0, 0));

    // 20 is height of toolbar at bottom
    int y = MAX_H / 2 - ((height() - 20) / 2 - vfoPos);
    p.drawLine(0, y + 1, width(), y + 1);
    p.drawLine(0, y, width(), y);
    p.drawLine(0, y - 1, width(), y - 1);
}

/*! event for mouse moved while left button is pressed

   detect when frequency scale is dragged
 */
void Bandmap::mouseMoveEvent(QMouseEvent *event)
{
    if (FreqLabel->underMouse()) {
        // constant here (50) affects sensitivity of dragging freq scale
        if (abs(event->pos().y() - dragPos) < 50) {
            int tmp = vfoPos + (event->pos().y() - dragPos);
            if (tmp > 0 && tmp < (height() - 21)) {
                vfoPos          = tmp;
                display->vfoPos = vfoPos;
                display->cornery = MAX_H / 2 - vfoPos;
                makeFreqScaleAbsolute();
                FreqLabel->setPixmap(freqPixmap);
                FreqLabel->update();
            }
        }
        dragPos = event->pos().y();
    }
}

/*!
   Mouse event handler: just for mouse clicks in CallLabel field
   left-clicks : qsy to frequency or label freq
   right-clicks: bring up menu
 */
void Bandmap::mousePressEvent(QMouseEvent *event)
{
    if (CallLabel->underMouse()) {
        mouse_y = event->y();
        if (event->button() == Qt::RightButton) {
            // right-mouse button; brings up menu
            QMenu menu(this);
            menu.addAction(scaleX1);
            menu.addAction(scaleX2);
            menu.addSeparator();
            menu.addAction(deleteAct);
            menu.addSeparator();
            menu.addAction(iqShowData);
            menu.exec(event->globalPos());
        } else if (event->button() == Qt::LeftButton && (event->x() < (FreqLabel->width() + display->width() +
                                                                       SIG_SYMBOL_X + 4 * SIG_SYMBOL_RAD))) {
            // left-mouse: qsy to signal
            qsyToNearest();
        }
    }
}

/*!
   returns next higher or lower detected signal

   higher if higher=true, otherwise lower
 */
int Bandmap::nextFreq(bool higher) const
{
    if (!settings.value(s_sdr_peakdetect[nrig],s_sdr_peakdetect_def[nrig]).toBool()) {
        return(0);
    }
    int    f  = centerFreq;
    int    df = SAMPLE_FREQ * 1000;

    Signal *s = &spectrumProcessor->sigList[0];
    if (higher) {
        for (int i = 0; i < SIG_MAX; i++) {
            if (s[i].active && s[i].f > centerFreq) {
                int d = s[i].f - centerFreq;
                if (d < df && d > SIG_MIN_FREQ_DIFF) {
                    f  = s[i].f;
                    df = d;
                }
            }
        }
    } else {
        for (int i = 0; i < SIG_MAX; i++) {
            if (s[i].active && s[i].f < centerFreq) {
                int d = centerFreq - s[i].f;
                if (d < df && d > SIG_MIN_FREQ_DIFF) {
                    f  = s[i].f;
                    df = d;
                }
            }
        }
    }
    if (f > band_limits[band][0] && f < band_limits[band][1]) {
        return(f);
    } else {
        return(centerFreq);
    }
}

/*! qsy to detected signal nearest to mouse
 */
void Bandmap::qsyToNearest()
{
    // note: need to subtract 20 from height due to height of checkbox bar at bottom of
    // bandmap
    int    f = centerFreq + (int) (1000.0 * SAMPLE_FREQ / (double) sizes.spec_length /  settings.value(s_sdr_scale[nrig],s_sdr_scale_def[nrig]).toInt()
                                   * (vfoPos - mouse_y));

    Signal *s = &spectrumProcessor->sigList[0];
    for (int i = 0; i < SIG_MAX; i++) {
        // find signal within 100 hz of click freq
        if (s[i].active && abs(s[i].f - f) < 100) {
            emit(qsy(nrig, s[i].f));
            break;
        }
    }
}

void Bandmap::resizeEvent(QResizeEvent * event)
{
    Q_UNUSED(event);
    if (vfoPos > (height() - 21)) {
        vfoPos          = height() - 21;
        display->vfoPos = vfoPos;
        display->cornery = MAX_H / 2 - vfoPos;
        makeFreqScaleAbsolute();
        FreqLabel->setPixmap(freqPixmap);
        FreqLabel->update();
    }
}


/*!
   Invert the spectrum
 */
void Bandmap::setInvert(bool t)
{
    display->setInvert(t);
    spectrumProcessor->setInvert(t);
    _invert = t;
}

/*! show the IQ dialog
 */
void Bandmap::showIQData()
{
    iqDialog->show();
    iqDialog->clearPlots();
    spectrumProcessor->setPlotPoints(true);
    // force recalculation of IQ fit
    spectrumProcessor->calcError(true);
}

/*! set default vfo position in center of display */
void Bandmap::setDefaultCenter()
{
    vfoPos          = display->height() / 2;
    display->vfoPos = vfoPos;
    display->cornery = MAX_H / 2 - vfoPos;
}

/*!
   Set center frequency
 */
void Bandmap::setFreq(int i, int b, const QList<BandmapEntry>& map)
{
    centerFreq = i;
    endFreqs[0] = centerFreq-48000-settings.value(s_sdr_offset[nrig],s_sdr_offset_def[nrig]).toInt();
    endFreqs[1] = centerFreq+48000-settings.value(s_sdr_offset[nrig],s_sdr_offset_def[nrig]).toInt();
    setWindowTitle("Bandmap "+bandName[b]+"m ["+QString::number(endFreqs[0]/1000)+"-"+QString::number(endFreqs[1]/1000)+"]");
    if (band != -1 && band != b) {
        // if band changed, clear all signals
        spectrumProcessor->clearSigs();
        spectrumProcessor->clearCQ();
    }
    band = b;
    spectrumProcessor->setFreq(i, b, endFreqs[0], endFreqs[1]);
    spectrumProcessor->resetAvg();
    makeFreqScaleAbsolute();
    FreqLabel->setPixmap(freqPixmap);
    FreqLabel->update();
    makeCall(map);
    CallLabel->setPixmap(callPixmap);
    CallLabel->update();
}

/*! set peak detection status

   does not clear signals from bandmap or change checkboxes
 */
void Bandmap::setMark(bool b)
{
    if (!initialized) return;

    settings.setValue(s_sdr_peakdetect[nrig],b);
    if (b) {
        txLabel->clear();
    } else {
        txLabel->setText("<font color=#FF0000>TX");
    }
}


/*!
   Set added IF offset (used for K3 for example)
 */
void Bandmap::setAddOffset(int i)
{
    spectrumProcessor->setAddOffset(i);
}

/*! set x1 scale
 */
void Bandmap::setScaleX1()
{
    settings.setValue(s_sdr_scale[nrig],1);
    display->setScale(1);
    scaleX1->setChecked(true);
    scaleX2->setChecked(false);
    pix_per_khz = sizes.spec_length * 1 / (double) SAMPLE_FREQ;
    pix_per_hz  = sizes.spec_length * 1 / (double) (SAMPLE_FREQ * 1000.0);
    makeFreqScaleAbsolute();
}

/*! set x2 scale
 */
void Bandmap::setScaleX2()
{
    settings.setValue(s_sdr_scale[nrig],2);
    display->setScale(2);
    scaleX1->setChecked(false);
    scaleX2->setChecked(true);
    pix_per_khz = sizes.spec_length * 2 / (double) SAMPLE_FREQ;
    pix_per_hz  = sizes.spec_length * 2 / (double) (SAMPLE_FREQ * 1000.0);
    makeFreqScaleAbsolute();
}

/*!
   Start bandmap
 */
bool Bandmap::start()
{
    if (!spectrumThread.isRunning()) {
        spectrumThread.start();
    }
    if (!audioThread.isRunning()) {
        audioThread.start();
    }
    emit(startReading());
    return(true);
}

/*! stops bandmap

  Normally do not call this directly, instead call close()
  */
void Bandmap::stop()
{
    audioReader->stopReader();
    if (audioThread.isRunning()) {
        audioThread.quit();
        audioThread.wait();
    }
    if (spectrumThread.isRunning()) {
        spectrumThread.quit();
        spectrumThread.wait();
    }
    disconnect(audioReader, SIGNAL(audioReady(unsigned char*, unsigned char)), spectrumProcessor, SLOT(processData(unsigned char*, unsigned char)));
    disconnect(spectrumProcessor, SIGNAL(spectrumReady(unsigned char*, unsigned char)), display, SLOT(plotSpectrum(unsigned char*, unsigned char)));
    spectrumProcessor->stopSpectrum();
    initialized = false;

    emit(done());
}

/*!
  returns closest peak-detected freqency to fin

  if freq difference is larger than SIG_MIN_SPOT_DIFF, returns fin
  */
int Bandmap::closestFreq(int fin) const
{
    int f=fin;
    if (spectrumProcessor) {
        f=spectrumProcessor->closestFreq(fin);
    }
    return f;
}
