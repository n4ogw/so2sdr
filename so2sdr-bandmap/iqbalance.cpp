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
#include "iqbalance.h"
#include <QMessageBox>
/*!
   note that the pixmap in the plot has dimensions IQ_PLOT_WIDTH x IQ_PLOT_HEIGHT
 */
IQBalance::IQBalance(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f)
{
    setupUi(this);
    connect(pushButtonRestart, SIGNAL(clicked()), this, SLOT(restartIQ()));
    connect(pushButtonClose,SIGNAL(clicked()),this,SLOT(close()));
    gainPixmap  = QPixmap(IQ_PLOT_WIDTH, IQ_PLOT_HEIGHT);
    phasePixmap = QPixmap(IQ_PLOT_WIDTH, IQ_PLOT_HEIGHT);
    clearPlots();

    // set default plot scales
    // gain: 0.9:1.1
    gainOffset = 1.0;
    gainScale  = (double) IQ_PLOT_HEIGHT / 0.2;

    // phase: -5:5 degrees
    phaseOffset = 0.0;
    phaseScale  = (double) IQ_PLOT_HEIGHT / 10.0;
}

IQBalance::~IQBalance()
{
}


/*! when restart button is clicked. Ask user to be
   sure they want to delete saved data
 */
void IQBalance::restartIQ()
{
    QMessageBox msgBox;
    msgBox.setText("This will delete all saved data. Proceed?");
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    switch (ret) {
    case QMessageBox::Cancel:
        break;
    case QMessageBox::Ok:
        emit(restart());
        break;
    default:
        break;
    }
}

/*!
   this is so the widget emits a signal when it is
   closed. The signal is connect to spectrum and turns
   off the point plotting
 */
void IQBalance::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    emit(closed(false));
}

void IQBalance::setGainScale(double min, double max)
{
    gainOffset = min + (max - min) * 0.5;
    gainScale  = (double) IQ_PLOT_HEIGHT / (max - min);
}

void IQBalance::setPhaseScale(double min, double max)
{
    phaseOffset = min + (max - min) * 0.5;
    phaseScale  = (double) IQ_PLOT_HEIGHT / (max - min);
}

void IQBalance::clearPlots()
{
    QPainter p(&gainPixmap);
    p.fillRect(gainPixmap.rect(), Qt::white);
    QPainter p2(&phasePixmap);
    p2.fillRect(phasePixmap.rect(), Qt::white);

    // using real rotated text might look better here. This is
    // quick/dirty way
    gainPlotLabel->clear();
    gainPlotLabel->setText("g\na\ni\nn");
    phasePlotLabel->clear();
    phasePlotLabel->setText("p\nh\na\ns\ne");

    gainPlot->setPixmap(gainPixmap);
    gainPlot->update();
    phasePlot->setPixmap(phasePixmap);
    phasePlot->update();
}

/*! plot a point on gain plot
 */
void IQBalance::plotGainPoint(int bin, double y)
{
    int      py = IQ_PLOT_HEIGHT / 2 - qRound((y - gainOffset) * gainScale);

    QPainter p(&gainPixmap);
    p.setPen(Qt::black);
    p.setBrush(Qt::SolidPattern);
    p.drawEllipse(bin, py, 2, 2);

    gainPlot->setPixmap(gainPixmap);
    gainPlot->update();
}

/*! plot function from coefficients of fit
 */
void IQBalance::plotGainFunc(double a0, double a1, double a2, double a3)
{
    QPainter p(&gainPixmap);
    p.setPen(Qt::red);
    p.setBrush(Qt::SolidPattern);

    // i is the frequency
    for (int i = -IQ_PLOT_WIDTH / 2; i < IQ_PLOT_WIDTH / 2; i++) {
        int    px   = i + IQ_PLOT_WIDTH / 2; // px is x index on plot
        double gain = a0;
        double x    = (double) i;
        double x0   = x;
        gain += a1 * x;
        x    *= x0;
        gain += a2 * x;
        x    *= x0;
        gain += a3 * x;
        int py = IQ_PLOT_HEIGHT / 2 - qRound((gain - gainOffset) * gainScale);
        p.drawPoint(px, py);
    }
    gainPlot->setPixmap(gainPixmap);
    gainPlot->update();
}
/*! plot a point on phase plot
   converts from FFT index to +/- freq, zero freq in center of plot
 */
void IQBalance::plotPhasePoint(int bin, double y)
{
    int      py = IQ_PLOT_HEIGHT / 2 - qRound((y - phaseOffset) * phaseScale);

    QPainter p(&phasePixmap);
    p.setPen(Qt::black);
    p.setBrush(Qt::SolidPattern);
    p.drawEllipse(bin, py, 2, 2);

    phasePlot->setPixmap(phasePixmap);
    phasePlot->update();
}

/*! plot function from coefficients of fit
 */
void IQBalance::plotPhaseFunc(double a0, double a1, double a2, double a3)
{
    QPainter p(&phasePixmap);
    p.setPen(Qt::red);
    p.setBrush(Qt::SolidPattern);

    // i is the frequency
    for (int i = -IQ_PLOT_WIDTH / 2; i < IQ_PLOT_WIDTH / 2; i++) {
        int    px    = i + IQ_PLOT_WIDTH / 2; // px is x index on plot
        double phase = a0;
        double x     = (double) i;
        double x0    = x;
        phase += a1 * x;
        x     *= x0;
        phase += a2 * x;
        x     *= x0;
        phase += a3 * x;
        int py = IQ_PLOT_HEIGHT / 2 - qRound((phase - phaseOffset) * phaseScale);
        p.drawPoint(px, py);
    }
    phasePlot->setPixmap(phasePixmap);
    phasePlot->update();
}
