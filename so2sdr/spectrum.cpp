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
#include <math.h>
#include <QDataStream>
#include <QDebug>
#include <QDir>
#include "spectrum.h"

#ifdef Q_OS_WIN
#include <windows.h>
#define _USE_MATH_DEFINES
#include <cmath>
#endif

void Spectrum::initialize(sampleSizes s, PaSampleFormat b, int nr, QString dir)
{
    nrig          = nr;
    userDirectory = dir;
    sizes         = s;
    sizeIQ        = sizes.sample_length / 8; // 512 IQ phase/gain bins
    // initialize FFTW
#ifdef Q_OS_WIN
    if (!fftwWinInit()) {
        return;
    }
    if (in) (fftw_freep) (in);
    if (out) (fftw_freep) (out);
    if (errfunc) (fftw_freep) (errfunc);
    if (plan) (fftw_destroy_planp) (plan);

    in      = (fftw_complex *) (fftw_mallocp) (sizeof(fftw_complex) * sizes.sample_length);
    out     = (fftw_complex *) (fftw_mallocp) (sizeof(fftw_complex) * sizes.sample_length);
    errfunc = (fftw_complex *) (fftw_mallocp) (sizeof(fftw_complex) * sizes.sample_length);
    plan    = (fftw_plan_dft_1dp) (sizes.sample_length, in, out, FFTW_FORWARD, FFTW_MEASURE);
#else
    if (in) fftw_free(in);
    if (out) fftw_free(out);
    if (errfunc) fftw_free(errfunc);
    if (plan) fftw_destroy_plan(plan);

    in      = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * sizes.sample_length);
    out     = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * sizes.sample_length);
    errfunc = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * sizes.sample_length);
    plan    = fftw_plan_dft_1d(sizes.sample_length, in, out, FFTW_FORWARD, FFTW_MEASURE);
#endif
    switch (b) {
    case paInt16:
        bits = 16;
        break;
    case paInt24:
        bits = 24;
        break;
    case paInt32:
        bits = 32;
        break;
    default:
        bits = 16;
    }

    if (output) delete [] output;
    output = new unsigned char[sizes.display_length];

    for (int i = 0; i < SIG_N_AVG; i++) {
        if (peakAvg[i]) delete [] peakAvg[i];
        peakAvg[i] = new double[sizes.spec_length];
    }
    if (spec_smooth) delete [] spec_smooth;
    spec_smooth = new double[sizes.sample_length];
    if (spec_tmp) delete [] spec_tmp;
    spec_tmp = new double[sizes.sample_length];

    if (spec_tmp2) delete [] spec_tmp2;
    spec_tmp2 = new double[sizes.sample_length];

    for (int i = 0; i < sizes.sample_length; i++) {
        spec_tmp[i]    = 0.;
        spec_smooth[i] = 0.;
        spec_tmp2[i]   = 0.;
    }

    if (tmp4) delete [] tmp4;
    tmp4 = new double[sizes.spec_length];
    if (window) delete [] window;
    window = new double[sizes.sample_length];

    makeWindow();

    for (int i = 0; i < sizes.spec_length; i++) {
        tmp4[i] = 0.;
    }
    if (sigOnCnt) delete [] sigOnCnt;
    sigOnCnt = new int[sizes.sample_length];

    if (sigOn) delete [] sigOn;
    sigOn = new bool[sizes.sample_length];

    for (int i = 0; i < sizes.sample_length; i++) {
        sigOnCnt[i] = 2;
        sigOn[i]    = false;
    }
    if (calibSigList) delete [] calibSigList;
    calibSigList = new CalibSignal[sizeIQ];
    for (int i = 0; i < FIT_ORDER; i++) {
        aGain[i]  = 0.;
        aPhase[i] = 0.;
    }
    addOffset=0;

    // intialize to unit gain and zero phase
    aGain[0] = 1.0;
    makeGainPhase();

    readError();
    calcError(true);
}

Spectrum::Spectrum(QSettings& s,QObject *parent) : QObject(parent),settings(s)
{
    peakDetectScale = 100;
    in              = 0;
    out             = 0;
    errfunc         = 0;
    plan            = 0;
    window          = 0;
    output          = 0;
    for (int i = 0; i < SIG_N_AVG; i++) {
        peakAvg[i] = 0;
    }
    spec_smooth  = 0;
    spec_tmp     = 0;
    spec_tmp2    = 0;
    tmp4         = 0;
    sigOnCnt     = 0;
    sigOn        = 0;
    calibSigList = 0;
    background   = 0;

    // initialize on 160m
    band           = 0;
    calibCnt       = 0;
    centerFreq     = 0;
    endFreqs[0]    = 0;
    endFreqs[1]    = 0;

    invert        = 1;
    iqPlotOpen    = false;
    findCQCnt     = 0;
    calcErrorNext = false;
    for (int i = 0; i < SIG_MAX; i++) {
        sigList[i].clear();
        sigListCQ[i].clear();
    }
    sigCQ      = 0;
    sigSpace   = 0;
    yOffset    = 0;
    peakAvgCnt = 0;
}


Spectrum::~Spectrum()
{
#ifdef Q_OS_WIN
    (fftw_destroy_planp) (plan);
    (fftw_freep) (in);
    (fftw_freep) (out);
    (fftw_freep) (errfunc);
#else
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
    fftw_free(errfunc);
#endif
    for (int i = 0; i < SIG_N_AVG; i++) {
        delete[] peakAvg[i];
    }
    delete[] calibSigList;
    delete[] sigOnCnt;
    delete[] sigOn;
    delete[] spec_smooth;
    delete[] spec_tmp;
    delete[] spec_tmp2;
    delete[] tmp4;
    delete[] window;
    delete[] output;
}

