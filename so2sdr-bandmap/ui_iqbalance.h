/********************************************************************************
** Form generated from reading UI file 'iqbalance.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_IQBALANCE_H
#define UI_IQBALANCE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_IQBalance
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QFrame *frame_3;
    QHBoxLayout *horizontalLayout_3;
    QLabel *phasePlotLabel;
    QLabel *phasePlot;
    QFrame *frame_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *gainPlotLabel;
    QLabel *gainPlot;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButtonClose;
    QPushButton *pushButtonRestart;

    void setupUi(QWidget *IQBalance)
    {
        if (IQBalance->objectName().isEmpty())
            IQBalance->setObjectName(QString::fromUtf8("IQBalance"));
        IQBalance->resize(570, 340);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(IQBalance->sizePolicy().hasHeightForWidth());
        IQBalance->setSizePolicy(sizePolicy);
        IQBalance->setMinimumSize(QSize(570, 340));
        IQBalance->setMaximumSize(QSize(570, 340));
        verticalLayout_2 = new QVBoxLayout(IQBalance);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        frame_3 = new QFrame(IQBalance);
        frame_3->setObjectName(QString::fromUtf8("frame_3"));
        QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(frame_3->sizePolicy().hasHeightForWidth());
        frame_3->setSizePolicy(sizePolicy1);
        frame_3->setMinimumSize(QSize(400, 100));
        frame_3->setMaximumSize(QSize(1024, 300));
        frame_3->setFrameShape(QFrame::StyledPanel);
        frame_3->setFrameShadow(QFrame::Raised);
        horizontalLayout_3 = new QHBoxLayout(frame_3);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        phasePlotLabel = new QLabel(frame_3);
        phasePlotLabel->setObjectName(QString::fromUtf8("phasePlotLabel"));
        sizePolicy.setHeightForWidth(phasePlotLabel->sizePolicy().hasHeightForWidth());
        phasePlotLabel->setSizePolicy(sizePolicy);
        phasePlotLabel->setMinimumSize(QSize(10, 120));
        phasePlotLabel->setMaximumSize(QSize(10, 120));

        horizontalLayout_3->addWidget(phasePlotLabel);

        phasePlot = new QLabel(frame_3);
        phasePlot->setObjectName(QString::fromUtf8("phasePlot"));
        sizePolicy.setHeightForWidth(phasePlot->sizePolicy().hasHeightForWidth());
        phasePlot->setSizePolicy(sizePolicy);
        phasePlot->setMinimumSize(QSize(512, 120));
        phasePlot->setMaximumSize(QSize(512, 120));

        horizontalLayout_3->addWidget(phasePlot);


        verticalLayout->addWidget(frame_3);

        frame_2 = new QFrame(IQBalance);
        frame_2->setObjectName(QString::fromUtf8("frame_2"));
        sizePolicy1.setHeightForWidth(frame_2->sizePolicy().hasHeightForWidth());
        frame_2->setSizePolicy(sizePolicy1);
        frame_2->setMinimumSize(QSize(400, 100));
        frame_2->setMaximumSize(QSize(1024, 300));
        frame_2->setFrameShape(QFrame::StyledPanel);
        frame_2->setFrameShadow(QFrame::Raised);
        horizontalLayout_2 = new QHBoxLayout(frame_2);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        gainPlotLabel = new QLabel(frame_2);
        gainPlotLabel->setObjectName(QString::fromUtf8("gainPlotLabel"));
        sizePolicy.setHeightForWidth(gainPlotLabel->sizePolicy().hasHeightForWidth());
        gainPlotLabel->setSizePolicy(sizePolicy);
        gainPlotLabel->setMinimumSize(QSize(10, 120));
        gainPlotLabel->setMaximumSize(QSize(10, 120));

        horizontalLayout_2->addWidget(gainPlotLabel);

        gainPlot = new QLabel(frame_2);
        gainPlot->setObjectName(QString::fromUtf8("gainPlot"));
        sizePolicy.setHeightForWidth(gainPlot->sizePolicy().hasHeightForWidth());
        gainPlot->setSizePolicy(sizePolicy);
        gainPlot->setMinimumSize(QSize(512, 120));
        gainPlot->setMaximumSize(QSize(512, 120));

        horizontalLayout_2->addWidget(gainPlot);


        verticalLayout->addWidget(frame_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setSizeConstraint(QLayout::SetMinimumSize);
        horizontalSpacer = new QSpacerItem(300, 20, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButtonClose = new QPushButton(IQBalance);
        pushButtonClose->setObjectName(QString::fromUtf8("pushButtonClose"));

        horizontalLayout->addWidget(pushButtonClose);

        pushButtonRestart = new QPushButton(IQBalance);
        pushButtonRestart->setObjectName(QString::fromUtf8("pushButtonRestart"));
        QFont font;
        font.setPointSize(10);
        pushButtonRestart->setFont(font);

        horizontalLayout->addWidget(pushButtonRestart);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_2->addLayout(verticalLayout);


        retranslateUi(IQBalance);

        QMetaObject::connectSlotsByName(IQBalance);
    } // setupUi

    void retranslateUi(QWidget *IQBalance)
    {
        IQBalance->setWindowTitle(QApplication::translate("IQBalance", "IQ Balance", 0, QApplication::UnicodeUTF8));
        phasePlotLabel->setText(QString());
        phasePlot->setText(QString());
        gainPlotLabel->setText(QString());
        gainPlot->setText(QString());
        pushButtonClose->setText(QApplication::translate("IQBalance", "Close", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        pushButtonRestart->setToolTip(QApplication::translate("IQBalance", "Clears all collected data", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        pushButtonRestart->setText(QApplication::translate("IQBalance", "Restart", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class IQBalance: public Ui_IQBalance {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IQBALANCE_H
