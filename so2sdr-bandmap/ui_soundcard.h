/********************************************************************************
** Form generated from reading UI file 'soundcard.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SOUNDCARD_H
#define UI_SOUNDCARD_H

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

class Ui_SoundCardSetup
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *label;
    QComboBox *APIComboBox;
    QLabel *label_2;
    QComboBox *SoundCardComboBox;
    QLabel *label_3;
    QComboBox *BitsComboBox;
    QLabel *label_4;
    QLineEdit *OffsetLineEdit;
    QCheckBox *checkBoxSwap;
    QCheckBox *checkBoxIQData;
    QCheckBox *checkBoxIq;
    QLabel *label_5;
    QComboBox *SampleRateComboBox;
    QLabel *label_6;
    QComboBox *fftComboBox;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *SoundCardSetup)
    {
        if (SoundCardSetup->objectName().isEmpty())
            SoundCardSetup->setObjectName(QString::fromUtf8("SoundCardSetup"));
        SoundCardSetup->resize(320, 300);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(SoundCardSetup->sizePolicy().hasHeightForWidth());
        SoundCardSetup->setSizePolicy(sizePolicy);
        SoundCardSetup->setMinimumSize(QSize(320, 300));
        SoundCardSetup->setMaximumSize(QSize(320, 300));
        SoundCardSetup->setModal(true);
        verticalLayout = new QVBoxLayout(SoundCardSetup);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        formLayout = new QFormLayout();
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        label = new QLabel(SoundCardSetup);
        label->setObjectName(QString::fromUtf8("label"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        APIComboBox = new QComboBox(SoundCardSetup);
        APIComboBox->setObjectName(QString::fromUtf8("APIComboBox"));

        formLayout->setWidget(0, QFormLayout::FieldRole, APIComboBox);

        label_2 = new QLabel(SoundCardSetup);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_2);

        SoundCardComboBox = new QComboBox(SoundCardSetup);
        SoundCardComboBox->setObjectName(QString::fromUtf8("SoundCardComboBox"));

        formLayout->setWidget(1, QFormLayout::FieldRole, SoundCardComboBox);

        label_3 = new QLabel(SoundCardSetup);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        formLayout->setWidget(4, QFormLayout::LabelRole, label_3);

        BitsComboBox = new QComboBox(SoundCardSetup);
        BitsComboBox->setObjectName(QString::fromUtf8("BitsComboBox"));

        formLayout->setWidget(4, QFormLayout::FieldRole, BitsComboBox);

        label_4 = new QLabel(SoundCardSetup);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        formLayout->setWidget(5, QFormLayout::LabelRole, label_4);

        OffsetLineEdit = new QLineEdit(SoundCardSetup);
        OffsetLineEdit->setObjectName(QString::fromUtf8("OffsetLineEdit"));

        formLayout->setWidget(5, QFormLayout::FieldRole, OffsetLineEdit);

        checkBoxSwap = new QCheckBox(SoundCardSetup);
        checkBoxSwap->setObjectName(QString::fromUtf8("checkBoxSwap"));

        formLayout->setWidget(6, QFormLayout::FieldRole, checkBoxSwap);

        checkBoxIQData = new QCheckBox(SoundCardSetup);
        checkBoxIQData->setObjectName(QString::fromUtf8("checkBoxIQData"));

        formLayout->setWidget(8, QFormLayout::FieldRole, checkBoxIQData);

        checkBoxIq = new QCheckBox(SoundCardSetup);
        checkBoxIq->setObjectName(QString::fromUtf8("checkBoxIq"));

        formLayout->setWidget(7, QFormLayout::FieldRole, checkBoxIq);

        label_5 = new QLabel(SoundCardSetup);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        formLayout->setWidget(3, QFormLayout::LabelRole, label_5);

        SampleRateComboBox = new QComboBox(SoundCardSetup);
        SampleRateComboBox->setObjectName(QString::fromUtf8("SampleRateComboBox"));

        formLayout->setWidget(3, QFormLayout::FieldRole, SampleRateComboBox);

        label_6 = new QLabel(SoundCardSetup);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        formLayout->setWidget(2, QFormLayout::LabelRole, label_6);

        fftComboBox = new QComboBox(SoundCardSetup);
        fftComboBox->setObjectName(QString::fromUtf8("fftComboBox"));

        formLayout->setWidget(2, QFormLayout::FieldRole, fftComboBox);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(SoundCardSetup);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);

        QWidget::setTabOrder(APIComboBox, SoundCardComboBox);
        QWidget::setTabOrder(SoundCardComboBox, fftComboBox);
        QWidget::setTabOrder(fftComboBox, SampleRateComboBox);
        QWidget::setTabOrder(SampleRateComboBox, BitsComboBox);
        QWidget::setTabOrder(BitsComboBox, OffsetLineEdit);
        QWidget::setTabOrder(OffsetLineEdit, checkBoxSwap);
        QWidget::setTabOrder(checkBoxSwap, checkBoxIq);
        QWidget::setTabOrder(checkBoxIq, checkBoxIQData);
        QWidget::setTabOrder(checkBoxIQData, buttonBox);

        retranslateUi(SoundCardSetup);
        QObject::connect(buttonBox, SIGNAL(accepted()), SoundCardSetup, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), SoundCardSetup, SLOT(reject()));

        QMetaObject::connectSlotsByName(SoundCardSetup);
    } // setupUi

    void retranslateUi(QDialog *SoundCardSetup)
    {
        SoundCardSetup->setWindowTitle(QApplication::translate("SoundCardSetup", "Sound Card Setup", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("SoundCardSetup", "Type", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        APIComboBox->setToolTip(QApplication::translate("SoundCardSetup", "Sound interface type. Significant only on Windows.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_2->setText(QApplication::translate("SoundCardSetup", "Device", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        SoundCardComboBox->setToolTip(QApplication::translate("SoundCardSetup", "Soundcard device. Devices that pass basic checks are marked with check.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_3->setText(QApplication::translate("SoundCardSetup", "Bits", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        BitsComboBox->setToolTip(QApplication::translate("SoundCardSetup", "Bits in each sound card sample.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_4->setText(QApplication::translate("SoundCardSetup", "IF offset (Hz)", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        OffsetLineEdit->setToolTip(QApplication::translate("SoundCardSetup", "IF offset: difference in Hz from SDR center frequency and center of display.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        checkBoxSwap->setToolTip(QApplication::translate("SoundCardSetup", "Invert I/Q channels. Use if spectrum is inverted.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkBoxSwap->setText(QApplication::translate("SoundCardSetup", "swap IQ", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        checkBoxIQData->setToolTip(QApplication::translate("SoundCardSetup", "Measure strong signals to correct IQ balance.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkBoxIQData->setText(QApplication::translate("SoundCardSetup", "Collect IQ correction data", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        checkBoxIq->setToolTip(QApplication::translate("SoundCardSetup", "Correct I/Q imbalance", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkBoxIq->setText(QApplication::translate("SoundCardSetup", "IQ correction", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("SoundCardSetup", "Sample Rate", 0, QApplication::UnicodeUTF8));
        SampleRateComboBox->clear();
        SampleRateComboBox->insertItems(0, QStringList()
         << QApplication::translate("SoundCardSetup", "48000", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SoundCardSetup", "96000", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SoundCardSetup", "192000", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        SampleRateComboBox->setToolTip(QApplication::translate("SoundCardSetup", "Sound card sample rate in Hz.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_6->setText(QApplication::translate("SoundCardSetup", "Speed", 0, QApplication::UnicodeUTF8));
        fftComboBox->clear();
        fftComboBox->insertItems(0, QStringList()
         << QApplication::translate("SoundCardSetup", "1", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SoundCardSetup", "1/2", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SoundCardSetup", "1/4", 0, QApplication::UnicodeUTF8)
        );
    } // retranslateUi

};

namespace Ui {
    class SoundCardSetup: public Ui_SoundCardSetup {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SOUNDCARD_H
