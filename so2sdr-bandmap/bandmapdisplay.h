/*! Copyright 2010-2024 R. Torsten Clay N4OGW

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
#ifndef BandmapDisplay_H
#define BandmapDisplay_H

#include <QPixmap>
#include <QSettings>
#include <QWidget>

/*!
   Low level BandmapDisplay of spectrum.
 */
class BandmapDisplay : public QWidget {
  Q_OBJECT

public:
  explicit BandmapDisplay(QWidget *parent = nullptr);
  ~BandmapDisplay();
  friend class So2sdrBandmap;

  void initialize(QSettings *s);
  bool invert() const;
  void setInvert(bool t);
  void setMark(bool b);
  void setScale(int s);
  void setVfoPos(int s);
  int y1() const { return cornery; }
  int y2() const { return (cornery + height()); }

public slots:
  void plotSpectrum(unsigned char *, unsigned char);
  void setSampleRate(unsigned int f) { samplerate = f; }

signals:
  void mouseClick();
  void displayMouseQSY(int);

protected:
  void paintEvent(QPaintEvent *event);
  void resizeEvent(QResizeEvent *event);
  void mousePressEvent(QMouseEvent *event);

private:
  int cornerx;
  int cornery;
  QPixmap pixmap;
  bool *cmap;
  bool _invert;
  bool mark;
  bool *markRgb0;
  bool *markRgb1;
  bool *markRgb2;
  int scale;
  int vfoPos;
  unsigned int samplerate;
  unsigned char *dataptr;
  QSettings *settings;
};

#endif
