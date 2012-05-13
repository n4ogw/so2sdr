#include "mainwindowimpl.h"
#include <QLayout>
//
MainWindowImpl::MainWindowImpl( QWidget * parent, Qt::WFlags f) : QMainWindow(parent, f)
{
    setupUi(this);

    mtw = new MyTelnetWidget(this);
    this->centralWidget()->layout()->addWidget(mtw);
}

void MainWindowImpl::on_connectButton_clicked()
{
    mtw->connectToHost("dxc.k5jz.net");
    connect(mtw, SIGNAL(bellReceived()),this,SLOT(alert()));
}

void MainWindowImpl::on_disconnectButton_clicked()
{
  //    mtw->disconnectTelnet();
}

void MainWindowImpl::alert()
{
    QApplication::beep();

}

//

