#ifndef WPMDIALOG_H
#define WPMDIALOG_H
#include "ui_wpmdialog.h"

class WPMDialog : public QDialog, public Ui::WPMDialog
{
Q_OBJECT

public:
    WPMDialog(QWidget *parent = 0);

// signals:
// void speedSet(int wpm);

public slots:
    void setSpeed(int wpm);

// private slots:
// void WPMUpdate();
};

#endif
