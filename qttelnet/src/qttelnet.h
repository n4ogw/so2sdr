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

#ifndef QTTELNET_H
#define QTTELNET_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QSize>
#include <QtCore/QRegExp>
#include <QtNetwork/QTcpSocket>

class QtTelnetPrivate;

#if defined(Q_WS_WIN)
#  if !defined(QT_QTTELNET_EXPORT) && !defined(QT_QTTELNET_IMPORT)
#    define QT_QTTELNET_EXPORT
#  elif defined(QT_QTTELNET_IMPORT)
#    if defined(QT_QTTELNET_EXPORT)
#      undef QT_QTTELNET_EXPORT
#    endif
#    define QT_QTTELNET_EXPORT __declspec(dllimport)
#  elif defined(QT_QTTELNET_EXPORT)
#    undef QT_QTTELNET_EXPORT
#    define QT_QTTELNET_EXPORT __declspec(dllexport)
#  endif
#else
#  define QT_QTTELNET_EXPORT
#endif

class QT_QTTELNET_EXPORT QtTelnet : public QObject
{
    Q_OBJECT
    friend class QtTelnetPrivate;
public:
    QtTelnet(QObject *parent = 0);
    ~QtTelnet();

    enum Control { GoAhead, InterruptProcess, AreYouThere, AbortOutput,
                   EraseCharacter, EraseLine, Break, EndOfFile, Suspend,
                   Abort };

    void connectToHost(const QString &host, quint16 port = 23);

    void login(const QString &user, const QString &pass);

    void setWindowSize(const QSize &size);
    void setWindowSize(int width, int height); // In number of characters
    QSize windowSize() const;
    bool isValidWindowSize() const;

    void setSocket(QTcpSocket *socket);
    QTcpSocket *socket() const;

    void setPromptPattern(const QRegExp &pattern);
    void setPromptString(const QString &pattern)
    { setPromptPattern(QRegExp(QRegExp::escape(pattern))); }
public Q_SLOTS:
    void close();
    void logout();
    void sendControl(Control ctrl);
    void sendData(const QString &data);
    void sendSync();

Q_SIGNALS:
    void loginRequired();
    void loginFailed();
    void loggedIn();
    void loggedOut();
    void connectionError(QAbstractSocket::SocketError error);
    void message(const QString &data);

public:
    void setLoginPattern(const QRegExp &pattern);
    void setLoginString(const QString &pattern)
    { setLoginPattern(QRegExp(QRegExp::escape(pattern))); }
    void setPasswordPattern(const QRegExp &pattern);
    void setPasswordString(const QString &pattern)
    { setPasswordPattern(QRegExp(QRegExp::escape(pattern))); }

private:
    QtTelnetPrivate *d;
};
#endif
