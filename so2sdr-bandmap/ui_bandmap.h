/********************************************************************************
** Form generated from reading UI file 'bandmap.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BANDMAP_H
#define UI_BANDMAP_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "bandmapdisplay.h"

QT_BEGIN_NAMESPACE

class Ui_Bandmap
{
public:
    QAction *actionRun;
    QAction *actionStop;
    QAction *actionSetup;
    QAction *actionQuit;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    BandmapDisplay *display;
    QLabel *FreqLabel;
    QLabel *CallLabel;
    QToolBar *toolBar;

    void setupUi(QMainWindow *Bandmap)
    {
        if (Bandmap->objectName().isEmpty())
            Bandmap->setObjectName(QString::fromUtf8("Bandmap"));
        Bandmap->resize(350, 531);
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(Bandmap->sizePolicy().hasHeightForWidth());
        Bandmap->setSizePolicy(sizePolicy);
        Bandmap->setMinimumSize(QSize(350, 0));
        Bandmap->setMaximumSize(QSize(800, 1450));
        actionRun = new QAction(Bandmap);
        actionRun->setObjectName(QString::fromUtf8("actionRun"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/player_play.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionRun->setIcon(icon);
        actionStop = new QAction(Bandmap);
        actionStop->setObjectName(QString::fromUtf8("actionStop"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/player_stop.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionStop->setIcon(icon1);
        actionSetup = new QAction(Bandmap);
        actionSetup->setObjectName(QString::fromUtf8("actionSetup"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/stock_properties.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSetup->setIcon(icon2);
        actionQuit = new QAction(Bandmap);
        actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/exit.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionQuit->setIcon(icon3);
        centralwidget = new QWidget(Bandmap);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        sizePolicy.setHeightForWidth(centralwidget->sizePolicy().hasHeightForWidth());
        centralwidget->setSizePolicy(sizePolicy);
        centralwidget->setMinimumSize(QSize(350, 150));
        centralwidget->setMaximumSize(QSize(805, 1400));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setSpacing(1);
        verticalLayout->setContentsMargins(2, 2, 2, 2);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(1);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        display = new BandmapDisplay(centralwidget);
        display->setObjectName(QString::fromUtf8("display"));
        sizePolicy.setHeightForWidth(display->sizePolicy().hasHeightForWidth());
        display->setSizePolicy(sizePolicy);
        display->setMinimumSize(QSize(75, 95));
        display->setMaximumSize(QSize(800, 1390));

        horizontalLayout->addWidget(display);

        FreqLabel = new QLabel(centralwidget);
        FreqLabel->setObjectName(QString::fromUtf8("FreqLabel"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(FreqLabel->sizePolicy().hasHeightForWidth());
        FreqLabel->setSizePolicy(sizePolicy1);
        FreqLabel->setMinimumSize(QSize(35, 95));
        FreqLabel->setMaximumSize(QSize(35, 1390));

        horizontalLayout->addWidget(FreqLabel);

        CallLabel = new QLabel(centralwidget);
        CallLabel->setObjectName(QString::fromUtf8("CallLabel"));
        CallLabel->setMinimumSize(QSize(110, 95));
        CallLabel->setMaximumSize(QSize(110, 1390));

        horizontalLayout->addWidget(CallLabel);


        verticalLayout->addLayout(horizontalLayout);

        Bandmap->setCentralWidget(centralwidget);
        toolBar = new QToolBar(Bandmap);
        toolBar->setObjectName(QString::fromUtf8("toolBar"));
        toolBar->setMinimumSize(QSize(0, 25));
        toolBar->setMaximumSize(QSize(16777215, 25));
        Bandmap->addToolBar(Qt::TopToolBarArea, toolBar);

        toolBar->addAction(actionRun);
        toolBar->addAction(actionStop);
        toolBar->addAction(actionSetup);
        toolBar->addAction(actionQuit);

        retranslateUi(Bandmap);

        QMetaObject::connectSlotsByName(Bandmap);
    } // setupUi

    void retranslateUi(QMainWindow *Bandmap)
    {
        Bandmap->setWindowTitle(QApplication::translate("Bandmap", "so2sdr-bandmap", 0, QApplication::UnicodeUTF8));
        actionRun->setText(QApplication::translate("Bandmap", "Run", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionRun->setToolTip(QApplication::translate("Bandmap", "Start bandmap", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionStop->setText(QApplication::translate("Bandmap", "Stop", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionStop->setToolTip(QApplication::translate("Bandmap", "Stop bandmap", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionSetup->setText(QApplication::translate("Bandmap", "setup", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionSetup->setToolTip(QApplication::translate("Bandmap", "Configure bandmap", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionQuit->setText(QApplication::translate("Bandmap", "quit", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionQuit->setToolTip(QApplication::translate("Bandmap", "Exit application", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        FreqLabel->setText(QString());
        CallLabel->setText(QString());
        toolBar->setWindowTitle(QApplication::translate("Bandmap", "toolBar", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Bandmap: public Ui_Bandmap {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BANDMAP_H
