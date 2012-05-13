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
#include "qttelnet.h"
#include <QtGui/QApplication>
#include <QtGui/QTextEdit>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QInputDialog>
#include <QtGui/QFont>
#include <QtCore/QEvent>
#include <QtGui/QShortcut>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QDialog>
#include <QtGui/QLayout>
#include <QtGui/QKeyEvent>
#include <QtGui/QScrollBar>

class LoginDialog : public QDialog
{
public:
    LoginDialog(QWidget *parent = 0)
        : QDialog(parent)
    {
        QGridLayout *gl = new QGridLayout;
        setLayout(gl);

        QLabel *l;

        l = new QLabel("&Login:", this);
        gl->addWidget(l, 0, 0);
        lle = new QLineEdit(this);
        gl->addWidget(lle, 0, 1);
        l->setBuddy(lle);

        l = new QLabel("&Password:", this);
        gl->addWidget(l, 1, 0);
        ple = new QLineEdit(this);
        ple->setEchoMode(QLineEdit::Password);
        gl->addWidget(ple, 1, 1);
        l->setBuddy(ple);

        QWidget *hbox = new QWidget(this);
        QHBoxLayout *hboxlayout = new QHBoxLayout;

        gl->addWidget(hbox, 2, 0, 1, 2);

        QPushButton *pb;
        pb = new QPushButton("&Ok");
        connect(pb, SIGNAL(clicked()), this, SLOT(accept()));
        hboxlayout->addWidget(pb);

        pb = new QPushButton("&Cancel");
        connect(pb, SIGNAL(clicked()), this, SLOT(reject()));
        hboxlayout->addWidget(pb);

        hbox->setLayout(hboxlayout);
    }

    QString login() const { return lle->text(); }
    QString password() const { return ple->text(); }

private:
    QLineEdit *lle, *ple;
};

