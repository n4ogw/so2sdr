/*! Copyright 2010-2022 R. Torsten Clay N4OGW

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
#include <QColor>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QSettings>
#include <QTimer>
#include <QtMath>
#include <QHostAddress>
#include <QUdpSocket>
#include <QX11Info>
#include <QXmlStreamWriter>
#include "so2sdr-bandmap.h"
#include "bandmap-tcp.h"
#include "utils.h"
#include "afedri.h"
#include "network.h"
#include "audioreader_portaudio.h"

/*! returns true if initialization was successful
 */
bool So2sdrBandmap::so2sdrBandmapOk() const
{
    return(initialized);
}
So2sdrBandmap::So2sdrBandmap(QStringList args, QWidget *parent) : QMainWindow(parent)
{
    setupUi(this);
    initPointers();
    initVariables();
    setUiSize();

    // check to see if user directory exists
    initialized = checkUserDirectory();
    settingsFile = userDirectory()+"/so2sdr-bandmap.ini";

    // check for optional command argument giving station config file name
    if (args.size() > 1) {
        settingsFile = args[1].trimmed();
        // Qt doesn't understand that ~/... implies home directory...
        if (settingsFile.left(1)=="~") {
            if (settingsFile.left(2)=="~/") {
                settingsFile=QDir::homePath()+settingsFile.right(settingsFile.size()-1);
            } else {
                // for cases like ~name : no easy way to parse, give up
                QMessageBox msgBox;
                msgBox.setText("Please use the complete path to the settings file.");
                msgBox.setInformativeText(settingsFile);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setDefaultButton(QMessageBox::Ok);
                msgBox.exec();
                close();
            }
        }
    }
    QFileInfo fi(settingsFile);
    if (!fi.exists()) {
        QMessageBox msgBox;
        msgBox.setText("The settings file "+settingsFile+" does not exist.");
        msgBox.setInformativeText("Do you want to create it?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        if (msgBox.exec()==QMessageBox::Cancel) {
            close();
        }
        firstTime=true;
    }
    settings = new  QSettings(settingsFile,QSettings::IniFormat,this);
    if (settings->status()!=QSettings::NoError) {
        errorBox.showMessage("ERROR: problem starting qsettings");
    }
    // if run the first time with default settings file for second radio,
    // set second radio
    if (firstTime && settingsFile.right(19)=="so2sdr-bandmap2.ini") {
        settings->setValue(s_sdr_nrig,1);
    }
    // restore window size and position
    QString tmp="BandmapWindow";
    settings->beginGroup(tmp);
    resize(settings->value("size", QSize(400, 594)).toSize());
    move(settings->value("pos", QPoint(200, 200)).toPoint());
    settings->endGroup();

    directory.setCurrent(dataDirectory());

    if (settings->value(s_sdr_reverse_scroll,s_sdr_reverse_scroll_def).toBool()) {
        horizontalLayout->removeWidget(CallLabel);
        horizontalLayout->removeWidget(FreqLabel);
        horizontalLayout->removeWidget(display);
        horizontalLayout->insertWidget(0,CallLabel);
        horizontalLayout->insertWidget(1,FreqLabel);
        horizontalLayout->insertWidget(2,display);
    }

    freqPixmap      = QPixmap(FreqLabel->width(), settings->value(s_sdr_fft,s_sdr_fft_def).toInt());
    callPixmap      = QPixmap(CallLabel->width(), settings->value(s_sdr_fft,s_sdr_fft_def).toInt());

    ipAddress= QHostAddress(QHostAddress::LocalHost).toString();
    if (!server.listen(QHostAddress::LocalHost,
                        settings->value(s_sdr_bandmap_tcp_port,s_sdr_bandmap_tcp_port_def).toInt())) {
        qDebug("couldn't start tcp server");
    }
    connect(&server, SIGNAL(newConnection()), this, SLOT(startConnection()));

    setFocusPolicy(Qt::StrongFocus);
    setFocusPolicy(Qt::NoFocus);
    display->setFocusPolicy(Qt::NoFocus);
    CallLabel->setFocusPolicy(Qt::NoFocus);
    FreqLabel->setFocusPolicy(Qt::NoFocus);

    checkBoxMark.setText("Mark");
    checkBoxMark.setToolTip("Enables signal detection.");
    toolBar->setMovable(false);
    QWidget* spacer1 = new QWidget();
    spacer1->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    toolBar->addWidget(spacer1);
    toolBar->addWidget(&checkBoxMark);
    txLabel.clear();
    txLabel.setText("<font color=#000000>TX");
    toolBar->addWidget(&txLabel);
    slider.setToolTip("Gain for signal detection. To the right is LESS sensitive.");
    slider.setOrientation(Qt::Horizontal);
    connect(&slider,SIGNAL(valueChanged(int)),this,SLOT(updateLevel(int)));
    slider.setFixedWidth(60);
    slider.setMaximum(200);
    slider.setMinimum(0);
    slider.setSingleStep(10);
    slider.setPageStep(50);
    slider.setValue(settings->value(s_sdr_level,s_sdr_level_def).toInt());
    toolBar->addWidget(&slider);
    QWidget* spacer2 = new QWidget();
    spacer2->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    toolBar->addWidget(spacer2);
    toolBar->addAction("&Help",this,SLOT(showHelp()));

    iqDialog  = new IQBalance(this);
    iqDialog->clearPlots();
    showToolBar = new QAction("&toolbar",this);
    scaleX1   = new QAction("Zoom x&1", this);
    scaleX2   = new QAction("Zoom x&2", this);
    deleteAct = new QAction("&Delete Call", this);
    checkBoxMark.setChecked(settings->value(s_sdr_peakdetect,s_sdr_peakdetect_def).toBool());

    iqShowData = new QAction("IQ Balance", this);
    connect(iqShowData, SIGNAL(triggered()), this, SLOT(showIQData()));
    connect(&checkBoxMark, SIGNAL(clicked()), this, SLOT(emitParams()));
    connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteCallMouse()));
    showToolBar->setCheckable(true);
    showToolBar->setChecked(true);
    connect(showToolBar,SIGNAL(triggered(bool)),this,SLOT(setShowToolbar(bool)));
    scaleX1->setCheckable(true);
    scaleX2->setCheckable(true);
    scaleX2->setChecked(false);
    scaleX1->setChecked(true);
    connect(scaleX1, SIGNAL(triggered()), this, SLOT(setScaleX1()));
    connect(scaleX2, SIGNAL(triggered()), this, SLOT(setScaleX2()));
    connect(actionRun,SIGNAL(triggered()),this,SLOT(start()));

    sdrSetup = new SDRDialog(*settings,uiSizes,this);
    connect(actionSetup,SIGNAL(triggered()),sdrSetup,SLOT(show()));
    connect(actionSetup,SIGNAL(triggered()),this,SLOT(disconnectSignals()));
    connect(sdrSetup,SIGNAL(setupErrors(QString)),&errorBox,SLOT(showMessage(QString)));
    connect(sdrSetup,SIGNAL(update()),this,SLOT(setSdrType()));
    connect(sdrSetup,SIGNAL(restartSdr()),this,SLOT(restartSdr()));
    connect(display, SIGNAL(displayMouseQSY(int)), this, SLOT(mouseQSYDelta(int)));
    toolBarHeight = toolBar->height();

    // select type of SDR, create data source sdrSource
    spectrumProcessor = new Spectrum(this,*settings,userDirectory());
    switch (static_cast<SdrType>(settings->value(s_sdr_type,s_sdr_type_def).toInt())) {
    case soundcard_t:
        sdrSource = new AudioReaderPortAudio(settingsFile);
        break;
    case afedri_t:
        sdrSource = new Afedri(settingsFile);
        break;
    case network_t:
        sdrSource = new NetworkSDR(settingsFile);
        break;
    }
    setSdrType();
    sdrSource->moveToThread(&sdrThread);
    connect(actionSetup,SIGNAL(triggered()),sdrSource,SLOT(stop()),Qt::DirectConnection);
    connect(&sdrThread,SIGNAL(started()),sdrSource,SLOT(initialize()));
    connect(sdrSource,SIGNAL(stopped()),&sdrThread,SLOT(quit()));
    connect(sdrSource,SIGNAL(stopped()),this,SLOT(disconnectSignals()));
    connect(sdrSource,SIGNAL(error(QString)),&errorBox,SLOT(showMessage(QString)));

    connect(spectrumProcessor, SIGNAL(spectrumReady(unsigned char*, unsigned char)), display,
            SLOT(plotSpectrum(unsigned char*, unsigned char)));
    connect(sdrSource, SIGNAL(ready(unsigned char *, unsigned char)),spectrumProcessor,
            SLOT(processData(unsigned char *, unsigned char)),Qt::QueuedConnection);
    connect(iqDialog, SIGNAL(closed(bool)), spectrumProcessor, SLOT(setPlotPoints(bool)));
    connect(iqDialog, SIGNAL(restart()), spectrumProcessor, SLOT(clearIQ()));
    connect(spectrumProcessor, SIGNAL(qsy(double)), this, SLOT(findQsy(double)));
    connect(spectrumProcessor, SIGNAL(clearPlot()), iqDialog, SLOT(clearPlots()));
    connect(spectrumProcessor, SIGNAL(gainPoint(int, double)), iqDialog, SLOT(plotGainPoint(int, double)));
    connect(spectrumProcessor, SIGNAL(phasePoint(int, double)), iqDialog, SLOT(plotPhasePoint(int, double)));
    connect(spectrumProcessor, SIGNAL(gainScale(double, double)), iqDialog, SLOT(setGainScale(double, double)));
    connect(spectrumProcessor, SIGNAL(phaseScale(double, double)), iqDialog, SLOT(setPhaseScale(double, double)));
    connect(spectrumProcessor, SIGNAL(plotGainFunc(double, double, double, double)), iqDialog,
           SLOT(plotGainFunc(double, double, double, double)));
    connect(spectrumProcessor, SIGNAL(plotPhaseFunc(double, double, double, double)), iqDialog,
           SLOT(plotPhaseFunc(double, double, double, double)));
    tuningTimer.setSingleShot(true);
    connect(&tuningTimer,SIGNAL(timeout()),this,SLOT(resetTuningTimer()));

    // vfoPos is the position of the red line indicating center
    vfoPos          = (height()-toolBarHeight)/ 2;
    dragPos         = vfoPos;
    display->setVfoPos(vfoPos);

    makeFreqScaleAbsolute();
    FreqLabel->setPixmap(freqPixmap);
    FreqLabel->update();

    startTimers();
    show();
}


