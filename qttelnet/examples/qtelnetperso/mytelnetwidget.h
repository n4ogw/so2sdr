#ifndef _MYTELNETWIDGET_H_
#define _MYTELNETWIDGET_H_

#include <QPlainTextEdit>
#include "qttelnet.h"
#include <QScrollBar>

class MyTelnetWidget : public QPlainTextEdit
{
    Q_OBJECT

public:
    MyTelnetWidget(QWidget *parent = 0);
    ~MyTelnetWidget();

    void connectToHost(const QString &host, const quint16 port = 23);
    void closeSession();

protected:
    void keyPressEvent(QKeyEvent *e);
    void mousePressEvent(QMouseEvent *me);

signals:
    void connected();
    void disconnected();
    void alert();

private slots:
    void sockConnected();
    void sockDisconnected();
    void processIncomingData(const QString &text);

private:
    void simulateOverWriteMode(QString text);

    QtTelnet *t;

};

#endif // MYTELNETWIDGET_H

