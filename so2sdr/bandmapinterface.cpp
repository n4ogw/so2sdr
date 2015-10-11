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
#include "bandmapinterface.h"
#include "bandmapentry.h"
#include "utils.h"
#include <QByteArray>
#include <QDebug>
#include <QHostAddress>
#include <QStringList>
#include <QDir>

bool BandmapInterface::bandmapon(int nr) const
{
    return bandmapOn[nr];
}

BandmapInterface::BandmapInterface(QSettings &s, QObject *parent) :
    QObject(parent),settings(s)
{
    for (int i=0;i<NRIG;i++) {
        band[i]=-1;
        cmdLen[i]=0;
        cmd[i]=0;
        bandmapOn[i]=false;
        bandmapAvailable[i]=false;
        switch (i) {
        case 0:
            connect(&bandmapProcess[0],SIGNAL(stateChanged(QProcess::ProcessState)),this,SLOT(launchBandmap1State(QProcess::ProcessState)));
            connect(&bandmapProcess[0],SIGNAL(error(QProcess::ProcessError)),this,SLOT(launchShowBandmapProcessError1(QProcess::ProcessError)));
            break;
        case 1:
            connect(&bandmapProcess[1],SIGNAL(stateChanged(QProcess::ProcessState)),this,SLOT(launchBandmap2State(QProcess::ProcessState)));
            connect(&bandmapProcess[1],SIGNAL(error(QProcess::ProcessError)),this,SLOT(launchShowBandmapProcessError2(QProcess::ProcessError)));
            break;
        }
    }
    socketUdp.bind(settings.value(s_sdr_udp,s_sdr_udp_def).toInt(), QUdpSocket::ShareAddress);
    connect(&socketUdp,SIGNAL(readyRead()),this,SLOT(udpRead()));
    connect(&socket[0],SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(socketError0(QAbstractSocket::SocketError)));
    connect(&socket[1],SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(socketError1(QAbstractSocket::SocketError)));
    connect(&socket[0],SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(launchTcpSocketStateChange1(QAbstractSocket::SocketState)));
    connect(&socket[1],SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(launchTcpSocketStateChange2(QAbstractSocket::SocketState)));
}

BandmapInterface::~BandmapInterface()
{
    for (int i=0;i<NRIG;i++) {
        closeBandmap(i);
        if (bandmapProcess[i].state() != QProcess::NotRunning) {
            bandmapProcess[i].close();
            bandmapProcess[i].waitForFinished(-1);
        }
    }
}

void BandmapInterface::launchTcpSocketStateChange1(QAbstractSocket::SocketState state)
{
    tcpSocketStateChange(0,state);
}

void BandmapInterface::launchTcpSocketStateChange2(QAbstractSocket::SocketState state)
{
    tcpSocketStateChange(1,state);
}

void BandmapInterface::tcpSocketStateChange(int nr, QAbstractSocket::SocketState state)
{
    if (state==QAbstractSocket::ConnectedState) {
        bandmapOn[nr]=true;
        switch (nr) {
        case 0:
            emit(bandmap1state(true));
            break;
        case 1:
            emit(bandmap2state(true));
            break;
        }
    } else if (state==QAbstractSocket::UnconnectedState) {
        bandmapOn[nr]=false;
        switch (nr) {
        case 0:
            emit(bandmap1state(false));
            break;
        case 1:
            emit(bandmap2state(false));
            break;
        }
    }
}

/*! QProcess error connection for bandmap 1 */
void BandmapInterface::launchShowBandmapProcessError1(QProcess::ProcessError err)
{
    showBandmapProcessError(0,err);
}

/*! QProcess error connection for bandmap 2 */
void BandmapInterface::launchShowBandmapProcessError2(QProcess::ProcessError err)
{
    showBandmapProcessError(1,err);
}

/*! Shows QProcess errors */
void BandmapInterface::showBandmapProcessError(int nr, QProcess::ProcessError err)
{
    Q_UNUSED(err)
    Q_UNUSED(nr)
}


/*! TCP socket error connection for bandmap 1 */
void BandmapInterface::socketError0(QAbstractSocket::SocketError err)
{
    socketError(0,err);
}

/*! TCP socket error connection for bandmap 2 */
void BandmapInterface::socketError1(QAbstractSocket::SocketError err)
{
    socketError(1,err);
}

/*! @todo implement TCP error handling */
void BandmapInterface::socketError(int nr, QAbstractSocket::SocketError err)
{
    Q_UNUSED(nr);
    Q_UNUSED(err);
}