So2sdrBandmap::~So2sdrBandmap()
{
    delete sdrSource;
    delete spectrumProcessor;
    delete deleteAct;
    iqDialog->close();
    delete iqDialog;
    delete iqShowData;
    delete sdrSetup;
    delete scaleX1;
    delete scaleX2;
    delete showToolBar;
    delete settings;
}

void So2sdrBandmap::disconnectSignals()
{
    disconnect(actionStop,SIGNAL(triggered()),sdrSource,SLOT(stop()));
}

void So2sdrBandmap::setShowToolbar(bool b)
{
    toolBar->setVisible(b);
    if (b) {
        toolBarHeight=toolBar->height();
    } else {
        toolBarHeight=0;
    }
    int dy = (height() - toolBarHeight) / 2 - vfoPos;
    int scale= settings->value(s_sdr_scale,s_sdr_scale_def).toInt();
    freqMin = centerFreq - (settings->value(s_sdr_fft,s_sdr_fft_def).toDouble() / 2 + dy) *
            settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toDouble()/(scale * settings->value(s_sdr_fft,s_sdr_fft_def).toDouble());
    freqMax = freqMin + (settings->value(s_sdr_fft,s_sdr_fft_def).toDouble() * settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toDouble()/
                         (scale * settings->value(s_sdr_fft,s_sdr_fft_def).toDouble()));
    display->setVfoPos(vfoPos);
    makeFreqScaleAbsolute();
    FreqLabel->setPixmap(freqPixmap);
    FreqLabel->update();
}

/*!
 * \brief So2sdrBandmap::updateLevel
 *  update level when slider position changes
 */
void So2sdrBandmap::updateLevel(int level)
{
    settings->setValue(s_sdr_level,level);
}

/*!
 * \brief So2sdrBandmap::restart
 *   called when SdrType has changed. Stop the current sdr, initialize and
 *   start the new one.
 */
void So2sdrBandmap::restartSdr()
{
    sdrSource->stop();
    if (sdrThread.isRunning()) {
        sdrThread.quit();
        sdrThread.wait();
    }
    spectrumProcessor->stopSpectrum();
    delete sdrSource;

    // start new one
    switch (static_cast<SdrType>(settings->value(s_sdr_type,s_sdr_type_def).toInt())) {
    case soundcard_t:
        sdrSource = new AudioReaderPortAudio(settingsFile);
        break;
    case afedri_t:
        sdrSource = new Afedri(settingsFile);
        break;
    case network_t:
        sdrSource = new NetworkSDR(settingsFile);
        break;
    }
    setSdrType();
    sdrSource->moveToThread(&sdrThread);
    connect(actionSetup,SIGNAL(triggered()),sdrSource,SLOT(stop()),Qt::DirectConnection);
    connect(&sdrThread,SIGNAL(started()),sdrSource,SLOT(initialize()));
    connect(sdrSource,SIGNAL(stopped()),&sdrThread,SLOT(quit()));
    connect(sdrSource,SIGNAL(stopped()),this,SLOT(disconnectSignals()));
    connect(sdrSource,SIGNAL(error(QString)),&errorBox,SLOT(showMessage(QString)));
    connect(sdrSource, SIGNAL(ready(unsigned char *, unsigned char)),spectrumProcessor,
            SLOT(processData(unsigned char *, unsigned char)));
}

