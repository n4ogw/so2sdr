#include <QDialog>
#include <QSpinBox>
#include "wpmdialog.h"

// form for ajusting CW speed

WPMDialog::WPMDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);

    connect(WPMbuttonBox, SIGNAL(rejected()), this, SLOT(close()));

    // connect(WPMbuttonBox,SIGNAL(accepted()),this,SLOT(WPMUpdate()));
}

void WPMDialog::setSpeed(int wpm)

// sets spin box with wpm
{
    WPMspinBox->setValue(wpm);
}

// void WPMDialog::WPMUpdate()
// update WPM from form input
// {
// WPM=WPMspinBox->value();
// }
