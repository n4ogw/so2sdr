/********************************************************************************
** Form generated from reading UI file 'afedrisetup.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_AFEDRISETUP_H
#define UI_AFEDRISETUP_H

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
#include <QtGui/QRadioButton>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_afedriSetup
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *tcpipaddressLineEdit;
    QLabel *label_11;
    QLineEdit *tcpportLineEdit;
    QLabel *label_10;
    QLineEdit *udpportLineEdit;
    QLabel *label_9;
    QComboBox *multiChannelComboBox;
    QLabel *label_4;
    QComboBox *channelComboBox;
    QLabel *label_2;
    QLineEdit *lineEditFreq1;
    QLabel *label_5;
    QLineEdit *lineEditFreq2;
    QLabel *label_6;
    QLineEdit *lineEditFreq3;
    QLabel *label_7;
    QLineEdit *lineEditFreq4;
    QLabel *label_3;
    QLineEdit *lineEditOffset;
    QLabel *label_13;
    QLineEdit *sampleFreqLineEdit;
    QCheckBox *checkBoxSwap;
    QLabel *label_8;
    QRadioButton *broadcastMasterButton;
    QRadioButton *broadcastSlaveButton;
    QRadioButton *broadcastOffButton;
    QLabel *label_12;
    QComboBox *fftComboBox;
    QDialogButtonBox *buttonBox;
    QButtonGroup *broadcastButtonGroup;

    void setupUi(QDialog *afedriSetup)
    {
        if (afedriSetup->objectName().isEmpty())
            afedriSetup->setObjectName(QString::fromUtf8("afedriSetup"));
        afedriSetup->setWindowModality(Qt::WindowModal);
        afedriSetup->resize(292, 510);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(afedriSetup->sizePolicy().hasHeightForWidth());
        afedriSetup->setSizePolicy(sizePolicy);
        afedriSetup->setMinimumSize(QSize(292, 510));
        afedriSetup->setMaximumSize(QSize(292, 510));
        afedriSetup->setModal(true);
        verticalLayout = new QVBoxLayout(afedriSetup);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        formLayout = new QFormLayout();
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        label = new QLabel(afedriSetup);
        label->setObjectName(QString::fromUtf8("label"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        tcpipaddressLineEdit = new QLineEdit(afedriSetup);
        tcpipaddressLineEdit->setObjectName(QString::fromUtf8("tcpipaddressLineEdit"));

        formLayout->setWidget(0, QFormLayout::FieldRole, tcpipaddressLineEdit);

        label_11 = new QLabel(afedriSetup);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_11);

        tcpportLineEdit = new QLineEdit(afedriSetup);
        tcpportLineEdit->setObjectName(QString::fromUtf8("tcpportLineEdit"));

        formLayout->setWidget(1, QFormLayout::FieldRole, tcpportLineEdit);

        label_10 = new QLabel(afedriSetup);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        formLayout->setWidget(2, QFormLayout::LabelRole, label_10);

        udpportLineEdit = new QLineEdit(afedriSetup);
        udpportLineEdit->setObjectName(QString::fromUtf8("udpportLineEdit"));

        formLayout->setWidget(2, QFormLayout::FieldRole, udpportLineEdit);

        label_9 = new QLabel(afedriSetup);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        formLayout->setWidget(3, QFormLayout::LabelRole, label_9);

        multiChannelComboBox = new QComboBox(afedriSetup);
        multiChannelComboBox->setObjectName(QString::fromUtf8("multiChannelComboBox"));

        formLayout->setWidget(3, QFormLayout::FieldRole, multiChannelComboBox);

        label_4 = new QLabel(afedriSetup);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        formLayout->setWidget(4, QFormLayout::LabelRole, label_4);

        channelComboBox = new QComboBox(afedriSetup);
        channelComboBox->setObjectName(QString::fromUtf8("channelComboBox"));

        formLayout->setWidget(4, QFormLayout::FieldRole, channelComboBox);

        label_2 = new QLabel(afedriSetup);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        formLayout->setWidget(5, QFormLayout::LabelRole, label_2);

        lineEditFreq1 = new QLineEdit(afedriSetup);
        lineEditFreq1->setObjectName(QString::fromUtf8("lineEditFreq1"));

        formLayout->setWidget(5, QFormLayout::FieldRole, lineEditFreq1);

        label_5 = new QLabel(afedriSetup);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        formLayout->setWidget(6, QFormLayout::LabelRole, label_5);

        lineEditFreq2 = new QLineEdit(afedriSetup);
        lineEditFreq2->setObjectName(QString::fromUtf8("lineEditFreq2"));
        lineEditFreq2->setAcceptDrops(true);

        formLayout->setWidget(6, QFormLayout::FieldRole, lineEditFreq2);

        label_6 = new QLabel(afedriSetup);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        formLayout->setWidget(7, QFormLayout::LabelRole, label_6);

        lineEditFreq3 = new QLineEdit(afedriSetup);
        lineEditFreq3->setObjectName(QString::fromUtf8("lineEditFreq3"));

        formLayout->setWidget(7, QFormLayout::FieldRole, lineEditFreq3);

        label_7 = new QLabel(afedriSetup);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        formLayout->setWidget(8, QFormLayout::LabelRole, label_7);

        lineEditFreq4 = new QLineEdit(afedriSetup);
        lineEditFreq4->setObjectName(QString::fromUtf8("lineEditFreq4"));

        formLayout->setWidget(8, QFormLayout::FieldRole, lineEditFreq4);

        label_3 = new QLabel(afedriSetup);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        formLayout->setWidget(9, QFormLayout::LabelRole, label_3);

        lineEditOffset = new QLineEdit(afedriSetup);
        lineEditOffset->setObjectName(QString::fromUtf8("lineEditOffset"));

        formLayout->setWidget(9, QFormLayout::FieldRole, lineEditOffset);

        label_13 = new QLabel(afedriSetup);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        formLayout->setWidget(11, QFormLayout::LabelRole, label_13);

        sampleFreqLineEdit = new QLineEdit(afedriSetup);
        sampleFreqLineEdit->setObjectName(QString::fromUtf8("sampleFreqLineEdit"));

        formLayout->setWidget(11, QFormLayout::FieldRole, sampleFreqLineEdit);

        checkBoxSwap = new QCheckBox(afedriSetup);
        checkBoxSwap->setObjectName(QString::fromUtf8("checkBoxSwap"));

        formLayout->setWidget(12, QFormLayout::FieldRole, checkBoxSwap);

        label_8 = new QLabel(afedriSetup);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        formLayout->setWidget(13, QFormLayout::LabelRole, label_8);

        broadcastMasterButton = new QRadioButton(afedriSetup);
        broadcastButtonGroup = new QButtonGroup(afedriSetup);
        broadcastButtonGroup->setObjectName(QString::fromUtf8("broadcastButtonGroup"));
        broadcastButtonGroup->addButton(broadcastMasterButton);
        broadcastMasterButton->setObjectName(QString::fromUtf8("broadcastMasterButton"));

        formLayout->setWidget(13, QFormLayout::FieldRole, broadcastMasterButton);

        broadcastSlaveButton = new QRadioButton(afedriSetup);
        broadcastButtonGroup->addButton(broadcastSlaveButton);
        broadcastSlaveButton->setObjectName(QString::fromUtf8("broadcastSlaveButton"));

        formLayout->setWidget(14, QFormLayout::FieldRole, broadcastSlaveButton);

        broadcastOffButton = new QRadioButton(afedriSetup);
        broadcastButtonGroup->addButton(broadcastOffButton);
        broadcastOffButton->setObjectName(QString::fromUtf8("broadcastOffButton"));
        broadcastOffButton->setChecked(true);

        formLayout->setWidget(15, QFormLayout::FieldRole, broadcastOffButton);

        label_12 = new QLabel(afedriSetup);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        formLayout->setWidget(10, QFormLayout::LabelRole, label_12);

        fftComboBox = new QComboBox(afedriSetup);
        fftComboBox->setObjectName(QString::fromUtf8("fftComboBox"));

        formLayout->setWidget(10, QFormLayout::FieldRole, fftComboBox);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(afedriSetup);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);

        QWidget::setTabOrder(tcpipaddressLineEdit, tcpportLineEdit);
        QWidget::setTabOrder(tcpportLineEdit, udpportLineEdit);
        QWidget::setTabOrder(udpportLineEdit, multiChannelComboBox);
        QWidget::setTabOrder(multiChannelComboBox, channelComboBox);
        QWidget::setTabOrder(channelComboBox, lineEditFreq1);
        QWidget::setTabOrder(lineEditFreq1, lineEditFreq2);
        QWidget::setTabOrder(lineEditFreq2, lineEditFreq3);
        QWidget::setTabOrder(lineEditFreq3, lineEditFreq4);
        QWidget::setTabOrder(lineEditFreq4, lineEditOffset);
        QWidget::setTabOrder(lineEditOffset, fftComboBox);
        QWidget::setTabOrder(fftComboBox, sampleFreqLineEdit);
        QWidget::setTabOrder(sampleFreqLineEdit, checkBoxSwap);
        QWidget::setTabOrder(checkBoxSwap, broadcastMasterButton);
        QWidget::setTabOrder(broadcastMasterButton, broadcastSlaveButton);
        QWidget::setTabOrder(broadcastSlaveButton, broadcastOffButton);
        QWidget::setTabOrder(broadcastOffButton, buttonBox);

        retranslateUi(afedriSetup);
        QObject::connect(buttonBox, SIGNAL(accepted()), afedriSetup, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), afedriSetup, SLOT(reject()));

        QMetaObject::connectSlotsByName(afedriSetup);
    } // setupUi

    void retranslateUi(QDialog *afedriSetup)
    {
        afedriSetup->setWindowTitle(QApplication::translate("afedriSetup", "Afedri Net SDR", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("afedriSetup", "IP Address", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        tcpipaddressLineEdit->setToolTip(QApplication::translate("afedriSetup", "IP address of Afedri SDR", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_11->setText(QApplication::translate("afedriSetup", "TCP port", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        tcpportLineEdit->setToolTip(QApplication::translate("afedriSetup", "TCP port number. Default often 50000.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_10->setText(QApplication::translate("afedriSetup", "UDP port", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        udpportLineEdit->setToolTip(QApplication::translate("afedriSetup", "UDP port number. Default often 50000.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_9->setText(QApplication::translate("afedriSetup", "Multichannel", 0, QApplication::UnicodeUTF8));
        multiChannelComboBox->clear();
        multiChannelComboBox->insertItems(0, QStringList()
         << QApplication::translate("afedriSetup", "Single", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("afedriSetup", "Dual", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("afedriSetup", "Quad", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        multiChannelComboBox->setToolTip(QApplication::translate("afedriSetup", "Allows dual or quad reception if supported by Afedri hardware.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_4->setText(QApplication::translate("afedriSetup", "Channel", 0, QApplication::UnicodeUTF8));
        channelComboBox->clear();
        channelComboBox->insertItems(0, QStringList()
         << QApplication::translate("afedriSetup", "1", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("afedriSetup", "2", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("afedriSetup", "3", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("afedriSetup", "4", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        channelComboBox->setToolTip(QApplication::translate("afedriSetup", "Channel number. Channels above 1 require dual or quad hardware.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_2->setText(QApplication::translate("afedriSetup", "Freq. 1 (Hz)", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        lineEditFreq1->setToolTip(QApplication::translate("afedriSetup", "Center frequency for SDR", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_5->setText(QApplication::translate("afedriSetup", "Freq. 2 (Hz)", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        lineEditFreq2->setToolTip(QApplication::translate("afedriSetup", "Center frequency, second receiver.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_6->setText(QApplication::translate("afedriSetup", "Freq. 3 (Hz)", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        lineEditFreq3->setToolTip(QApplication::translate("afedriSetup", "Center frequency, third receiver", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_7->setText(QApplication::translate("afedriSetup", "Freq. 4 (Hz)", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        lineEditFreq4->setToolTip(QApplication::translate("afedriSetup", "Center frequency, fourth receiver.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_3->setText(QApplication::translate("afedriSetup", "IF Offset (Hz)", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        lineEditOffset->setToolTip(QApplication::translate("afedriSetup", "IF offset: difference in Hz from SDR center frequency and center of display.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_13->setText(QApplication::translate("afedriSetup", "Sample Rate (Hz)", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        sampleFreqLineEdit->setToolTip(QApplication::translate("afedriSetup", "Sample rate of SDR. Not all values can be set with Afedri, see Afedri docs.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        checkBoxSwap->setToolTip(QApplication::translate("afedriSetup", "Invert I/Q channels. Use if spectrum is inverted.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkBoxSwap->setText(QApplication::translate("afedriSetup", "swap IQ", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("afedriSetup", "Broadcast", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        broadcastMasterButton->setToolTip(QApplication::translate("afedriSetup", "Turn on UDP broadcast option. Master controls the SDR.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        broadcastMasterButton->setText(QApplication::translate("afedriSetup", "Master", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        broadcastSlaveButton->setToolTip(QApplication::translate("afedriSetup", "Connect to UDP broadcast as Slave. Cannot change any SDR parameters. Use for multichannel SDR's.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        broadcastSlaveButton->setText(QApplication::translate("afedriSetup", "Slave", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        broadcastOffButton->setToolTip(QApplication::translate("afedriSetup", "Do not use UDP broadcast protocol.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        broadcastOffButton->setText(QApplication::translate("afedriSetup", "Off", 0, QApplication::UnicodeUTF8));
        label_12->setText(QApplication::translate("afedriSetup", "Speed", 0, QApplication::UnicodeUTF8));
        fftComboBox->clear();
        fftComboBox->insertItems(0, QStringList()
         << QApplication::translate("afedriSetup", "1", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("afedriSetup", "1/2", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("afedriSetup", "1/4", 0, QApplication::UnicodeUTF8)
        );
    } // retranslateUi

};

namespace Ui {
    class afedriSetup: public Ui_afedriSetup {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AFEDRISETUP_H