/*!
 * \brief So2sdrBandmap::setSdrType
 *  this gets triggered when the type of SDR is selected in SDRDialog, or other options change
 */
void So2sdrBandmap::setSdrType()
{
    unsigned int speed=1;
    switch (static_cast<SdrType>(settings->value(s_sdr_type,s_sdr_type_def).toInt())) {
    case soundcard_t:
        iqShowData->setEnabled(true);
        settings->setValue(s_sdr_sample_freq,settings->value(s_sdr_sound_sample_freq,s_sdr_sound_sample_freq_def).toInt());
        speed=settings->value(s_sdr_sound_speed,s_sdr_sound_speed_def).toUInt();
        break;
    case afedri_t:
        iqShowData->setEnabled(false);
        settings->setValue(s_sdr_sample_freq,settings->value(s_sdr_afedri_sample_freq,s_sdr_afedri_sample_freq_def).toInt());
        settings->setValue(s_sdr_tcp_ip,settings->value(s_sdr_afedri_tcp_ip,s_sdr_afedri_tcp_ip_def).toString());
        settings->setValue(s_sdr_tcp_port,settings->value(s_sdr_afedri_tcp_port,s_sdr_afedri_tcp_port_def).toInt());
        settings->setValue(s_sdr_udp_port,settings->value(s_sdr_afedri_udp_port,s_sdr_afedri_udp_port_def).toInt());
        speed=settings->value(s_sdr_afedri_speed,s_sdr_afedri_speed_def).toUInt();
        break;
    case network_t:
        iqShowData->setEnabled(false);
        settings->setValue(s_sdr_sample_freq,settings->value(s_sdr_net_sample_freq,s_sdr_net_sample_freq_def).toInt());
        settings->setValue(s_sdr_tcp_ip,settings->value(s_sdr_net_tcp_ip,s_sdr_net_tcp_ip_def).toString());
        settings->setValue(s_sdr_tcp_port,settings->value(s_sdr_net_tcp_port,s_sdr_net_tcp_port_def).toInt());
        settings->setValue(s_sdr_udp_port,settings->value(s_sdr_net_udp_port,s_sdr_net_udp_port_def).toInt());
        speed=settings->value(s_sdr_net_speed,s_sdr_net_speed_def).toUInt();
        break;
    }
    settings->setValue(s_sdr_offset,sdrSetup->offset(getBand(centerFreq)));
    settings->setValue(s_sdr_swapiq,sdrSetup->invert(getBand(centerFreq)));
    settings->setValue(s_sdr_speed,speed);
    unsigned int fft,period;
    switch (settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toInt()) {
    case 48000:
        fft=2048*speed;
        break;
    case 96000:
    case 100000: // for Afedri net dual
        fft=4096*speed;
        break;
    case 192000:
    case 200000: // for Afedri net dual
        fft=8192*speed;
        break;
    default:
        fft=4096;
    }
    settings->setValue(s_sdr_fft,fft);
    period=fft/4;
    switch (settings->value(s_sdr_bits,s_sdr_bits_def).toInt()) {
    case 0: // 16 bit
        sizes.chunk_size   = settings->value(s_sdr_fft,s_sdr_fft_def).toUInt() * 2 * 2;
        sizes.advance_size = period * 2 * 2;
        break;
    case 1: // 24 bit
        sizes.chunk_size   = settings->value(s_sdr_fft,s_sdr_fft_def).toUInt() * 2 * 3;
        sizes.advance_size = period * 2 * 3;
        break;
    case 2: // 32 bit
        sizes.chunk_size   = settings->value(s_sdr_fft,s_sdr_fft_def).toUInt() * 2 * 4;
        sizes.advance_size = period * 2 * 4;
        break;
    }
    spectrumProcessor->setFFTSize(sizes);
    spectrumProcessor->updateParams();
    sdrSource->setSampleSizes(sizes);
    bandMapName="So2sdrBandmap"+QByteArray::number(settings->value(s_sdr_nrig,s_sdr_nrig_def).toInt()+1);
    display->initialize(settings);
    vfoPos          = (height()-toolBarHeight)/ 2;
    dragPos         = vfoPos;
    display->setVfoPos(vfoPos);

    switch (settings->value(s_sdr_scale,s_sdr_scale_def).toInt()) {
    case 1:
        setScaleX1();
        break;
    case 2:
        setScaleX2();
        break;
    }
    horizontalLayout->removeWidget(CallLabel);
    horizontalLayout->removeWidget(FreqLabel);
    horizontalLayout->removeWidget(display);
    if (settings->value(s_sdr_reverse_scroll,s_sdr_reverse_scroll_def).toBool()) {
        horizontalLayout->insertWidget(0,CallLabel);
        horizontalLayout->insertWidget(1,FreqLabel);
        horizontalLayout->insertWidget(2,display);
    } else {
        horizontalLayout->insertWidget(0,display);
        horizontalLayout->insertWidget(1,FreqLabel);
        horizontalLayout->insertWidget(2,CallLabel);
    }
    freqPixmap      = QPixmap(FreqLabel->width(), settings->value(s_sdr_fft,s_sdr_fft_def).toInt());
    callPixmap      = QPixmap(CallLabel->width(), settings->value(s_sdr_fft,s_sdr_fft_def).toInt());
    makeFreqScaleAbsolute();
    FreqLabel->setPixmap(freqPixmap);
    FreqLabel->update();
    if (settings->value(s_sdr_n1mm,s_sdr_n1mm_def).toBool()) {
        socketUdpN1MM.close();
        socketUdpN1MM.bind(settings->value(s_sdr_n1mm_port,s_sdr_n1mm_port_def).toInt(), QUdpSocket::ShareAddress);
        connect(&socketUdpN1MM,SIGNAL(readyRead()),this,SLOT(udpRead()));
    } else {
        if (socketUdpN1MM.isOpen()) {
            socketUdpN1MM.close();
        }
    }
    settings->sync();
}

/*! called from So2sdr periodically to update IQ error fit
 */
void So2sdrBandmap::calc()
{
    spectrumProcessor->setCalcError();
}

/*! reimplemented close event. Closes IQ plot if it is
   open
 */
void So2sdrBandmap::closeEvent(QCloseEvent *event)
{
    stopTimers();
    closeIQ();

    // save geometry
    QString tmp="BandmapWindow";
    settings->beginGroup(tmp);
    settings->setValue("size", size());
    settings->setValue("pos", pos());
    settings->endGroup();
    settings->sync();

    event->accept();
}

/*! close IQ dialog
 */
void So2sdrBandmap::closeIQ()
{
    iqDialog->hide();
}

/*!
   Delete call via mouse
 */
