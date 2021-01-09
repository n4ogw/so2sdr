/*! Copyright 2010-2021 R. Torsten Clay N4OGW

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
#ifndef MULTDISPLAY_H
#define MULTDISPLAY_H

#include <QByteArray>
#include <QList>
#include <QTextEdit>
#include <QWidget>

/*! widget to display multipliers. Has extra features for grid square display
 *
 */
class MultDisplay : public QTextEdit
{
    Q_OBJECT

public:
    MultDisplay(QWidget *parent = nullptr);
    void setGridMode(bool);
    void drawGrids();
    void updateMults();
public slots:
    void setCenterGrid(QByteArray);
    void setMults(QList<QByteArray> list) {mults=list;}
    void setNeededMults(QList<QByteArray> list) {neededMults=list;}
protected:
    bool eventFilter(QObject*, QEvent* e);
private:
    bool gridMode;
    QByteArray centerGrid;
    QList<QByteArray> mults;
    QList<QByteArray> neededMults;
    char centerField1;
    char centerField2;
    char centerNr1;
    char centerNr2;
    char upperLeft[4];
};

#endif // MULTDISPLAY_H
