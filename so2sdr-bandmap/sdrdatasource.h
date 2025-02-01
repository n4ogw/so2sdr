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
#ifndef SDRDATASOURCE_H
#define SDRDATASOURCE_H

#include "defines.h"
#include <QObject>
#include <QSettings>
#include <QString>

class SdrDataSource : public QObject {
  Q_OBJECT
public:
  explicit SdrDataSource(const QString &settingsFile, QObject *parent = nullptr);
  ~SdrDataSource();
  virtual unsigned int sampleRate() const = 0;
  void setSampleSizes(sampleSizes s);
  bool isRunning() { return running; }
  virtual bool isSlave() const = 0;

signals:
  void error(const QString &);
  void ready(unsigned char *, unsigned int);
  void resetRfFlag();
  void stopped();
  void realSampleRateSignal(unsigned int);
  void clockFreqSignal(unsigned int);

public slots:
  virtual void stop() = 0;
  virtual void initialize() = 0;
  virtual void setRfFreq(double f) = 0;
  virtual void setRfFreqChannel(double f, int c) = 0;

protected:
  sampleSizes sizes;
  QSettings *settings;
  std::atomic<bool> stopFlag;
  std::atomic<bool> running;
  std::atomic<double> rfFreq;
};

#endif // SDRDATASOURCE_H
