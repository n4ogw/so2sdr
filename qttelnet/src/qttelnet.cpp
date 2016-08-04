/****************************************************************************
** 
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
** 
** This file is part of a Qt Solutions component.
**
** Commercial Usage  
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
** 
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
** 
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.1, included in the file LGPL_EXCEPTION.txt in this
** package.
** 
** GNU General Public License Usage 
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
** 
** Please note Third Party Software included with Qt Solutions may impose
** additional restrictions and it is the user's responsibility to ensure
** that they have met the licensing requirements of the GPL, LGPL, or Qt
** Solutions Commercial license and the relevant license of the Third
** Party Software they are using.
** 
** If you are unsure which license is appropriate for your use, please
** contact Nokia at qt-info@nokia.com.
** 
****************************************************************************/

/*!
    \class QtTelnet
    \brief The QtTelnet class proveds an API to connect to Telnet servers,
    issue commands and receive replies.

    When a QtTelnet object has been created, you need to call
    connectToHost() to establish a connection with a Telnet server.
    When the connection is established the connected() signal is
    emitted. At this point you should call login(). The
    QtTelnet object will emit connectionError() if the connection
    fails, and authenticationFailed() if the login() failed.

    Once the connection has been successfully established and
    you've logged in you can send control messages using sendControl()
    and data using sendData(). Connect to the message() signal to
    receive data from the Telnet server. The connection is closed with
    close().

    You can use your own socket if you call setSocket() before
    connecting. The socket used by QtTelnet is available from
    socket().
*/
#include "qttelnet.h"
#include <QtNetwork/QTcpSocket>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QVariant>
#include <QtCore/QSocketNotifier>
#include <QtCore/QBuffer>
#include <QtCore/QVarLengthArray>

#ifdef Q_WS_WIN
#  include <winsock2.h>
#endif
#if defined (Q_OS_UNIX)
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#endif

//#define QTTELNET_DEBUG

#ifdef QTTELNET_DEBUG
#include <QtCore/QDebug>
#endif

class QtTelnetAuth
{
public:
    enum State { AuthIntermediate, AuthSuccess, AuthFailure };

    QtTelnetAuth(char code) : st(AuthIntermediate), cd(code) {};
    virtual ~QtTelnetAuth() {}

    int code() const { return cd; }
    State state() const { return st; }
    void setState(State state) { st = state; };

    virtual QByteArray authStep(const QByteArray &data) = 0;

private:
    State st;
    int   cd;
};

class QtTelnetReceiveBuffer
{
public:
    QtTelnetReceiveBuffer() : bytesAvailable(0) {}
    void append(const QByteArray &data) { buffers.append(data); }
    void push_back(const QByteArray &data) { buffers.prepend(data); }
    long size() const { return bytesAvailable; }
    QByteArray readAll()
    {
        QByteArray a;
        while (!buffers.isEmpty()) {
            a.append(buffers.takeFirst());
        }
        return a;
    }

private:
    QList<QByteArray> buffers;
    long bytesAvailable;
};

namespace Common // RFC854
{
    // Commands
    const uchar CEOF  = 236;
    const uchar SUSP  = 237;
    const uchar ABORT = 238;
    const uchar SE    = 240;
    const uchar NOP   = 241;
    const uchar DM    = 242;
    const uchar BRK   = 243;
    const uchar IP    = 244;
    const uchar AO    = 245;
    const uchar AYT   = 246;
    const uchar EC    = 247;
    const uchar EL    = 248;
    const uchar GA    = 249;
    const uchar SB    = 250;
    const uchar WILL  = 251;
    const uchar WONT  = 252;
    const uchar DO    = 253;
    const uchar DONT  = 254;
    const uchar IAC   = 255;

    // Types
    const char IS    = 0;
    const char SEND  = 1;