void So2sdrBandmap::deleteCallMouse()
{

    double f = centerFreq + (settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toDouble()
                                   / settings->value(s_sdr_fft,s_sdr_fft_def).toDouble() /  settings->value(s_sdr_scale,s_sdr_scale_def).toDouble()
                                   * (vfoPos - mouse_y+toolBarHeight));
    QByteArray call="";
    for (int i=0;i<callList.size();i++) {
        if ((callList.at(i).freq-f)<SIG_MIN_SPOT_DIFF) {
            call=callList.at(i).call;
            break;
        }
    }
    if (!call.isEmpty()) {
        writeUdpXML(f,call,true);
    }
}

void So2sdrBandmap::emitParams()
{
    settings->setValue(s_sdr_peakdetect,checkBoxMark.isChecked());
    display->setMark(settings->value(s_sdr_peakdetect,s_sdr_peakdetect_def).toBool());
    spectrumProcessor->updateParams();
}


/*! returns current invert setting
 */
bool So2sdrBandmap::invert() const
{
    return(_invert);
}

/*!
   make pixmap with bandmap calls
 */
void So2sdrBandmap::makeCall()
{
    QPainter p(&callPixmap);
    p.fillRect(callPixmap.rect(), Qt::lightGray);

    p.setPen(Qt::black);
    QFont font;
    font.setPointSize(BANDMAP_FONT_POINT_SIZE);
    p.setFont(font);
    for (int i = 0; i < settings->value(s_sdr_fft,s_sdr_fft_def).toInt(); i++) {
        display->cmap[i] = false;
    }

    // draw callsigns
    // cmap[i] is true if the call near i is a dupe. Used to color signal on bandmap
    int scale= settings->value(s_sdr_scale,s_sdr_scale_def).toInt();
    display->setMark(true);
    double fm = centerFreq - settings->value(s_sdr_fft,s_sdr_fft_def).toInt() / 2 * settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toInt()/(scale * settings->value(s_sdr_fft,s_sdr_fft_def).toInt());
    for (int i=0;i<callList.size();i++) {
        if (callList.at(i).freq < fm || callList.at(i).freq > freqMax) continue;
        int pix_offset = qRound((callList.at(i).freq - fm) * pix_per_hz);
        int y          = settings->value(s_sdr_fft,s_sdr_fft_def).toInt() - pix_offset;

        int l1 = qRound(y - uiSizes.width / 2);
        if (l1 < 0) l1 = 0;
        int l2 = y + qRound(uiSizes.width / 2);
        if (l2 >= settings->value(s_sdr_fft,s_sdr_fft_def).toInt()) l2 = settings->value(s_sdr_fft,s_sdr_fft_def).toInt() - 1;

        if (callList.at(i).mark) {
            // make line wider when scale is *2
            int l1 = y - 1 - 2 * scale;
            if (l1 < 0) l1 = 0;
            int l2 = y + 2 + 2 * scale;
            if (l2 >= settings->value(s_sdr_fft,s_sdr_fft_def).toInt()) l2 = settings->value(s_sdr_fft,s_sdr_fft_def).toInt() - 1;
            for (int j = l1; j < l2; j++) {
                display->cmap[j] = true;
                display->markRgb0[j]=callList.at(i).markRgb[0];
                display->markRgb1[j]=callList.at(i).markRgb[1];
                display->markRgb2[j]=callList.at(i).markRgb[2];
            }
            p.setPen(QColor(callList.at(i).rgbCall[0],callList.at(i).rgbCall[1],callList.at(i).rgbCall[2]));
        } else {
            p.setPen(Qt::black);
        }
        pix_offset = qRound((callList.at(i).freq - freqMin) * pix_per_hz);
        y          = settings->value(s_sdr_fft,s_sdr_fft_def).toInt() - pix_offset;
        // uiSizes.height/3 is fudge factor to center callsign
        if (settings->value(s_sdr_reverse_scroll,s_sdr_reverse_scroll_def).toBool()) {
            QFontMetrics fm(font);
            p.drawText(callPixmap.width() - qRound(uiSizes.height/3 - uiSizes.width*1.5) - fm.horizontalAdvance(callList.at(i).call),
                       y+ qRound(uiSizes.width / 3), callList.at(i).call);
        } else {
            p.drawText(qRound(uiSizes.width*1.5), y+ qRound(uiSizes.height/3), callList.at(i).call);
        }
    }
    p.setPen(Qt::black);

    // draw symbol for each signal
    if (settings->value(s_sdr_peakdetect,s_sdr_peakdetect_def).toBool()) {
        p.setBrush(Qt::SolidPattern);
        Signal *sigs = &spectrumProcessor->sigList[0];
        for (int i = 0; i < SIG_MAX; i++) {
            if (sigs[i].active) {
                if (sigs[i].f<freqMin || sigs[i].f>freqMax) continue;
                int pix_offset = qRound((sigs[i].f - freqMin) * pix_per_hz + uiSizes.rad / 2);
                int y          = settings->value(s_sdr_fft,s_sdr_fft_def).toInt() - pix_offset;
                if (y < 0 || y >= settings->value(s_sdr_fft,s_sdr_fft_def).toInt()) {
                    continue;
                }
                if (settings->value(s_sdr_reverse_scroll,s_sdr_reverse_scroll_def).toBool()) {
                    p.drawEllipse(callPixmap.width() - qRound(uiSizes.rad - uiSizes.width/2), y, qRound(uiSizes.rad), qRound(uiSizes.rad));
                } else {
                    p.drawEllipse(qRound(uiSizes.width/2), y, qRound(uiSizes.rad), qRound(uiSizes.rad));
                }
            }
        }
    }
}

/*!
   draw frequency scale pixmap with actual KHz
 */
void So2sdrBandmap::makeFreqScaleAbsolute()
{
    int fft=settings->value(s_sdr_fft,s_sdr_fft_def).toInt();
    int dy = (height() - toolBarHeight) / 2 - vfoPos;
    int scale= settings->value(s_sdr_scale,s_sdr_scale_def).toInt();
    freqMin = centerFreq - (fft / 2 + dy) * settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toInt()/(scale * fft);
    int      bottom_start      = (qFloor(freqMin / 1000) + 1) * 1000;
    int      bottom_pix_offset = qFloor((bottom_start - freqMin) * fft * scale) /
            settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toInt();
    int      j                 = (bottom_start / 1000) % 1000;
    int      i, i0 = fft - bottom_pix_offset;
    freqMax = freqMin + (fft * settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toInt()/(scale * fft));
    QPainter p(&freqPixmap);
    p.fillRect(freqPixmap.rect(), Qt::lightGray);
    p.setPen(Qt::black);
    QFont   font;
    font.setPointSize(BANDMAP_FONT_POINT_SIZE);
    QString tmp;
    i = i0;
    int     k = 1;
    while (i > 0) {
        if (settings->value(s_sdr_reverse_scroll,s_sdr_reverse_scroll_def).toBool()) {
            p.drawLine(freqPixmap.width() - qRound(uiSizes.width*0.5), i, freqPixmap.width()-1, i);
        } else {
            p.drawLine(0, i, qRound(uiSizes.width*0.5), i);
        }
        // if center freq never set, don't make negative freq labels
        int jj;
        if (qAbs(centerFreq)<1) jj=abs(j);
        else jj=j;
        if (jj < 10) {
            tmp = "00" + QString::number(jj);
        } else if (jj < 100) {
            tmp = "0" + QString::number(jj);
        } else {
            tmp = QString::number(jj);
        }
        p.drawText(qRound(uiSizes.width), i + qRound(uiSizes.height*0.4), tmp);
        j++;
        j = j % 1000;
        i = i0 - qRound(k * pix_per_khz) - 2;
        k++;
    }

    // draw red line at center freq
    p.setPen(qRgb(255, 0, 0));

    int y = fft / 2 - ((height()  - toolBarHeight) / 2 - vfoPos);
    p.drawLine(0, y + 1, width(), y + 1);
    p.drawLine(0, y, width(), y);
    p.drawLine(0, y - 1, width(), y - 1);
}