/*!
 * \brief BandmapInterface::addSpot
 *  adds a call to the bandmap nr
 * \param nr  : bandmap number (0,NRIG-1)
 * \param spot
 */
void BandmapInterface::addSpot(int nr,const BandmapEntry &spot)
{
    if (bandmapOn[nr] && socket[nr].state()==QAbstractSocket::ConnectedState)
    {
        const char cmd=BANDMAP_CMD_ADD_CALL;
        QByteArray str=spot.call;
        str=str+","+QByteArray::number(spot.f)+",";
        if (spot.dupe) {
            str=str+char(0xff)+char(0x00)+char(0xff)+char(0xff)+char(0x00)+char(0xff);
            str=str+char(0x01);
        } else {
            str=str+char(0x00)+char(0x00)+char(0x00)+char(0xff)+char(0xff)+char(0xff);
            str=str+char(0x00);
        }
        char len=str.length();
        if (socket[nr].write(&cmd,1)==-1) {
            qDebug("bandmapinterface addSpot write error 1");
        }
        if (socket[nr].write(&len,1)==-1) {
            qDebug("bandmapinterface addSpot write error 2");
        }
        if (socket[nr].write(str.data(),len)==-1) {
            qDebug("bandmapinterface addSpot write error 3");
        }
    }
}

/*!
 * \brief BandmapInterface::removeSpot
 *  adds a call to the bandmap nr
 * \param nr  : bandmap number (0,NRIG-1)
 * \param spot
 *
 *
 */
void BandmapInterface::removeSpot(int nr,const BandmapEntry &spot)
{
    if (bandmapOn[nr] && socket[nr].state()==QAbstractSocket::ConnectedState)
    {
        const char cmd=BANDMAP_CMD_DELETE_CALL;
        char len=spot.call.size();
        if (socket[nr].write(&cmd,1)==-1) {
            qDebug("bandmapinterface removeSpot write error 1");
        }
        if (socket[nr].write(&len,1)==-1) {
            qDebug("bandmapinterface removeSpot write error 2");
        }
        if (socket[nr].write(spot.call.data(),len)==-1) {
            qDebug("bandmapinterface removeSpot write error 3");
        }
    }
}

/*! request qsy to next higher (higher=true) or lower
 *(higher=false) marked signal on bandmap nr.
 *
 * If available, the frequency will be returned by TCP
 */
void BandmapInterface::nextFreq(int nr, bool higher)
{
    if (bandmapOn[nr] && socket[nr].state()==QAbstractSocket::ConnectedState) {
        char cmd;
        if (higher) cmd=BANDMAP_CMD_QSY_UP;
        else cmd=BANDMAP_CMD_QSY_DOWN;
        const char len=0;

        if (socket[nr].write(&cmd,1)==-1) {
            qDebug("bandmapinterface nextFreq write error 1");
        }
        if (socket[nr].write(&len,1)==-1) {
            qDebug("bandmapinterface nextFreq write error 2");
        }
    }
}

/*!
 * \brief BandmapInterface::syncCalls
 *
 * send callsign list to bandmap
 *
 * \param nr radio number (0,1)
 * \param spotList : list of spots for this band
 */
void BandmapInterface::syncCalls(int nr, QList<BandmapEntry> &spotList)
{
    if (nr<0 || nr>=NRIG) return;

    if (bandmapOn[nr] && socket[nr].state()==QAbstractSocket::ConnectedState)
    {
        // command to clear list
        char len=0;
        char cmd=BANDMAP_CMD_CLEAR;

        if (socket[nr].write(&cmd,1)==-1) {
            qDebug("bandmapinterface syncCalls write error 1");
        }
        if (socket[nr].write(&len,1)==-1) {
            qDebug("bandmapinterface syncCalls write error 2");
        }

        // send calls on this band
        for (int i = 0; i < spotList.size(); i++) {
            addSpot(nr,spotList.at(i));
        }
    }
}

/*!
 * \brief BandmapInterface::udpRead
 *  process UDP packets coming from bandmap
 */
void BandmapInterface::udpRead()
{
    // read network data and pass to XML reader
    while (socketUdp.hasPendingDatagrams()) {
        QByteArray data;
        data.resize(socketUdp.pendingDatagramSize());
        socketUdp.readDatagram(data.data(),data.size());
        xmlReader.addData(data);
    }
    xmlParse();
}

/*! parse xml data coming from so2sdr-bandmap
 */