    const char Authentication = 37; // RFC1416,
                                    // implemented to always return NULL
    const char SuppressGoAhead = 3; // RFC858
    const char Echo = 1; // RFC857, not implemented (returns WONT/DONT)
    const char LineMode = 34; // RFC1184, implemented
    const uchar LineModeEOF = 236, // RFC1184, not implemented
                LineModeSUSP = 237,
                LineModeABORT = 238;
    const char Status = 5; // RFC859, should be implemented!
    const char Logout = 18; // RFC727, implemented
    const char TerminalType = 24; // RFC1091,
                                  // implemented to always return UNKNOWN
    const char NAWS = 31; // RFC1073, implemented
    const char TerminalSpeed = 32; // RFC1079, not implemented
    const char FlowControl = 33; // RFC1372, should be implemented?
    const char XDisplayLocation = 35; // RFC1096, not implemented
    const char EnvironmentOld = 36; // RFC1408, should not be implemented!
    const char Environment = 39; // RFC1572, should be implemented
    const char Encrypt = 38; // RFC2946, not implemented

#ifdef QTTELNET_DEBUG
    QString typeStr(char op)
    {
        QString str;
        switch (op) {
        case IS:
            str = "IS";
            break;
        case SEND:
            str = "SEND";
            break;
        default:
            str = QString("Unknown common type (%1)").arg(op);
        }
        return str;
    }
    QString operationStr(char op)
    {
        QString str;
        switch (quint8(op)) {
        case quint8(WILL):
            str = "WILL";
            break;
        case quint8(WONT):
            str = "WONT";
            break;
        case quint8(DO):
            str = "DO";
            break;
        case quint8(DONT):
            str = "DONT";
            break;
        case quint8(SB):
            str = "SB";
            break;
        default:
            str = QString("Unknown operation (%1)").arg(quint8(op));
        }
        return str;
    }

    QString optionStr(char op)
    {
        QString str;
        switch (op) {
        case Authentication:
            str = "AUTHENTICATION";
            break;
        case SuppressGoAhead:
            str = "SUPPRESS GO AHEAD";
            break;
        case Echo:
            str = "ECHO";
            break;
        case LineMode:
            str = "LINEMODE";
            break;
        case Status:
            str = "STATUS";
            break;
        case Logout:
            str = "LOGOUT";
            break;
        case TerminalType:
            str = "TERMINAL-TYPE";
            break;
        case TerminalSpeed:
            str = "TERMINAL-SPEED";
            break;
        case NAWS:
            str = "NAWS";
            break;
        case FlowControl:
            str = "TOGGLE-FLOW-CONTROL";
            break;
        case XDisplayLocation:
            str = "X-DISPLAY-LOCATION";
            break;
        case EnvironmentOld:
            str = "ENVIRON";
            break;
        case Environment:
            str = "NEW-ENVIRON";
            break;
        case Encrypt:
            str = "ENCRYPT";
            break;
        default:
            str = QString("Unknown option (%1)").arg(op);
        }
        return str;
    }
#endif
};

namespace Auth // RFC1416
{
    enum Auth
    {
        REPLY = 2,
        NAME
    };
    enum Types
    {
        AUTHNULL, // Can't have enum values named NULL :/
        KERBEROS_V4,
        KERBEROS_V5,
        SPX,
        SRA = 6,
        LOKI = 10
    };
    enum Modifiers
    {
        AUTH_WHO_MASK = 1,
        AUTH_CLIENT_TO_SERVER = 0,
        AUTH_SERVER_TO_CLIENT = 1,
        AUTH_HOW_MASK = 2,
        AUTH_HOW_ONE_WAY = 0,
        AUTH_HOW_MUTUAL = 2
    };
    enum SRA
    {
        SRA_KEY = 0,
        SRA_USER = 1,
        SRA_CONTINUE = 2,
        SRA_PASSWORD = 3,
        SRA_ACCEPT = 4,
        SRA_REJECT = 5
    };

#ifdef QTTELNET_DEBUG
    QString authStr(int op)
    {
        QString str;
        switch (op) {
        case REPLY:
            str = "REPLY";
            break;
        case NAME:
            str = "NAME";
            break;
        default:
            str = QString("Unknown auth (%1)").arg(op);
        }
        return str;
    }
    QString typeStr(int op)
    {
        QString str;
        switch (op) {
        case AUTHNULL:
            str = "NULL";
            break;
        case KERBEROS_V4:
            str = "KERBEROS_V4";
            break;
        case KERBEROS_V5:
            str = "KERBEROS_V5";
            break;
        case SPX:
            str = "SPX";
            break;
        case SRA:
            str = "SRA";
            break;
        case LOKI:
            str = "LOKI";
            break;
        default:
            str = QString("Unknown auth type (%1)").arg(op);
        }
        return str;
    }
    QString whoStr(int op)
    {
        QString str;
        op = op & AUTH_WHO_MASK;
        switch (op) {
        case AUTH_CLIENT_TO_SERVER:
            str = "CLIENT";
            break;
        case AUTH_SERVER_TO_CLIENT:
            str = "SERVER";
            break;
        default:
            str = QString("Unknown who type (%1)").arg(op);
        }
        return str;
    }
    QString howStr(int op)
    {
        QString str;
        op = op & AUTH_HOW_MASK;
        switch (op) {
        case AUTH_HOW_ONE_WAY:
            str = "ONE-WAY";
            break;
        case AUTH_HOW_MUTUAL:
            str = "MUTUAL";
            break;
        default:
            str = QString("Unknown how type (%1)").arg(op);
        }
        return str;
    }
    QString sraStr(int op)
    {
        QString str;
        switch (op) {
        case SRA_KEY:
            str = "KEY";
            break;
        case SRA_REJECT:
            str = "REJECT";
            break;
        case SRA_ACCEPT:
            str = "ACCEPT";
            break;
        case SRA_USER:
            str = "USER";
            break;
        case SRA_CONTINUE:
            str = "CONTINUE";
            break;
        case SRA_PASSWORD:
            str = "PASSWORD";
            break;
        default:
            str = QString("Unknown SRA option (%1)").arg(op);
        }
        return str;
    }
#endif
};

