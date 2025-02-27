/*! Copyright 2010-2025 R. Torsten Clay N4OGW

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

#include "call.h"
#include "defines.h"
#include "signal.h"
#include <QFile>
#include <QObject>
#include <QSettings>
#include <fftw3.h>
#include <portaudio.h>

/*!
   Spectrum calculation: FFT of audio data, etc
 */
class Spectrum : public QObject {
  Q_OBJECT

public:
  Spectrum(QObject *parent, QSettings &s, const QString &dir);
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
  void setTx(bool b) { tx = b; }
  void setCalcError();

signals:
  void clearPlot();
  void findCQMessage(QString);
  void gainPoint(int, double);
  void gainScale(double, double);
  void qsy(double);
  void phasePoint(int, double);
  void phaseScale(double, double);
  void plotGainFunc(double, double, double, double);
  void plotPhaseFunc(double, double, double, double);
  void spectrumReady(unsigned char *, unsigned char);

public slots:
  void clearIQ();
  void processData(unsigned char *, unsigned int);
  void setPlotPoints(bool);
  void startFindCQ(double low, double high, const QList<Call> &callList);
  void updateParams();

private:
  QSettings &settings;
  bool calcErrorNext;
  bool iqCorrect;
  bool iqData;
  bool iqPlotOpen;
  bool isTuning;
  bool peakDetect;
  bool swapIq;
  bool tx;
  CalibSignal *calibSigList;
  double aGain[FIT_ORDER];
  double aPhase[FIT_ORDER];
  double *peakAvg[SIG_N_AVG];
  double *spec_smooth;
  double *spec_tmp;
  double *spec_tmp2;
  double *tmp4;
  double *window;
  fftw_complex *errfunc;
  fftw_complex *in;
  fftw_complex *out;
  fftw_plan plan;
  double addOffset;
  int bits;
  int calibCnt;
  double centerFreq;
  double endFreqs[2];
  int findCQCnt;
  int fftSize;
  int invert;
  double offset;
  int offsetSign;
  int peakAvgCnt;
  int sampleFreq;
  int scale;
  double sigCQ;
  int sizeIQ;
  QString userDirectory;
  sampleSizes sizes;
  Signal sigListCQ[SIG_MAX];
  Signal sigList[SIG_MAX];
  unsigned char background;
  unsigned char *output;
  unsigned long advance_size;
  unsigned long chunk_size;

  void complexMult(double a[], double b[], double c[]) const;
  void detectPeaks(double bg, double sigma, double spec[]);
  void fitErrors();
  void findCQ(double flow, double fhigh, const QList<Call> &callList);
  void gaussElim(double a[FIT_ORDER][FIT_ORDER], double y[FIT_ORDER], int n);
  void interp2(double in[], double out[], double);
  void makeGainPhase();
  void makeWindow();
  void measureBackground(double &background, double &sigma,
                         double spec[]) const;
  void measureBackgroundLog(double &background, double &sigma,
                            double spec[]) const;
  void measureIQError(double bg, double spec[]);
  bool readError();
  bool saveError();
};
#endif
