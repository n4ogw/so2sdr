/*! Copyright 2010-2012 R. Torsten Clay N4OGW

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
#include "glbandmap.h"
#include "defines.h"
#include "spectrum.h"

GLBandmap::GLBandmap(QWidget *parent) : QWidget(parent)

// : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAutoFillBackground(false);
    pixmap = QPixmap(MAX_W, MAX_H);
    QPainter p(&pixmap);
    p.fillRect(pixmap.rect(), QColor(0, 0, 0));
    vfoPos    = height() / 2;
    cornerx   = MAX_W - width();
    cornery   = MAX_H / 2 - vfoPos;
    _invert   = false;
    nrig      = 0;
}

GLBandmap::~GLBandmap()
{
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

/*! return invert status
 */
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
   advance spectrum one scan line to left. The spectrum is plotted on the
   QPixmap pixmap, which then gets drawn on the screen in paintEvent
 */
void GLBandmap::plotSpectrum(unsigned char *data, unsigned char bg)
{
    unsigned char cut;
    if (bg < 225) cut = bg + 30;
    else cut = bg;
    pixmap.scroll(-1, 0, QRect(cornerx, cornery, width(), height()));
    int      dy = height() / 2 - vfoPos;
    QPainter painter(&pixmap);
    bool spotCalls=settings->value(s_sdr_mark,s_sdr_mark_def).toBool();
    if (!_invert) {
        for (int i = (MAX_H - height()) / 2 - dy, j = (MAX_H + height()) / 2 - 1 + dy; j >= (MAX_H - height()) / 2 + dy; i++, j--) {
            if (cmap[j] && data[i] > cut && spotCalls) {
                painter.setPen(qRgb(data[i], 0, data[i]));
            } else {
                painter.setPen(qRgb(data[i], data[i], data[i]));
            }
            painter.drawPoint(MAX_W - 1, j);
        }
    } else {
        for (int i = (MAX_H - height()) / 2 - dy, j = (MAX_H - height()) / 2 + dy; j < MAX_H; i++, j++) {
            if (cmap[j] && data[i] > cut && spotCalls) {
                painter.setPen(qRgb(data[i], 0, data[i]));
            } else {
                painter.setPen(qRgb(data[i], data[i], data[i]));
            }
            painter.drawPoint(MAX_W - 1, j);
        }
    }
    painter.end();
    update();
}

/*! Widget resize event: updates corners of pixmap which map to the widget corners */
void GLBandmap::resizeEvent(QResizeEvent * event)
{
    Q_UNUSED(event);
    cornerx = MAX_W - width();
    cornery = MAX_H / 2 - vfoPos;
}

/*! draw pixmap on the widget */
void GLBandmap::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.drawPixmap(0, 0, pixmap, cornerx, cornery, width(), height());
}