namespace LineMode // RFC1184
{
    const char Mode = 1;
    const char ForwardMask = 2;
    const char SLC = 3;
    enum Modes
    {
        EDIT = 1,
        TRAPSIG = 2,
        MODE_ACK = 4,
        SOFT_TAB = 8,
        LIT_ECHO = 16
    };
    enum SLCs
    {
        SLC_SYNCH = 1,
        SLC_BRK = 2,
        SLC_IP = 3,
        SLC_AO =  4,
        SLC_AYT = 5,
        SLC_EOR = 6,
        SLC_ABORT = 7,
        SLC_EOF = 8,
        SLC_SUSP = 9,
        SLC_EC = 10,
        SLC_EL = 11,
        SLC_EW = 12,
        SLC_RP = 13,
        SLC_LNEXT = 14,
        SLC_XON = 15,
        SLC_XOFF = 16,
        SLC_FORW1 = 17,
        SLC_FORW2 = 18,
        SLC_MCL = 19,
        SLC_MCR = 20,
        SLC_MCWL = 21,
        SLC_MCWR = 22,
        SLC_MCBOL = 23,
        SLC_MCEOL = 24,
        SLC_INSRT = 25,
        SLC_OVER = 26,
        SLC_ECR = 27,
        SLC_EWR = 28,
        SLC_EBOL = 29,
        SLC_EEOL = 30,
        SLC_DEFAULT = 3,
        SLC_VALUE = 2,
        SLC_CANTCHANGE = 1,
        SLC_NOSUPPORT = 0,
        SLC_LEVELBITS = 3,
        SLC_ACK = 128,
        SLC_FLUSHIN = 64,
        SLC_FLUSHOUT = 32
    };
};

class QtTelnetAuthNull : public QtTelnetAuth
{
public:
    QtTelnetAuthNull() : QtTelnetAuth(0) {}

    QByteArray authStep(const QByteArray &data);
};

QByteArray QtTelnetAuthNull::authStep(const QByteArray &data)
{
    Q_ASSERT(data[0] == Common::Authentication);

    if (data.size() < 2 || data[1] != Common::SEND)
        return QByteArray();

    char buf[8] = {(char)Common::IAC, (char)Common::SB, Common::Authentication,
                   Common::IS, Auth::AUTHNULL, 0, // CLIENT|ONE-WAY
                   (char)Common::IAC, (char)Common::SE};
    setState(AuthSuccess);
    return QByteArray(buf, sizeof(buf));
}

class QtTelnetPrivate : public QObject
{
    Q_OBJECT
public:
    QtTelnetPrivate(QtTelnet *parent);
    ~QtTelnetPrivate();

    QMap<char, bool> modes;
    QList< QPair<uchar, uchar> > osent;

    QtTelnet *q;
    QTcpSocket *socket;
    QtTelnetReceiveBuffer buffer;
    QSocketNotifier *notifier;

    QSize windowSize;

    bool connected, nocheckp;
    bool triedlogin, triedpass, firsttry;

    QMap<int, QtTelnetAuth*> auths;
    QtTelnetAuth *curauth;
    bool nullauth;

    QRegExp loginp, passp, promptp;
    QString login, pass;

    bool allowOption(int oper, int opt);
    void sendOptions();
    void sendCommand(const QByteArray &command);
    void sendCommand(const char *command, int length);
    void sendCommand(const char operation, const char option);
    void sendString(const QString &str);
    bool replyNeeded(uchar operation, uchar option);
    void setMode(uchar operation, uchar option);
    bool alreadySent(uchar operation, uchar option);
    void addSent(uchar operation, uchar option);
    void sendWindowSize();

