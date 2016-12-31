/*! Copyright 2010-2017 R. Torsten Clay N4OGW

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
#include <QScrollBar>
#include <QSettings>

#include "qttelnet.h"
#include "defines.h"
#include "telnet.h"

Telnet::Telnet(QSettings &s, QWidget *parent) : QWidget(parent),settings(s)
{
    setupUi(this);
    telnet = 0;
    telnet = new QtTelnet();
    connect(TelnetConnectButton, SIGNAL(clicked()), this, SLOT(connectTelnet()));
    connect(TelnetDisconnectButton, SIGNAL(clicked()), this, SLOT(disconnectTelnet()));
    connect(telnet, SIGNAL(message(QString)), this, SLOT(showText(QString)));
    TelnetComboBox->setEditable(true);
    hosts.clear();
    buffer.clear();
    lineEdit->clear();
    lineEdit->setEnabled(false);
    TelnetTextEdit->setReadOnly(true);
    TelnetComboBox->setFocus();
    TelnetComboBox->clear();
    connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(sendText()));

    // get addresses from settings
    int sz=settings.beginReadArray(s_telnet_addresses);
    for (int i=0;i<sz;i++) {
        settings.setArrayIndex(i);
        TelnetComboBox->addItem(settings.value("address","").toString());
    }
    settings.endArray();

    // restore window geometry
    settings.beginGroup("TelnetWindow");
    resize(settings.value("size", QSize(400, 594)).toSize());
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    settings.endGroup();
}

Telnet::~Telnet()
{
    disconnectTelnet();
    delete telnet;
}

void Telnet::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    // disconnect
    disconnectTelnet();

    // update saved address list
    int n=TelnetComboBox->count();
    settings.beginWriteArray(s_telnet_addresses,n);
    for (int i=0;i<n;i++) {
        settings.setArrayIndex(i);
        settings.setValue("address",TelnetComboBox->itemText(i));
    }
    settings.endArray();
    // save window geometry
    settings.beginGroup("TelnetWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();

    emit(done(false));
}

/*!
   Open a connection
 */
void Telnet::connectTelnet()
{
    QString host = TelnetComboBox->currentText();
    if (host.isEmpty()) return;

    host = host.trimmed();

    // get port number
    int i = host.indexOf(":");
    int port;
    if (i == -1) {
        port = 23; // default port number
    } else {
        QString tmp = host.mid(i + 1, host.size() - 1);
        bool    ok  = false;
        port = tmp.toInt(&ok, 10);
        if (!ok) {
            port = 23;
        }
        host.truncate(i);
    }

    telnet->connectToHost(host, port);
    lineEdit->setEnabled(true);
    lineEdit->setFocus();
}

/*!
   Close connection
 */
void Telnet::disconnectTelnet()
{
    telnet->logout();
    telnet->close();
    lineEdit->setEnabled(false);
}

/*!
   Send text currently held by line edit
 */
void Telnet::sendText()
{
    telnet->sendData(lineEdit->text());
    lineEdit->clear();
}

/*!
   process incoming text. Decode spots and pass everything to telnet window.
 */
void Telnet::showText(QString txt)
{
    // is there a better way to recognize dx spot?
    if (txt.contains("DX de") || (txt.contains("<") && txt.contains(">"))) {
        QString call = txt.section(' ', 4, 4, QString::SectionSkipEmpty).toUpper();

        // don't spot station callsign
        if (call != settings.value(s_call,s_call_def)) {
            bool ok;
            QString freq = txt.section(' ', 3, 3, QString::SectionSkipEmpty);
            double  x = freq.toDouble(&ok);
            int f = (int) (x * 1000);
            emit(dxSpot(call.toLatin1(), f));
        }
    }
    txt.remove(QRegExp("[^a-zA-Z/.\\d\\s]")); // remove all except letters, numbers, ., and /
    buffer = buffer + txt;
    if (buffer.size() > MAX_TELNET_CHARS) {
        // remove text from top of buffer
        int i = buffer.size() - MAX_TELNET_CHARS;
        buffer.remove(0, i);
    }
    TelnetTextEdit->setText(buffer);
    QScrollBar *s = TelnetTextEdit->verticalScrollBar();
    s->setValue(s->maximum());
}
