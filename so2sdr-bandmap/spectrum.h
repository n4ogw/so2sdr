/*! Copyright 2010-2021 R. Torsten Clay N4OGW

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

#include <QObject>
#include <portaudio.h>
#include <QFile>
#include <QSettings>
#include "defines.h"
#include "signal.h"
#include "call.h"
#ifdef Q_OS_LINUX
#include <fftw3.h>
#endif

/*!
   Spectrum calculation: FFT of audio data, etc

   This is a separate thread
 */
class Spectrum : public QObject
{
Q_OBJECT

public:
    Spectrum(QObject *parent,QSettings &s,QString dir);
    friend class So2sdrBandmap;

    ~Spectrum();
    void calcError(bool force);
    void clearCQ();
    void clearSigs();
    double closestFreq(double) const;
    void resetAvg();
    void setAddOffset(double f);
    void stopSpectrum();
    void setFFTSize(sampleSizes s);
    void setFreq(double, double, double);
    void setInvert(bool);
    void setPeakDetect(bool);
    void setTuning(bool);
    void setCalcError();

signals:
    void spectrumReady(unsigned char *, unsigned char);
    void findCQMessage(QString);
    void clearPlot();
    void qsy(double);
    void gainPoint(int, double);
    void phasePoint(int, double);
    void gainScale(double, double);
    void phaseScale(double, double);
    void plotGainFunc(double, double, double, double);
    void plotPhaseFunc(double, double, double, double);

public slots:
    void processData(unsigned char *, unsigned char);
    void clearIQ();
    void setPlotPoints(bool);
    void startFindCQ(double low, double high, QList<Call> &callList);
    void updateParams();

private:
    QSettings    &settings;
    bool          calcErrorNext;
    bool          iqCorrect;
    bool          iqData;
    bool          iqPlotOpen;
    bool          isTuning;
    bool          peakDetect;
    bool          swapIq;
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
    double        addOffset;
    int           bits;
    int           calibCnt;
    double        centerFreq;
    double        endFreqs[2];
    int           findCQCnt;
    int           fftSize;
    int           invert;
    double        offset;
    int           offsetSign;
    int           peakAvgCnt;
    int           sampleFreq;
    int           scale;
    double        sigCQ;
    int           sizeIQ;
    QString       userDirectory;
    sampleSizes   sizes;
    Signal        sigListCQ[SIG_MAX];
    Signal        sigList[SIG_MAX];
    unsigned char background;
    unsigned char *output;
    unsigned long advance_size;
    unsigned long chunk_size;

    void complexMult(double a[], double b[], double c[]) const;
    void detectPeaks(double bg, double sigma, double spec[]);
    void fitErrors();
    void findCQ(double flow, double fhigh, QList<Call> &callList);
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