void BandmapInterface::xmlParse()
{
    while (!xmlReader.atEnd() && !xmlReader.hasError()) {
        QXmlStreamReader::TokenType token=xmlReader.readNext();
        if (token==QXmlStreamReader::StartDocument) continue;

        if (token==QXmlStreamReader::StartElement) {
            if (xmlReader.name()=="bandmap") {
                int nr=-1;
                int f=0;
                QByteArray call="";
                QXmlStreamAttributes attr=xmlReader.attributes();

                if (attr.hasAttribute("RadioNr")) {
                    bool ok;
                    nr=attr.value("RadioNr").toString().toInt(&ok)-1;
                    if (ok && (nr==0 || nr==1)) {
                        bandmapAvailable[nr]=true;
                    }
                }

                if (attr.hasAttribute("freq")) {
                    f=attr.value("freq").toString().toInt();
                    if (nr==0 || nr==1) {
                        emit(qsy(nr,f));
                    }
                }

                if (attr.hasAttribute("call")) {
                    call=attr.value("call").toString().toAscii();
                }

                if (attr.hasAttribute("operation")) {
                    if (attr.value("operation").toString()=="delete" && !call.isEmpty() && f>0) {
                        emit(removeCall(call,getBand(f)));
                    }
                }
            }
        }
    }
    xmlReader.clear();

}

void BandmapInterface::launchBandmap1State(QProcess::ProcessState state)
{
    setBandmapState(0,state);
}

void BandmapInterface::launchBandmap2State(QProcess::ProcessState state)
{
    setBandmapState(1,state);
}


void BandmapInterface::setBandmapState(int nr,QProcess::ProcessState state)
{
    switch (state) {
    case QProcess::NotRunning:
        bandmapOn[nr]=false;
        bandmapAvailable[nr]=false;
        band[nr]=-1;
        switch (nr) {
        case 0:
            emit(bandmap1state(false));
            break;
        case 1:
            emit(bandmap2state(false));
            break;
        }
    case QProcess::Starting:
        break;
    case QProcess::Running:
        break;
    }
}

void BandmapInterface::connectTcp()
{
    for (int i=0;i<NRIG;i++) {
        if (!bandmapOn[i]) {
            if (bandmapAvailable[i] && socket[i].state()==QAbstractSocket::UnconnectedState) {
                socket[i].connectToHost(QHostAddress::LocalHost,settings.value(s_sdr_port[i],s_sdr_port_def[i]).toInt());
            }
        }
    }
}


/*!
 * \brief BandmapInterface::setAddOffset Sets IF offset for bandmap
 * \param nr radio number (0,1)
 * \param f frequency (Hz)
 */
void BandmapInterface::setAddOffset(int f,int nr)
{
    if (nr<0 || nr>=NRIG) return;

    if (bandmapOn[nr] && socket[nr].state()==QAbstractSocket::ConnectedState)
    {
        const char cmd=BANDMAP_CMD_SET_ADD_OFFSET;
        QByteArray str=QByteArray::number(f);
        char len=str.length();

        if (socket[nr].write(&cmd,1)!=1) {
            qDebug("bandmapinterface %d freq write error 1!",nr);
        }
        if (socket[nr].write(&len,1)!=1) {
            qDebug("bandmapinterface %d freq write error 2!",nr);
        }
        if (socket[nr].write(str.data(),len)!=len) {
            qDebug("bandmapinterface %d freq write error 3!",nr);
        }
    }
}


/*!
 * \brief BandmapInterface::bandmapSetFreq Send command to bandmap setting center frequency
 * \param f frequency (Hz)
 * \param nr radio number (0,1)
 */
void BandmapInterface::bandmapSetFreq(int f,int nr)
{
    if (f<0 || nr<0 || nr>=NRIG) return;
    if (bandmapOn[nr] && socket[nr].state()==QAbstractSocket::ConnectedState)
    {
        band[nr]=getBand(f);

        const char cmd=BANDMAP_CMD_SET_FREQ;
        QByteArray str=QByteArray::number(f);
        char len=str.length();

        if (socket[nr].write(&cmd,1)!=1) {
            qDebug("bandmapinterface %d freq write error 1!",nr);
        }
        if (socket[nr].write(&len,1)!=1) {
            qDebug("bandmapinterface %d freq write error 2!",nr);
        }
        if (socket[nr].write(str.data(),len)!=len) {
            qDebug("bandmapinterface %d freq write error 3!",nr);
        }
    }
}

/*! set limits for open frequency finder
 */