/*! event for mouse moved while left button is pressed

   detect when frequency scale is dragged
 */
void So2sdrBandmap::mouseMoveEvent(QMouseEvent *event)
{
    if (FreqLabel->underMouse()) {
        // constant here (50) affects sensitivity of dragging freq scale
        if (abs(event->pos().y() - dragPos) < 50) {
            int tmp = vfoPos + (event->pos().y() - dragPos);
            if (tmp > 0 && tmp < (height() - 25 - toolBarHeight)) {
                vfoPos          = tmp;
                display->setVfoPos(vfoPos);
                makeFreqScaleAbsolute();
                FreqLabel->setPixmap(freqPixmap);
                FreqLabel->update();
            }
        }
        dragPos = event->pos().y();
    }
}

/*!
   Mouse event handler: just for mouse clicks in CallLabel field
   left-clicks : qsy to frequency or label freq
   right-clicks: bring up menu
 */
void So2sdrBandmap::mousePressEvent(QMouseEvent *event)
{
    if (CallLabel->underMouse()) {
        mouse_y = event->y();
        if (event->button() == Qt::RightButton) {
            // right-mouse button; brings up menu
            QMenu menu(this);
            menu.addAction(showToolBar);
            menu.addSeparator();
            menu.addAction(scaleX1);
            menu.addAction(scaleX2);
            menu.addSeparator();
            menu.addAction(deleteAct);
            menu.addSeparator();
            menu.addAction(iqShowData);
            menu.exec(event->globalPos());
        } else if (event->button() == Qt::LeftButton && (event->x() < (FreqLabel->width() + display->width() +
                                                                       SIG_SYMBOL_X + 4 * SIG_SYMBOL_RAD))) {
            // left-mouse: qsy to signal
            qsyToNearest();
        }
    }
}

/*!
   returns next higher or lower detected signal

   higher if higher=true, otherwise lower
 */
double So2sdrBandmap::nextFreq(bool higher) const
{
    if (!settings->value(s_sdr_peakdetect,s_sdr_peakdetect_def).toBool()) {
        return(0);
    }
    double   f  = centerFreq;
    double   df = settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toDouble();

    Signal *s = &spectrumProcessor->sigList[0];
    if (higher) {
        for (int i = 0; i < SIG_MAX; i++) {
            if (s[i].active && s[i].f > centerFreq) {
                double d = s[i].f - centerFreq;
                if (d < df && d > SIG_MIN_FREQ_DIFF) {
                    f  = s[i].f;
                    df = d;
                }
            }
        }
    } else {
        for (int i = 0; i < SIG_MAX; i++) {
            if (s[i].active && s[i].f < centerFreq) {
                double d = centerFreq - s[i].f;
                if (d < df && d > SIG_MIN_FREQ_DIFF) {
                    f  = s[i].f;
                    df = d;
                }
            }
        }
    }
    if (f > band_limits[band][0] && f < band_limits[band][1]) {
        return(f);
    } else {
        return(centerFreq);
    }
}

/*!
 * \brief So2sdrBandmap::qsyToNearest
   Triggered when mouse is clicked on callsign field. If click is near a black dot, issue
   a UDP packet for qsy to this frequency.
*/
void So2sdrBandmap::qsyToNearest()
{
    double    f = centerFreq +  (settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toDouble()
                                 /  settings->value(s_sdr_fft,s_sdr_fft_def).toDouble() /  settings->value(s_sdr_scale,s_sdr_scale_def).toInt()
                                 * (vfoPos - mouse_y+toolBarHeight));
    Signal *s = &spectrumProcessor->sigList[0];
    for (int i = 0; i < SIG_MAX; i++) {
        // find signal within 100 hz of click freq
        if (s[i].active && qAbs(s[i].f - f) < 100) {
            writeUdpXML(s[i].f,"",false);
            break;
        }
    }
}

/*!
 * \brief So2sdrBandmap::mouseQSYDelta
 *    Triggered when mouse is clicked on the bandmap itself.
 * \param delta frequency change in Hz
 */
void So2sdrBandmap::mouseQSYDelta(int delta)
{
    writeUdpXML(centerFreq+delta,"",false);
}

void So2sdrBandmap::resizeEvent(QResizeEvent * event)
{
    Q_UNUSED(event)
    if (vfoPos > (height() -   toolBarHeight)) {
        vfoPos          = height() -  toolBarHeight;
        display->setVfoPos(vfoPos);
    }
    makeFreqScaleAbsolute();
    FreqLabel->setPixmap(freqPixmap);
    FreqLabel->update();
}


/*!
   Invert the spectrum
 */
void So2sdrBandmap::setInvert(bool t)
{
    display->setInvert(t);
    spectrumProcessor->setInvert(t);
    _invert = t;
}

/*! show the IQ dialog
 */
void So2sdrBandmap::showIQData()
{
    iqDialog->show();
    iqDialog->clearPlots();
    spectrumProcessor->setPlotPoints(true);
    // force recalculation of IQ fit
    spectrumProcessor->calcError(true);
}

/*! set default vfo position in center of display */
void So2sdrBandmap::setDefaultCenter()
{
    vfoPos = (height() - toolBarHeight) / 2;
    display->setVfoPos(vfoPos);
}

/*! set x1 scale
 */
void So2sdrBandmap::setScaleX1()
{
    settings->setValue(s_sdr_scale,1);
    display->setScale(1);
    scaleX1->setChecked(true);
    scaleX2->setChecked(false);
    spectrumProcessor->updateParams();
    pix_per_khz = settings->value(s_sdr_fft,s_sdr_fft_def).toDouble() / (settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toDouble()/1000);
    pix_per_hz  = settings->value(s_sdr_fft,s_sdr_fft_def).toDouble() / (settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toDouble());

    makeFreqScaleAbsolute();
    FreqLabel->setPixmap(freqPixmap);
    FreqLabel->update();
}

/*! set x2 scale
 */
