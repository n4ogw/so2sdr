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
#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <QTimer>
#include <QObject>
#include <portaudio.h>
#include <QFile>
#include <QSettings>
#include <QTimer>
#include "defines.h"
#include "signal.h"
#ifdef Q_OS_WIN
#include "fftw3.h"
#endif
#ifdef Q_OS_LINUX
#include <fftw3.h>
#endif

class QTimer;

/*!
   Spectrum calculation: FFT of audio data, etc

   This is a separate thread
 */
class Spectrum : public QObject
{
Q_OBJECT

public:
    Spectrum(QSettings *s,QObject *parent = 0);
    friend class Bandmap;

    ~Spectrum();
    unsigned char bg();
    void calcError(bool force);
    void clearCQ();
    void clearSigs();
    int closestFreq(int) const;
    int findCQFreq();
    void initialize(sampleSizes s, PaSampleFormat b, int nr, QString dir);
    void resetAvg();
    void setAddOffset(int i);
    void stopSpectrum();
    void setFreq(int, int);
    void setInvert(bool);
    void setNrig(int);
    void setCalcError();
    int getPeakDetectScale() const;

signals:
    void done();
    void spectrumReady(unsigned char *, unsigned char);
    void dataProcessed();
    void findCQMessage(QString);
    void finished();
    void clearPlot();
    void qsy(int, int);
    void gainPoint(int, double);
    void phasePoint(int, double);
    void gainScale(double, double);
    void phaseScale(double, double);
    void plotGainFunc(double, double, double, double);
    void plotPhaseFunc(double, double, double, double);

public slots:
    void processData(unsigned char *, unsigned char);
    void clearIQ();
    void setPeakDetectScale(int);
    void setPlotPoints(bool);
    void startFindCQ();

private:
    bool          calcErrorNext;
    bool          iqPlotOpen;
    bool          *sigOn;
    CalibSignal   *calibSigList;
    double        aGain[FIT_ORDER];
    double        aPhase[FIT_ORDER];
    double        *peakAvg[SIG_N_AVG];
    double        *spec_smooth;
    double        *spec_tmp;
    double        *spec_tmp2;
    double        *tmp4;
    double        *window;
    fftw_complex  *errfunc;
    fftw_complex  *in;
    fftw_complex  *out;
    fftw_plan     plan;
    int addOffset;
    int           band;
    int           bits;
    int           calibCnt;
    int           centerFreq;
    int           findCQCnt;
    int           invert;
    int           nrig;
    int           peakAvgCnt;
    int           peakDetectScale;
    int           sigCQ;
    int           *sigOnCnt;
    int           sigSpace;
    int           sizeIQ;
    int           yOffset;
    QString       userDirectory;
    sampleSizes   sizes;
    Signal        sigListCQ[SIG_MAX];
    Signal        sigListCQtmp[SIG_MAX];
    Signal        sigList[SIG_MAX];
    unsigned char background;
    unsigned char *output;
    unsigned long advance_size;
    unsigned long chunk_size;
    QSettings *settings;

#ifdef Q_OS_WIN
    fftw_malloc_ptr       fftw_mallocp;
    fftw_free_ptr         fftw_freep;
    fftw_plan_dft_1d_ptr  fftw_plan_dft_1dp;
    fftw_destroy_plan_ptr fftw_destroy_planp;
    fftw_destroy_plan_ptr fftw_executep;
#endif

    void clickRemove(double bg, double dev, double spec[]) const;
    void complexMult(double a[], double b[], double c[]) const;
    void detectPeaks(double bg, double sigma, double spec[]);
    void fitErrors();
#ifdef Q_OS_WIN
    bool fftwWinInit();
#endif
    void findCQ();
    void gaussElim(double a[FIT_ORDER][FIT_ORDER], double y[FIT_ORDER], int n);
    void interp2(double in[], double out[], double);
    void makeGainPhase();
    void makeWindow();
    void measureBackground(double &background, double &sigma, double spec[]) const;
    void measureBackgroundLog(double &background, double &sigma, double spec[]) const;
    void measureIQError(double bg, double spec[]);
    bool readError();
    bool saveError();
};
#endif
