/********************************************************************************
** Form generated from reading UI file 'helpdialog.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HELPDIALOG_H
#define UI_HELPDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTextBrowser>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_HelpDialog
{
public:
    QVBoxLayout *verticalLayout;
    QTextBrowser *HelpTextEdit;
    QHBoxLayout *horizontalLayout;
    QPushButton *homeButton;
    QPushButton *backButton;
    QPushButton *forwardButton;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton;

    void setupUi(QDialog *HelpDialog)
    {
        if (HelpDialog->objectName().isEmpty())
            HelpDialog->setObjectName(QString::fromUtf8("HelpDialog"));
        HelpDialog->resize(600, 477);
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(HelpDialog->sizePolicy().hasHeightForWidth());
        HelpDialog->setSizePolicy(sizePolicy);
        HelpDialog->setMinimumSize(QSize(480, 200));
        HelpDialog->setMaximumSize(QSize(1024, 1024));
        verticalLayout = new QVBoxLayout(HelpDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        HelpTextEdit = new QTextBrowser(HelpDialog);
        HelpTextEdit->setObjectName(QString::fromUtf8("HelpTextEdit"));
        QFont font;
        font.setPointSize(10);
        HelpTextEdit->setFont(font);

        verticalLayout->addWidget(HelpTextEdit);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        homeButton = new QPushButton(HelpDialog);
        homeButton->setObjectName(QString::fromUtf8("homeButton"));
        QIcon icon(QIcon::fromTheme(QString::fromUtf8("top")));
        homeButton->setIcon(icon);

        horizontalLayout->addWidget(homeButton);

        backButton = new QPushButton(HelpDialog);
        backButton->setObjectName(QString::fromUtf8("backButton"));
        QIcon icon1(QIcon::fromTheme(QString::fromUtf8("back")));
        backButton->setIcon(icon1);

        horizontalLayout->addWidget(backButton);

        forwardButton = new QPushButton(HelpDialog);
        forwardButton->setObjectName(QString::fromUtf8("forwardButton"));
        QIcon icon2(QIcon::fromTheme(QString::fromUtf8("forward")));
        forwardButton->setIcon(icon2);

        horizontalLayout->addWidget(forwardButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButton = new QPushButton(HelpDialog);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setAutoDefault(true);

        horizontalLayout->addWidget(pushButton);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(HelpDialog);

        QMetaObject::connectSlotsByName(HelpDialog);
    } // setupUi

    void retranslateUi(QDialog *HelpDialog)
    {
        HelpDialog->setWindowTitle(QApplication::translate("HelpDialog", "SO2SDR Help", 0, QApplication::UnicodeUTF8));
        homeButton->setText(QString());
        backButton->setText(QString());
        forwardButton->setText(QString());
        pushButton->setText(QApplication::translate("HelpDialog", "Close", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class HelpDialog: public Ui_HelpDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HELPDIALOG_H