/*! peakDetectScale controls sensitivity for peak detection

   0   = least sensitive
   100 = default
   200 = most sensitive
 */
int Spectrum::getPeakDetectScale() const
{
    return(200 - peakDetectScale);
}

/*! set value of peakDectScale from value of slider in Bandmap

   0  =least sensitive
   100= default
   200= most sensitive

 */
void Spectrum::setPeakDetectScale(int i)
{
    peakDetectScale = 200 - i;
}

/*! setting calcErrorNext=true will trigger a IQ error calculation
   on the next sweep */
void Spectrum::setCalcError()
{
    calcErrorNext = true;
}

/*! select plotting of IQ phase/gain points
 */
void Spectrum::setPlotPoints(bool b)
{
    iqPlotOpen = b;
}

/*! return CQ frequency
 */
int Spectrum::findCQFreq()
{
    int f = sigCQ;
    return(f);
}

/*! clear CQ finding state; use when changing bands
 */
void Spectrum::clearCQ()
{
    // clear list of freqs
    for (int i = 0; i < SIG_MAX; i++) {
        sigListCQ[i].clear();
    }
    sigCQ = 0;
}

/*! start CQ finding process

 */
void Spectrum::startFindCQ()
{
    findCQ();
    if (sigCQ) {
        emit(qsy(nrig, sigCQ));
        QString tmp = "QSY to " + QString::number(sigCQ / 1000) + " KHz";
        emit(findCQMessage(tmp));
    }
}

/*! find best CQ freqs among detected signals. Once freq is found,
   qsy is emitted
 */
void Spectrum::findCQ()
{
    // peak detect must be turned on
    if (!settings.value(s_sdr_peakdetect[nrig],s_sdr_peakdetect_def[nrig]).toBool()) {
        return;
    }
    sigCQ = 0;

    // make work copy of list
    for (int i = 0; i < SIG_MAX; i++) {
        sigListCQtmp[i].active = sigListCQ[i].active;
        sigListCQtmp[i].f      = sigListCQ[i].f;
    }

    // add limits to list of freqs
    int findCQLimit[2];
    findCQLimit[0]=settings.value(s_sdr_cqlimit_low[band],cqlimit_default_low[band]).toInt();
    findCQLimit[1]=settings.value(s_sdr_cqlimit_high[band],cqlimit_default_high[band]).toInt();

    // check to make sure limits are within freq range covered by bandmap; if not,
    // adjust ends
    if (findCQLimit[0]<endFreqs[0]) findCQLimit[0]=endFreqs[0];
    if (findCQLimit[1]>endFreqs[1]) findCQLimit[1]=endFreqs[1];

    int indx = -1;
    for (int i = 0; i < SIG_MAX; i++) {
        if (!sigListCQtmp[i].active) {
            indx = i;
            break;
        }
    }
    if (indx != -1) {
        sigListCQtmp[indx].f      = findCQLimit[0];
        sigListCQtmp[indx].active = true;

        int indx2 = -1;
        for (int i = indx + 1; i < SIG_MAX; i++) {
            if (!sigListCQtmp[i].active) {
                indx2 = i;
                break;
            }
        }
        if (indx2 != -1) {
            sigListCQtmp[indx2].f      = findCQLimit[1];
            sigListCQtmp[indx2].active = true;
        }
    }

    // sort signal list by frequency (insertion sort)
    // place "inactive" entries at end of list
    Signal s;
    for (int i = 1; i < SIG_MAX; i++) {
        s = sigListCQtmp[i];
        int  j    = i - 1;
        bool done = false;
        while (!done) {
            if ((sigListCQtmp[j].active && s.active && sigListCQtmp[j].f > s.f) ||
                (!sigListCQtmp[j].active && s.active)) {
                sigListCQtmp[j + 1] = sigListCQtmp[j];
                j--;
                if (j < 0) {
                    done = true;
                }
            } else {
                done = true;
            }
            sigListCQtmp[j + 1] = s;
        }
    }

    // find best cq freq based on max space between adjacent signals
    for (int i = 0; i < SIG_MAX; i++) {
        if (!sigListCQtmp[i].active) break;

        if (sigListCQtmp[i].f < findCQLimit[0]) {
            sigListCQtmp[i].fcq = findCQLimit[0];
        } else if (sigListCQtmp[i].f > findCQLimit[1]) {
            sigListCQtmp[i].fcq = findCQLimit[1];
        } else {
            sigListCQtmp[i].fcq = sigListCQtmp[i].f;
        }
        if (i > 0) {
            sigListCQtmp[i].space = sigListCQtmp[i].fcq - sigListCQtmp[i - 1].fcq;
        } else {
            sigListCQtmp[i].space = 0;
        }
    }

    sigSpace = 0;
    sigCQ    = 0;
    for (int i = 1; i < SIG_MAX; i++) {
        if (!sigListCQtmp[i].active) break;
        if (sigListCQtmp[i].space > sigSpace) {
            sigCQ    = sigListCQtmp[i].fcq - sigListCQtmp[i].space / 2;
            sigSpace = sigListCQtmp[i].space;
        }
    }
}

/*! set radio number (0 or 1)
 */
void Spectrum::setNrig(int n)
{
    nrig = n;
}

/*! make gain/phase table from fit to errors
 */