void BandmapInterface::setFreqLimits(int nr,int flow,int fhigh)
{
    if (fhigh<flow || nr<0 || nr>=NRIG) return;
    if (bandmapOn[nr] && socket[nr].state()==QAbstractSocket::ConnectedState)
    {
        QByteArray str;
        char cmd=BANDMAP_CMD_SET_LOWER_FREQ;
        str=QByteArray::number(flow);
        if (str.length()>11) return;

        char len=str.length();
        if (socket[nr].write(&cmd,1)==-1) {
            qDebug("bandmapinterface %d freq write error 1!",nr);
        }
        if (socket[nr].write(&len,1)==-1) {
            qDebug("bandmapinterface %d freq write error 2!",nr);
        }
        if (socket[nr].write(str.data(),str.length())==-1) {
            qDebug("bandmapinterface %d freq write error 3!",nr);
        }

        cmd=BANDMAP_CMD_SET_UPPER_FREQ;

        str=QByteArray::number(fhigh);
        if (str.length()>11) return;
        len=str.length();
        if (socket[nr].write(&cmd,1)==-1) {
            qDebug("bandmapinterface %d freq write error 1!",nr);
        }
        if (socket[nr].write(&len,1)==-1) {
            qDebug("bandmapinterface %d freq write error 2!",nr);
        }
        if (socket[nr].write(str.data(),str.length())==-1) {
            qDebug("bandmapinterface %d freq write error 3!",nr);
        }
    }
}


/*!
 * \brief BandmapInterface::closeBandmap
 *  sends tcp command to close bandmap, then closes tcp connection
 * and closes qprocess.
 * \param nr which radio
 */
void BandmapInterface::closeBandmap(int nr)
{
    if (nr<0 || nr>NRIG) return;

    if (bandmapOn[nr]  && socket[nr].state()==QAbstractSocket::ConnectedState)
    {
        const char len=0;
        const char cmd=BANDMAP_CMD_QUIT;

        if (socket[nr].write(&cmd,1)==-1) {
            qDebug("bandmapinterface quit write error 1");
        }
        if (socket[nr].write(&len,1)==-1) {
            qDebug("bandmapinterface quit write error 2");
        }
        socket[nr].disconnectFromHost();
        socket[nr].waitForDisconnected(5000);
        bandmapProcess[nr].waitForFinished(5000);
        bandmapAvailable[nr]=false;
    }
}


/*!
   show/remove bandmap windows
 */
void BandmapInterface::showBandmap(int nr, int checkboxState)
{
    if (checkboxState == Qt::Checked) {
        if (!bandmapOn[nr]) {
            QStringList args;
            args <<  settings.value(s_sdr_config[nr],s_sdr_config_def[nr]).toString();
#ifdef Q_OS_LINUX
            bandmapProcess[nr].start(settings.value(s_sdr_path[nr],QCoreApplication::applicationDirPath()+"/so2sdr-bandmap").toString(),args);
#endif
#ifdef Q_OS_WIN
            bandmapProcess[nr].start(settings.value(s_sdr_path[nr],QCoreApplication::applicationDirPath()+"/so2sdr-bandmap.exe").toString(),args);
#endif
            bandmapProcess[nr].waitForStarted();
        }
    } else if (checkboxState == Qt::Unchecked) {
        if (bandmapOn[nr]) {
            closeBandmap(nr);
        }
    }
}


/*! set transmit status for bandmap nr
 */
void BandmapInterface::setBandmapTxStatus(bool b, int nr)
{
    if (bandmapOn[nr] && socket[nr].state()==QAbstractSocket::ConnectedState) {
        const char len=0;
        char cmd;
        if (b) {
            cmd=BANDMAP_CMD_TX;
        } else {
            cmd=BANDMAP_CMD_RX;
        }
        socket[nr].write(&cmd,1);
        socket[nr].write(&len,1);
    }
}

/*! send command to find open frequency on radio nr
 */
void BandmapInterface::findFreq(int nr)
{
    if (bandmapOn[nr] && socket[nr].state()==QAbstractSocket::ConnectedState) {
        const char len=0;
        char cmd=BANDMAP_CMD_FIND_FREQ;
        socket[nr].write(&cmd,1);
        socket[nr].write(&len,1);
    }
}

/*! returns current band, or -1 if not set
 *
 */
int BandmapInterface::currentBand(int nr) const
{
    return band[nr];
}

void BandmapInterface::setInvert(int nr, bool b)
{
    if (bandmapOn[nr] && socket[nr].state()==QAbstractSocket::ConnectedState) {
        const char len=1;
        char cmd=BANDMAP_CMD_SET_INVERT;
        socket[nr].write(&cmd,1);
        socket[nr].write(&len,1);
        if (b) {
            char c=0x01;
            socket[nr].write(&c,1);
        } else {
            char c=0x00;
            socket[nr].write(&c,1);
        }
    }
}
