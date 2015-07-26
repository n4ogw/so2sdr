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
#ifndef GLBANDMAP_H
#define GLBANDMAP_H

#include <QWidget>
#include <QPixmap>
#include "defines.h"


/*!
   Low level display of spectrum. Change QWidget to QGLWidget to use OpenGL. In tests
   so far, this does not seem to make any difference in performance.

 */
class GLBandmap : public QWidget
{
Q_OBJECT

public:
    GLBandmap(QWidget *parent = 0);
    ~GLBandmap();
    friend class Bandmap;

    void initialize(int nr, sampleSizes s);
    bool invert() const;
    void setInvert(bool t);
    void setMark(bool b);
    void setScale(int s);

public slots:
    void plotSpectrum(unsigned char *, unsigned char);

signals:
    void mouseClick();
    void GLBandmapMouseQSY(int, int);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent * event);
    void mousePressEvent(QMouseEvent * event);

private:
    int     cornerx;
    int     cornery;
    QPixmap pixmap;
    bool          cmap[MAX_H];
    bool          _invert;
    bool          mark;
    int           nrig;
    int           scale;
    int           vfoPos;
    sampleSizes   sizes;
    unsigned char cut;
    unsigned char *dataptr;
};

#endif
