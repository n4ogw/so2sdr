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
#ifndef BANDOFFSETSETUP_H
#define BANDOFFSETSETUP_H

#include "defines.h"
#include "ui_bandoffsetsetup.h"
#include <QAction>
#include <QDialog>
#include <QMenu>
#include <QModelIndex>
#include <QPoint>
#include <QSettings>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

class ComboBoxItemDelegate : public QStyledItemDelegate {
  Q_OBJECT
public:
  ComboBoxItemDelegate(QObject *parent = nullptr);
  ~ComboBoxItemDelegate();
  int bandNameToIndex(const QString &name);

protected:
  virtual QWidget *createEditor(QWidget *parent,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const;
  virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
  virtual void setModelData(QWidget *editor, QAbstractItemModel *model,
                            const QModelIndex &index) const;
};

class BandOffsetSetup : public QDialog, public Ui::bandOffsetSetup {
  Q_OBJECT

public:
  BandOffsetSetup(QSettings &s, SdrType stype, QWidget *parent = nullptr);
  ~BandOffsetSetup();
  bool hasOffset(int band) const;
  bool invert(int band) const;
  int offset(int band) const;

private slots:
  void addRow();
  void removeRow();
  void updateSettings();

private:
  SdrType sdr;
  QSettings &settings;
  QStandardItemModel *model;
  ComboBoxItemDelegate *delegate;

  void updateFromSettings();
};

#endif // BANDOFFSETSETUP_H
