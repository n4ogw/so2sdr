/********************************************************************************
** Form generated from reading UI file 'sdrdialog.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SDRDIALOG_H
#define UI_SDRDIALOG_H

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
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_SDRDialog
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *label_3;
    QComboBox *comboBoxIDNumber;
    QLabel *label_4;
    QLineEdit *tcpPortLineEdit;
    QLineEdit *udpPortLineEdit;
    QLabel *label;
    QComboBox *comboBoxSdrType;
    QPushButton *configureButton;
    QLabel *label_2;
    QLineEdit *lineEditIntegTime;
    QLabel *label_5;
    QLabel *n1MMUDPLabel;
    QCheckBox *n1mmUdpCheckBox;
    QLabel *label_6;
    QLineEdit *n1mmUdpLineEdit;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *SDRDialog)
    {
        if (SDRDialog->objectName().isEmpty())
            SDRDialog->setObjectName(QString::fromUtf8("SDRDialog"));
        SDRDialog->resize(300, 279);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(SDRDialog->sizePolicy().hasHeightForWidth());
        SDRDialog->setSizePolicy(sizePolicy);
        SDRDialog->setMinimumSize(QSize(300, 275));
        SDRDialog->setMaximumSize(QSize(300, 290));
        SDRDialog->setModal(true);
        verticalLayout = new QVBoxLayout(SDRDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        formLayout = new QFormLayout();
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        label_3 = new QLabel(SDRDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label_3);

        comboBoxIDNumber = new QComboBox(SDRDialog);
        comboBoxIDNumber->setObjectName(QString::fromUtf8("comboBoxIDNumber"));
        comboBoxIDNumber->setMaxCount(2);
        comboBoxIDNumber->setMinimumContentsLength(1);

        formLayout->setWidget(0, QFormLayout::FieldRole, comboBoxIDNumber);

        label_4 = new QLabel(SDRDialog);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_4);

        tcpPortLineEdit = new QLineEdit(SDRDialog);
        tcpPortLineEdit->setObjectName(QString::fromUtf8("tcpPortLineEdit"));

        formLayout->setWidget(1, QFormLayout::FieldRole, tcpPortLineEdit);

        udpPortLineEdit = new QLineEdit(SDRDialog);
        udpPortLineEdit->setObjectName(QString::fromUtf8("udpPortLineEdit"));

        formLayout->setWidget(2, QFormLayout::FieldRole, udpPortLineEdit);

        label = new QLabel(SDRDialog);
        label->setObjectName(QString::fromUtf8("label"));

        formLayout->setWidget(5, QFormLayout::LabelRole, label);

        comboBoxSdrType = new QComboBox(SDRDialog);
        comboBoxSdrType->setObjectName(QString::fromUtf8("comboBoxSdrType"));

        formLayout->setWidget(5, QFormLayout::FieldRole, comboBoxSdrType);

        configureButton = new QPushButton(SDRDialog);
        configureButton->setObjectName(QString::fromUtf8("configureButton"));

        formLayout->setWidget(6, QFormLayout::FieldRole, configureButton);

        label_2 = new QLabel(SDRDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        formLayout->setWidget(7, QFormLayout::LabelRole, label_2);

        lineEditIntegTime = new QLineEdit(SDRDialog);
        lineEditIntegTime->setObjectName(QString::fromUtf8("lineEditIntegTime"));

        formLayout->setWidget(7, QFormLayout::FieldRole, lineEditIntegTime);

        label_5 = new QLabel(SDRDialog);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        formLayout->setWidget(2, QFormLayout::LabelRole, label_5);

        n1MMUDPLabel = new QLabel(SDRDialog);
        n1MMUDPLabel->setObjectName(QString::fromUtf8("n1MMUDPLabel"));

        formLayout->setWidget(3, QFormLayout::LabelRole, n1MMUDPLabel);

        n1mmUdpCheckBox = new QCheckBox(SDRDialog);
        n1mmUdpCheckBox->setObjectName(QString::fromUtf8("n1mmUdpCheckBox"));

        formLayout->setWidget(3, QFormLayout::FieldRole, n1mmUdpCheckBox);

        label_6 = new QLabel(SDRDialog);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        formLayout->setWidget(4, QFormLayout::LabelRole, label_6);

        n1mmUdpLineEdit = new QLineEdit(SDRDialog);
        n1mmUdpLineEdit->setObjectName(QString::fromUtf8("n1mmUdpLineEdit"));

        formLayout->setWidget(4, QFormLayout::FieldRole, n1mmUdpLineEdit);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(SDRDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);

        QWidget::setTabOrder(comboBoxIDNumber, tcpPortLineEdit);
        QWidget::setTabOrder(tcpPortLineEdit, udpPortLineEdit);
        QWidget::setTabOrder(udpPortLineEdit, n1mmUdpCheckBox);
        QWidget::setTabOrder(n1mmUdpCheckBox, n1mmUdpLineEdit);
        QWidget::setTabOrder(n1mmUdpLineEdit, comboBoxSdrType);
        QWidget::setTabOrder(comboBoxSdrType, configureButton);
        QWidget::setTabOrder(configureButton, lineEditIntegTime);
        QWidget::setTabOrder(lineEditIntegTime, buttonBox);

        retranslateUi(SDRDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), SDRDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), SDRDialog, SLOT(reject()));
        QObject::connect(n1mmUdpCheckBox, SIGNAL(toggled(bool)), n1mmUdpLineEdit, SLOT(setEnabled(bool)));

        QMetaObject::connectSlotsByName(SDRDialog);
    } // setupUi

    void retranslateUi(QDialog *SDRDialog)
    {
        SDRDialog->setWindowTitle(QApplication::translate("SDRDialog", "Setup", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("SDRDialog", "Bandmap ID number", 0, QApplication::UnicodeUTF8));
        comboBoxIDNumber->clear();
        comboBoxIDNumber->insertItems(0, QStringList()
         << QApplication::translate("SDRDialog", "1", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SDRDialog", "2", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        comboBoxIDNumber->setToolTip(QApplication::translate("SDRDialog", "Radio number of this bandmap", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_4->setText(QApplication::translate("SDRDialog", "TCP port", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("SDRDialog", "SDR type", 0, QApplication::UnicodeUTF8));
        configureButton->setText(QApplication::translate("SDRDialog", "Configure", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("SDRDialog", "CQ Finder time (sec)", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        lineEditIntegTime->setToolTip(QApplication::translate("SDRDialog", "Length of time required for a channel to be considered empty.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_5->setText(QApplication::translate("SDRDialog", "UDP broadcast port", 0, QApplication::UnicodeUTF8));
        n1MMUDPLabel->setText(QApplication::translate("SDRDialog", "N1MM+ UDP", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("SDRDialog", "port", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class SDRDialog: public Ui_SDRDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SDRDIALOG_H