    int  parsePlaintext(const QByteArray &data);
    int parseIAC(const QByteArray &data);
    bool isOperation(const uchar c);
    bool isCommand(const uchar c);
    QByteArray getSubOption(const QByteArray &data);
    void parseSubAuth(const QByteArray &data);
    void parseSubTT(const QByteArray &data);
    void parseSubNAWS(const QByteArray &data);
    uchar opposite(uchar operation, bool positive);

    void consume();

    void setSocket(QTcpSocket *socket);

public slots:
    void socketConnected();
    void socketConnectionClosed();
    void socketReadyRead();
    void socketError(QAbstractSocket::SocketError error);
    void socketException(int);
};

QtTelnetPrivate::QtTelnetPrivate(QtTelnet *parent)
    : q(parent), socket(0), notifier(0),
      connected(false), nocheckp(false),
      triedlogin(false), triedpass(false), firsttry(true),
      curauth(0), nullauth(false),
      loginp("ogin:\\s*$"), passp("assword:\\s*$")
{
    setSocket(new QTcpSocket(this));
}

QtTelnetPrivate::~QtTelnetPrivate()
{
    delete socket;
    delete notifier;
    delete curauth;
}

void QtTelnetPrivate::setSocket(QTcpSocket *s)
{
    if (socket) {
        q->logout();
        socket->flush();
    }
    delete socket;
    socket = s;
    connected = false;
    if (socket) {
        connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
        connect(socket, SIGNAL(disconnected()),
                this, SLOT(socketConnectionClosed()));
        connect(socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
        connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(socketError(QAbstractSocket::SocketError)));
    }
}

/*
   Returns the opposite value of the one we pass in.
*/
uchar QtTelnetPrivate::opposite(uchar operation, bool positive)
{
    if (operation == Common::DO)
        return (positive ? Common::WILL : Common::WONT);
    else if (operation == Common::DONT) // Not allowed to say WILL
        return Common::WONT;
    else if (operation == Common::WILL)
        return (positive ? Common::DO : Common::DONT);
    else if (operation == Common::WONT) // Not allowed to say DO
        return Common::DONT;
    return 0;
}

void QtTelnetPrivate::consume()
{
    const QByteArray data = buffer.readAll();
    int currpos = 0;
    int prevpos = -1;;
    while (prevpos < currpos && currpos < data.size()) {
        prevpos = currpos;
        const uchar c = uchar(data[currpos]);
        if (c == Common::DM)
            ++currpos;
        else if (c == Common::IAC)
            currpos += parseIAC(data.mid(currpos));
        else // Assume plain text
            currpos += parsePlaintext(data.mid(currpos));
    }
    if (currpos < data.size())
        buffer.push_back(data.mid(currpos));
}

bool QtTelnetPrivate::isCommand(const uchar c)
{
    return (c == Common::DM);
}

bool QtTelnetPrivate::isOperation(const uchar c)
{
    return (c == Common::WILL || c == Common::WONT
            || c == Common::DO ||c == Common::DONT);
}

QByteArray QtTelnetPrivate::getSubOption(const QByteArray &data)
{
    Q_ASSERT(!data.isEmpty() && uchar(data[0]) == Common::IAC);

    if (data.size() < 4 || uchar(data[1]) != Common::SB)
        return QByteArray();

    for (int i = 2; i < data.size() - 1; ++i) {
        if (uchar(data[i]) == Common::IAC && uchar(data[i+1]) == Common::SE) {
            return data.mid(2, i-2);
        }
    }
    return QByteArray();
}

void QtTelnetPrivate::parseSubNAWS(const QByteArray &data)
{
    Q_UNUSED(data);
}

void QtTelnetPrivate::parseSubTT(const QByteArray &data)
{
    Q_ASSERT(!data.isEmpty() && data[0] == Common::TerminalType);

    if (data.size() < 2 || data[1] != Common::SEND)
        return;

    const char c1[4] = { (char)Common::IAC, (char)Common::SB,
                         (char)Common::TerminalType, (char)Common::IS};
    sendCommand(c1, sizeof(c1));
    sendString("UNKNOWN");
    const char c2[2] = {(char)Common::IAC, (char)Common::SE };
    sendCommand(c2, sizeof(c2));
}

