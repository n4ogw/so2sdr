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
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPixmap>
#include "bandmapdisplay.h"
#include "defines.h"
#include "spectrum.h"

BandmapDisplay::BandmapDisplay(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);
    pixmap = QPixmap(4096, 4096);
    QPainter p(&pixmap);
    p.fillRect(pixmap.rect(), QColor(0, 0, 0));
    _invert   = false;
    scale     = 1;
    mark      = true;
    samplerate= 96000;
    vfoPos = height()/2;
    cornery = 4096/2 - vfoPos;
    cornerx = MAX_W - width();
    cmap=0;
    markRgb0=0;
    markRgb1=0;
    markRgb2=0;
}

void BandmapDisplay::initialize(QSettings *s)
{
    settings=s;
    samplerate=settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toInt();
    pixmap = QPixmap(MAX_W, settings->value(s_sdr_fft,s_sdr_fft_def).toInt());
    QPainter p(&pixmap);
    p.fillRect(pixmap.rect(), QColor(0, 0, 0));
    cornerx   = MAX_W - width();
    if (!cmap) delete [] cmap;
    if (!markRgb0) delete [] markRgb0;
    if (!markRgb1) delete [] markRgb1;
    if (!markRgb2) delete [] markRgb2;
    cmap = new bool[settings->value(s_sdr_fft,s_sdr_fft_def).toInt()];
    markRgb0 = new bool[settings->value(s_sdr_fft,s_sdr_fft_def).toInt()];
    markRgb1 = new bool[settings->value(s_sdr_fft,s_sdr_fft_def).toInt()];
    markRgb2 = new bool[settings->value(s_sdr_fft,s_sdr_fft_def).toInt()];
    for (int i = 0; i < settings->value(s_sdr_fft,s_sdr_fft_def).toInt(); i++) {
        cmap[i] = false;
        markRgb0[i]=true;
        markRgb1[i]=true;
        markRgb2[i]=true;
    }
}

void BandmapDisplay::setVfoPos(int s)
{
    vfoPos=s;
    cornery   = settings->value(s_sdr_fft,s_sdr_fft_def).toInt() / 2 - vfoPos;
}

/*!
 * \brief BandmapDisplay::setScale
 * \param s = BandmapDisplay scale: either 1 or 2
 */
void BandmapDisplay::setScale(int s)
{
    scale=s;
}

/*!
 * \brief BandmapDisplay::setMark
 * \param b = true: highlight dupe signals
 */
void BandmapDisplay::setMark(bool b)
{
    mark=b;
}

BandmapDisplay::~BandmapDisplay()
{
    delete [] cmap;
    delete [] markRgb0;
    delete [] markRgb1;
    delete [] markRgb2;
}

/*!
   mouse handler: if mouse left clicked, emits QSY frequency change in +/- Hz
 */
void BandmapDisplay::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int y = event->y();

        // compute QSY as change in frequency
        int delta_f = (int) (samplerate / (double) settings->value(s_sdr_fft,s_sdr_fft_def).toInt() / scale* (vfoPos - y));
        emit(mouseClick());
        emit(displayMouseQSY(delta_f));
    }
}


/*!
   option to invert BandmapDisplay. Needed for K2 on high bands or CW reverse.
 */
void BandmapDisplay::setInvert(bool t)
{
    _invert = t;
}

/*! return invert status
 */
bool BandmapDisplay::invert() const
{
    return(_invert);
}

/*!
   advance spectrum one scan line to left. The spectrum is plotted on the
   QPixmap pixmap, which then gets drawn on the screen in paintEvent
 */
void BandmapDisplay::plotSpectrum(unsigned char *data, unsigned char bg)
{
    int hgt=height();
    unsigned char cut;
    if (bg < 225) cut = bg + 30;
    else cut = bg;
    if (settings->value(s_sdr_reverse_scroll,s_sdr_reverse_scroll_def).toBool()) {
        pixmap.scroll(1, 0, QRect(cornerx, cornery, width(), hgt));
    } else {
        pixmap.scroll(-1, 0, QRect(cornerx, cornery, width(), hgt));
    }
    int      dy = hgt / 2 - vfoPos;
    QPainter painter(&pixmap);
    if (!_invert) {
        for (int i = (settings->value(s_sdr_fft,s_sdr_fft_def).toInt() - hgt) / 2 - dy, j = (settings->value(s_sdr_fft,s_sdr_fft_def).toInt() + hgt) / 2 - 1 + dy; j >= (settings->value(s_sdr_fft,s_sdr_fft_def).toInt() - hgt) / 2 + dy; i++, j--) {
            if (cmap[j] && data[i] > cut && mark) {
                unsigned char r=data[i];
                unsigned char g=data[i];
                unsigned char b=data[i];
                if (!markRgb0[j]) r=0;
                if (!markRgb1[j]) g=0;
                if (!markRgb2[j]) b=0;
                painter.setPen(qRgb(r,g,b));
            } else {
                painter.setPen(qRgb(data[i], data[i], data[i]));
            }
            if (settings->value(s_sdr_reverse_scroll,s_sdr_reverse_scroll_def).toBool()) {
                painter.drawPoint(MAX_W - width(), j);
            } else {
                painter.drawPoint(MAX_W - 1, j);
            }
        }
    } else {
        for (int i = (settings->value(s_sdr_fft,s_sdr_fft_def).toInt() - hgt) / 2 - dy, j = (settings->value(s_sdr_fft,s_sdr_fft_def).toInt() - hgt) / 2 + dy; j < settings->value(s_sdr_fft,s_sdr_fft_def).toInt(); i++, j++) {
            if (mark && cmap[j] && data[i] > cut) {
                unsigned char r=data[i];
                unsigned char g=data[i];
                unsigned char b=data[i];
                if (!markRgb0[j]) r=0;
                if (!markRgb1[j]) g=0;
                if (!markRgb2[j]) b=0;
                painter.setPen(qRgb(r,g,b));
            } else {
                painter.setPen(qRgb(data[i], data[i], data[i]));
            }
            if (settings->value(s_sdr_reverse_scroll,s_sdr_reverse_scroll_def).toBool()) {
                painter.drawPoint(MAX_W - width(), j);
            } else {
                painter.drawPoint(MAX_W - 1, j);
            }
        }
    }
    painter.end();
    update();
}

/*! Widget resize event: updates corners of pixmap which map to the widget corners */
void BandmapDisplay::resizeEvent(QResizeEvent * event)
{
    Q_UNUSED(event);
    cornerx = MAX_W - width();
    cornery = settings->value(s_sdr_fft,s_sdr_fft_def).toInt() / 2 - vfoPos;
}

/*! draw pixmap on the widget */
void BandmapDisplay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.drawPixmap(0, 0, pixmap, cornerx, cornery, width(), height());
}
