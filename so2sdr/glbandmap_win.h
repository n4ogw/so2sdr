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
#ifndef GLBANDMAP_H
#define GLBANDMAP_H

#include <QSettings>
#include <QWidget>
#include "defines.h"

/*!
   Low level display of spectrum, Windows version
 */


class GLBandmap : public QWidget

{
Q_OBJECT

public:
    GLBandmap(QWidget *parent = 0);
    ~GLBandmap();
    friend class Bandmap;

	void initialize(QSettings *set,int nr, sampleSizes s);
    bool invert() const;
    void setInvert(bool t);

public slots:
    void plotSpectrum(unsigned char *, unsigned char);
 
signals:
    void GLBandmapMouseQSY(int, int);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent * event);
    void mousePressEvent(QMouseEvent * event);

private:
    bool          cmap[MAX_H];
    bool          _invert;
    int           nrig;
    int           vfoPos;
    sampleSizes   sizes;
    unsigned char cut;
    unsigned char *dataptr;
	QSettings *settings;
};

#endif