void QtTelnetPrivate::parseSubAuth(const QByteArray &data)
{
    Q_ASSERT(data[0] == Common::Authentication);

    if (!curauth && data[1] == Common::SEND) {
        int pos = 2;
        while (pos < data.size() && !curauth) {
            curauth = auths[data[pos]];
            pos += 2;

            if (curauth) {
                emit q->loginRequired();
                break;
            }
        }
        if (!curauth) {
            curauth = new QtTelnetAuthNull;
            nullauth = true;
            if (loginp.isEmpty() && passp.isEmpty()) {
                // emit q->loginRequired();
                nocheckp = true;
            }
        }
    }
    if (curauth) {
        const QByteArray a = curauth->authStep(data);
        if (!a.isEmpty())
            sendCommand(a);

        if (curauth->state() == QtTelnetAuth::AuthFailure)
            emit q->loginFailed();
        else if (curauth->state() == QtTelnetAuth::AuthSuccess) {
            if (loginp.isEmpty() && passp.isEmpty())
                emit q->loggedIn();
            if (!nullauth)
                nocheckp = true;
        }
    }
}

/*
  returns number of bytes consumed
*/
int QtTelnetPrivate::parseIAC(const QByteArray &data)
{
    if (data.isEmpty())
        return 0;

    Q_ASSERT(uchar(data.at(0)) == Common::IAC);

    if (data.size() >= 3 && isOperation(data[1])) { // IAC, Operation, Option
        const uchar operation = data[1];
        const uchar option = data[2];
        if (operation == Common::WONT && option == Common::Logout) {
            q->close();
            return 3;
        }
        if (operation == Common::DONT && option == Common::Authentication) {
            if (loginp.isEmpty() && passp.isEmpty())
                emit q->loggedIn();
            nullauth = true;
        }
        if (replyNeeded(operation, option)) {
            bool allowed = allowOption(operation, option);
            sendCommand(opposite(operation, allowed), option);
            setMode(operation, option);
        }
        return 3;
    }
    if (data.size() >= 2 && isCommand(data[1])) { // IAC Command
        return 2;
    }

    QByteArray suboption = getSubOption(data);
    if (suboption.isEmpty())
        return 0;

    // IAC SB Operation SubOption [...] IAC SE
    switch (suboption[0]) {
    case Common::Authentication:
        parseSubAuth(suboption);
        break;
    case Common::TerminalType:
        parseSubTT(suboption);
        break;
    case Common::NAWS:
        parseSubNAWS(data);
        break;
    default:
        qWarning("QtTelnetPrivate::parseIAC: unknown suboption %d",
                 quint8(suboption.at(0)));
        break;
    }
    return suboption.size() + 4;
}

int QtTelnetPrivate::parsePlaintext(const QByteArray &data)
{
    int consumed = 0;
    int length = data.indexOf('\0');
    if (length == -1) {
        length = data.size();
        consumed = length;
    } else {
        consumed = length + 1; // + 1 for removing '\0'
    }

    QString text = QString::fromLocal8Bit(data.constData(), length);

    if (!nocheckp && nullauth) {
        if (!promptp.isEmpty() && promptp.indexIn(text) != -1) {
            emit q->loggedIn();
            nocheckp = true;
        }
    }
    if (!nocheckp && nullauth) {
        if (!loginp.isEmpty() && loginp.indexIn(text) != -1) {
            if (triedlogin || firsttry) {
                emit q->message(text);    // Display the login prompt
                text.clear();
                emit q->loginRequired();  // Get a (new) login
                firsttry = false;
            }
            if (!triedlogin) {
                q->sendData(login);
                triedlogin = true;
            }
        }
        if (!passp.isEmpty() && passp.indexIn(text) != -1) {
            if (triedpass || firsttry) {
                emit q->message(text);    // Display the password prompt
                text.clear();
                emit q->loginRequired();  // Get a (new) pass
                firsttry = false;
            }
            if (!triedpass) {
                q->sendData(pass);
                triedpass = true;
                // We don't have to store the password anymore
                pass.fill(' ');
                pass.resize(0);
            }
        }
    }

    if (!text.isEmpty())
        emit q->message(text);
    return consumed;
}

bool QtTelnetPrivate::replyNeeded(uchar operation, uchar option)
{
    if (operation == Common::DO || operation == Common::DONT) {
        // RFC854 requires that we don't acknowledge
        // requests to enter a mode we're already in
        if (operation == Common::DO && modes[option])
            return false;
        if (operation == Common::DONT && !modes[option])
            return false;
    }
    return true;
}

