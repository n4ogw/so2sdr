/*! Copyright 2010-2022 R. Torsten Clay N4OGW

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
#include "wsjtxcalldialog.h"
#include <QLayout>
#include <QObject>
#include <QString>
#include <QAbstractItemView>
#include <QTableView>

WsjtxCallDialog::WsjtxCallDialog(QSettings &s,uiSize sizes, int nrig, QWidget *parent) : QDialog(parent),settings(s)
{
    setupUi(this);
    tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    tableView->verticalHeader()->setDefaultSectionSize(qRound(sizes.height));
    tableView->setSelectionMode(QAbstractItemView::NoSelection);
    tableView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    adjustSize();
    _nrig=nrig;
    setWindowTitle("WSJT-X Radio "+QString::number(_nrig+1));
    hideDupesCheckBox->setChecked(false);
    tableView->verticalHeader()->setVisible(false);
    connect(hideDupesCheckBox,SIGNAL(stateChanged(int)),this,SLOT(hideDupesChanged(int)));
    connect(this,SIGNAL(finished(int)),this,SLOT(dialogFinished(int)));
    reader = new UDPReader(_nrig,s);
    connect(reader,SIGNAL(wsjtxQso(Qso *)),this,SIGNAL(wsjtxQso(Qso *)));
    connect(reader,SIGNAL(dupeCheck(Qso *)),this,SIGNAL(dupeCheck(Qso *)));
    delegate = new wsjtxDelegate();
    tableView->setItemDelegateForColumn(WSJTX_SQL_COL_CALL,delegate);
    tableView->setItemDelegateForColumn(WSJTX_SQL_COL_GRID,delegate);
    tableView->setItemDelegateForColumn(WSJTX_SQL_COL_AGE,delegate);
    tableView->setItemDelegateForColumn(WSJTX_SQL_COL_SNR,delegate);
    tableView->setItemDelegateForColumn(WSJTX_SQL_COL_FREQ,delegate);
    tableView->setItemDelegateForColumn(WSJTX_SQL_COL_RX,delegate);
    tableView->setItemDelegateForColumn(WSJTX_SQL_COL_LAST,delegate);
    tableView->setItemDelegateForColumn(WSJTX_SQL_COL_SEQ,delegate);
    proxy=new QSortFilterProxyModel(this);
    proxy->setSourceModel(reader->tableModel());
    tableView->setModel(proxy);
    tableView->setColumnHidden(WSJTX_SQL_COL_LAST,true);
    tableView->setColumnHidden(WSJTX_SQL_COL_DUPE,true);
    tableView->setColumnHidden(WSJTX_SQL_COL_MULT,true);
    tableView->setColumnHidden(WSJTX_SQL_COL_MSG,true);
    tableView->setColumnHidden(WSJTX_SQL_COL_TIME,true);
    tableView->setColumnHidden(WSJTX_SQL_COL_DT,true);
    tableView->setColumnHidden(WSJTX_SQL_COL_MODE,true);
    tableView->setColumnHidden(WSJTX_SQL_COL_CONF,true);
    tableView->setColumnWidth(WSJTX_SQL_COL_RX,qRound(sizes.width*5));
    tableView->setColumnWidth(WSJTX_SQL_COL_SEQ,qRound(sizes.width*6));
    tableView->setColumnWidth(WSJTX_SQL_COL_AGE,qRound(sizes.width*5));
    tableView->setColumnWidth(WSJTX_SQL_COL_SNR,qRound(sizes.width*6));
    tableView->setColumnWidth(WSJTX_SQL_COL_GRID,qRound(sizes.width*8));
    tableView->setColumnWidth(WSJTX_SQL_COL_FREQ,qRound(sizes.width*8));
    tableView->setSortingEnabled(true);
    connect(tableView,SIGNAL(doubleClicked(const QModelIndex &)),reader,SLOT(callClicked(const QModelIndex &)));
}

WsjtxCallDialog::~WsjtxCallDialog()
{
    delete proxy;
    delete delegate;
    reader->stop();
    delete reader;
}

void WsjtxCallDialog::hideDupesChanged(int i)
{
    switch (i) {
    case Qt::Unchecked:
        settings.setValue(s_wsjtx_hide_dupes[_nrig],true);
        reader->setDupeDisplay(false);
        break;
    case Qt::Checked:
        settings.setValue(s_wsjtx_hide_dupes[_nrig],false);
        reader->setDupeDisplay(true);
        break;
    default:
        break;
    }
}

void WsjtxCallDialog::dialogFinished(int i)
{
    Q_UNUSED(i)
    emit(wsjtxDialog(false));
}

void WsjtxCallDialog::setNrig(int n)
{
    if (n>=0 && n<=1) {
        _nrig=n;
        setWindowTitle("WSJX-X Radio "+QString::number(_nrig+1));
        hideDupesCheckBox->setChecked(settings.value(s_wsjtx_hide_dupes[n],s_wsjtx_hide_dupes_def).toBool());
    }
}

void WsjtxCallDialog::setFreq(double f)
{
    reader->setFreq(f);
}

void WsjtxCallDialog::enable(bool b)
{
    reader->enable(b);
}
