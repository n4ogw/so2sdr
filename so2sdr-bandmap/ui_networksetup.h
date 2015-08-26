/********************************************************************************
** Form generated from reading UI file 'networksetup.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NETWORKSETUP_H
#define UI_NETWORKSETUP_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFormLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_networkSetup
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *tcpipaddressLineEdit;
    QLabel *label_2;
    QLineEdit *tcpportLineEdit;
    QLabel *label_5;
    QLineEdit *udpportLineEdit;
    QLabel *label_3;
    QLineEdit *lineEditOffset;
    QCheckBox *checkBoxSwap;
    QLabel *label_4;
    QComboBox *fftComboBox;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *networkSetup)
    {
        if (networkSetup->objectName().isEmpty())
            networkSetup->setObjectName(QString::fromUtf8("networkSetup"));
        networkSetup->resize(262, 230);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(networkSetup->sizePolicy().hasHeightForWidth());
        networkSetup->setSizePolicy(sizePolicy);
        networkSetup->setMinimumSize(QSize(262, 230));
        networkSetup->setMaximumSize(QSize(262, 230));
        networkSetup->setModal(true);
        verticalLayout = new QVBoxLayout(networkSetup);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        formLayout = new QFormLayout();
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        label = new QLabel(networkSetup);
        label->setObjectName(QString::fromUtf8("label"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        tcpipaddressLineEdit = new QLineEdit(networkSetup);
        tcpipaddressLineEdit->setObjectName(QString::fromUtf8("tcpipaddressLineEdit"));

        formLayout->setWidget(0, QFormLayout::FieldRole, tcpipaddressLineEdit);

        label_2 = new QLabel(networkSetup);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_2);

        tcpportLineEdit = new QLineEdit(networkSetup);
        tcpportLineEdit->setObjectName(QString::fromUtf8("tcpportLineEdit"));

        formLayout->setWidget(1, QFormLayout::FieldRole, tcpportLineEdit);

        label_5 = new QLabel(networkSetup);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        formLayout->setWidget(2, QFormLayout::LabelRole, label_5);

        udpportLineEdit = new QLineEdit(networkSetup);
        udpportLineEdit->setObjectName(QString::fromUtf8("udpportLineEdit"));

        formLayout->setWidget(2, QFormLayout::FieldRole, udpportLineEdit);

        label_3 = new QLabel(networkSetup);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        formLayout->setWidget(4, QFormLayout::LabelRole, label_3);

        lineEditOffset = new QLineEdit(networkSetup);
        lineEditOffset->setObjectName(QString::fromUtf8("lineEditOffset"));

        formLayout->setWidget(4, QFormLayout::FieldRole, lineEditOffset);

        checkBoxSwap = new QCheckBox(networkSetup);
        checkBoxSwap->setObjectName(QString::fromUtf8("checkBoxSwap"));

        formLayout->setWidget(5, QFormLayout::FieldRole, checkBoxSwap);

        label_4 = new QLabel(networkSetup);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        formLayout->setWidget(3, QFormLayout::LabelRole, label_4);

        fftComboBox = new QComboBox(networkSetup);
        fftComboBox->setObjectName(QString::fromUtf8("fftComboBox"));

        formLayout->setWidget(3, QFormLayout::FieldRole, fftComboBox);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(networkSetup);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);

        QWidget::setTabOrder(tcpipaddressLineEdit, tcpportLineEdit);
        QWidget::setTabOrder(tcpportLineEdit, udpportLineEdit);
        QWidget::setTabOrder(udpportLineEdit, fftComboBox);
        QWidget::setTabOrder(fftComboBox, lineEditOffset);
        QWidget::setTabOrder(lineEditOffset, checkBoxSwap);
        QWidget::setTabOrder(checkBoxSwap, buttonBox);

        retranslateUi(networkSetup);
        QObject::connect(buttonBox, SIGNAL(accepted()), networkSetup, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), networkSetup, SLOT(reject()));

        QMetaObject::connectSlotsByName(networkSetup);
    } // setupUi

    void retranslateUi(QDialog *networkSetup)
    {
        networkSetup->setWindowTitle(QApplication::translate("networkSetup", "Network SDR", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("networkSetup", "IP Address", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        tcpipaddressLineEdit->setToolTip(QApplication::translate("networkSetup", "IP address of SDR", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_2->setText(QApplication::translate("networkSetup", "TCP port", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        tcpportLineEdit->setToolTip(QApplication::translate("networkSetup", "TCP port number", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_5->setText(QApplication::translate("networkSetup", "UDP Port", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        udpportLineEdit->setToolTip(QApplication::translate("networkSetup", "UDP port number", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_3->setText(QApplication::translate("networkSetup", "IF Offset (Hz)", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        lineEditOffset->setToolTip(QApplication::translate("networkSetup", "IF offset: difference in Hz from SDR center frequency and center of display.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        checkBoxSwap->setToolTip(QApplication::translate("networkSetup", "Invert I/Q channels. Use if spectrum is inverted.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkBoxSwap->setText(QApplication::translate("networkSetup", "swap IQ", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("networkSetup", "Speed", 0, QApplication::UnicodeUTF8));
        fftComboBox->clear();
        fftComboBox->insertItems(0, QStringList()
         << QApplication::translate("networkSetup", "1", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("networkSetup", "1/2", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("networkSetup", "1/4", 0, QApplication::UnicodeUTF8)
        );
    } // retranslateUi

};

namespace Ui {
    class networkSetup: public Ui_networkSetup {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NETWORKSETUP_H