void QtTelnetPrivate::setMode(uchar operation, uchar option)
{
    if (operation != Common::DO && operation != Common::DONT)
        return;

    modes[option] = (operation == Common::DO);
    if (option == Common::NAWS && modes[Common::NAWS])
        sendWindowSize();
}

void QtTelnetPrivate::sendWindowSize()
{
    if (!modes[Common::NAWS])
        return;
    if (!q->isValidWindowSize())
        return;

    short h = htons(windowSize.height());
    short w = htons(windowSize.width());
    const char c[9] = { (char)Common::IAC, (char)Common::SB, Common::NAWS,
                        (char)(w & 0x00ff), (char)(w >> 8), (char)(h & 0x00ff), (char)(h >> 8),
                        (char)Common::IAC, (char)Common::SE };
    sendCommand(c, sizeof(c));
}

void QtTelnetPrivate::addSent(uchar operation, uchar option)
{
    osent.append(QPair<uchar, uchar>(operation, option));
}

bool QtTelnetPrivate::alreadySent(uchar operation, uchar option)
{
    QPair<uchar, uchar> value(operation, option);
    if (osent.contains(value)) {
        osent.removeAll(value);
        return true;
    }
    return false;
}

void QtTelnetPrivate::sendString(const QString &str)
{
    if (!connected || str.length() == 0)
        return;

    socket->write(str.toLocal8Bit());
}

void QtTelnetPrivate::sendCommand(const QByteArray &command)
{
    if (!connected || command.isEmpty())
        return;

    if (command.size() == 3) {
        const char operation = command.at(1);
        const char option = command.at(2);
        if (alreadySent(operation, option))
            return;
        addSent(operation, option);
    }
    socket->write(command);
}

void QtTelnetPrivate::sendCommand(const char operation, const char option)
{
  const char c[3] = { (char)Common::IAC, operation, option };
    sendCommand(c, 3);
}

void QtTelnetPrivate::sendCommand(const char *command, int length)
{
    QByteArray a(command, length);
    sendCommand(a);
}

bool QtTelnetPrivate::allowOption(int /*oper*/, int opt)
{
    if (opt == Common::Authentication ||
        opt == Common::SuppressGoAhead ||
        opt == Common::LineMode ||
        opt == Common::Status ||
        opt == Common::Logout ||
        opt == Common::TerminalType)
        return true;
    if (opt == Common::NAWS && q->isValidWindowSize())
        return true;
    return false;
}

void QtTelnetPrivate::sendOptions()
{
    sendCommand(Common::WILL, Common::Authentication);
    sendCommand(Common::DO, Common::SuppressGoAhead);
    sendCommand(Common::WILL, Common::LineMode);
    sendCommand(Common::DO, Common::Status);
    if (q->isValidWindowSize())
        sendCommand(Common::WILL, Common::NAWS);
}

void QtTelnetPrivate::socketConnected()
{
    connected = true;
    delete notifier;
    notifier = new QSocketNotifier(socket->socketDescriptor(),
                                   QSocketNotifier::Exception, this);
    connect(notifier, SIGNAL(activated(int)),
            this, SLOT(socketException(int)));
    sendOptions();
}

void QtTelnetPrivate::socketException(int)
{
  //    qDebug("out-of-band data received, should handle that here!");
}

void QtTelnetPrivate::socketConnectionClosed()
{
    delete notifier;
    notifier = 0;
    connected = false;
    emit q->loggedOut();
}

void QtTelnetPrivate::socketReadyRead()
{
    buffer.append(socket->readAll());
    consume();
}

void QtTelnetPrivate::socketError(QAbstractSocket::SocketError error)
{
    emit q->connectionError(error);
}

/*!
    \enum QtTelnet::Control

    This enum specifies control messages you can send to the Telnet server
    using sendControl().

    \value GoAhead Sends the \c GO \c AHEAD control message, meaning that the
    server can continue to send data.

    \value InterruptProcess Interrupts the current running process on
    the server. This is the equivalent of pressing \key{Ctrl+C} in most
    terminal emulators.

    \value AreYouThere Sends the \c ARE \c YOU \c THERE control
    message, to check if the connection is still alive.

    \value AbortOutput Temporarily suspends the output from the server.
    The output will resume if you send this control message again.

    \value EraseCharacter Erases the last entered character.

    \value EraseLine Erases the last line.

    \value Break Sends the \c BREAK control message.

    \value EndOfFile Sends the \c END \c OF \c FILE control message.

    \value Suspend Suspends the current running process on the server.
    Equivalent to pressing \key{Ctrl+Z} in most terminal emulators.

    \value Abort Sends the \c ABORT control message.

    \sa sendControl()
*/

