/*! Copyright 2010-2025 R. Torsten Clay N4OGW

   This file is part of so2sdr.

    so2sdr is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    so2sdr is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with so2sdr.  If not, see <http://www.gnu.org/licenses/>.

 */
#include "helpdialog.h"
#include "utils.h"
#include <QCommonStyle>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QStyle>
#include <QTextStream>

HelpDialog::HelpDialog(QString fileName, QWidget *parent) : QDialog(parent) {
  setupUi(this);
  QCommonStyle style;
  connect(pushButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(homeButton, SIGNAL(clicked()), this, SLOT(home()));
  homeButton->setIcon(style.standardIcon(QStyle::SP_ArrowUp));
  connect(backButton, SIGNAL(clicked()), HelpTextEdit, SLOT(backward()));
  backButton->setIcon(style.standardIcon(QStyle::SP_ArrowBack));
  connect(forwardButton, SIGNAL(clicked()), HelpTextEdit, SLOT(forward()));
  forwardButton->setIcon(style.standardIcon(QStyle::SP_ArrowForward));
  HelpTextEdit->setSearchPaths(
      QStringList(dataDirectory() + "so2sdr-bandmap-help/"));
  QFile file(fileName);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream ts(&file);
    HelpTextEdit->setText(ts.readAll());
  }
  file.close();
}

void HelpDialog::home() { HelpTextEdit->scrollToAnchor("top"); }
