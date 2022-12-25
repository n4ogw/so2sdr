/*! Copyright 2010-2023 R. Torsten Clay N4OGW

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
#include "bandoffsetsetup.h"
#include "defines.h"
#include <QApplication>
#include <QComboBox>
#include <QCursor>
#include <QDebug>
#include <QModelIndexList>
#include <QStandardItem>
#include <QString>

BandOffsetSetup::BandOffsetSetup(QSettings &s, SdrType stype, uiSize sizes,
                                 QWidget *parent)
    : QDialog(parent), settings(s) {
  setupUi(this);
  tableView->setFixedWidth(qRound(sizes.width * 45));
  adjustSize();
  setFixedSize(size());
  sdr = stype;
  model = new QStandardItemModel(0, 3, this);
  model->setHorizontalHeaderItem(0, new QStandardItem(QString("Band")));
  model->setHorizontalHeaderItem(1, new QStandardItem(QString("offset (Hz)")));
  model->setHorizontalHeaderItem(2, new QStandardItem(QString("Swap IQ")));
  delegate = new ComboBoxItemDelegate(tableView);
  tableView->setItemDelegate(delegate);
  tableView->setModel(model);
  updateFromSettings();
  connect(deletePushButton, SIGNAL(clicked()), this, SLOT(removeRow()));
  connect(addPushButton, SIGNAL(clicked()), this, SLOT(addRow()));
  connect(this, SIGNAL(accepted()), this, SLOT(updateSettings()));
}

BandOffsetSetup::~BandOffsetSetup() {
  delete delegate;
  delete model;
}

/*! remove selected rows in table
 */
void BandOffsetSetup::removeRow() {
  QModelIndexList selected = tableView->selectionModel()->selectedRows();
  int countRow = selected.count();
  for (int i = countRow; i > 0; i--) {
    model->removeRow(selected.at(i - 1).row(), QModelIndex());
  }
}

void BandOffsetSetup::addRow() {

  QList<QStandardItem *> itemList;
  itemList.clear();
  QStandardItem *item1 = new QStandardItem("band");
  itemList.append(item1);
  QStandardItem *item2 = new QStandardItem("0");
  itemList.append(item2);
  QStandardItem *item3 = new QStandardItem("");
  item3->setCheckable(true);
  itemList.append(item3);
  model->appendRow(itemList);
}

bool BandOffsetSetup::hasOffset(int band) const {
  for (int i = 0; i < model->rowCount(); i++) {
    if (delegate->bandNameToIndex(
            model->item(i, 0)->data(Qt::DisplayRole).toString()) == band)
      return true;
  }
  return false;
}

int BandOffsetSetup::offset(int band) const {
  for (int i = 0; i < model->rowCount(); i++) {
    if (delegate->bandNameToIndex(
            model->item(i, 0)->data(Qt::DisplayRole).toString()) == band)
      return model->item(i, 1)->data(Qt::DisplayRole).toInt();
  }
  return 0;
}

bool BandOffsetSetup::invert(int band) const {
  for (int i = 0; i < model->rowCount(); i++) {
    if (delegate->bandNameToIndex(
            model->item(i, 0)->data(Qt::DisplayRole).toString()) == band) {
      if (model->item(i, 2)->checkState() == Qt::Checked) {
        return true;
      } else {
        return false;
      }
    }
  }
  return false;
}

void BandOffsetSetup::updateFromSettings() {
  int sz = 0;
  switch (sdr) {
  case afedri_t:
    sz = settings.beginReadArray("custom_offset_afedri");
    break;
  case soundcard_t:
    sz = settings.beginReadArray("custom_offset_soundcard");
    break;
  case network_t:
    sz = settings.beginReadArray("custom_offset_netsdr");
    break;
  case rtl_t:
    sz = settings.beginReadArray("custom_offset_rtl");
    break;
  }
  if (!sz) {
    settings.endArray();
    return;
  }
  for (int i = 0; i < sz; i++) {
    settings.setArrayIndex(i);
    QList<QStandardItem *> itemList;
    itemList.clear();
    QStandardItem *item1 =
        new QStandardItem(bandName[settings.value("band", 0).toInt()]);
    itemList.append(item1);
    QStandardItem *item2 =
        new QStandardItem(settings.value("offset", 0).toString());
    itemList.append(item2);
    QStandardItem *item3 = new QStandardItem("");
    item3->setCheckable(true);
    if (settings.value("invert", false).toBool()) {
      item3->setCheckState(Qt::Checked);
    } else {
      item3->setCheckState(Qt::Unchecked);
    }
    itemList.append(item3);
    model->appendRow(itemList);
  }
  settings.endArray();
}

void BandOffsetSetup::updateSettings() {
  if (!model->rowCount())
    return;
  switch (sdr) {
  case afedri_t:
    settings.beginWriteArray("custom_offset_afedri");
    break;
  case soundcard_t:
    settings.beginWriteArray("custom_offset_soundcard");
    break;
  case network_t:
    settings.beginWriteArray("custom_offset_netsdr");
    break;
  case rtl_t:
    settings.beginWriteArray("custom_offset_rtl");
    break;
  }
  for (int i = 0; i < model->rowCount(); i++) {
    settings.setArrayIndex(i);
    settings.setValue("band",
                      delegate->bandNameToIndex(
                          model->item(i, 0)->data(Qt::DisplayRole).toString()));
    settings.setValue("offset",
                      model->item(i, 1)->data(Qt::DisplayRole).toInt());
    if (model->item(i, 2)->checkState() == Qt::Checked)
      settings.setValue("invert", true);
    else {
      settings.setValue("invert", false);
    }
  }
  settings.endArray();
}

ComboBoxItemDelegate::ComboBoxItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {}

ComboBoxItemDelegate::~ComboBoxItemDelegate() {}

int ComboBoxItemDelegate::bandNameToIndex(QString name) {
  for (int i = 0; i < N_BANDS; i++) {
    if (name == bandName[i])
      return i;
  }
  return 0;
}

QWidget *ComboBoxItemDelegate::createEditor(QWidget *parent,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const {
  // ComboBox ony in column 0
  if (index.column() != 0)
    return QStyledItemDelegate::createEditor(parent, option, index);

  // Create the combobox and populate it
  QComboBox *box = new QComboBox(parent);
  for (int i = 0; i < N_BANDS; i++)
    box->addItem(bandName[i]);
  return box;
}

void ComboBoxItemDelegate::setEditorData(QWidget *editor,
                                         const QModelIndex &index) const {
  if (QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
    // get the index of the text in the combobox that matches the current value
    // of the item
    QString currentText = index.data(Qt::EditRole).toString();
    int cbIndex = cb->findText(currentText);
    // if it is valid, adjust the combobox
    if (cbIndex >= 0)
      cb->setCurrentIndex(cbIndex);
  } else {
    QStyledItemDelegate::setEditorData(editor, index);
  }
}

void ComboBoxItemDelegate::setModelData(QWidget *editor,
                                        QAbstractItemModel *model,
                                        const QModelIndex &index) const {
  if (QComboBox *cb = qobject_cast<QComboBox *>(editor))
    // save the current text of the combo box as the current value of the item
    model->setData(index, cb->currentText(), Qt::EditRole);
  else
    QStyledItemDelegate::setModelData(editor, model, index);
}