void Spectrum::makeGainPhase()
{
    for (int i = 0; i < sizes.sample_length; i++) {
        int ix;
        if (i < sizes.sample_length / 2) {
            ix = i;
        } else {
            ix = i - sizes.sample_length;
        }
        double phase = aPhase[0];
        double gain  = aGain[0];
        double x     = (double) (ix / 8);
        double x0    = x;
        for (int j = 1; j < FIT_ORDER; j++) {
            phase += aPhase[j] * x;
            gain  += aGain[j] * x;
            x     *= x0;
        }
        errfunc[i][0] = gain * cos(phase * M_PI / 180.0);
        errfunc[i][1] = gain * sin(phase * M_PI / 180.0);
    }
}


/*! stop spectrum thread
 */
void Spectrum::stopSpectrum()
{
    saveError();
}

/*!
   main thread function. Get data buffer, perform fft and scaling,
   write to spectrum buffer.
 */
void Spectrum::processData(unsigned char *data, unsigned char bptr)
{
    unsigned int  j    = bptr * sizes.advance_size;
    unsigned char *ptr = &data[j];
    for (int i = 0; i < sizes.sample_length; i++) {
        double tmpr;
        double tmpi;
        switch (bits) {
        case 16:
        {
            // convert 16 bit
            int ii = ptr[1];
            ii = (ii << 8) | ptr[0];
            if (ii & 0x8000) ii |= ~0xffff;
            tmpr = ii / 32768.0;
            ptr += 2;
            ii   = ptr[1];
            ii   = (ii << 8) | ptr[0];
            if (ii & 0x8000) ii |= ~0xffff;
            tmpi = ii / 32768.0;
            ptr += 2;
            j   += 4;
        }
        break;
        case 24:
        {
            // convert 24 bit signed integer sample to floating point by placing data into
            // 32-bit signed int
            // data is in ptr[0](LSB) ... ptr[2](MSB)
            //
            int ii = (ptr[2] << 24) | (ptr[1] << 16) | (ptr[0] << 8);

            // divide by (2^31 - 1) - (2^8 -1) = 2147483647 - 255 = 2147483392
            // actual float range then [1.0: -1.000000119]
            tmpr = ii / 2147483392.0;
            ptr += 3;

            // repeat for other stereo channel sample
            ii   = (ptr[2] << 24) | (ptr[1] << 16) | (ptr[0] << 8);
            tmpi = ii / 2147483392.0;
            ptr += 3;
            j   += 6;
        }
        break;
        case 32:
        {
            // convert 32 bit sample to floating point
            // data is in ptr[0](LSB)..ptr[3](MSB)

            // put data into 32-bit signed int
            int ii = (ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];

            // divide by (2^31 - 1) = 2147483647
            tmpr = ii / 2147483647.0;
            ptr += 4;

            // repeat for other stereo channel sample
            ii   = (ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];
            tmpi = ii / 2147483647.0;
            ptr += 4;
            j   += 8;
        }
        break;
        default:
            tmpr = 0.;
            tmpi = 0.;
            break;
        }
        if (!settings.value(s_sdr_swapiq[nrig],s_sdr_swapiq_def[nrig]).toBool()) {
            in[i][0] = tmpr * window[i];
            in[i][1] = tmpi * window[i];
        } else {
            in[i][0] = tmpi * window[i];
            in[i][1] = tmpr * window[i];
        }

        if (j == sizes.chunk_size) {
            // make buffer circular
            j   = 0;
            ptr = &data[0];
        }
    }

    // done reading raw data, emit signal so audioReader can procede
    emit(done());
#ifdef Q_OS_WIN
    (fftw_executep) (plan);
#else
    fftw_execute(plan);
#endif
    if (settings.value(s_sdr_iqcorrect[nrig],s_sdr_iqcorrect_def[nrig]).toBool()) {
        for (int i = 0; i < sizes.sample_length; i++) {
            // correct IQ imbalance
            int    j    = (sizes.sample_length - i) % sizes.sample_length;
            double real = out[i][0] + out[j][0] - (out[i][1] + out[j][1]) * errfunc[i][1] + (out[i][0] - out[j][0]) * errfunc[i][0];
            double imag = out[i][1] - out[j][1] + (out[i][1] + out[j][1]) * errfunc[i][0] + (out[i][0] - out[j][0]) * errfunc[i][1];
            spec_tmp[i] = real * real + imag * imag;
        }
    } else {
        for (int i = 0; i < sizes.sample_length; i++) {
            spec_tmp[i] = (out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        }
    }

    double bga, sigma;
    measureBackground(bga, sigma, spec_tmp);

    if (settings.value(s_sdr_iqdata[nrig],s_sdr_iqdata_def[nrig]).toBool()) {
        if (calibCnt == (SIG_CALIB_FREQ - 1)) {
            measureIQError(bga, spec_tmp);
            calibCnt = 0;
        } else {
            calibCnt++;
        }
    }

    for (int i = 0; i < sizes.sample_length; i++) {
        spec_tmp[i] = log(spec_tmp[i]);
#ifdef Q_OS_LINUX
        if (isinf(spec_tmp[i])) spec_tmp[i] = -1.0e-16;
#endif
    }

    measureBackgroundLog(bga, sigma, spec_tmp);

    // put upper limit on background. Prevents display "blacking out"
    // from static crashes
    if (bga > 0.0) bga = 0.0;
    if (settings.value(s_sdr_peakdetect[nrig],s_sdr_peakdetect_def[nrig]).toBool()) {
        detectPeaks(bga, sigma, spec_tmp);
    }

    if (settings.value(s_sdr_click[nrig],s_sdr_click_def[nrig]).toBool()) {
        clickRemove(bga, sigma, spec_tmp);

        // re-measure background since click removal changes it
        // measureBackgroundLog(bga,sigma,spec_tmp);
    }

    if (settings.value(s_sdr_scale[nrig],s_sdr_scale_def[nrig]).toInt() == 2) {
        interp2(spec_tmp, tmp4, bga); // expand by 2 using linear interpolation
    } else {
        // IF offset included here
        double tmp = ((settings.value(s_sdr_offset[nrig],s_sdr_offset_def[nrig]).toInt()-addOffset) *
                      sizes.spec_length * settings.value(s_sdr_scale[nrig],s_sdr_scale_def[nrig]).toInt()) / (SAMPLE_FREQ * 1000.0);
        int offsetPix = -(int) tmp;
        for (int i = 0; i < sizes.spec_length; i++) {
            unsigned int j = (sizes.spec_length - offsetPix - MAX_H / 2 + i) % sizes.spec_length;
            spec_tmp[j] = (spec_tmp[j] - bga + 2.0) * 25.0;
            if (spec_tmp[j] < 0.0) {
                spec_tmp[j] = 0.0;
            } else if (spec_tmp[j] > 255.0) {
                spec_tmp[j] = 255.0;
            }
            tmp4[i] = spec_tmp[j];
        }
    }
    unsigned int cnt = 0;
    for (int i = 0; i < sizes.display_length; i++) {
        output[i] = (unsigned char) tmp4[i];
        cnt      += output[i];
    }
    background = cnt / sizes.display_length;  // background measurement
    emit(spectrumReady(output, background));

    if (calcErrorNext) {
        calcError(false);
        calcErrorNext = false;
    }
}

void Spectrum::setAddOffset(int i) {
    addOffset=i;
}

/*! Background in terms of pixel brightness (0:255)
 */
unsigned char Spectrum::bg()
{
    int b = background;
    return(b);
}


/*!
   Set center frequency in Hz
    f=center freq
    b=band index
    low, high=lowest, highest frequencies
 */
void Spectrum::setFreq(int f, int b, int low, int high) {
    centerFreq = f;
    band       = b;
    sigSpace   = 0;
    endFreqs[0]=low;
    endFreqs[1]=high;
}

void Spectrum::clickRemove(double bg, double dev, double spec[]) const
{
    Q_UNUSED(bg);

    // click identification: in log spectrum, key clicks appear
    // with approximately linear dependence in frequency.  This code
    // looks for a linear increase/decrease in the spectrum and
    // removes it. To avoid removing the signal too, the last few
    // points before the slope changes are not modified.

    // first find clicks starting from zero freq and increasing
    int i = 1;
    do {
        // check for 3 successive increasing values in spectrum
        // may want to play around with this criteria
        if (spec[i + 1] > spec[i] &&
            spec[i + 2] > spec[i + 1]) {
            // find point where increase stops
            int m = i + 3;
            do {
                if (spec[m] < spec[m - 1]) break;
                m++;
            } while (m < sizes.sample_length);
            int i2 = m - 4;

            if ((i2 - i) < 2) {
                i++;
                continue;
            }

            // estimate slope
            double s = (spec[i2] - spec[i]) / (i2 - i);

            // spectral subtraction
            for (int j = i; j <= i2; j++) {
                double x = s * (j - i) + dev;
                spec[j] -= x;
            }
            i = m;
        } else {
            i++;
        }
    } while (i < (sizes.sample_length - 3));

    // now sweep in opposite direction
    i = sizes.sample_length - 2;
    do {
        if (spec[i - 1] > spec[i] &&
            spec[i - 2] > spec[i - 1]) {
            // find point where increase stops
            int m = i - 3;
            do {
                if (spec[m] < spec[m + 1]) break;
                m--;
            } while (m >= 0);
            int i2 = m + 4;

            if ((i - i2) < 2) {
                i--;
                continue;
            }

            // estimate slope
            double s = (spec[i2] - spec[i]) / (i - i2);

            // spectral subtraction
            for (int j = i; j >= i2; j--) {
                double x = s * (i - j) + dev;
                spec[j] -= x;
            }
            i = m;
        } else {
            i--;
        }
    } while (i > 2);
}

/*! peak detection/signal ID in spectrum

 */
void Spectrum::detectPeaks(double bg, double sigma, double spec[])
{
    // smooth spectrum with a moving average
    // average includes -2,+2 around a given point, 2k+1=5 total points
    // end points are a special case
    spec_smooth[0]                       = 0.5 * (spec[0] + spec[1]);
    spec_smooth[1]                       = 1.0 / 3.0 * (spec[0] + spec[1] + spec[2]);
    spec_smooth[sizes.sample_length - 1] = 0.5 * (spec[sizes.sample_length - 1] + spec[sizes.sample_length - 2]);
    spec_smooth[sizes.sample_length - 2] = 1.0 / 3.0 * (spec[sizes.sample_length - 1] + spec[sizes.sample_length - 2] + spec[sizes.sample_length - 3]);

    peakAvg[peakAvgCnt][0]                       = spec[0];
    peakAvg[peakAvgCnt][1]                       = spec[1];
    peakAvg[peakAvgCnt][sizes.sample_length - 1] = spec[sizes.sample_length - 1];
    peakAvg[peakAvgCnt][sizes.sample_length - 2] = spec[sizes.sample_length - 2];
    for (int i = 2; i < (sizes.sample_length - 2); i++) {
        peakAvg[peakAvgCnt][i] = 1.0 / 5.0 * (spec[i] + spec[i - 1] + spec[i + 1] + spec[i + 2] + spec[i - 2]);
    }
    if (peakAvgCnt != (SIG_N_AVG - 1)) {
        peakAvgCnt++;
        return;
    }
    peakAvgCnt = 0;
    int offset=(settings.value(s_sdr_offset[nrig],s_sdr_offset_def[nrig]).toInt()-addOffset);

    // average collected scans
    for (int i = 0; i < sizes.sample_length; i++) {
        spec_smooth[i] = 0.;
    }
    for (int j = 0; j < SIG_N_AVG; j++) {
        for (int i = 0; i < sizes.sample_length; i++) {
            spec_smooth[i] += peakAvg[j][i];
        }
    }
    for (int i = 0; i < sizes.sample_length; i++) {
        spec_smooth[i] *= 1.0 / SIG_N_AVG;
    }

    // now look for peaks
    for (int i = 0; i < SIG_MAX; i++) {
        if (sigList[i].active) sigList[i].n--;
        if (sigListCQ[i].active) sigListCQ[i].n--;
    }

    // note that values are negative here
    double cut = bg + peakDetectScale * sigma;
    int    ipk;
    int    i = 1;
    while (i < sizes.sample_length) {
        if (spec_smooth[i] > cut) {
            // look for next point below cut
            int j = i + 1;
            while (j < sizes.sample_length) {
                if (spec_smooth[j] < cut) break;
                j++;
            }

            // not a good peak
            if (j == sizes.sample_length ||
                ((j - i) < SIG_PEAK_MIN_WIDTH) ||
                ((j - i) > SIG_PEAK_MAX_WIDTH)) {
                i = j + 1;
                continue;
            } else {
                // peak position
                double x = spec_smooth[i];
                ipk = i;
                for (int l = i + 1; l < j; l++) {
                    if (spec_smooth[l] > x) {
                        ipk = l;
                        x   = spec_smooth[l];
                    }
                }
                i = j;

                // frequency
                int freq;
                if (ipk > sizes.sample_length / 2) {
                    // neg freqs
                    freq = centerFreq - invert * offset - invert*qRound((sizes.sample_length - ipk) * SAMPLE_FREQ * 1000.0 / sizes.sample_length);
                } else {
                    // positive freqs
                    freq = centerFreq - invert * offset + invert*qRound(ipk * SAMPLE_FREQ * 1000.0 / sizes.sample_length);
                }

                // is this a known signal already?
                bool match = false;
                int  indx  = -1;
                int findCQTime=settings.value(s_sdr_cqtime,s_sdr_cqtime_def).toInt();
                for (int j = 0; j < SIG_MAX; j++) {
                    if (abs(sigList[j].f - freq) < SIG_MIN_FREQ_DIFF) {
                        match = true;
                        indx  = j;
                        sigList[j].cnt++;
                        sigList[j].fsum += freq; // update with new frequency
                        sigList[j].f     = qRound(sigList[j].fsum / sigList[j].cnt);

                        // once signal has been detected 5 times, it becomes "active"
                        if (sigList[j].cnt > 5) {
                            sigList[j].n      = (int) (SIG_KEEP_TIME / 0.008 / SIG_N_AVG);
                            sigList[j].active = true;

                            // add active signals to CQ finding list
                            int indx2 = -1;
                            for (int k = 0; k < SIG_MAX; k++) {
                                // check if already found, if so just update freq
                                if (sigListCQ[k].active && abs(sigListCQ[k].f - freq) < SIG_MIN_FREQ_DIFF) {
                                    indx2          = k;
                                    sigListCQ[k].f = freq;

                                    // reset the counter for this signal
                                    sigListCQ[k].n = (int) (findCQTime / 0.008 / SIG_N_AVG);
                                    break;
                                }
                            }
                            if (indx2 == -1) {
                                for (int k = 0; k < SIG_MAX; k++) {
                                    if (!sigListCQ[k].active) {
                                        indx2 = k;
                                        break;
                                    }
                                }

                                // make sure we haven't run out of slots
                                if (indx2 != -1) {
                                    sigListCQ[indx2].active = true;
                                    sigListCQ[indx2].f      = freq;
                                    sigListCQ[indx2].n      = (int) (findCQTime / 0.008 / SIG_N_AVG);
                                }
                            }
                        }
                        break;
                    }
                }
                if (!match) {
                    // new signal. Find an empty slot
                    indx = -1;
                    for (int k = 0; k < SIG_MAX; k++) {
                        if (sigList[k].cnt == 0) {
                            indx = k;
                            break;
                        }
                    }
                    if (indx != -1) {
                        sigList[indx].f    = freq;
                        sigList[indx].fsum = freq;
                        sigList[indx].cnt  = 1;
                    }
                }
            }
        }
        i++;
    }

    // remove old sigs
    for (int i = 0; i < SIG_MAX; i++) {
        if (sigList[i].active) {
            if (sigList[i].n < 0) {
                sigList[i].active = false;
                sigList[i].cnt    = 0;
                sigList[i].fsum   = 0;
            }
        }
        if (sigListCQ[i].active) {
            if (sigListCQ[i].n < 0) {
                sigListCQ[i].active = false;
            }
        }
    }
}

/*! clear all IQ data
 */
void Spectrum::clearIQ()
{
    for (int i = 0; i < sizeIQ; i++) {
        calibSigList[i].n       = 0;
        calibSigList[i].zsum[0] = 0.;
        calibSigList[i].zsum[1] = 0.;
        calibSigList[i].gain    = 1.0;
        calibSigList[i].phase   = 0.;
    }
    if (iqPlotOpen) emit(clearPlot());

    // reset fit params
    for (int i = 0; i < FIT_ORDER; i++) {
        aGain[i]  = 0.;
        aPhase[i] = 0.;
    }
}

/*! clear list of detected signals
 */
void Spectrum::clearSigs()
{
    for (int i = 0; i < SIG_MAX; i++) {
        sigList[i].active = false;
        sigList[i].n      = 0;
        sigList[i].cnt    = 0;
        sigList[i].fsum   = 0;
        sigList[i].fcq    = 0;
        sigList[i].space  = 0;
        sigList[i].f      = 0;
    }
}

/*! reset averaging counter
 */
void Spectrum::resetAvg()
{
    peakAvgCnt = 0;
}

/*! set whether IF is inverted or not
 */
void Spectrum::setInvert(bool b)
{
    if (b) invert = -1;
    else invert = 1;
}

/*! take measurements of strong signals for IQ balancing
 */
void Spectrum::measureIQError(double bg, double spec[])
{
  // if (settings.value(s_sdr_iqdata[nrig],s_sdr_iqdata_def[nrig]).toBool()==false) return;

    double a1[2];
    double a2[2];
    double z[2];
    double p;

    // cut level is 30 dB over background
    double cut_calib = bg * 1000.0;

    for (int i = 1; i < sizes.sample_length; i++) {
        if (spec[i] > cut_calib) {
            int j = i / 8;
            a1[0] = out[i][0];
            a1[1] = out[i][1];
            a2[0] = out[sizes.sample_length - i][0];
            a2[1] = out[sizes.sample_length - i][1];
            complexMult(z, a1, a2);
            p     = a1[0] * a1[0] + a1[1] * a1[1] + a2[0] * a2[0] + a2[1] * a2[1];
            z[0] /= p;
            z[1] /= p;
            calibSigList[j].n++;
            calibSigList[j].zsum[0] += z[0];
            calibSigList[j].zsum[1] += z[1];
        }
    }
}


/*! calculate gain and phase error for IQ balancing

   if force=true, will fit errors regardless of status of "measure data" option. This
   is needed to fit the error read in from disk when the bandmaps starts. Otherwise, fits
   only need to be done if data is being collected.
 */
void Spectrum::calcError(bool force)
{
    if (!force && !settings.value(s_sdr_iqdata[nrig],s_sdr_iqdata_def[nrig]).toBool()) return;
    double t;
    double phase, phaseDeg;
    double gain;
    double minPhase = 1000.0;
    double minGain  = 10.0;
    double maxPhase = 0.;
    double maxGain  = 0.;
    int    cnt      = 0;

    if (iqPlotOpen) emit(clearPlot());

    // skip points at end points
    int mid1 = sizeIQ / 2 - 5;
    int mid2 = mid1 + 10;

    for (int i = 5; i < (sizeIQ - 5); i++) {
        // skip points near zero freq
        if (i > mid1 && i < mid2) continue;

        if (calibSigList[i].n > 500) {
            calibSigList[i].z[0]  = calibSigList[i].zsum[0] / calibSigList[i].n;
            calibSigList[i].z[1]  = calibSigList[i].zsum[1] / calibSigList[i].n;
            t                     = sqrt(1.0 - 4.0 * calibSigList[i].z[0] * calibSigList[i].z[0]);
            phase                 = asin(2.0 * calibSigList[i].z[1] / t);
            phaseDeg              = phase * 180.0 / M_PI;
            calibSigList[i].phase = phaseDeg;
            if (phaseDeg > maxPhase) {
                maxPhase = phaseDeg;
            }
            if (phaseDeg < minPhase) {
                minPhase = phaseDeg;
            }
            gain                 = t / (1.0 - 2.0 * calibSigList[i].z[0]);
            calibSigList[i].gain = gain;
            if (gain > maxGain) {
                maxGain = gain;
            }
            if (gain < minGain) {
                minGain = gain;
            }
            cnt++;
        }
    }
    if (iqPlotOpen) {
        emit(gainScale(minGain, maxGain));
        emit(phaseScale(minPhase, maxPhase));
        for (int i = 5; i < (sizeIQ - 5); i++) {
            if (i > mid1 && i < mid2) continue;
            if (calibSigList[i].n > 500) {
                int j = (i + sizeIQ / 2) % sizeIQ;
                emit(gainPoint(j, calibSigList[i].gain));
                emit(phasePoint(j, calibSigList[i].phase));
            }
        }
    }
    if (cnt > 10) {
        fitErrors();
        if (iqPlotOpen) {
            emit(plotGainFunc(aGain[0], aGain[1], aGain[2], aGain[3]));
            emit(plotPhaseFunc(aPhase[0], aPhase[1], aPhase[2], aPhase[3]));
        }
    }
}

/* read in saved IQ error data
 */
bool Spectrum::readError()
{
    QDir       directory;
    directory.setCurrent(userDirectory);
    QByteArray fname = "iq" + QByteArray::number(nrig) + ".dat";
    QFile      file(fname);
    if (!file.open(QIODevice::ReadOnly)) {
        return(false);
    }
    QDataStream in(&file);
    for (int i = 0; i < sizeIQ; i++) {
        calibSigList[i].n       = 0;
        calibSigList[i].zsum[0] = 0.;
        calibSigList[i].zsum[1] = 0.;
    }
    while (!in.atEnd()) {
        int       i;
        long long n;
        double    x, y;
        in >> i;
        in >> n;
        in >> x >> y;
        if (i >= 0 && i < sizeIQ) {
            calibSigList[i].n       = n;
            calibSigList[i].zsum[0] = x;
            calibSigList[i].zsum[1] = y;
            calibSigList[i].z[0]    = calibSigList[i].zsum[0] / calibSigList[i].n;
            calibSigList[i].z[1]    = calibSigList[i].zsum[1] / calibSigList[i].n;
        }
    }
    file.close();
    return(true);
}

/* save IQ data to disk
 */
bool Spectrum::saveError()
{
    QDir       directory;
    directory.setCurrent(userDirectory);
    QByteArray fname = "iq" + QByteArray::number(nrig) + ".dat";
    QFile      file(fname);
    if (!file.open(QIODevice::WriteOnly)) {
        return(false);
    }
    QDataStream fstr(&file);
    for (int i = 0; i < sizeIQ; i++) {
        if (calibSigList[i].n) {
            fstr << i;
            fstr << calibSigList[i].n;
            fstr << calibSigList[i].zsum[0];
            fstr << calibSigList[i].zsum[1];
        }
    }
    file.close();
    return(true);
}

void Spectrum::complexMult(double a[], double b[], double c[]) const

// a=b*c
{
    a[0] = b[0] * c[0] - b[1] * c[1];
    a[1] = b[1] * c[0] + b[0] * c[1];
}

/*!
   Interpolate spectrum: expand by factor of 2 with linear interpolation

   spectrum still centered around zero freqs
 */
void Spectrum::interp2(double in[], double out[], double bga)
{
    int offset=(settings.value(s_sdr_offset[nrig],s_sdr_offset_def[nrig]).toInt()-addOffset);
    double tmp = (offset * sizes.spec_length * settings.value(s_sdr_scale[nrig],s_sdr_scale_def[nrig]).toInt()) / (SAMPLE_FREQ * 1000.0);
    int offsetPix = -(int) tmp;

    unsigned int j0 = (sizes.spec_length - offsetPix / 2 - MAX_H / 4) % sizes.spec_length;
    in[j0] = (in[j0] - bga + 2.0) * 25.0;
    for (int i = 0; i < sizes.display_length / 2; i++) {
        unsigned int j   = (sizes.spec_length - offsetPix / 2 - MAX_H / 4 + i) % sizes.spec_length;
        unsigned int jp1 = (j + 1) % sizes.spec_length;

        in[jp1]    = (in[jp1] - bga + 2.0) * 25.0;
        out[i * 2] = in[j];
        if (out[i * 2] < 0.0) {
            out[i * 2] = 0.0;
        } else if (out[i * 2] > 255.0) {
            out[i * 2] = 255.0;
        }
        double s = (in[jp1] - in[j]) * 0.2;
        out[i * 2 + 1] = in[j] + s;
        if (out[i * 2 + 1] < 0.0) {
            out[i * 2 + 1] = 0.0;
        } else if (out[i * 2 + 1] > 255.0) {
            out[i * 2 + 1] = 255.0;
        }
    }
}

/*!
   measures average background level of spectrum
 */
void Spectrum::measureBackground(double &background, double &sigma, double spec[]) const
{
    // estimate average and standard error; only keep every 4th point
    double avg = 0.;
    for (int i = 0; i < sizes.sample_length; i += 4) {
        avg += spec[i];
    }

    avg /= (sizes.sample_length * 0.25);

    // now get avg background, ignoring large signals
    background = 0.;
    int    n   = 0;
    double bg2 = 0.;
    for (int i = 0; i < sizes.sample_length; i += 4) {
        if (spec[i] < 5.0 * avg) {
            background += spec[i];
            bg2        += spec[i] * spec[i];
            n++;
        }
    }
    if (n != 0) {
        background /= (double) n;
        bg2        /= (double) n;
        sigma       = 1.0 / (n - 1) * (bg2 - background * background);
        if (sigma > 1.0e-6) sigma = sqrt(sigma);
        else sigma = 0.;
    } else {
        background = avg;
        sigma      = 0.;
    }
}

/*!
   measures average background level of (log) spectrum
 */
void Spectrum::measureBackgroundLog(double &background, double &sigma, double spec[]) const
{
    // estimate average and standard error; only keep every 4th point
    double avg = 0.;
    for (int i = 0; i < sizes.sample_length; i += 4) {
        avg += spec[i];
    }
    avg /= (sizes.sample_length / 4);

    // now get avg background, ignoring signals 6dB over background
    background = 0.;
    int    n   = 0;
    double bg2 = 0.;
    for (int i = 0; i < sizes.sample_length; i++) {
        if (spec[i] < (avg + 1.38)) {
            background += spec[i];
            bg2        += spec[i] * spec[i];
            n++;
        }
    }
    if (n != 0) {
        background /= (double) n;
        bg2        /= (double) n;
        sigma       = 1.0 / (n - 1) * (bg2 - background * background);
        if (sigma > 1.0e-6) sigma = sqrt(sigma);
        else sigma = 0.;
    } else {
        background = avg;
        sigma      = 0.;
    }
}


#ifdef Q_OS_WIN
bool Spectrum::fftwWinInit()

// initialize FFTW DLL under windows
{
    HINSTANCE hLib = LoadLibraryA("libfftw3-3.dll");

    if (hLib == NULL) {
        qDebug("LoadLibrary libfftw3-3.dll failed");
        return(false);
    }
    fftw_mallocp = (fftw_malloc_ptr) GetProcAddress(hLib, "fftw_malloc");
    if (fftw_mallocp == NULL) {
        qDebug("GetProcAddress for fftw_malloc Failed.");
        return(false);
    }
    fftw_freep = (fftw_free_ptr) GetProcAddress(hLib, "fftw_free");
    if (fftw_freep == NULL) {
        qDebug("GetProcAddress for fftw_free Failed.");
        return(false);
    }
    fftw_plan_dft_1dp = (fftw_plan_dft_1d_ptr) GetProcAddress(hLib, "fftw_plan_dft_1d");
    if (fftw_plan_dft_1dp == NULL) {
        qDebug("GetProcAddress for fftw_plan_dft_1d Failed.");
        return(false);
    }
    fftw_destroy_planp = (fftw_destroy_plan_ptr) GetProcAddress(hLib, "fftw_destroy_plan");
    if (fftw_destroy_planp == NULL) {
        qDebug("GetProcAddress for fftw_destroy_plan Failed.");
        return(false);
    }
    fftw_executep = (fftw_destroy_plan_ptr) GetProcAddress(hLib, "fftw_execute");
    if (fftw_executep == NULL) {
        qDebug("GetProcAddress for fftw_execute Failed.");
        return(false);
    }
    return(true);
}
#endif

/*! calculate FFT window coefficients (Nuttall)
 */
void Spectrum::makeWindow()
{
    const double a0 = 0.355768;
    const double a1 = 0.487396;
    const double a2 = 0.144232;
    const double a3 = 0.012604;

    double       tmp = 2.0 * M_PI / (sizes.sample_length - 1.0);
    for (int i = 0; i < sizes.sample_length; i++) {
        window[i] = a0 - a1*cos(i*tmp) + a2*cos(2.0 * i * tmp) - a3*cos(3.0 * tmp * i);
    }
}

/*! Perform least-squares fit to polynomial of gain and phase errors
 */
void Spectrum::fitErrors()
{
    double a[FIT_ORDER][FIT_ORDER];
    double sum[FIT_ORDER * 2];
    int    nxo2 = sizeIQ / 2;

    // count number of freqs that have a reasonable amount of data
    // skip points at end points
    int mid1 = nxo2 - 5;
    int mid2 = mid1 + 10;
    int n    = 0;
    const int N_CUT=500;
    for (int i = 5; i < (sizeIQ - 5); i++) {
        if (i > mid1 && i < mid2) continue;

        if (calibSigList[i].n > N_CUT) n++;
    }

    // not enough data points. May want to check for distribution of points
    // evenly covering freq interval
    if (n < 10) return;

    // sum of x to various powers
    for (int i = 0; i < FIT_ORDER; i++) {
        aGain[i]  = 0.;
        aPhase[i] = 0.;
    }
    for (int i = 0; i < FIT_ORDER * 2; i++) {
        sum[i] = 0.;
    }
    sum[0] = (double) n;

    double gain, phase;
    for (int k = 5; k < (sizeIQ - 5); k++) {
        if (k > mid1 && k < mid2) continue;

        if (calibSigList[k].n > N_CUT) {
            int ix = k; // ix is frequency of bin
            if (k >= nxo2) {
                // - freqs
                ix -= sizeIQ;
            }
            gain  = calibSigList[k].gain;
            phase = calibSigList[k].phase;

            double x0 = (double) ix;
            double x  = x0;

            aGain[0]  += gain;
            aPhase[0] += phase;

            sum[1]    += x;
            aGain[1]  += x * gain;
            aPhase[1] += x * phase;
            x         *= x0;

            sum[2]    += x;
            aGain[2]  += x * gain;
            aPhase[2] += x * phase;
            x         *= x0;

            sum[3]    += x;
            aGain[3]  += x * gain;
            aPhase[3] += x * phase;
            x         *= x0;

            sum[4] += x;
            x      *= x0;
            sum[5] += x;
            x      *= x0;
            sum[6] += x;
        }
    }

    // fit gain
    for (int i = 0; i < FIT_ORDER; i++) {
        for (int j = 0; j < FIT_ORDER; j++) {
            a[i][j] = sum[i + j];
        }
    }
    gaussElim(a, aGain, FIT_ORDER);

    // fit phase
    for (int i = 0; i < FIT_ORDER; i++) {
        for (int j = 0; j < FIT_ORDER; j++) {
            a[i][j] = sum[i + j];
        }
    }
    gaussElim(a, aPhase, FIT_ORDER);
    makeGainPhase();
}

/*! very simple Gaussian elimination
 */
void Spectrum::gaussElim(double a[FIT_ORDER][FIT_ORDER], double y[FIT_ORDER], int n)
{
    int    max;
    double t;
    for (int i = 0; i < n; i++) {
        max = i;
        for (int j = i + 1; j < n; j++) {
            if (a[j][i] > a[max][i])
                max = j;
        }
        for (int j = 0; j < n; j++) {
            t         = a[max][j];
            a[max][j] = a[i][j];
            a[i][j]   = t;
        }
        t      = y[max];
        y[max] = y[i];
        y[i]   = t;
        for (int k = i + 1; k < n; k++) {
            y[k] -= a[k][i] / a[i][i] * y[i];
        }
        for (int j = n - 1; j >= i; j--) {
            for (int k = i + 1; k < n; k++) {
                a[k][j] -= a[k][i] / a[i][i] * a[i][j];
            }
        }
    }
    for (int i = n - 1; i >= 0; i--) {
        y[i]    = y[i] / a[i][i];
        a[i][i] = 1.0;
        for (int j = i - 1; j >= 0; j--) {
            y[j]   -= a[j][i] * y[i];
            a[j][i] = 0.;
        }
    }
}

/*!
  returns closest peak-detected freqency to fin

  if freq difference is larger than SIG_MIN_SPOT_DIFF, returns fin
  */
int Spectrum::closestFreq(int fin) const
{
    int f=fin;
    int delta=SIG_MIN_SPOT_DIFF;
    for (int i=0;i<SIG_MAX;i++) {
        if (sigList[i].active) {
            if (abs(fin-sigList[i].f)<delta) {
                delta=abs(fin-sigList[i].f);
                f=sigList[i].f;
            }
        }
    }
    return f;
}
