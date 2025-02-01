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
#ifndef MENUSTYLE_H
#define MENUSTYLE_H
#include <QProxyStyle>
#include <QStyle>
#include <QStyleHintReturn>
#include <QStyleOption>
#include <QWidget>

// used to prevent menubar from grabbing focus when Alt pressed. See
//
// https://bugreports.qt.io/browse/QTBUG-77355
//
// this workaround is from
//
// https://stackoverflow.com/questions/37020992/qt-prevent-menubar-from-grabbing-focus-after-alt-pressed-on-windows

class MenuStyle : public QProxyStyle {
public:
  int styleHint(StyleHint stylehint, const QStyleOption *opt,
                const QWidget *widget, QStyleHintReturn *returnData) const {
    if (stylehint == QStyle::SH_MenuBar_AltKeyNavigation)
      return 0;

    return QProxyStyle::styleHint(stylehint, opt, widget, returnData);
  }
};

#endif // MENUSTYLE_H