void So2sdrBandmap::setScaleX2()
{
    settings->setValue(s_sdr_scale,2);
    display->setScale(2);
    scaleX1->setChecked(false);
    scaleX2->setChecked(true);
    spectrumProcessor->updateParams();
    pix_per_khz = settings->value(s_sdr_fft,s_sdr_fft_def).toDouble() * 2 / (settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toDouble()/1000);
    pix_per_hz  = settings->value(s_sdr_fft,s_sdr_fft_def).toDouble() * 2 / (settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toDouble());

    makeFreqScaleAbsolute();
    FreqLabel->setPixmap(freqPixmap);
    FreqLabel->update();
}

/*!
   Start bandmap
 */
void So2sdrBandmap::start()
{
    if (sdrThread.isRunning())
    {
        return;
    }
    connect(actionStop,SIGNAL(triggered()),sdrSource,SLOT(stop()));
    sdrThread.start();
}

/*! stops bandmap
  */
void So2sdrBandmap::stop()
{
    if (sdrThread.isRunning()) {
        sdrThread.quit();
        sdrThread.wait();
    }
    spectrumProcessor->stopSpectrum();
}

/*!
  returns closest peak-detected freqency to fin

  if freq difference is larger than SIG_MIN_SPOT_DIFF, returns fin
  */
double So2sdrBandmap::closestFreq(double fin) const
{
    double f=fin;
    if (spectrumProcessor) {
        f=spectrumProcessor->closestFreq(fin);
    }
    return f;
}

void So2sdrBandmap::initPointers()
{
    sdrSetup = nullptr;
    iqDialog  = nullptr;
    deleteAct = nullptr;
    iqShowData = nullptr;
    scaleX1 = nullptr;
    scaleX2 = nullptr;
    spectrumProcessor = nullptr;
    help = nullptr;
}

void So2sdrBandmap::initVariables()
{
    callList.clear();
    cmdLen=0;
    cmd=0;
    flow=0;
    fhigh=0;
    initialized = false;
    centerFreq  = 0;
    band=getBand(centerFreq);
    bandName.clear();
    endFreqs[0] = 0;
    endFreqs[1] = 0;
    addOffset =   0;
    _invert   =   false;
    bandMapName="So2sdrBandmap1";
    firstTime=false;
}

void So2sdrBandmap::quit()
{
    stop();
    close();
}

/*! check to see if userDirectory() exists. If not, give options to
   create it.

   returns true if directory exists or was created; false if program
   should exit.
 */
bool So2sdrBandmap::checkUserDirectory()
{
    QDir dir;
    if (dir.exists(userDirectory())) return(true);

    QMessageBox msg;
    msg.setWindowTitle("Error");
    msg.setText("User data directory " + userDirectory() + " does not exist.");
    msg.setInformativeText("Create it?");
    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msg.setDefaultButton(QMessageBox::Yes);
    int ret = msg.exec();
    switch (ret) {
    case QMessageBox::Yes:

        // create directory
        if (dir.mkdir(userDirectory())) {
            return(true);
        } else {
            msg.setText("Could not create directory <" + userDirectory() + ">");
            msg.exec();
            return(false);
        }
        break;
    case QMessageBox::Cancel:
        // this will abort the program (return false)
        break;
    default:
        // never reached
        break;
    }
    return(false);
}

void So2sdrBandmap::startTimers()
{
    for (int i=0;i<N_BANDMAP_TIMERS;i++) timerId[i]=startTimer(timerSettings[i]);
}

void So2sdrBandmap::stopTimers()
{
    for (int i=0;i<N_BANDMAP_TIMERS;i++) {
        if (timerId[i]) killTimer(timerId[i]);
    }
}

void So2sdrBandmap::timerEvent(QTimerEvent *event)
{

    if (event->timerId() == timerId[0]) {
        // update frequency
        spectrumProcessor->resetAvg();
        makeCall();
        CallLabel->setPixmap(callPixmap);
        CallLabel->update();
    } else if (event->timerId() == timerId[1]) {
        // UDP beacon
        writeUdpXML(0,"",false);
    } else if (event->timerId() == timerId[2]) {
        // update IQ balance plot
        if (settings->value(s_sdr_type,s_sdr_type_def).toInt()==soundcard_t) {
            calc();
        }
    }
}

/*! read data from TCP socket. Connects to signal readyRead of socket
 */
void So2sdrBandmap::readData()
{
    // continue on as long as data is available
    while (socket->bytesAvailable()) {
        // first read command and length
        char buff[2];
        if (cmdLen==0) {
            if (socket->bytesAvailable()<2) return;
            qint64 n=socket->read(buff,2);
            if (n==2) {
                cmd=buff[0];
                cmdLen=buff[1];
            }
        }
        QByteArray data;
        data.clear();
        if (cmdLen>0) {
            if  (socket->bytesAvailable()<cmdLen) return;
            data=socket->read(cmdLen);
        }
        bool ok=false;
        double f;
        double ff;
        switch (cmd) {
        case BANDMAP_CMD_SET_FREQ: // set frequency
            f=data.toDouble(&ok);
            if  (ok && qAbs(centerFreq-f)>0) {
                centerFreq=f;
                spectrumProcessor->setTuning(true);
                tuningTimer.start(TUNING_TIMEOUT);
                int b=getBand(f);
                if (b==BAND_NONE) return;
                setBandName(b);
                endFreqs[0] = centerFreq-
                        settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toInt()/2
                        -settings->value(s_sdr_offset,s_sdr_offset_def).toInt();
                endFreqs[1] = centerFreq+
                        settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toInt()/2
                        -settings->value(s_sdr_offset,s_sdr_offset_def).toInt();

                setWindowTitle("Bandmap "+bandName+" ["+QString::number(endFreqs[0]/1000,'f',0)+"-"+QString::number(endFreqs[1]/1000,'f',0)+"]");
                if (band != b) {
                    // if band changed, clear all signals
                    spectrumProcessor->clearSigs();
                    spectrumProcessor->clearCQ();
                    settings->setValue(s_sdr_offset,sdrSetup->offset(getBand(centerFreq)));
                    settings->setValue(s_sdr_swapiq,sdrSetup->invert(getBand(centerFreq)));
                    spectrumProcessor->updateParams();
                    band=b;
                }
                spectrumProcessor->setFreq(centerFreq, endFreqs[0], endFreqs[1]);
                spectrumProcessor->resetAvg();
                makeFreqScaleAbsolute();
                FreqLabel->setPixmap(freqPixmap);
                FreqLabel->update();
            }
            break;
        case BANDMAP_CMD_SET_LOWER_FREQ: // set freq finder lower limit
            ff=data.toDouble(&ok);
            if (ok) {
                flow=ff;
            }
            break;
        case BANDMAP_CMD_SET_UPPER_FREQ: // set freq finder upper limit
            ff=data.toDouble(&ok);
            if (ok) {
                fhigh=ff;
            }
            break;
        case BANDMAP_CMD_QUIT: // quit program
            quit();
            break;
        case BANDMAP_CMD_TX: // set transmit state
            txLabel.setText("<font color=#FF0000>TX");
            spectrumProcessor->setPeakDetect(false);
            settings->setValue(s_sdr_peakdetect,false);
            break;
        case BANDMAP_CMD_RX: // cancel transmit state
            txLabel.setText("<font color=#000000>TX");
            spectrumProcessor->setPeakDetect(true);
            settings->setValue(s_sdr_peakdetect,true);
            break;
        case BANDMAP_CMD_FIND_FREQ: // find open frequency
            if (qAbs(centerFreq)>0 && qAbs(flow)>0 && qAbs(fhigh)>0) {
                spectrumProcessor->startFindCQ(flow,fhigh,callList);
            }
            break;
        case BANDMAP_CMD_SET_INVERT: // invert spectrum
            if (data[0]==char(0x00)) {
                setInvert(false);
            } else {
                setInvert(true);
            }
            break;
        case BANDMAP_CMD_SET_ADD_OFFSET: // set additional IF offset
            f=data.toDouble(&ok);
            if (ok) {
                spectrumProcessor->setAddOffset(f);
            }
            break;
        case BANDMAP_CMD_ADD_CALL: // add callsign
            addCall(data);
            break;
        case BANDMAP_CMD_CLEAR: // clear callsign list
            callList.clear();
            break;
        case BANDMAP_CMD_DELETE_CALL: // delete callsign
            deleteCall(data);
            break;
        case BANDMAP_CMD_DELETE_CALL_FREQ: // delete call at a specific frequency
            f=data.toDouble(&ok);
            deleteCallFreq(f);
            break;
        case BANDMAP_CMD_QSY_UP:  // qsy to next higher signal
            qsyNext(true);
            break;
        case BANDMAP_CMD_QSY_DOWN:  // qsy to next lower signal
            qsyNext(false);
            break;
        }
        cmd=0;
        cmdLen=0;
    }
}