/*!
    Constructs a QtTelnet object.

    You must call connectToHost() before calling any of the other
    member functions.

    The \a parent is sent to the QObject constructor.

    \sa connectToHost()
*/
QtTelnet::QtTelnet(QObject *parent)
    : QObject(parent), d(new QtTelnetPrivate(this))
{
}

/*!
    Destroys the QtTelnet object. This will also close
    the connection to the server.

    \sa logout()
*/
QtTelnet::~QtTelnet()
{
    delete d;
}

/*!
    Calling this function will make the QtTelnet object attempt to
    connect to a Telnet server specified by the given \a host and \a
    port.

    The connected() signal is emitted if the connection
    succeeds, and the connectionError() signal is emitted if the
    connection fails. Once the connection is establishe you must call
    login().

    \sa close()
*/
void QtTelnet::connectToHost(const QString &host, quint16 port)
{
    if (d->connected)
        return;
    d->socket->connectToHost(host, port);
}

/*!
    Closes the connection to a Telnet server.

    \sa connectToHost() login()
*/
void QtTelnet::close()
{
    if (!d->connected)
        return;
    delete d->notifier;
    d->notifier = 0;
    d->connected = false;
    d->socket->close();
    emit loggedOut();
}

/*!
    Sends the control message \a ctrl to the Telnet server the
    QtTelnet object is connected to.

    \sa Control sendData() sendSync()
*/
void QtTelnet::sendControl(Control ctrl)
{
    bool sendsync = false;
    char c;
    switch (ctrl) {
    case InterruptProcess: // Ctrl-C
        c = Common::IP;
        sendsync = true;
        break;
    case AbortOutput: // suspend/resume output
        c = Common::AO;
        sendsync = true;
        break;
    case Break:
        c = Common::BRK;
        break;
    case Suspend: // Ctrl-Z
        c = Common::SUSP;
        break;
    case EndOfFile:
        c = Common::CEOF;
        break;
    case Abort:
        c = Common::ABORT;
        break;
    case GoAhead:
        c = Common::GA;
        break;
    case AreYouThere:
        c = Common::AYT;
        sendsync = true;
        break;
    case EraseCharacter:
        c = Common::EC;
        break;
    case EraseLine:
        c = Common::EL;
        break;
    default:
        return;
    }
    const char command[2] = {(char)Common::IAC, c};
    d->sendCommand(command, sizeof(command));
    if (sendsync)
        sendSync();
}

/*!
    Sends the string \a data to the Telnet server. This is often a
    command the Telnet server will execute.

    \sa sendControl()
*/
void QtTelnet::sendData(const QString &data)
{
    if (!d->connected)
        return;

    QByteArray str = data.toLocal8Bit();
    d->socket->write(str);
    // RTC this change is needed for DX cluster use!
    // d->socket->write("\r\n\0", 3);
    d->socket->write("\n", 1);
}

/*!
    This function will log you out of the Telnet server.
    You cannot send any other data after sending this command.

    \sa login() sendData() sendControl()
*/
void QtTelnet::logout()
{
    d->sendCommand(Common::DO, Common::Logout);
}

/*!
    Sets the client window size to \a size.

    The width and height are given in number of characters.
    Non-visible clients should pass an invalid size (i.e. QSize()).

    \sa isValidWindowSize()
*/
void QtTelnet::setWindowSize(const QSize &size)
{
    setWindowSize(size.width(), size.height());
}

/*!
    Sets the client window size.

    The \a width and \a height are given in number of characters.

    \overload
*/
void QtTelnet::setWindowSize(int width, int height)
{
    bool wasvalid = isValidWindowSize();

    d->windowSize.setWidth(width);
    d->windowSize.setHeight(height);

    if (wasvalid && isValidWindowSize())
        d->sendWindowSize();
    else if (isValidWindowSize())
        d->sendCommand(Common::WILL, Common::NAWS);
    else if (wasvalid)
        d->sendCommand(Common::WONT, Common::NAWS);
}

/*!
    Returns the window's size. This will be an invalid size
    if the Telnet server is not using the NAWS option (RFC1073).

    \sa isValidWindowSize()
*/
QSize QtTelnet::windowSize() const
{
    return (d->modes[Common::NAWS] ? d->windowSize : QSize());
}

/*!
    Returns true if the window size is valid, i.e.
    windowSize().isValid() returns true; otherwise returns false.

    \sa setWindowSize()
*/
bool QtTelnet::isValidWindowSize() const
{
    return windowSize().isValid();
}