class TelnetClient : public QMainWindow
{
    Q_OBJECT
public:
    TelnetClient(QWidget *par = 0)
        : QMainWindow(par), t(new QtTelnet)
    {
        //t->setPromptString("Welcome");

        QWidget *vbox = new QWidget(this);
        QVBoxLayout *vboxlayout = new QVBoxLayout();

        textEdit = new QTextEdit();
        lineEdit = new QLineEdit();
        status = new QLabel();
        quit = new QPushButton("Quit");
        connect(quit, SIGNAL(clicked()), this, SLOT(close()));

        vboxlayout->addWidget(textEdit);
        vboxlayout->addWidget(lineEdit);
        vboxlayout->addWidget(status);
        vboxlayout->addWidget(quit);
        vbox->setLayout(vboxlayout);
        setCentralWidget(vbox);

        QFont fnt = textEdit->font();
        fnt.setFamily("Fixed");
        textEdit->setFont(fnt);

        lineEdit->installEventFilter(this);
        installEventFilter(this);

        QShortcut *shortcut;

        shortcut = new QShortcut(this);
        shortcut->setKey(Qt::CTRL + Qt::Key_D);
        connect(shortcut, SIGNAL(activated()),
                this, SLOT(deleteCharOrLogout()));

        shortcut = new QShortcut(this);
        shortcut->setKey(Qt::CTRL + Qt::Key_Z);
        connect(shortcut, SIGNAL(activated()), this, SLOT(suspend()));

        shortcut = new QShortcut(this);
        shortcut->setKey(Qt::CTRL + Qt::Key_C);
        connect(shortcut, SIGNAL(activated()), this, SLOT(kill()));

        textEdit->setReadOnly(true);

        connect(lineEdit, SIGNAL(returnPressed()),
                this, SLOT(lineReturnPressed()));

        connect(t, SIGNAL(message(const QString &)),
                this, SLOT(telnetMessage(const QString &)));
	//        connect(t, SIGNAL(loginRequired()),
	//      this, SLOT(telnetLoginRequired()));
	//nnect(t, SIGNAL(loginFailed()),
	//      this, SLOT(telnetLoginFailed()));
	//nnect(t, SIGNAL(loggedOut()),
	//      this, SLOT(telnetLoggedOut()));
	// connect(t, SIGNAL(loggedIn()),
	//      this, SLOT(telnetLoggedIn()));
	// connect(t, SIGNAL(connectionError(QAbstractSocket::SocketError)),
	//      this, SLOT(telnetConnectionError(QAbstractSocket::SocketError)));

        QString host = QInputDialog::getText(this,
                                             "Host name",
                                             "Host name of Telnet server",
                                             QLineEdit::Normal,
                                             "localhost");
        host = host.trimmed();
	//	QRegExp rx1("\\b(Connected)\\b");
	//	t->setPromptPattern(rx1);
	//	QRegExp rx2("\\b(call:)\\b");
	//	t->setLoginPattern(rx2);
        if (!host.isEmpty())
            t->connectToHost(host);
        else
            textEdit->setPlainText("Not connected");
    }

protected:
    void resizeEvent(QResizeEvent *ev)
    {
        QFontMetrics fm(font());
        int lh = fm.lineSpacing();
        int cw = fm.width(QChar('X'));
        QRect r = textEdit->visibleRegion().boundingRect();
        int dx = ev->oldSize().width() - r.width();
        int dy = ev->oldSize().height() - r.height();

        t->setWindowSize((ev->size().width() - dx) / cw,
                         (ev->size().height() - dy) / lh);
    }
    bool eventFilter(QObject *obj, QEvent *event)
    {
        if (event->type() == QEvent::ShortcutOverride) {
            QKeyEvent *ke = static_cast<QKeyEvent *>(event);
            if (ke->modifiers() & Qt::ControlModifier) {
                switch (ke->key()) {
                case Qt::Key_C:
                case Qt::Key_D:
                case Qt::Key_Z:
                    return true;
                default:
                    break;
                }
            }
        }
        return QObject::eventFilter(obj, event);
    }

private slots:
    void telnetMessage(const QString &msg)
    {
        textEdit->append(stripCR(msg));
        QScrollBar *s = textEdit->verticalScrollBar();
        s->setValue(s->maximum());
    }
    void telnetLoginRequired()
    {
        LoginDialog ld(this);
        if (ld.exec() == LoginDialog::Accepted)
            t->login(ld.login(), ld.password());
        else
            t->logout();
    }
    void telnetLoginFailed()
    {
        status->setText("Login failed");
    }
    void telnetLoggedOut()
    {
        lineEdit->setEnabled(false);
        status->setText("Logged out");
    }
    void telnetLoggedIn()
    {
      lineEdit->setEnabled(true);
      status->setText("Logged in");
    }
    void telnetConnectionError(QAbstractSocket::SocketError error)
    {
        status->setText(QString("Connection error: %1").arg(error));
    }
    void suspend()
    {
        t->sendControl(QtTelnet::Suspend);
    }
    void kill()
    {
        t->sendControl(QtTelnet::InterruptProcess);
    }
    void lineReturnPressed()
    {
        t->sendData(lineEdit->text());
	lineEdit->clear();
    }
    void deleteCharOrLogout()
    {
        if (lineEdit->text().isEmpty()) {
            t->logout();
        } else {
            int cpos = lineEdit->cursorPosition();
            QString txt = lineEdit->text();
            if (cpos < (int)txt.length()) {
                lineEdit->setText(txt.remove(cpos, 1));
                lineEdit->setCursorPosition(cpos);
            }
        }
    }

private:
    QString stripCR(const QString &msg)
    {
        QString nmsg(msg);
        nmsg.remove('\r');
        nmsg.remove(QRegExp("\033\\[[0-9;]*[A-Za-z]")); // Also remove terminal control codes
        return nmsg;
    }

private:
    QtTelnet *t;
    QTextEdit *textEdit;
    QLineEdit *lineEdit;
    QLabel *status;
    QPushButton *quit;
};

int main( int argc, char **argv )
{
    QApplication a(argc, argv);

    TelnetClient t;
    t.show();
    return a.exec();
}

#include "main.moc"