/*!
   Finds next higher or lower detected signal

   higher if higher=true, otherwise lower
   sends found freq to controlling process via TCP
 */
void So2sdrBandmap::qsyNext(bool higher)
{
    if (!settings->value(s_sdr_peakdetect,s_sdr_peakdetect_def).toBool()) {
        return;
    }
    double    f  = centerFreq;
    double    df = settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toDouble();

    Signal *s = &spectrumProcessor->sigList[0];
    if (higher) {
        for (int i = 0; i < SIG_MAX; i++) {
            if (s[i].active && s[i].f > centerFreq) {
                double d = s[i].f - centerFreq;
                if (d < df && d > SIG_MIN_FREQ_DIFF) {
                    f  = s[i].f;
                    df = d;
                }
            }
        }
    } else {
        for (int i = 0; i < SIG_MAX; i++) {
            if (s[i].active && s[i].f < centerFreq) {
                double d = centerFreq - s[i].f;
                if (d < df && d > SIG_MIN_FREQ_DIFF) {
                    f  = s[i].f;
                    df = d;
                }
            }
        }
    }
    // return freq by UDP
    if (f<=0 || qAbs(f-centerFreq)<1.0) return;
    writeUdpXML(f,"",false);
}

/*! delete call from callsign list
 *
 * if call is present more than once, all entries with that call are removed
 *
 */
void So2sdrBandmap::deleteCall(QByteArray data)
{
    QList<Call>::iterator iter = callList.begin();
    while (iter != callList.end()) {
      if ((*iter).call == data)
        iter = callList.erase(iter);
      else
        ++iter;
    }
}

/*! delete call from callsign list at freq f
 *
 * finds all calls within SIG_MIN_FREQ_DIFF of f and deletes them
 *   (this could delete more than one call)
 *
 */
void So2sdrBandmap::deleteCallFreq(double f)
{
    QList<Call>::iterator iter = callList.begin();
    while (iter != callList.end()) {
      if (qAbs((*iter).freq - f) < SIG_MIN_FREQ_DIFF)
        iter = callList.erase(iter);
      else
        ++iter;
    }
}

/*! parse command string adding callsign to bandmap
 */
void So2sdrBandmap::addCall(QByteArray data)
{
    Call newcall;
    int len=data.length();
    int i1=data.indexOf(',',0);
    if (i1==len) return;
    newcall.call=data.mid(0,i1);
    int i2=data.indexOf(',',i1+1);
    bool ok;
    newcall.freq=data.mid(i1+1,i2-i1-1).toDouble(&ok);
    if (!ok) return;
    if (len!=(i2+8)) return;
    newcall.rgbCall[0]=data[i2+1];
    newcall.rgbCall[1]=data[i2+2];
    newcall.rgbCall[2]=data[i2+3];
    if (data.at(i2+4)==0x00) {
        newcall.markRgb[0]=false;
    } else {
        newcall.markRgb[0]=true;
    }
    if (data.at(i2+5)==0x00) {
        newcall.markRgb[1]=false;
    } else {
        newcall.markRgb[1]=true;
    }
    if (data.at(i2+6)==0x00) {
        newcall.markRgb[2]=false;
    } else {
        newcall.markRgb[2]=true;
    }
    if (data.at(i2+7) != 0x00) {
        newcall.mark=true;
    } else {
        newcall.mark=false;
    }
    callList.append(newcall);
}

/*! start TCP connection
 */
