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
#include "multdisplay.h"
#include "centergriddialog.h"
#include <QByteArray>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QTextCursor>
#include <QtMath>

MultDisplay::MultDisplay(QWidget *parent)
    : QTextEdit(parent), centerGrid("EM53") {
  gridMode = false;
  centerField1 = 'E';
  centerField2 = 'M';
  centerNr1 = '5';
  centerNr2 = '3';
  mults.clear();
  neededMults.clear();
  viewport()->setCursor(Qt::ArrowCursor);
  installEventFilter(this);
}

void MultDisplay::setGridMode(bool b) {
  gridMode = b;
  if (b) {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  } else {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  }
}

bool MultDisplay::eventFilter(QObject *o, QEvent *e) {
  Q_UNUSED(o)

  // if in grid mode, right mouse brings up center grid dialog
  if (gridMode && e->type() == QEvent::MouseButtonPress) {
    QMouseEvent *ev = static_cast<QMouseEvent *>(e);
    if (ev->button() == Qt::RightButton) {
      CenterGridDialog *dialog = new CenterGridDialog(this);
      connect(dialog, SIGNAL(grid(QByteArray)), this,
              SLOT(setCenterGrid(QByteArray)));
      dialog->show();
      return true;
    } else if (ev->button() == Qt::LeftButton) {
      // compute new center grid from mouse position
      int x = ev->pos().x();
      int y = ev->pos().y();
      int width = viewport()->width();
      int gridWidth = QWidget::fontMetrics().averageCharWidth() * 5;
      int nw = width / gridWidth;
      int woffset = qCeil((width - nw * gridWidth) /
                          (QWidget::fontMetrics().averageCharWidth() * 2.0));
      x = x - woffset * QWidget::fontMetrics().averageCharWidth();
      x = x / gridWidth;
      y = qRound((static_cast<qreal>(y) - document()->documentMargin()) /
                 QWidget::fontMetrics().height());
      for (int i = 0; i < x; i++) {
        upperLeft[2]++;
        if (upperLeft[2] == 10) {
          upperLeft[2] = 0;
          upperLeft[0]++;
          if (upperLeft[0] == 'R')
            upperLeft[0] = 'A';
        }
      }
      for (int i = 0; i < y; i++) {
        upperLeft[3]--;
        if (upperLeft[3] < 0) {
          upperLeft[3] = 9;
          upperLeft[1]--;
          if (upperLeft[1] < 'A')
            upperLeft[1] = 'R';
        }
      }
      centerField1 = upperLeft[0];
      centerField2 = upperLeft[1];
      centerNr1 = upperLeft[2] + 48;
      centerNr2 = upperLeft[3] + 48;
      centerGrid.clear();
      centerGrid = centerGrid + centerField1 + centerField2 + char(centerNr1) +
                   char(centerNr2);
      drawGrids();
      return true;
    }
  }
  return false;
}

void MultDisplay::setCenterGrid(QByteArray b) {
  char tmp[4];
  b = b.toUpper();
  if (b.at(0) >= 'A' && b.at(0) <= 'R') {
    tmp[0] = b.at(0);
  } else {
    return;
  }
  if (b.at(1) >= 'A' && b.at(1) <= 'R') {
    tmp[1] = b.at(1);
  } else {
    return;
  }
  if (b.at(2) >= '0' && b.at(2) <= '9') {
    tmp[2] = b.at(2);
  } else {
    return;
  }
  if (b.at(3) >= '0' && b.at(3) <= '9') {
    tmp[3] = b.at(3);
  } else {
    return;
  }
  centerField1 = tmp[0];
  centerField2 = tmp[1];
  centerNr1 = tmp[2];
  centerNr2 = tmp[3];
  centerGrid = b;
  drawGrids();
}

void MultDisplay::updateMults() {
  if (gridMode) {
    drawGrids();
  } else {
    QString text;
    text.clear();
    for (int i = 0; i < mults.size(); i++) {
      if (neededMults.contains(mults.at(i))) {
	text = text + "<font color=#AAAAAA>" + mults.at(i) + " ";
      } else {
	text = text + "<font color=#FF0000>" + mults.at(i) + " ";
      }
    }
    setHtml(text);
  }
}

/*! draws grid squares in mult window, centered at centerGrid
 *  worked grids are
 */
void MultDisplay::drawGrids() {
  int height = viewport()->height();
  int width = viewport()->width();
  int gridWidth = QWidget::fontMetrics().averageCharWidth() * 5;
  char nw = width / gridWidth;
  int woffset = qCeil((width - nw * gridWidth) /
                      (QWidget::fontMetrics().averageCharWidth() * 2.0));
  char nh = height / QWidget::fontMetrics().height();

  upperLeft[0] = centerField1;
  upperLeft[1] = centerField2;
  upperLeft[2] = (centerNr1 - 48) - nw / 2;
  while (upperLeft[2] < 0) {
    upperLeft[2] += 10;
    upperLeft[0]--;
    if (upperLeft[0] < 'A')
      upperLeft[0] = 'R';
  }
  upperLeft[3] = (centerNr2 - 48) + nh / 2;
  while (upperLeft[3] >= 10) {
    upperLeft[3] -= 10;
    upperLeft[1]++;
    if (upperLeft[1] > 'R')
      upperLeft[1] = 'A';
  }
  QByteArray text;
  text.clear();
  char tmpf2 = upperLeft[1];
  char tmpr2 = upperLeft[3];
  for (int i = 0; i < nh; i++) {
    char tmpr1 = upperLeft[2];
    char tmpf1 = upperLeft[0];
    for (int k = 0; k < woffset; k++)
      text = text + "&nbsp;";
    for (int j = 0; j < nw; j++) {
      QByteArray grid;
      grid.clear();
      grid = grid + tmpf1 + tmpf2 + char(tmpr1 + 48) + char(tmpr2 + 48);
      QByteArray displayGrid;
      if (neededMults.contains(grid)) {
        displayGrid = "<font color=#AAAAAA>" + grid;
      } else {
        displayGrid = "<font color=#FF0000>" + grid;
      }
      text = text + displayGrid + ' ';
      tmpr1++;
      if (tmpr1 == 10) {
        tmpr1 = 0;
        tmpf1 = tmpf1 + 1;
        if (tmpf1 > 'R')
          tmpf1 = 'A';
      }
    }
    text = text + '\n';
    tmpr2--;
    if (tmpr2 < 0) {
      tmpr2 = 9;
      tmpf2--;
      if (tmpf2 < 'A')
        tmpf2 = 'R';
    }
  }
  setText(text);
}
