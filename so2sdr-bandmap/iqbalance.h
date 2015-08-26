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
#ifndef IQBALANCE_H
#define IQBALANCE_H
#include "ui_iqbalance.h"
#include <QDialog>
#include <QPainter>
#include <QPixmap>
#include <QCloseEvent>
#include <QWidget>
#include "defines.h"

const int IQ_PLOT_WIDTH=512;
const int IQ_PLOT_HEIGHT=120;

class IQBalance : public QWidget, public Ui::IQBalance
{
Q_OBJECT

public:
    IQBalance(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~IQBalance();
    friend class So2sdrBandmap;

public slots:
    void clearPlots();
    void plotGainPoint(int bin, double y);
    void plotGainFunc(double, double, double, double);
    void plotPhasePoint(int bin, double y);
    void plotPhaseFunc(double, double, double, double);
    void setGainScale(double, double);
    void setPhaseScale(double, double);

signals:
    void closed(bool);
    void restart();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void restartIQ();

private:
    double  gainOffset;
    double  gainScale;
    double  phaseOffset;
    double  phaseScale;
    QPixmap gainPixmap;
    QPixmap phasePixmap;
};

#endif