void So2sdrBandmap::startConnection()
{
    socket = server.nextPendingConnection();
    connect(socket, SIGNAL(disconnected()),socket, SLOT(deleteLater()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(readData()));
}

int So2sdrBandmap::getBand(double f)
{
    int g=qRound(f / 1000000.0);
    switch (g) {
    case 1: case 2: return(BAND160);
         // 160
    case 3: case 4: return(BAND80);
         // 80
    case 5: return(BAND60);
         // 60
    case 6: case 7: return(BAND40);
         // 40
    case 9: case 10: return(BAND30);
         // 30
    case 13: case 14: return(BAND20);
         // 20
    case 18: return(BAND17);
         // 17
    case 20: case 21: return(BAND15);
         // 15
    case 24: case 25: return(BAND12);
         // 12
    case 27: case 28: case 29: case 30: case 31: return(BAND10);
        // 10
    case 49: case 50:case 51:case 52:case 53:case 54: case 55: case 56: return(BAND6);
         // 6
    case 143: case 144:case 145:case 146:case 147:case 148: case 149: return(BAND2);
         // 2
    case 219:case 220:case 221:case 222:case 223:case 224:case 225: return(BAND222);
        // 220 MHz
    }
    // handle UHF
    double fmhz=f/1000000;
    if (fmhz>419 && fmhz<451) {
        return(BAND420);
    } else if (fmhz>901 && fmhz<929) {
        return(BAND902);
    } else if (fmhz>1239 && fmhz<1301) {
        return(BAND1240);
    }
    // 630m, 2200m
    double khz=f/1000;
    if (khz>400 && khz<500) {
        return(BAND630);
    } else if (khz>100 && khz<150) {
        return(BAND2200);
    }
    // @todo microwave bands!
    return(BAND_NONE);
}

void So2sdrBandmap::setBandName(int b)
{
    const QString bands[N_BANDS] = { "160", "80", "40", "20", "15", "10", "60", "30", "17", "12", "6", "2","1.25m",
                                        "70cm","33cm","1.2G","2.3G","3.3G","5.6G","10G","24G","47G","76G","122G","134G","241G","630m","2200m"};
    if (b<0 || b>=N_BANDS) {
        bandName.clear();
    } else {
        bandName=bands[b];
    }
}

/* slot called from spectrumProcessor with qsy frequency.
 * this frequency is returned to the connected program through tcp
*/
void So2sdrBandmap::findQsy(double f)
{
    if (f<=0 || socket->state()!=QAbstractSocket::ConnectedState || socket->state()==QAbstractSocket::ClosingState) return;
    writeUdpXML(f,"",false);

    // update bandmap with new frequency
    centerFreq=f;
    endFreqs[0] = centerFreq-
            settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toInt()/2
            -settings->value(s_sdr_offset,s_sdr_offset_def).toInt();
    endFreqs[1] = centerFreq+
            settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toInt()/2
            -settings->value(s_sdr_offset,s_sdr_offset_def).toInt();
    spectrumProcessor->setFreq(centerFreq, endFreqs[0], endFreqs[1]);
    spectrumProcessor->resetAvg();
    makeFreqScaleAbsolute();
    FreqLabel->setPixmap(freqPixmap);
    FreqLabel->update();

}

/*!
 * \brief So2sdrBandmap::writeUdpXML writes XML message to UDP with bandmap status.
 * \param freq  if nonzero, sends updated frequency
 * \param call if not empty, sends callsign
 * \param del if true, send "delete" message to delete callsign
 */
void So2sdrBandmap::writeUdpXML(double freq,QByteArray call,bool del)
{
    QByteArray msg;
    QXmlStreamWriter stream(&msg);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("So2sdr");
    stream.writeStartElement("bandmap");
    stream.writeAttribute("RadioNr",QString::number(settings->value(s_sdr_nrig,s_sdr_nrig).toInt()+1));
    stream.writeAttribute("winid",QString::number(winId()));
    if (!call.isEmpty()) {
        stream.writeAttribute("call",call);
    }
    if (del) {
        stream.writeAttribute("operation","delete");
    }
    if (freq>0) {
        stream.writeAttribute("freq",QString::number(freq,'f'));
    }
    stream.writeEndElement();
    stream.writeEndElement();
    stream.writeEndDocument();
    socketUdp.writeDatagram(msg.data(), msg.size(),QHostAddress::LocalHost,
                            settings->value(s_sdr_bandmap_udp_port,s_sdr_bandmap_udp_port_def).toInt());
}

/*!
 * \brief So2sdrBandmap::udpRead
 *  process incoming UDP packets
 */
void So2sdrBandmap::udpRead()
{
    // read network data and pass to XML reader
    while (socketUdpN1MM.hasPendingDatagrams()) {
        QByteArray data;
        data.resize(socketUdpN1MM.pendingDatagramSize());
        socketUdpN1MM.readDatagram(data.data(),data.size());
        xmlReader.addData(data);
    }
    xmlParseN1MM();
}

/*! parse udp data coming from N1MM+
 * set center frequency if the number of this bandmap (nr=0,1) matches
 * RadioNr (1,2) of N1MM+
 */
void So2sdrBandmap::xmlParseN1MM()
{
    int nr=-1;
    int f=0;
    while (!xmlReader.atEnd() && !xmlReader.hasError()) {
        QXmlStreamReader::TokenType token=xmlReader.readNext();
        if (token==QXmlStreamReader::StartDocument) continue;

        if (token==QXmlStreamReader::StartElement) {
            if (xmlReader.name()=="RadioNr") {
                bool ok;
                nr=xmlReader.readElementText().toInt(&ok)-1;
                if (!ok) nr=-1;
            }
            if (xmlReader.name()=="Freq") {
                bool ok;
                // note N1MM+ freqs not in Hz, multiply by 10
                f=xmlReader.readElementText().toInt(&ok)*10;
                if (ok && f>0 && nr==settings->value(s_sdr_nrig,s_sdr_nrig_def).toInt()) {
                    centerFreq=f;
                    int b=getBand(f);
                    setBandName(b);

                    endFreqs[0] = centerFreq-
                            settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toInt()/2
                            -settings->value(s_sdr_offset,s_sdr_offset_def).toInt();
                    endFreqs[1] = centerFreq+
                            settings->value(s_sdr_sample_freq,s_sdr_sample_freq_def).toInt()/2
                            -settings->value(s_sdr_offset,s_sdr_offset_def).toInt();

                    setWindowTitle("Bandmap "+bandName+" ["+QString::number(endFreqs[0]/1000)+"-"+QString::number(endFreqs[1]/1000)+"]");
                    if (band != b) {
                        spectrumProcessor->clearSigs();
                        spectrumProcessor->clearCQ();
                        band=b;
                    }
                    spectrumProcessor->setFreq(centerFreq, endFreqs[0], endFreqs[1]);
                    spectrumProcessor->resetAvg();
                    makeFreqScaleAbsolute();
                    FreqLabel->setPixmap(freqPixmap);
                    FreqLabel->update();
                }
            }
        }
    }
    xmlReader.clear();
}

void So2sdrBandmap::showHelp()
{
    if (help == nullptr) {
        // open help file and display it
        directory.setCurrent(dataDirectory()+"/so2sdr-bandmap-help");
        help = new HelpDialog("so2sdr-bandmap-help.html", this);
    }
    help->show();
    help->setFocus();
}

void So2sdrBandmap::resetTuningTimer()
{
    spectrumProcessor->setTuning(false);
}

/*! resize UI elements based on actual font size
 */
void So2sdrBandmap::setUiSize()
{
    if (QX11Info::appDpiX()>100) {
        setWindowIcon(QIcon(dataDirectory() + "/icon48x48.png"));
    } else {
        setWindowIcon(QIcon(dataDirectory() + "/icon24x24.png"));
    }
    QFont font10("sans", BANDMAP_FONT_POINT_SIZE);
    QFontMetricsF fm10(font10);
    uiSizes.height=fm10.height();
    uiSizes.width=fm10.horizontalAdvance("0");
    uiSizes.rad=uiSizes.width/1.5;
    QFont font9("sans", 9);
    QFontMetricsF fm9(font9);
    uiSizes.smallHeight=fm9.height();
    uiSizes.smallWidth=fm9.horizontalAdvance("0");
    toolBar->setFixedHeight(qRound(uiSizes.height*1.5));
    actionRun->setIcon(QIcon(":/icons/player_play.svg"));
    actionStop->setIcon(QIcon(":/icons/player_stop.svg"));
    actionSetup->setIcon(QIcon(":/icons/stock_properties.svg"));
    actionQuit->setIcon(QIcon(":/icons/exit.svg"));
    FreqLabel->setFixedWidth(qRound(uiSizes.width*6));
}
