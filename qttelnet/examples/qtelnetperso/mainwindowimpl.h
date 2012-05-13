#ifndef MAINWINDOWIMPL_H
#define MAINWINDOWIMPL_H
//
#include <QMainWindow>
#include "ui_mainwindow.h"
//#include "../libs/src/mytelnetwidget.h"
#include "mytelnetwidget.h"
//
class MainWindowImpl : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

public:
    MainWindowImpl( QWidget * parent = 0, Qt::WFlags f = 0 );

private slots:
    void on_connectButton_clicked();
    void on_disconnectButton_clicked();
    void alert();

private:
    MyTelnetWidget *mtw;
};
#endif
