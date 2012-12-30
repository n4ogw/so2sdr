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
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include "glbandmap_win.h"
#include "defines.h"
#include "spectrum.h"

/*! This version for Windows only: it draws directly on the screen rather
   than into a pixmap */
GLBandmap::GLBandmap(QWidget *parent) : QWidget(parent)

// : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    _invert   = false;
    nrig      = 0;
    dataptr   = NULL;
    vfoPos    = height() / 2;
    for (int i = 0; i < MAX_H; i++) cmap[i] = false;
}

GLBandmap::~GLBandmap()
{
}

/*! widget resize event. Currently on windows just use the default
   event handler */
void GLBandmap::resizeEvent(QResizeEvent * event)
{
    Q_UNUSED(event);
}

/*!
   mouse handler: if mouse left clicked, emits QSY frequency change in +/- Hz
 */
void GLBandmap::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int y = event->y();

        // compute QSY as change in frequency
        int delta_f = (int) (1000.0 * SAMPLE_FREQ / (double) sizes.spec_length /
                             settings->value(s_sdr_scale[nrig],s_sdr_scale_def[nrig]).toInt( )* (vfoPos - y));
        emit(GLBandmapMouseQSY(nrig, delta_f));
    }
}

/*!
   option to invert display. Needed for K2 on high bands or CW reverse.
 */
void GLBandmap::setInvert(bool t)
{
    _invert = t;
}

/*! returns current invert status */
bool GLBandmap::invert() const
{
    return(_invert);
}


/*!
   Initialize
 */
void GLBandmap::initialize(QSettings *set,int nr, sampleSizes s)
{
    settings=set;
    nrig  = nr;
    sizes = s;
}

/*!
   advance spectrum one scan line to left: Signal emitted by Spectrum
   connects here
 */
void GLBandmap::plotSpectrum(unsigned char *data, unsigned char bg)
{
    if (bg < 225) {
        cut = bg + 30;
    } else {
        cut = bg;
    }
    dataptr = data;
    scroll(-1, 0);

    // this will trigger the redraw
    repaint();
}

/*!
   Paint event that draws he next scanline of the spectrum. In Windows
   get better performance drawing directly on widget.
 */
void GLBandmap::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // in case data not available yet
    if (dataptr == NULL) {
        return;
    }
    bool spotCalls=settings->value(s_sdr_mark,s_sdr_mark_def).toBool();	
    // draw new scanline
    int      dy = height() / 2 - vfoPos;
    int      w  = width();
    int      h  = height();
    QPainter p(this);
    if (!_invert) {
        for (int i = (MAX_H - h) / 2 - dy, j = h - 1; j >= 0; i++, j--) {
            if (cmap[j] && dataptr[i] > cut && spotCalls) {
                p.setPen(qRgb(dataptr[i], 0, dataptr[i]));
            } else {
                p.setPen(qRgb(dataptr[i], dataptr[i], dataptr[i]));
            }
            p.drawPoint(w - 1, j);
        }
    } else {
        for (int i = (MAX_H - h) / 2 - dy, j = 0; j < h; i++, j++) {
            if (cmap[j] && dataptr[i] > cut && spotCalls) {
                p.setPen(qRgb(dataptr[i], 0, dataptr[i]));
            } else {
                p.setPen(qRgb(dataptr[i], dataptr[i], dataptr[i]));
            }
            p.drawPoint(w - 1, j);
        }
    }
    p.end();
}