/*!
    Set the \a socket to be used in the communication.

    This function allows you to use your own QSocket subclass. You
    should call this function before calling connectToHost(); if you
    call it after a connection has been established the connection
    will be closed, so in all cases you will need to call
    connectToHost() after calling this function.

    \sa socket(), connectToHost(), logout()
*/
void QtTelnet::setSocket(QTcpSocket *socket)
{
    d->setSocket(socket);
}

/*!
    Returns the QTcpSocket instance used by this telnet object.

    \sa setSocket()
*/
QTcpSocket *QtTelnet::socket() const
{
    return d->socket;
}

/*!
    Sends the Telnet \c SYNC sequence, meaning that the Telnet server
    should discard any data waiting to be processed once the \c SYNC
    sequence has been received. This is sent using a TCP urgent
    notification.

    \sa sendControl()
*/
void QtTelnet::sendSync()
{
    if (!d->connected)
        return;
    d->socket->flush(); // Force the socket to send all the pending data before
                        // sending the SYNC sequence.
    int s = d->socket->socketDescriptor();
    char tosend = (char)Common::DM;
    ::send(s, &tosend, 1, MSG_OOB); // Send the DATA MARK as out-of-band
}

/*!
    Sets the expected shell prompt pattern.

    The \a pattern is used to automatically recognize when the client
    has successfully logged in. When a line is read that matches the
    \a pattern, the loggedIn() signal will be emitted.

    \sa login(), loggedIn()
*/
void QtTelnet::setPromptPattern(const QRegExp &pattern)
{
    d->promptp = pattern;
}

/*!
    \fn void QtTelnet::setPromptString(const QString &pattern)

    Sets the expected shell prompt to \a pattern.

    \overload
*/

/*!
    Sets the expected login pattern.

    The \a pattern is used to automatically recognize when the server
    asks for a username. If no username has been set, the
    loginRequired() signal will be emitted.

    \sa login()
*/
void QtTelnet::setLoginPattern(const QRegExp &pattern)
{
    d->loginp = pattern;
}

/*!
    \fn void QtTelnet::setLoginString(const QString &login)

    Sets the expected login string to \a login.

    \overload
*/

/*!
    Sets the expected password prompt pattern.

    The \a pattern is used to automatically recognize when the server
    asks for a password. If no password has been set, the loginRequired()
    signal will be emitted.

    \sa login()
*/
void QtTelnet::setPasswordPattern(const QRegExp &pattern)
{
    d->passp = pattern;
}

/*!
    \fn void QtTelnet::setPasswordString(const QString &pattern)

    Sets the expected password prompt to \a pattern.

    \overload
*/

/*!
    Sets the \a username and \a password to be used when logging in to
    the server.

    \sa setLoginPattern(), setPasswordPattern()
*/
void QtTelnet::login(const QString &username, const QString &password)
{
    d->triedpass = d->triedlogin = false;
    d->login = username;
    d->pass = password;
}

/*!
    \fn void QtTelnet::loginRequired()

    This signal is emitted when the QtTelnet class sees
    that the Telnet server expects authentication and you
    have not already called login().

    As a reply to this signal you should either call
    login() or logout()

    \sa login(), logout()
*/

/*!
    \fn void QtTelnet::loginFailed()

    This signal is emitted when the login has failed.
    Do note that you might in certain cases see several
    loginRequired() signals being emitted but no
    loginFailed() signals. This is due to the Telnet
    specification not requiring the Telnet server to
    support reliable authentication methods.

    \sa login(), loginRequired()
*/

/*!
    \fn void QtTelnet::loggedOut()

    This signal is emitted when you have called logout()
    and the Telnet server has actually logged you out.

    \sa logout(), login()
*/

/*!
    \fn void QtTelnet::loggedIn()

    This signal is emitted when you have been logged in
    to the server as a result of the login() command
    being called. Do note that you might never see this
    signal even if you have been logged in, due to the
    Telnet specification not requiring Telnet servers
    to notify clients when users are logged in.

    \sa login(), setPromptPattern()
*/

/*!
    \fn void QtTelnet::connectionError(QAbstractSocket::SocketError error)

    This signal is emitted if the underlying socket
    implementation receives an error. The \a error
    argument is the same as being used in
    QSocket::connectionError()
*/

/*!
    \fn void QtTelnet::message(const QString &data)

    This signal is emitted when the QtTelnet object
    receives more \a data from the Telnet server.

    \sa sendData()
*/

#include "qttelnet.moc"

