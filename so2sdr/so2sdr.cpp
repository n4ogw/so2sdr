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
#include <QByteArray>
#include <QChar>
#include <QColor>
#include <QDateTime>
#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QErrorMessage>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFlags>
#include <QFont>
#include <QFontMetricsF>
#include <QHeaderView>
#include <QKeyEvent>
#include <QList>
#include <QMainWindow>
#include <QMessageBox>
#include <QPalette>
#include <QScreen>
#include <QScrollBar>
#include <QSettings>
#include <QSize>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>
#include <QStyle>
#include <QTimer>
#include <QUrl>

#include "bandmapinterface.h"
#include "cabrillodialog.h"
#include "contestoptdialog.h"
#include "cwdialog.h"
#include "cwmanager.h"
#include "cwmessagedialog.h"
#include "defines.h"
#include "dupesheet.h"
#include "filedownloader.h"
#include "hamlib/rig.h"
#include "helpdialog.h"
#include "history.h"
#include "keyboardhandler.h"
#include "log.h"
#include "master.h"
#include "newcontestdialog.h"
#include "notedialog.h"
#include "qso.h"
#include "radiodialog.h"
#include "sdrdialog.h"
#include "serial.h"
#include "settingsdialog.h"
#include "so2r.h"
#include "so2sdr.h"
#include "ssbmessagedialog.h"
#include "stationdialog.h"
#include "telnet.h"
#include "wsjtxcalldialog.h"

So2sdr::So2sdr(QStringList args, QWidget *parent) : QMainWindow(parent) {
  setupUi(this);
  initPointers();
  initVariables();

  qRegisterMetaType<rmode_t>("rmode_t");
  qRegisterMetaType<pbwidth_t>("pbwidth_t");
  qRegisterMetaType<Qso>("Qso");

  // check to see if user directory exists
  initialized = checkUserDirectory();

  settingsFile = userDirectory() + "/so2sdr.ini";
  // check for optional command argument giving station config file name
  if (args.size() > 0) {
    settingsFile = args[0];
  }
  settings = new QSettings(settingsFile, QSettings::IniFormat);

  errorBox = new QErrorMessage(this);
  errorBox->setModal(true);
  errorBox->setFont(QFont("Sans", 10));

  setWindowIcon(QIcon(dataDirectory() + "/icon48x48.png"));
  setFocusPolicy(Qt::StrongFocus);

  if (!iconValid.load(dataDirectory() + "/check.png")) {
    qDebug("file check.png missing");
  }
  // pointers for each radio
  for (int i = 0; i < NRIG; i++) {
    wpmLineEditPtr[i]->setFocusPolicy(Qt::NoFocus);
  }
  rLabelPtr[0] = new QLabel("<font color=#FF0000>R1:OFF /font>");
  rLabelPtr[1] = new QLabel("<font color=#FF0000>R2:OFF </font>");
  winkeyLabel = new QLabel("<font color=#FF0000>WK:OFF </font>");
  grabLabel = new QLabel("<font color=#0000FF>Grab");
  offPtr = new QLabel("");
  autoCQStatus = new QLabel("");
  duelingCQStatus = new QLabel("");
  toggleStatus = new QLabel("");
  autoSendStatus = new QLabel("");
  twoKeyboardStatus = new QLabel("");
  progress.setMinimum(0);
  progress.setMaximum(100);
  progress.setValue(100);
  scriptProcess = new QProcess();
  scriptProcess->setWorkingDirectory(userDirectory() + "/wav");
  So2sdrStatusBar->addPermanentWidget(offPtr);
  So2sdrStatusBar->addPermanentWidget(grabLabel);
  grabLabel->hide();
  So2sdrStatusBar->addPermanentWidget(autoSendStatus);
  So2sdrStatusBar->addPermanentWidget(duelingCQStatus);
  So2sdrStatusBar->addPermanentWidget(autoCQStatus);
  So2sdrStatusBar->addPermanentWidget(toggleStatus);
  So2sdrStatusBar->addPermanentWidget(twoKeyboardStatus);
  So2sdrStatusBar->addPermanentWidget(rLabelPtr[0]);
  So2sdrStatusBar->addPermanentWidget(rLabelPtr[1]);
  So2sdrStatusBar->addPermanentWidget(winkeyLabel);
  for (int i = 0; i < NRIG; i++) {
    clearWorked(i);
  }
  MultTextEdit->setReadOnly(true);
  MultTextEdit->setTextInteractionFlags(Qt::NoTextInteraction);
  MasterTextEdit->setReadOnly(true);
  MasterTextEdit->setDisabled(true);
  TimeDisplay->setText(
      QDateTime::currentDateTimeUtc().toString("MM-dd hh:mm:ss"));
  updateNrDisplay();
  updateCty();

  // start radio 1; radio 2 will be started after radiodialog mfg and model info
  // is filled out
  cat[0] = new RigSerial(0, settingsFile);

  options = new ContestOptionsDialog(this);
  connect(options, SIGNAL(rescore()), this, SLOT(rescore()));
  connect(options, SIGNAL(updateOffTime()), this, SLOT(updateOffTime()));
  options->hide();
  cabrillo = new CabrilloDialog(this);
  cabrillo->hide();
  station = new StationDialog(*settings, this);
  connect(station, SIGNAL(stationUpdate()), this, SLOT(stationUpdate()));
  station->hide();
  progsettings = new SettingsDialog(*settings, this);
  connect(progsettings, SIGNAL(settingsUpdate()), this, SLOT(settingsUpdate()));
  progsettings->hide();
  cwMessage = new CWMessageDialog(CWType, this);
  cwMessage->hide();
  ssbMessage = new SSBMessageDialog(this);
  connect(ssbMessage, SIGNAL(finished()), this, SLOT(messageFinished()));
  connect(ssbMessage, SIGNAL(sendMsg(QByteArray, bool)), this,
          SLOT(expandMacro(QByteArray, bool)));
  connect(ssbMessage, SIGNAL(recordingStatus(bool)), this,
          SLOT(showRecordingStatus(bool)));
  ssbMessage->hide();
  radios = new RadioDialog(*settings, *cat[0], this);
  radios->hide();

  // move radio 1 to its thread and connect signals
  cat[0]->moveToThread(&catThread[0]);
  connect(&catThread[0], SIGNAL(started()), cat[0], SLOT(run()));
  connect(cat[0], SIGNAL(radioError(const QString &)), errorBox,
          SLOT(showMessage(const QString &)));
  connect(this, SIGNAL(qsyExact1(double)), cat[0], SLOT(qsyExact(double)));
  connect(this, SIGNAL(setRigMode1(rmode_t)), cat[0],
          SLOT(setRigMode(rmode_t)));
  // start radio 2
  cat[1] = new RigSerial(1, settingsFile);
  cat[1]->moveToThread(&catThread[1]);
  connect(&catThread[1], SIGNAL(started()), cat[1], SLOT(run()));
  connect(cat[1], SIGNAL(radioError(const QString &)), errorBox,
          SLOT(showMessage(const QString &)));
  connect(this, SIGNAL(qsyExact2(double)), cat[1], SLOT(qsyExact(double)));
  connect(this, SIGNAL(setRigMode2(rmode_t)), cat[1],
          SLOT(setRigMode(rmode_t)));
  wsjtx[0] = new WsjtxCallDialog(*settings, 0, this);
  wsjtx[1] = new WsjtxCallDialog(*settings, 1, this);
  connect(wsjtx[0], SIGNAL(wsjtxQso(Qso *)), this, SLOT(logWsjtx(Qso *)));
  connect(wsjtx[1], SIGNAL(wsjtxQso(Qso *)), this, SLOT(logWsjtx(Qso *)));
  settings->beginGroup("WsjtxWindow1");
  wsjtx[0]->resize(settings->value("size", QSize(900, 400)).toSize());
  wsjtx[0]->move(settings->value("pos", QPoint(400, 400)).toPoint());
  settings->endGroup();
  settings->beginGroup("WsjtxWindow2");
  wsjtx[1]->resize(settings->value("size", QSize(900, 400)).toSize());
  wsjtx[1]->move(settings->value("pos", QPoint(400, 400)).toPoint());
  settings->endGroup();
  cwDialog = new CWDialog(*settings, this);
  cwDialog->hide();
  QDir::setCurrent(dataDirectory());
  sdr = new SDRDialog(*settings, this);
  sdr->hide();
  bandmap = new BandmapInterface(*settings, this);
  connect(bandmap, SIGNAL(removeCall(double, int)), this,
          SLOT(removeSpotFreq(double, int)));
  connect(bandmap, SIGNAL(qsy1(double)), cat[0], SLOT(qsyExact(double)));
  connect(bandmap, SIGNAL(qsy2(double)), cat[1], SLOT(qsyExact(double)));
  connect(bandmap, SIGNAL(sendMsg(QString)), So2sdrStatusBar,
          SLOT(showMessage(QString)));
  notes = new NoteDialog(this);
  newContest = new NewDialog(this);
  connect(newContest, SIGNAL(newContestError(QString)), errorBox,
          SLOT(showMessage(QString)));
  newContest->readContestList(dataDirectory() + "contest_list.dat");
  newContest->hide();
  connect(newContest, SIGNAL(finished(int)), this, SLOT(setupNewContest(int)));
  RateLabel->setText("Rate=0");
  HourRateLabel->setText("0/hr");
  connect(wpmLineEditPtr[0], SIGNAL(textEdited(QString)), this,
          SLOT(launch_enterCWSpeed0(QString)));
  connect(wpmLineEditPtr[1], SIGNAL(textEdited(QString)), this,
          SLOT(launch_enterCWSpeed1(QString)));
  connect(actionOpen, SIGNAL(triggered()), this, SLOT(openFile()));
  connect(actionNewContest, SIGNAL(triggered()), newContest, SLOT(show()));
  connect(actionWinkey, SIGNAL(triggered()), cwDialog, SLOT(show()));
  connect(actionStation, SIGNAL(triggered()), station, SLOT(show()));
  connect(actionSettings, SIGNAL(triggered()), progsettings, SLOT(show()));
  connect(actionRadios, SIGNAL(triggered()), radios, SLOT(show()));
  connect(actionContestOptions, SIGNAL(triggered()), options, SLOT(show()));
  connect(actionSDR, SIGNAL(triggered()), sdr, SLOT(show()));
  connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));
  connect(actionHelp, SIGNAL(triggered()), this, SLOT(showHelp()));
  connect(lineEditCall1, SIGNAL(textEdited(const QString &)), this,
          SLOT(prefixCheck1(const QString &)));
  connect(lineEditCall2, SIGNAL(textEdited(const QString &)), this,
          SLOT(prefixCheck2(const QString &)));
  connect(lineEditExchange1, SIGNAL(textEdited(const QString &)), this,
          SLOT(exchCheck1(const QString &)));
  connect(lineEditExchange2, SIGNAL(textEdited(const QString &)), this,
          SLOT(exchCheck2(const QString &)));
  connect(options, SIGNAL(accepted()), this, SLOT(updateOptions()));
  connect(actionCW_Messages, SIGNAL(triggered()), cwMessage, SLOT(show()));
  connect(actionSSB_Messages, SIGNAL(triggered()), ssbMessage, SLOT(show()));
  initDupeSheet();
  connect(bandmap, SIGNAL(bandmap1state(bool)), this, SLOT(sendCalls1(bool)));
  connect(bandmap, SIGNAL(bandmap2state(bool)), this, SLOT(sendCalls2(bool)));
  connect(bandmap, SIGNAL(bandmap1state(bool)), bandmapAction1,
          SLOT(setChecked(bool)));
  connect(bandmap, SIGNAL(bandmap2state(bool)), bandmapAction2,
          SLOT(setChecked(bool)));
  connect(bandmapAction1, SIGNAL(triggered(bool)), this,
          SLOT(showBandmap1(bool)));
  connect(bandmapAction2, SIGNAL(triggered(bool)), this,
          SLOT(showBandmap2(bool)));
  connect(grabAction, SIGNAL(triggered(bool)), this, SLOT(setGrab(bool)));
  connect(twokeyboardAction, SIGNAL(triggered(bool)), this,
          SLOT(setTwokeyboard(bool)));
  connect(wsjtxAction1, SIGNAL(triggered(bool)), this, SLOT(showWsjtx1(bool)));
  connect(wsjtxAction2, SIGNAL(triggered(bool)), this, SLOT(showWsjtx2(bool)));
  connect(wsjtx[0], SIGNAL(wsjtxDialog(bool)), wsjtxAction1,
          SLOT(setChecked(bool)));
  connect(wsjtx[1], SIGNAL(wsjtxDialog(bool)), wsjtxAction2,
          SLOT(setChecked(bool)));
  connect(telnetAction, SIGNAL(triggered(bool)), this, SLOT(showTelnet(bool)));

  lineEditCall[0]->installEventFilter(this);
  lineEditCall[1]->installEventFilter(this);
  lineEditExchange[0]->installEventFilter(this);
  lineEditExchange[1]->installEventFilter(this);
  const QObjectList clist = children();
  for (int i = 0; i < clist.size(); i++) {
    clist.at(i)->installEventFilter(this);
  }
  removeEventFilter(cabrillo);
  removeEventFilter(wpmLineEditPtr[0]);
  removeEventFilter(wpmLineEditPtr[1]);

  contestDirectory = QDir::homePath();
  so2r = new So2r(*settings, this, this);
  connect(so2r, SIGNAL(error(QString)), errorBox,
          SLOT(showMessage(const QString &)));
  connect(actionSo2r, SIGNAL(triggered()), so2r, SLOT(showDialog()));
  connect(so2r, SIGNAL(setRX1(const QString &)), RX1,
          SLOT(setStyleSheet(const QString &)));
  connect(so2r, SIGNAL(setRX2(const QString &)), RX2,
          SLOT(setStyleSheet(const QString &)));
  connect(so2r, SIGNAL(setTX1(const QString &)), TX1,
          SLOT(setStyleSheet(const QString &)));
  connect(so2r, SIGNAL(setTX2(const QString &)), TX2,
          SLOT(setStyleSheet(const QString &)));
  connect(so2r, SIGNAL(textSent(const QString &, int)), So2sdrStatusBar,
          SLOT(showMessage(const QString &, int)));
  connect(ssbMessage, SIGNAL(setPtt(int, int)), so2r, SLOT(setPtt(int, int)));
  readStationSettings();
  WPMLineEdit->setReadOnly(true);
  // set background of WPM speed edit box to grey
  QPalette palette(wpmLineEditPtr[0]->palette());
  palette.setColor(QPalette::Base, this->palette().color(QPalette::Window));
  for (int i = 0; i < NRIG; i++) {
    wpmLineEditPtr[i]->setText(QString::number(wpm[i]));
    wpmLineEditPtr[i]->setPalette(palette);
  }
  // help dialog
  QDir::setCurrent(dataDirectory() + "/help");
  help = new HelpDialog("so2sdrhelp.html", this);

  setFontSize();

  // start CW manager
  cw = new CWManager(*settings, this);
  connect(cwDialog, SIGNAL(setType(cwtype)), cw, SLOT(setType(cwtype)));
  connect(cwDialog, SIGNAL(startCw()), this, SLOT(startCw()));
  connect(cw, SIGNAL(winkeyVersion(int)), cwDialog,
          SLOT(setWinkeyVersionLabel(int)));
  connect(cw, SIGNAL(tx(bool, int)), bandmap,
          SLOT(setBandmapTxStatus(bool, int)));
  connect(so2r, SIGNAL(So2rMiniTx(bool, int)), bandmap,
          SLOT(setBandmapTxStatus(bool, int)));
  connect(cwDialog, SIGNAL(startCw()), cw, SLOT(open()));
  connect(radios, SIGNAL(startRadios()), this, SLOT(openRadios()));
  connect(cw, SIGNAL(textSent(const QString &, int)), So2sdrStatusBar,
          SLOT(showMessage(const QString &, int)));
  connect(cw, SIGNAL(cwCanceled()), So2sdrStatusBar, SLOT(clearMessage()));
  connect(cw, SIGNAL(CWError(const QString &)), errorBox,
          SLOT(showMessage(const QString &)));
  connect(cw, SIGNAL(finished()), this, SLOT(messageFinished()));
  connect(so2r, SIGNAL(So2rMiniFinished()), this, SLOT(messageFinished()));
  connect(cw, SIGNAL(so2rMiniLoadbuff(const QByteArray &)), so2r,
          SLOT(so2rMiniLoadbuff(const QByteArray &)));
  connect(cw, SIGNAL(so2rMiniSendCW()), so2r, SLOT(so2rMiniSendCW()));
  connect(cw, SIGNAL(so2rMiniCancelCW()), so2r, SLOT(so2rMiniCancelCW()));
  connect(so2r, SIGNAL(So2rMiniTx(bool, int)), cw,
          SLOT(so2rMiniSetSending(bool, int)));
  connect(cw, SIGNAL(so2rMiniSpeed(int)), so2r, SLOT(so2rMiniSpeed(int)));
  startCw();

  openRadios();
  switchAudio(activeRadio);
  switchTransmit(activeRadio);
  callFocus[activeRadio] = true;
  setEntryFocus(activeRadio);
  for (int i = 0; i < NRIG; i++) {
    lineEditCall[i]->setEnabled(false);
    lineEditExchange[i]->setEnabled(false);
  }
  updateRadioFreq();

  menubar->setNativeMenuBar(false);
  disableUI();
  connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanup()));
  connect(qApp, SIGNAL(aboutToQuit()), cat[0], SLOT(stopSerial()));
  connect(qApp, SIGNAL(aboutToQuit()), cat[1], SLOT(stopSerial()));
  // restore window geometry
  settings->beginGroup("MainWindow");
  resize(settings->value("size", QSize(720, 579)).toSize());
  move(settings->value("pos", QPoint(200, 200)).toPoint());
  settings->endGroup();
  show();
}

So2sdr::~So2sdr() {
  if (history)
    delete history;
  if (log)
    delete log;
  QSqlDatabase::removeDatabase("LOG");
  if (catThread[0].isRunning()) {
    catThread[0].quit();
    catThread[0].wait();
  }
  if (catThread[1].isRunning()) {
    catThread[1].quit();
    catThread[1].wait();
  }
  cat[0]->deleteLater();
  cat[1]->deleteLater();
  stopKeyboardHandler();
  if (kbdHandler[0])
    kbdHandler[0]->deleteLater();
  if (kbdHandler[1])
    kbdHandler[1]->deleteLater();
  delete cabrillo;
  delete radios;
  delete cw;
  delete cwMessage;
  delete ssbMessage;
  delete errorBox;
  delete cwDialog;
  delete station;
  delete sdr;
  delete options;
  delete notes;
  delete newContest;
  delete offPtr;
  delete rLabelPtr[0];
  delete rLabelPtr[1];
  delete winkeyLabel;
  delete grabLabel;
  if (downloader)
    delete downloader;
  if (master)
    delete master;
  delete dupesheet[0];
  delete dupesheet[1];
  if (telnet)
    delete telnet;
  delete bandmap;
  scriptProcess->close();
  delete scriptProcess;
  if (qso[0])
    delete qso[0];
  if (qso[1])
    delete qso[1];
  settings->sync();
  delete settings;
  if (csettings)
    delete csettings;
}

/*!
   add a new qso to the log
*/
void So2sdr::addQso(Qso *qso) {
  log->addQso(qso);
  LogTableView->scrollToBottom();
}

/*! updates things depending on contents of Station dialog
 */
void So2sdr::stationUpdate() {
  // called before contest loaded, callsign changed
  if (!log) {
    setWindowTitle("SO2SDR:" + settings->value(s_call, s_call_def).toString());
    return;
  }

  // zone
  if (log->zoneType() == 0) {
    log->setMyZone(settings->value(s_cqzone, s_cqzone_def).toInt());
  } else {
    log->setMyZone(settings->value(s_ituzone, s_ituzone_def).toInt());
  }

  // callsign changed, with contest loaded
  if (csettings) {
    QString name =
        csettings->value(c_contestname, c_contestname_def).toString().toUpper();
    int indx = fileName.lastIndexOf("/");
    QString tmp = fileName.mid(indx + 1, fileName.size() - indx);
    setWindowTitle(settings->value(s_call, s_call_def).toString() + " : " +
                   tmp + " : " +
                   csettings->value(c_contestname_displayed, name).toString());
  }
  Qso tmp(2);
  tmp.call = settings->value(s_call, s_call_def).toString().toLatin1();
  bool b;
  log->setCountry(log->idPfx(&tmp, b));
  log->setContinent(tmp.continent);
}

/*!
    Update after General Settings changes
 */
void So2sdr::settingsUpdate() {
  setFontSize();

  switchAudio(activeRadio);
  switchTransmit(activeRadio);
  if (autoSend) {
    if (settings->value(s_settings_autosend_mode, s_settings_autosend_mode_def)
                .toInt() == 0 ||
        csettings->value(c_sprintmode, c_sprintmode_def)
            .toBool()) { // semi-auto
      autoSendStatus->setText("<font color=#006699>AutoSend(ESM)</font>");
    } else {
      autoSendStatus->setText(
          "<font color=#006699>AutoSend(" +
          QString::number(
              settings->value(s_settings_autosend, s_settings_autosend_def)
                  .toInt()) +
          ")</font>");
    }
  }

  if (autoCQMode) {
    autoCQStatus->setText(
        "<font color=#5200CC>AutoCQ (" +
        QString::number(
            settings->value(s_settings_cqrepeat, s_settings_cqrepeat_def)
                    .toInt() /
                1000.0,
            'f', 1) +
        "s)</font>");
  }
  wsjtx[0]->enable(
      settings->value(s_wsjtx_enable[0], s_wsjtx_enable_def).toBool());
  wsjtx[1]->enable(
      settings->value(s_wsjtx_enable[1], s_wsjtx_enable_def).toBool());
  wsjtx[0]->replay();
  wsjtx[1]->replay();
}

/*! this will have clean-up code
 */
void So2sdr::cleanup() {
  saveSpots();
  quit();
}

/*!
   called when Winkey started or restarted
 */
void So2sdr::startCw() {
  cw->open();
  cw->setSpeed(wpm[activeRadio]);
  so2r->switchTransmit(activeRadio);
  cw->switchTransmit(activeRadio);
}

/*! change grab status
 */
void So2sdr::setGrab(bool s) {
  if (s) {
    // disable two keyboard if grab enabled
    twokeyboard = false;
    grab = true;
    twoKeyboard();
    grabWidget->setFocus();
    grabWidget->activateWindow();
  } else {
    grab = false;
    twoKeyboard();
  }
}

/*! change two keyboard status
 */
void So2sdr::setTwokeyboard(bool s) {
  twokeyboard = s;
  if (s) {
    // needed to hold window focus
    grabWidget->setFocus();
    grabWidget->activateWindow();
  } else {
    lineEditCall[0]->setStyleSheet("");
    lineEditCall[1]->setStyleSheet("");
  }
  // disable grabbing if two keyboard enabled
  if (s && grab) {
    grab = false;
  }
  twoKeyboard();
}

/*!
 * \brief So2sdr::setEntryFocus
 * Sets focus and grab status to current line edit
 */
void So2sdr::setEntryFocus(int nr) {
  if (callFocus[nr]) {
    lineEditCall[nr]->setFocus();
    lineEditCall[nr]->deselect();
    if (grab || twokeyboard) {
      lineEditCall[nr]->activateWindow();
      raise();
    }
    grabWidget = lineEditCall[nr];
    if (twokeyboard) {
      lineEditCall[nr]->setMyFocus(true);
      lineEditExchange[nr]->setMyFocus(false);
    }
  } else {
    lineEditExchange[nr]->setFocus();
    lineEditExchange[nr]->deselect();
    if (grab || twokeyboard) {
      lineEditExchange[nr]->activateWindow();
      raise();
    }
    grabWidget = lineEditExchange[nr];
    if (twokeyboard) {
      lineEditCall[nr]->setMyFocus(false);
      lineEditExchange[nr]->setMyFocus(true);
    }
  }
}

void So2sdr::quit() {
  for (int i = 0; i < NRIG; i++) {
    if (telnetOn) {
      telnet->close();
      telnetOn = false;
    }
  }
  close();
}

/*! starts radio RigSerial thread running. May be
 * called again if radio parameters change
 */
void So2sdr::openRadios() {
  stopTimers();
  if (catThread[0].isRunning()) {
    catThread[0].quit();
    catThread[0].wait();
  }
  catThread[0].start();
  if (catThread[1].isRunning()) {
    catThread[1].quit();
    catThread[1].wait();
  }
  catThread[1].start();
  startTimers();
  if (settings->value(s_radios_ptt_type[0], s_radios_ptt_type_def).toInt() >
      0) {
    so2r->setPtt(0, 0);
  }
  if (settings->value(s_radios_ptt_type[1], s_radios_ptt_type_def).toInt() >
      0) {
    so2r->setPtt(1, 0);
  }
}

/*!
   Enter a log note
 */
void So2sdr::writeNote() {
  notes->enterNote(fileName, contestDirectory, TimeDisplay->text(), grab);
}

/*!
   turn off UI elements that should not be used until a
   contest has been selected
 */
void So2sdr::disableUI() {
  for (int i = 0; i < NRIG; i++) {
    lineEditCall[i]->setEnabled(false);
    lineEditExchange[i]->setEnabled(false);
  }
  cwMessage->setEnabled(false);
  ssbMessage->setEnabled(false);
  options->setEnabled(false);
  actionCW_Messages->setEnabled(false);
  actionSSB_Messages->setEnabled(false);
  actionContestOptions->setEnabled(false);
  actionSave->setEnabled(false);
  actionADIF->setEnabled(false);
  actionCabrillo->setEnabled(false);
  actionHistory->setEnabled(false);
  actionHistory->setText("Update history from log");
  grabAction->setEnabled(false);
  twokeyboardAction->setEnabled(false);
  actionImport_Cabrillo->setEnabled(false);
  dupesheetAction1->setEnabled(false);
  dupesheetAction2->setEnabled(false);
  telnetAction->setEnabled(false);
  bandmapAction1->setEnabled(false);
  bandmapAction2->setEnabled(false);
  wsjtxAction1->setEnabled(false);
  wsjtxAction2->setEnabled(false);
  uiEnabled = false;
}

/*!
   Enable UI
 */
void So2sdr::enableUI() {
  for (int i = 0; i < NRIG; i++) {
    lineEditCall[i]->setEnabled(true);
    lineEditExchange[i]->setEnabled(false);
  }
  bandmapAction1->setEnabled(true);
  dupesheetAction1->setEnabled(true);
  bandmapAction2->setEnabled(true);
  dupesheetAction2->setEnabled(true);
  cwMessage->setEnabled(true);
  ssbMessage->setEnabled(true);
  actionSSB_Messages->setEnabled(true);
  options->setEnabled(true);
  actionCW_Messages->setEnabled(true);
  actionContestOptions->setEnabled(true);
  actionSave->setEnabled(true);
  actionADIF->setEnabled(true);
  actionCabrillo->setEnabled(true);
  grabAction->setEnabled(true);
  twokeyboardAction->setEnabled(true);
  actionImport_Cabrillo->setEnabled(true);
  telnetAction->setEnabled(true);
  wsjtxAction1->setEnabled(true);
  wsjtxAction2->setEnabled(true);
  uiEnabled = true;
  callFocus[activeRadio] = true;
  setEntryFocus(activeRadio);
}

/*!
   set up new contest
 */
void So2sdr::setupNewContest(int result) {
  if (!result)
    return;
  QString fname = newContest->selectedContest();

  // set an initial name
  QString tmp = contestDirectory + "/" + QString(fname);
  tmp.chop(4); // remove ".cfg"
  tmp = tmp + QDateTime::currentDateTimeUtc().toString("MMddyy") + ".cfg";
  QString newfname;
  newfname.clear();
  newfname = QFileDialog::getSaveFileName(this, "New config file", tmp,
                                          "Config Files (*.cfg)");

  // if user presses cancel
  if (newfname.isEmpty())
    return;

  // add .cfg at end of filename if it is not there
  newfname = newfname.trimmed();
  if (newfname.right(4).toUpper() != ".CFG") {
    newfname = newfname + ".cfg";
  }

  // if file already exists, remove it first
  QFileInfo oldfileInfo(newfname);
  if (oldfileInfo.exists()) {
    // remove cfg file
    QFile tmp(newfname);
    tmp.remove();
    // remove log file if it exists
    QString s = newfname;
    s.chop(4);
    s = s + ".log";
    QFileInfo oldLogfileInfo(s);
    if (oldLogfileInfo.exists()) {
      QFile tmp(s);
      tmp.remove();
    }
  }
  // copy standard config to contest directory
  // check for local copy
  QFileInfo userStdFile(userDirectory() + "/" + fname);
  QFile stdFile;
  if (userStdFile.exists()) {
    stdFile.setFileName(userDirectory() + "/" + fname);
  } else {
    stdFile.setFileName(dataDirectory() + "/" + fname);
  }
  stdFile.copy(newfname);
  fileName = newfname;
  if (setupContest()) {
    // disable option to start another new contest: must close and restart
    // program
    actionNewContest->setDisabled(true);
    actionOpen->setDisabled(true);
  } else {
    return;
  }
  int i = fileName.lastIndexOf("/");
  if (i != -1) {
    // set contest directory based on where user wants to save
    contestDirectory = fileName;
    contestDirectory.truncate(i);
  }
  QDir::setCurrent(contestDirectory);
  // show the contest options dialog, put cursor on sent exchange entry
  options->show();
  options->sent[0]->setFocus();
}

/*!
   update when changes made in Contest Options dialog
 */
void So2sdr::updateOptions() {
  if (csettings->value(c_showmode, c_showmode_def).toBool()) {
    LogTableView->setColumnHidden(SQL_COL_ADIF_MODE, false);
  } else {
    LogTableView->setColumnHidden(SQL_COL_ADIF_MODE, true);
  }
  updateBreakdown();
  updateMults(activeRadio);
  setEntryFocus(activeRadio);
  startMaster();
  if (csettings->value(c_historymode, c_historymode_def).toBool()) {
    history->startHistory();
    if (history->isOpen()) {
      actionHistory->setEnabled(true);
      actionHistory->setText(
          "Update History (" +
          csettings->value(c_historyfile, c_historyfile_def).toString() +
          ") from Log");
    } else {
      actionHistory->setEnabled(false);
      actionHistory->setText("Update History from Log");
    }
  } else {
    actionHistory->setEnabled(false);
    actionHistory->setText("Update History from Log");
  }
}

/*! check to make sure log file matches the current version (has the same number
 * of sql columns). Returns true if yes or if the file does not exist; false if
 * the number of columns does not match.
 */
bool So2sdr::checkLogFileVersion(QString fname) {
  if (fname.isEmpty()) {
    return false;
  }
  fname = fname.remove(".cfg") + ".log";
  QFileInfo info(fname);
  if (!info.exists()) {
    return true;
  }
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "TEST");
  db.setConnectOptions("QSQLITE_OPEN_READONLY");
  db.setDatabaseName(fname);
  if (!db.open()) {
    // can't open database - wrong sqlite version or some other error
    return false;
  }
  // check to make sure number of columns matches
  QSqlQuery query(db);
  query.prepare("SELECT * FROM log");
  query.exec();
  if (query.record().count() != SQL_N_COL) {
    db.close();
    return false;
  }
  db.close();
  return true;
}
/*!
   open contest cfg file and set up program
 */
bool So2sdr::setupContest() {
  QFileInfo info(fileName);
  if (!info.exists())
    return false;

  QDir::setCurrent(contestDirectory);
  bool ok = checkLogFileVersion(fileName);
  QSqlDatabase::removeDatabase("TEST");
  if (!ok) {
    QMessageBox::critical(
        this, "ERROR",
        "Cannot open log file. File may be from an earlier version of SO2SDR "
        "incompatible with this version. You can use File->Import Cabrillo to "
        "read this log.");
    return false;
  }
  csettings = new QSettings(fileName, QSettings::IniFormat);
  if (!QFile::exists(fileName)) {
    delete csettings;
    return false;
  }
  qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

  // construct dialogs that depend on contest ini file settings
  QString cname =
      csettings->value(c_contestname, c_contestname_def).toString().toUpper();
  if (cname.isEmpty()) {
    delete csettings;
    return false;
  }
  cwMessage->initialize(csettings);
  ssbMessage->initialize(csettings, settings);
  log = new Log(*csettings, *settings, this);
  log->setLatLon(station->lat(), station->lon());
  log->selectContest();
  qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
  connect(log, SIGNAL(logEditDone(QSqlRecord, QSqlRecord)), this,
          SLOT(updateSpotlistEdit(QSqlRecord, QSqlRecord)));
  connect(log, SIGNAL(startLogEdit()), this, SLOT(startLogEdit()));
  connect(log, SIGNAL(errorMessage(QString)), errorBox,
          SLOT(showMessage(QString)));
  connect(log, SIGNAL(progressCnt(int)), &progress, SLOT(setValue(int)));
  connect(log, SIGNAL(progressMax(int)), &progress, SLOT(setMaximum(int)));
  connect(log, SIGNAL(multByBandEnabled(bool)), options->MultsByBandCheckBox,
          SLOT(setEnabled(bool)));
  connect(log, SIGNAL(update()), this, SLOT(rescore()));
  connect(log, SIGNAL(update()), So2sdrStatusBar, SLOT(clearMessage()));
  connect(wsjtx[0], SIGNAL(dupeCheck(Qso *)), log,
          SLOT(mobileDupeCheck(Qso *)));
  connect(wsjtx[1], SIGNAL(dupeCheck(Qso *)), log,
          SLOT(mobileDupeCheck(Qso *)));
  log->initializeContest();
  qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

  // make extra exchange fields inactive/hidden. Focus sent exchange entry
  for (int i = log->nExch(); i < 4; i++) {
    options->sent[i]->setEnabled(false);
    options->sent[i]->hide();
    options->sentName[i]->setEnabled(false);
    options->sentName[i]->hide();
    cabrillo->sent[i]->setEnabled(false);
    cabrillo->sent[i]->hide();
  }

  // disable user from changing some specific exchange fields (RST, #). The
  // actual text 'RST' and '#' will be taken from the value in the .cfg file
  for (int i = 0; i < log->nExch(); i++) {
    if (log->exchType(i) == RST || log->exchType(i) == QsoNumber) {
      options->sent[i]->setEnabled(false);
    }
  }
  options->initialize(csettings);
  options->sent[0]->setFocus();
  // setup band totals on screen
  for (int i = 0; i < 6; i++) {
    bandLabel[i]->setText(log->bandLabel(i));
    if (!log->bandLabelEnable(i)) {
      bandQsoLabel[i]->hide();
      multLabel[0][i]->hide();
      multLabel[1][i]->hide();
    }
  }
  history = new History(*csettings, this);
  connect(history, SIGNAL(message(const QString &, int)), So2sdrStatusBar,
          SLOT(showMessage(const QString &, int)));
  connect(log, SIGNAL(addQsoHistory(const Qso *)), history,
          SLOT(addQso(const Qso *)));
  if (csettings->value(c_historymode, c_historymode_def).toBool()) {
    history->startHistory();
    if (history->isOpen()) {
      actionHistory->setEnabled(true);
      actionHistory->setText(
          "Update History (" +
          csettings->value(c_historyfile, c_historyfile_def).toString() +
          ") from Log");
    } else {
      actionHistory->setEnabled(false);
      actionHistory->setText("Update History from Log");
    }
  }
  qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
  connect(log, SIGNAL(clearDupe()), So2sdrStatusBar, SLOT(clearMessage()));
  connect(log, SIGNAL(errorMessage(QString)), errorBox,
          SLOT(showMessage(QString)));
  connect(log, SIGNAL(progressCnt(int)), &progress, SLOT(setValue(int)));
  connect(log, SIGNAL(progressMax(int)), &progress, SLOT(setMaximum(int)));
  station->SunLabel->setText("Sunrise/Sunset: " + log->ctyPtr()->mySunTimes() +
                             " z");
  master = new Master();
  connect(master, SIGNAL(masterError(const QString &)), errorBox,
          SLOT(showMessage(const QString &)));
  startMaster();
  qso[0] = new Qso(log->nExch());
  qso[1] = new Qso(log->nExch());
  for (int i = 0; i < log->nExch(); i++) {
    qso[0]->setExchangeType(i, log->exchType(i));
    qso[1]->setExchangeType(i, log->exchType(i));
  }
  QDir::setCurrent(contestDirectory);
  log->openLogFile(fileName);
  qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

  QString name =
      csettings->value(c_contestname, c_contestname_def).toString().toUpper();
  int indx = fileName.lastIndexOf("/");
  QString tmp = fileName.mid(indx + 1, fileName.size() - indx);
  setWindowTitle(settings->value(s_call, s_call_def).toString() + " : " + tmp +
                 " : " +
                 csettings->value(c_contestname_displayed, name).toString());
  multNameLabel[0]->setText(
      csettings->value(c_mult_name1, c_mult_name1_def).toString());
  multNameLabel[1]->setText(
      csettings->value(c_mult_name2, c_mult_name2_def).toString());
  readExcludeMults();
  for (int i = 0; i < NRIG; i++) {
    lineEditCall[i]->setEnabled(true);
    lineEditExchange[i]->setEnabled(false);
  }
  callFocus[activeRadio] = true;
  setEntryFocus(activeRadio);
  initLogView();
  loadSpots();
  qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
  rescore();
  qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
  connect(actionADIF, SIGNAL(triggered()), this, SLOT(exportADIF()));
  connect(actionCabrillo, SIGNAL(triggered()), this, SLOT(showCabrillo()));
  connect(cabrillo, SIGNAL(accepted()), this, SLOT(exportCabrillo()));
  connect(actionImport_Cabrillo, SIGNAL(triggered()), this,
          SLOT(importCabrillo()));
  connect(actionHistory, SIGNAL(triggered()), this, SLOT(updateHistory()));
  nrSent = log->rowCount() + 1;
  updateNrDisplay();
  updateBreakdown();
  MultTextEdit->setGridMode(log->gridMults());
  MultTextEdit->setCenterGrid(
      settings->value(s_grid, s_grid_def).toByteArray());
  updateMults(activeRadio);
  clearWorked(0);
  clearWorked(1);
  So2sdrStatusBar->showMessage("Read " + fileName, 3000);
  if (csettings->value(c_off_time_enable, c_off_time_enable_def).toBool())
    updateOffTime();

  wsjtx[0]->enable(
      settings->value(s_wsjtx_enable[0], s_wsjtx_enable_def).toBool());
  wsjtx[1]->enable(
      settings->value(s_wsjtx_enable[1], s_wsjtx_enable_def).toBool());
  wsjtx[0]->replay();
  wsjtx[1]->replay();

  // now allow log columns to be resized
  LogTableView->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Interactive);
  enableUI();
  emit contestReady();
  return (true);
}

/*!
   initialize supercheck partial
   slot called when master checkbox clicked

   if file exists in user directory, use that first; otherwise use data
   directory file
 */
void So2sdr::startMaster() {
  if (csettings->value(c_mastermode, c_mastermode_def).toBool()) {
    QDir::setCurrent(userDirectory());
    QString filename =
        csettings->value(c_masterfile, c_masterfile_def).toString();
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
      master->initialize(file);
    } else {
      QDir::setCurrent(dataDirectory());
      QFile file(filename);
      if (file.open(QIODevice::ReadOnly)) {
        master->initialize(file);
      } else {
        errorBox->showMessage("ERROR: can't open file " + filename);
      }
    }
  }
}

/*!
 * \brief So2sdr::updateHistory
 * update the exchange history.
 *
 */
void So2sdr::updateHistory() {
  if (csettings->value(c_historymode, c_historymode_def).toBool()) {
    progress.setLabelText("Updating history");
    log->updateHistory();
  }
}

void So2sdr::exportADIF() {
  if (fileName.isEmpty())
    return;
  QString afname = fileName.remove(".cfg") + ".adi";
  QFile adifFile(afname);

  // check for overwriting existing file
  if (adifFile.exists()) {
    QMessageBox *msg;
    msg = new QMessageBox(this);
    msg->setWindowTitle("Warning");
    msg->setText("File " + afname + " exists.");
    msg->setInformativeText("Overwrite it?");
    msg->setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msg->setDefaultButton(QMessageBox::Yes);
    int ret = msg->exec();
    switch (ret) {
    case QMessageBox::Yes:
      break;
    case QMessageBox::Cancel:
      So2sdrStatusBar->showMessage("ADIF export canceled", 3000);
      msg->deleteLater();
      return;
    default:
      break;
    }
    msg->deleteLater();
  }
  if (!adifFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return;
  }
  if (log->exportADIF(&adifFile)) {
    So2sdrStatusBar->showMessage("Saved ADIF " + afname, 3000);
  } else {
    So2sdrStatusBar->showMessage("error creating ADIF " + afname, 3000);
  }
}

/*! Fill and show Cabrillo entry form

  First check if sent exchange has been entered
 */
void So2sdr::showCabrillo() {
  bool ok = true;
  for (int i = 0; i < log->nExch(); i++) {
    if (options->sent[i]->text().isEmpty()) {
      ok = false;
    }
  }
  if (!ok) {
    QMessageBox msgBox;
    msgBox.setText("Enter sent exchange in Config->Contest Options first.");
    msgBox.exec();
    return;
  }
  cabrillo->initialize(settings, csettings);
  cabrillo->labelClaimedScore->setNum(log->score());
  cabrillo->show();
}

/*! call cabrillo export in log module
 */
void So2sdr::exportCabrillo() {
  QString cfname = fileName;
  cfname = cfname.remove(".cfg") + ".cbr";
  QFile cbrFile(cfname);

  // check for overwriting existing file
  if (cbrFile.exists()) {
    QMessageBox *msg;
    msg = new QMessageBox(this);
    msg->setWindowTitle("Warning");
    msg->setText("File " + cfname + " exists.");
    msg->setInformativeText("Overwrite it?");
    msg->setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msg->setDefaultButton(QMessageBox::Yes);
    int ret = msg->exec();
    switch (ret) {
    case QMessageBox::Yes:
      break;
    case QMessageBox::Cancel:
      So2sdrStatusBar->showMessage("Cabrillo export canceled", 3000);
      msg->deleteLater();
      return;
      break;
    default:
      break;
    }
    msg->deleteLater();
  }
  if (!cbrFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    So2sdrStatusBar->showMessage("Can't write Cabrillo file " + cfname, 3000);
    return;
  }
  cabrillo->writeHeader(&cbrFile, log->score());
  log->exportCabrillo(
      &cbrFile, settings->value(s_call, s_call_def).toString(),
      csettings->value(c_sentexch1, c_sentexch1_def).toString(),
      csettings->value(c_sentexch2, c_sentexch2_def).toString(),
      csettings->value(c_sentexch3, c_sentexch3_def).toString(),
      csettings->value(c_sentexch4, c_sentexch4_def).toString());
  So2sdrStatusBar->showMessage("Saved Cabrillo " + cfname, 3000);
}

/*! import a Cabrillo log
 */
void So2sdr::importCabrillo() {
  QDir::setCurrent(contestDirectory);
  QString cabFile = QFileDialog::getOpenFileName(
      this, tr("Import Cabrillo log"), contestDirectory,
      tr("Cabrillo Files (*.cbr)"));
  if (cabFile.isEmpty())
    return;

  progress.setLabelText("Importing Cabrillo");
  progress.setWindowModality(Qt::WindowModal);
  progress.setMinimumDuration(1000);
  progress.setValue(0);

  log->importCabrillo(cabFile);
  LogTableView->scrollToBottom();
  nrSent = log->rowCount() + 1;
}

/*!
Clear any selected cells in log and remove log editing message from status bar
*/
void So2sdr::clearEditSelection(QWidget *editor) {
  Q_UNUSED(editor)
  So2sdrStatusBar->clearMessage();
  LogTableView->clearSelection();
}

/*!
Show status bar message when log edit starts
 */
void So2sdr::startLogEdit() {
  So2sdrStatusBar->showMessage("EDITING LOG: PRESS ESC to abort");
}

/*!
 after a qso edit, update callsign in list of spots. Also reset cursor.
 */
void So2sdr::updateSpotlistEdit(QSqlRecord origRecord, QSqlRecord r) {
  // update spot list; replace any occurence of this call with the updated one;
  // dupe status will be updated as well
  QByteArray call = origRecord.value(SQL_COL_CALL).toByteArray();
  QByteArray newCall = r.value(SQL_COL_CALL).toByteArray();
  for (int b = 0; b < N_BANDS; b++) {
    for (int i = 0; i < spotList[b].size(); i++) {
      if (call == spotList[b][i].call) {
        double f = spotList[b].at(i).f;
        removeSpot(call, b);
        addSpot(newCall, f);
      }
    }
  }
  if (!grab) {
    lineEditCall[activeRadio]->setFocus();
  }
  LogTableView->clearSelection();
}

/*!
   initialize log view
 */
void So2sdr::initLogView() {
  LogTableView->setShowGrid(true);
  LogTableView->verticalHeader()->hide();
  LogTableView->verticalHeader()->setDefaultAlignment(Qt::AlignLeft);
  LogTableView->verticalHeader()->setSectionsClickable(false);
  LogTableView->verticalHeader()->setDefaultSectionSize(16);
  LogTableView->setModel(log->mod());

  for (int i = 0; i < SQL_N_COL; i++) {
    LogTableView->setItemDelegateForColumn(i, log->delegate());
  }
  LogTableView->setEditTriggers(QAbstractItemView::DoubleClicked);
  LogTableView->setSelectionMode(QAbstractItemView::SingleSelection);
  LogTableView->setSortingEnabled(false);
  for (int i = 0; i < SQL_N_COL; i++) {
    LogTableView->setColumnHidden(i, true);
  }
  LogTableView->setDragEnabled(false);
  LogTableView->removeEventFilter(this);

  // 6 columns shown for all contests: qso #, time, call, freq, mode, valid flag
  LogTableView->setColumnHidden(SQL_COL_NR, false);
  LogTableView->setColumnHidden(SQL_COL_TIME, false);
  LogTableView->setColumnHidden(SQL_COL_CALL, false);
  LogTableView->setColumnHidden(SQL_COL_FREQ, false);
  if (csettings->value(c_showmode, c_showmode_def).toBool()) {
    LogTableView->setColumnHidden(SQL_COL_ADIF_MODE, false);
  }
  LogTableView->setColumnHidden(SQL_COL_VALID, false);

  // set default column widths
  for (int i = 0; i < SQL_N_COL; i++) {
    LogTableView->setColumnWidth(
        i,
        qRound(csettings->value(c_col_width_item, c_col_width_def[i]).toInt() *
               sizes.textWidth));
  }
  // set contest-specific column widths
  // first are sent data fields
  unsigned f = log->sntFieldShown();
  int cnt = 0;
  if (f & 1) {
    LogTableView->setColumnHidden(SQL_COL_SNT1, false);
    LogTableView->setColumnWidth(
        SQL_COL_SNT1, qRound(log->fieldWidth(cnt) * sizes.textWidth));
    cnt++;
  }
  if (f & 2) {
    LogTableView->setColumnHidden(SQL_COL_SNT2, false);
    LogTableView->setColumnWidth(
        SQL_COL_SNT2, qRound(log->fieldWidth(cnt) * sizes.textWidth));
    cnt++;
  }
  if (f & 4) {
    LogTableView->setColumnHidden(SQL_COL_SNT3, false);
    LogTableView->setColumnWidth(
        SQL_COL_SNT3, qRound(log->fieldWidth(cnt) * sizes.textWidth));
    cnt++;
  }
  if (f & 8) {
    LogTableView->setColumnHidden(SQL_COL_SNT4, false);
    LogTableView->setColumnWidth(
        SQL_COL_SNT4, qRound(log->fieldWidth(cnt) * sizes.textWidth));
    cnt++;
  }
  f = log->rcvFieldShown();
  if (f & 1) {
    LogTableView->setColumnHidden(SQL_COL_RCV1, false);
    LogTableView->setColumnWidth(
        SQL_COL_RCV1, qRound(log->fieldWidth(cnt) * sizes.textWidth));
    cnt++;
  }
  if (f & 2) {
    LogTableView->setColumnHidden(SQL_COL_RCV2, false);
    LogTableView->setColumnWidth(
        SQL_COL_RCV2, qRound(log->fieldWidth(cnt) * sizes.textWidth));
    cnt++;
  }
  if (f & 4) {
    LogTableView->setColumnHidden(SQL_COL_RCV3, false);
    LogTableView->setColumnWidth(
        SQL_COL_RCV3, qRound(log->fieldWidth(cnt) * sizes.textWidth));
    cnt++;
  }
  if (f & 8) {
    LogTableView->setColumnHidden(SQL_COL_RCV4, false);
    LogTableView->setColumnWidth(
        SQL_COL_RCV4, qRound(log->fieldWidth(cnt) * sizes.textWidth));
  }
  if (log->showQsoPtsField()) {
    LogTableView->setColumnHidden(SQL_COL_PTS, false);
  }
  for (int i = 0; i < SQL_N_COL; i++) {
    LogTableView->model()->setHeaderData(i, Qt::Horizontal, log->columnName(i),
                                         Qt::DisplayRole);
    LogTableView->model()->setHeaderData(i, Qt::Horizontal, Qt::AlignLeft,
                                         Qt::TextAlignmentRole);
  }
  LogTableView->horizontalHeader()->setStretchLastSection(true);
  LogTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  LogTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  // restore saved column widths
  int ncol = csettings->beginReadArray(c_col_width_group);
  for (int i = 0; i < ncol; i++) {
    csettings->setArrayIndex(i);
    LogTableView->setColumnWidth(
        i, csettings->value(c_col_width_item, c_col_width_def[i]).toInt());
  }
  csettings->endArray();

  QHeaderView *header = LogTableView->verticalHeader();
  header->setSectionResizeMode(QHeaderView::Fixed);
  header->setDefaultSectionSize(qRound(sizes.textHeight));

  LogTableView->scrollToBottom();
  LogTableView->show();
}

void So2sdr::about() {
  aboutOn = true;
  QMessageBox::about(
      this, "SO2SDR",
      "<p>SO2SDR " + Version + " Copyright 2010-2025 R.T. Clay N4OGW</p>" +
          "  Qt library version: " + qVersion() +
          +"<li>hamlib http://www.hamlib.org " + hamlib_version +
          "<li>QtSolutions_Telnet 2.1" +
          "<li>MASTER.DTA algorithm, IQ balancing: Alex Shovkoplyas VE3NEA, "
          "http://www.dxatlas.com</ul>" +
          "<hr><p>SO2SDR is free software: you can redistribute it and/or "
          "modify it under the terms of the GNU General Public License " +
          "as published by the Free Software Foundation, either version 3 of "
          "the License, or any later version, "
          "http://www.gnu.org/licenses/</p>");
  aboutOn = false;
}

/*! update rate display.
 */
void So2sdr::updateRate() {
  int sum = 0;

  // number of qso's in last clock hour
  for (int i = 0; i < 60; i++) {
    sum += rateCount[i];
  }
  HourRateLabel->setText(QString::number(sum) + "/hr");

  // short-time rate estimate:
  // add up rate count of over last RATE_AVG_MINUTES minutes
  // rateCount[] is a circular buffer
  sum = 0;
  int j = ratePtr;
  for (int i = 0; i < RATE_AVG_MINUTES; i++) {
    sum += rateCount[j];
    j--;
    if (j < 0)
      j = 59;
  }
  ratePtr++;
  ratePtr = ratePtr % 60;

  // zero out next bin
  rateCount[ratePtr] = 0;
  RateLabel->setText("Rate=" +
                     QString::number(qRound(sum * 60.0 / RATE_AVG_MINUTES)));

  updateOffTime();
}

/*!
 * \brief So2sdr::updateOffTime calculate and update off-time display
 */
void So2sdr::updateOffTime() {
  if (csettings->value(c_off_time_enable, c_off_time_enable_def).toBool()) {
    offPtr->setText(log->offTime(options->offMinimumLineEdit->text().toInt(),
                                 options->startDateTimeEdit->dateTime(),
                                 options->endDateTimeEdit->dateTime()));
  } else {
    offPtr->clear();
  }
}

/*!
   process timer events

   The timer frequencies are define in defines.h, timerSettings
 */
void So2sdr::timerEvent(QTimerEvent *event) {
  if (event->timerId() == timerId[2]) {
    // auto-CQ, dueling CQ, autoSend triggers, 100 ms resolution
    if (autoCQMode)
      autoCQ();
    if (duelingCQMode)
      duelingCQ();
    if (autoSend)
      autoSendExch();
  } else if (event->timerId() == timerId[1]) {
    // radio updates; every 300 mS
    // (the actual serial poll time is set in serial.cpp and may be different)
    updateRadioFreq();

    // check bandmap
    checkSpot(0);
    checkSpot(1);
  } else if (event->timerId() == timerId[0]) {
    // clock update; every 1000 mS
    TimeDisplay->setText(
        QDateTime::currentDateTimeUtc().toString("MM-dd hh:mm:ss"));
    // update rate display at beginning of minute
    if (QDateTime::currentDateTimeUtc().time().second() == 0 && log) {
      updateRate();
    }
    // check bandmap tcp connection
    bandmap->connectTcp();

    // update dupsheet
    if (nDupesheet()) {
      populateDupesheet();
    }

    // update bandmap calls
    decaySpots();
  } else if (event->timerId() == timerId[3]) {
    if (wsjtx[0]) {
      wsjtx[0]->decayCalls();
    }
    if (wsjtx[1]) {
      wsjtx[1]->decayCalls();
    }
  }
}

void So2sdr::autoCQActivate(bool state) {
  if (csettings->value(c_sprintmode, c_sprintmode_def).toBool())
    return; // disabled in Sprint mode
  autoCQMode = state;
  if (autoCQMode) {
    duelingCQActivate(false);
    clearR2CQ(activeRadio ^ 1);
    autoCQRadio = activeRadio;
    autoCQStatus->setText(
        "<font color=#5200CC>AutoCQ (" +
        QString::number(
            settings->value(s_settings_cqrepeat, s_settings_cqrepeat_def)
                    .toInt() /
                1000.0,
            'f', 1) +
        "s)</font>");
    autoCQModePause = false;
    autoCQModeWait = true;
    sendLongCQ = true;
    if (lineEditCall[autoCQRadio]->text().isEmpty() && !cw->isSending() &&
        !ssbMessage->isPlaying()) {
      switchTransmit(autoCQRadio);
      sendFunc(0, Qt::NoModifier); // send F1 immediately
    }
  } else {
    autoCQStatus->clear();
  }
}

void So2sdr::autoSendActivate(bool state) {
  autoSend = state;
  if (autoSend) {
    if (settings->value(s_settings_autosend_mode, s_settings_autosend_mode_def)
                .toInt() == 0 ||
        csettings->value(c_sprintmode, c_sprintmode_def)
            .toBool()) { // semi-auto
      autoSendStatus->setText("<font color=#006699>AutoSend(ESM)</font>");
    } else {
      autoSendStatus->setText(
          "<font color=#006699>AutoSend(" +
          QString::number(
              settings->value(s_settings_autosend, s_settings_autosend_def)
                  .toInt()) +
          ")</font>");
    }
  } else {
    autoSendStatus->clear();
  }
}

void So2sdr::duelingCQActivate(bool state) {
  if (csettings->value(c_sprintmode, c_sprintmode_def).toBool())
    return; // disabled in Sprint mode
  if (twokeyboard)
    return; // disabled in two keyboard mode

  if (cat[0]->band() != BAND_NONE && cat[1]->band() != BAND_NONE) {
    if (cat[0]->band() == cat[1]->band()) {
      So2sdrStatusBar->showMessage("Dueling CQ disabled: same band", 5000);
      state = false;
    }
  }
  duelingCQMode = state;
  if (duelingCQMode) {
    autoCQActivate(false);
    autoSendStatus->hide();
    clearR2CQ(activeRadio ^ 1);
    if (activeTxRadio != activeRadio) {
      switchTransmit(activeRadio);
    }
    if (altDActive) {
      QPalette palette(lineEditCall[altDActiveRadio]->palette());
      palette.setColor(QPalette::Base, CQ_COLOR);
      lineEditCall[altDActiveRadio]->setPalette(palette);
      lineEditExchange[altDActiveRadio]->setPalette(palette);
      altDActive = 0;
      callSent[altDActiveRadio] = false;
      setCqMode(altDActiveRadio);
    }
    duelingCQStatus->setText("<font color=#006B00>DuelingCQ (AUTO)</font>");
    toggleMode = true;
    toggleStatus->clear();
    sendLongCQ = true;
    duelingCQWait = true;
    if (lineEditCall[activeRadio]->text().isEmpty()) {
      duelingCQModePause = false;
      if (!cw->isSending() && !ssbMessage->isPlaying()) {
        enter(Qt::NoModifier); // start CQ immediately
      } else {
        switchRadios(
            false); // focus other radio to begin dueling or toggle sequence
      }
    } else {
      duelingCQModePause = true;
    }
  } else {
    duelingCQStatus->clear();
    toggleMode = false;
    toggleStatus->clear();
    if (activeRadio != activeTxRadio && !autoCQMode &&
        lineEditCall[activeRadio]->text().isEmpty()) {
      switchRadios(false); // move focus to where last transmit
    }
    autoSendStatus->show();
  }
}

/*!
 automatically send call and exchange after user defined characters
 toggles with Alt/-
 Buffers and sends call letters until CW catches up
 Backspaced changes before CW catches up to buffer results in stopped CW

 winkey echo is turned off while call is sent
 */
void So2sdr::autoSendExch() {

  if (cat[activeTxRadio]->modeType() != CWType)
    return;
  if (autoSendTrigger && !autoSendPause) {
    if (!activeR2CQ && !duelingCQMode && !toggleMode) {

      cqQsoInProgress[activeTxRadio] =
          true; // set here so F2 sprint message correct
      if (activeTxRadio == activeRadio && !exchangeSent[activeRadio] &&
          cqMode[activeRadio] &&
          !(altDActive && altDActiveRadio == activeRadio)) {

        int comp =
            QString::compare(autoSendCall, lineEditCall[activeRadio]->text(),
                             Qt::CaseInsensitive);
        if (comp < 0) {
          cw->setEchoMode(false);
          int cindx = lineEditCall[activeRadio]->text().length() -
                      autoSendCall.length();
          autoSendCall = lineEditCall[activeRadio]->text();
          QString callDiff = lineEditCall[activeRadio]->text().right(cindx);
          send(callDiff.toLatin1(), false);
        } else if (comp > 0) {
          cw->setEchoMode(true);
          cw->cancelcw();
          autoSendPause = true;
          autoSendCall.clear();
        } else { // calls equal
          autoSendExch_exch();
          So2sdrStatusBar->showMessage(
              QString::number(activeRadio + 1) + ":" + autoSendCall, 1700);
          cw->setEchoMode(true);
        }

      } else if (activeTxRadio != activeRadio && !exchangeSent[activeTxRadio] &&
                 cqMode[activeTxRadio] &&
                 !(altDActive && altDActiveRadio == activeTxRadio)) {
        // focused other radio, no more call changes
        autoSendExch_exch();

      } else {
        autoSendCall.clear();
      }

    } else {
      autoSendCall.clear();
    }

  } else {
    if (settings->value(s_settings_autosend_mode, s_settings_autosend_mode_def)
                .toInt() == 1 &&
        !csettings->value(c_sprintmode, c_sprintmode_def).toBool() &&
        lineEditCall[activeRadio]->text().length() >=
            settings->value(s_settings_autosend, s_settings_autosend_def)
                .toInt() &&
        !autoSendPause && !lineEditCall[activeRadio]->text().contains("?")) {
      autoSendTrigger = true;
    }
  }
}

/*!
 AutoSend exchange handler
 */
void So2sdr::autoSendExch_exch() {
  if (!cw->isSending() &&
      !ssbMessage->isPlaying()) { // wait until complete call is sent, then
                                  // start exchange
    autoSendTrigger = false;
    autoSendPause = false;
    // duplicated from pieces of So2sdr::enter, {CALL_ENTERED} removed in Exch
    // message macro
    if (nrSent == nrReserved[activeTxRadio ^ 1]) {
      nrReserved[activeTxRadio] = nrSent + 1;
    } else {
      nrReserved[activeTxRadio] = nrSent;
    }
    int m = cat[activeTxRadio]->modeType();
    QByteArray tmpExch;
    if (qso[activeTxRadio]->dupe &&
        csettings->value(c_dupemode, c_dupemode_def).toInt() == STRICT_DUPES) {
      tmpExch =
          csettings->value(c_dupe_msg[m], c_dupe_msg_def[m]).toByteArray();
      tmpExch.replace("{CALL_ENTERED}", "");
    } else {
      tmpExch = csettings->value(c_cq_exc[m], c_cq_exc_def[m]).toByteArray();
      tmpExch.replace("{CALL_ENTERED}", "");
    }
    if (activeRadio != activeTxRadio)
      tmpExch.prepend("{R2}");
    expandMacro(tmpExch, false);
    exchangeSent[activeTxRadio] = true;
    callSent[activeTxRadio] =
        true; // set this true as well in case mode switched to S&P
    origCallEntered[activeTxRadio] = qso[activeTxRadio]->call;
    updateNrDisplay();
    cqQsoInProgress[activeTxRadio] = true;
    excMode[activeTxRadio] = true;
    lineEditExchange[activeTxRadio]->setEnabled(true);
    prefillExch(activeTxRadio);
    if (activeRadio == activeTxRadio) {
      lineEditExchange[activeTxRadio]->setFocus();
      lineEditExchange[activeTxRadio]->deselect();
      if (grab) {
        // lineEditExchange[activeTxRadio]->grabKeyboard();
        lineEditExchange[activeTxRadio]->activateWindow();
      }
      grabWidget = lineEditExchange[activeTxRadio];
    }
    callFocus[activeTxRadio] = false;
    if (lineEditExchange[activeTxRadio]->text().simplified().isEmpty()) {
      lineEditExchange[activeTxRadio]->clear();
    } else {
      lineEditExchange[activeTxRadio]->setText(
          lineEditExchange[activeTxRadio]->text().simplified() + " ");
    }
    autoSendCall.clear();
  }
}

/*!
 Automatic repeating CQ, user-defined delay
 */
void So2sdr::autoCQ() {

  int delay =
      settings->value(s_settings_cqrepeat, s_settings_cqrepeat_def).toInt();
  if (autoCQModeWait) {
    if (cw->isSending() ||
        ssbMessage->isPlaying()) { // prevent switching hysteresis
      autoCQModeWait = false;
    }
  } else if (autoCQModePause) {
    autoCQStatus->setText("<font color=#5200CC>AutoCQ (SLEEP)</font>");
  } else if (!lineEditCall[autoCQRadio]->text().isEmpty() || altDActive > 2) {
    autoCQModePause = true;
  } else if (cw->isSending() || ssbMessage->isPlaying()) {
    cqTimer.start();
    autoCQStatus->setText(
        "<font color=#5200CC>AutoCQ (" +
        QString::number(
            settings->value(s_settings_cqrepeat, s_settings_cqrepeat_def)
                    .toInt() /
                1000.0,
            'f', 1) +
        "s)</font>");
  } else if (cqTimer.elapsed() > delay) {
    autoCQModeWait = true;
    switchTransmit(autoCQRadio);
    if (sendLongCQ) {
      sendFunc(0, Qt::NoModifier);
    } else {
      sendFunc(1, Qt::NoModifier);
    }
  } else {
    autoCQStatus->setText(
        "<font color=#5200CC>AutoCQ (" +
        QString::number(
            settings->value(s_settings_cqrepeat, s_settings_cqrepeat_def)
                        .toInt() /
                    1000.0 -
                (static_cast<double>(cqTimer.elapsed() / 1000.0)),
            'f', 1) +
        "s)</font>");
  }
}

/*!
  Increment AutoCQ +/- 0.1 sec: alt-PgUP / alt-PgDN
 */
void So2sdr::autoCQdelay(bool incr) {
  if (incr) {
    settings->setValue(
        s_settings_cqrepeat,
        settings->value(s_settings_cqrepeat, s_settings_cqrepeat_def).toInt() +
            100);
  } else {
    settings->setValue(
        s_settings_cqrepeat,
        settings->value(s_settings_cqrepeat, s_settings_cqrepeat_def).toInt() -
            100);
    if (settings->value(s_settings_cqrepeat, s_settings_cqrepeat_def).toInt() <
        0) {
      settings->setValue(s_settings_cqrepeat, 0);
    }
  }
  progsettings->CQRepeatLineEdit->setText(QString::number(
      settings->value(s_settings_cqrepeat, s_settings_cqrepeat_def).toInt() /
          1000.0,
      'f', 1));
  settings->sync();
  if (autoCQMode && (cw->isSending() || ssbMessage->isPlaying())) {
    autoCQStatus->setText(
        "<font color=#5200CC>AutoCQ (" +
        QString::number(
            settings->value(s_settings_cqrepeat, s_settings_cqrepeat_def)
                    .toInt() /
                1000.0,
            'f', 1) +
        "s)</font>");
  } else {
    So2sdrStatusBar->showMessage(
        "CQ DELAY: " + QString::number(settings->value(s_settings_cqrepeat,
                                                       s_settings_cqrepeat_def)
                                               .toInt() /
                                           1000.0,
                                       'f', 1),
        2000);
  }
}

/*!
 Dueling CQ, user-defined delay
 */
void So2sdr::duelingCQ() {

  int delay = static_cast<int>(
      settings->value(s_settings_duelingcqdelay, s_settings_duelingcqdelay_def)
          .toDouble() *
      1000.0);

  if (!cqMode[activeRadio])
    setCqMode(activeRadio);
  if (!cqMode[activeRadio ^ 1])
    setCqMode(activeRadio ^ 1);

  if (duelingCQWait) {
    if (cw->isSending() ||
        ssbMessage->isPlaying()) { // prevent switching hysteresis
      duelingCQWait = false;
    }
  } else if (duelingCQModePause) {
    // prevent further processing if paused
  } else if (!lineEditCall[activeRadio]->text().isEmpty() ||
             !lineEditCall[activeRadio ^ 1]->text().isEmpty()) {
    duelingCQModePause = true;
  } else if (cw->isSending() || ssbMessage->isPlaying()) {
    cqTimer.start();
  } else if (cqTimer.elapsed() > delay) {
    duelingCQWait = true;
    enter(Qt::NoModifier);
  }
  // process status outside of control loop
  if (duelingCQModePause) {
    duelingCQStatus->setText("<font color=#006B00>DuelingCQ (SLEEP) </font>");
  } else {
    duelingCQStatus->setText("<font color=#006B00>DuelingCQ (AUTO)</font>");
  }
}

/*!
     swap freqs between radios
 */
void So2sdr::swapRadios() {
  duelingCQActivate(false);
  autoCQActivate(false);

  double old_f[NRIG] = {cat[0]->getRigFreq(), cat[1]->getRigFreq()};
  updateMults(0);
  updateMults(1);
  if (!cat[0] || !cat[1])
    return;
  qsy(0, old_f[1], true);
  qsy(1, old_f[0], true);
}

void So2sdr::toggleStereo() {
  so2r->toggleStereo(activeRadio);
  so2r->updateIndicators(activeRadio);
}

/*!
 Switch Audio
 */
void So2sdr::switchAudio(int r) {
  so2r->switchAudio(r);
  so2r->updateIndicators(activeRadio);
}

/*!
  Switch transmit focus
 */
void So2sdr::switchTransmit(int r, int CWspeed) {
  if (cat[r]->modeType() == CWType) {
    cw->cancelcw();
  }
  if (cat[r]->modeType() == PhoneType && ssbMessage->isPlaying()) {
    ssbMessage->cancelMessage();
  }
  if (CWspeed) {
    cw->setSpeed(CWspeed);
  } else {
    cw->setSpeed(wpm[r]);
  }
  if (r != activeTxRadio) {
    autoSendTrigger = false;
    autoSendPause = false;
    cw->switchTransmit(r);
    so2r->switchTransmit(r);
    if (cat[r]->modeType() == PhoneType) {
      ssbMessage->switchRadio(r);
    }
  }
  so2r->updateIndicators(activeRadio);
  activeTxRadio = r;
}

/*!
   Switch active radio

   Alt+R

   switchcw (default=true) controls whether cw/ssb is switched or not
 */
void So2sdr::switchRadios(bool switchcw) {
  if (switchcw && !altDActive)
    autoCQActivate(false);
  activeRadio = activeRadio ^ 1;
  clearR2CQ(activeRadio);
  switchAudio(activeRadio);
  if (switchcw) {
    switchTransmit(activeRadio);
  }
  setEntryFocus(activeRadio);
  if (callFocus[activeRadio]) {
    if (qso[activeRadio]->call.isEmpty()) {
      lineEditCall[activeRadio]->setCursorPosition(0);
    }
  } else {
    if (qso[activeRadio]->exch.isEmpty()) {
      lineEditExchange[activeRadio]->setCursorPosition(0);
    }
  }
  MasterTextEdit->clear();
  updateRadioFreq();
  updateBreakdown();
  /*!
     @todo Implement option of mults display following non-active radio
   */
  updateMults(activeRadio);
}

/*!
   Start prefix check on radio 1
 */
void So2sdr::prefixCheck1(const QString &call) {
  int i = lineEditCall[0]->cursorPosition();
  lineEditCall[0]->setText(call.toUpper());
  lineEditCall[0]->setCursorPosition(i);
  prefixCheck(0, call.toUpper());
}

/*!
   Start prefix check on radio 2
 */
void So2sdr::prefixCheck2(const QString &call) {
  int i = lineEditCall[1]->cursorPosition();
  lineEditCall[1]->setText(call.toUpper());
  lineEditCall[1]->setCursorPosition(i);
  prefixCheck(1, call.toUpper());
}

/*!
   Start exchange check on radio 1
 */
void So2sdr::exchCheck1(const QString &exch) {
  int i = lineEditExchange[0]->cursorPosition();
  lineEditExchange[0]->setText(exch.toUpper());
  lineEditExchange[0]->setCursorPosition(i);
  editingExchange[0] = true;
  exchCheck(0, exch.toUpper());
}

/*!
   Start exchange check on radio 2
 */
void So2sdr::exchCheck2(const QString &exch) {
  int i = lineEditExchange[1]->cursorPosition();
  lineEditExchange[1]->setText(exch.toUpper());
  lineEditExchange[1]->setCursorPosition(i);
  editingExchange[1] = true;
  exchCheck(1, exch.toUpper());
}

/*!
 Check exchange of radio nr, show multiplier status
 */
void So2sdr::exchCheck(int nr, const QString &exch) {
  if (qso[nr]->call.isEmpty()) {
    qso[nr]->valid = false;
    return; // do nothing unless we have a callsign
  }
  // recopy call; needed in case call was changed by a previous
  // exchange edit
  qso[nr]->call = lineEditCall[nr]->text().toLatin1();
  qso[nr]->exch = exch.toLatin1();
  qso[nr]->valid = log->validateExchange(qso[nr]);
  if (qso[nr]->valid) {
    updateWorkedMult(nr);
    validLabel[nr]->setPixmap(iconValid);
  } else {
    validLabel[nr]->clear();
  }
  // update colors in case dupe status changed
  setDupeColor(nr, qso[nr]->dupe);
  // update bearing- some contests compute this from grid square
  labelBearing[nr]->setText(QString::number(qso[nr]->bearing) + QChar(176));
  if (qso[nr]->distance >= 0) {
    labelCountry[nr]->setText("<font color=#0000FF>" +
                              QString::number(qso[nr]->distance) + " km");
    labelLPBearing[nr]->clear();
  } else {
    labelLPBearing[nr]->setText(
        "<font color=#0000FF>" +
        QString::number(((qso[nr]->bearing + 180) % 360)) + QChar(176));
  }
}

/*!
   Supercheck partial a partial call
 */
void So2sdr::superPartial(QByteArray partial) {
  static QByteArray lastcalls = "";
  QByteArray calls;
  calls.clear();
  master->search(partial, calls);

  // previous result saved in lastcalls; if nothing found, just show
  // last non-null result
  if (calls.isEmpty()) {
    calls = lastcalls;
  } else {
    lastcalls = calls;
  }
  MasterTextEdit->append("<font color=#000000>" + calls);
  MasterTextEdit->moveCursor(QTextCursor::Start);
  MasterTextEdit->ensureCursorVisible();
}

/*!
   log partial checking. Returns true if the partial/call is a dupe

   if external = true, qso is from an external source like wsjtx and may not
   correspond to either radio.
 */
bool So2sdr::logPartial(int nrig, const QByteArray &partial, bool external,
                        Qso *externalQso) {
  Qso *partialQso;
  if (external) {
    partialQso = externalQso;
  } else {
    partialQso = qso[nrig];
  }
  QList<QByteArray> calls;
  QList<unsigned int> worked;
  QString txt("<font color=#0000FF>");
  QList<int> mults1;
  QList<int> mults2;
  partialQso->worked = 0;
  searchPartial(partialQso, partial, calls, worked, mults1, mults2);
  const int nc = calls.size();
  for (int i = 0; i < nc; i++) {
    // for contests where stations can be worked only once on any band
    if (!log->dupeCheckingByBand()) {
      // all are dupes; show in red
      if (calls.at(i) == partial) {
        partialQso->mult[0] = mults1.at(i);
        partialQso->mult[1] = mults2.at(i);
        partialQso->worked = worked[i];
      }
      txt.append("<font color=#FF0000>" + calls.at(i) +
                 " <font color=#000000>");
    } else {
      // show dupe with current band in grey, available calls from other bands
      // in blue
      if (bits[partialQso->band] & worked.at(i)) {
        txt.append("<font color=#AAAAAA>" + calls.at(i) +
                   " <font color=#000000>");
      } else {
        txt.append("<font color=#0000FF>" + calls.at(i) +
                   " <font color=#000000>");
      }
      if (calls.at(i) == partial) {
        partialQso->mult[0] = mults1.at(i);
        partialQso->mult[1] = mults2.at(i);
        partialQso->worked = worked[i];
      }
    }
  }
  MasterTextEdit->setText(txt);
  if (!external) {
    updateWorkedDisplay(nrig, partialQso->worked);
    updateWorkedMult(nrig);
  } else {
    // don't know which radio the external qso is on (if either)
    updateWorkedDisplay(0, partialQso->worked);
    updateWorkedDisplay(1, partialQso->worked);
    updateWorkedMult(0);
    updateWorkedMult(1);
  }
  return (qso[nrig]->dupe);
}

/*!
   callsign entry window edited
 */
void So2sdr::prefixCheck(int nrig, const QString &call) {
  if (lineEditExchange[nrig]->text().isEmpty()) {
    qso[nrig]->clear();
  }
  qso[nrig]->call = call.toLatin1();
  qso[nrig]->call = qso[nrig]->call.toUpper();

  // check/supercheck partial callsign fragment
  // don't do anything unless at least 2 chars entered
  if (qso[nrig]->call.size() > 1) {
    qso[nrig]->mode = cat[nrig]->mode();
    qso[nrig]->modeType = getModeType(qso[nrig]->mode);
    qso[nrig]->adifMode = getAdifMode(qso[nrig]->mode);
    qso[nrig]->freq = cat[nrig]->getRigFreq();
    qso[nrig]->band = getBand(qso[nrig]->freq);
    qso[nrig]->time = QDateTime::currentDateTimeUtc();
    bool qsy;
    int pp = log->idPfx(qso[nrig], qsy);

    // 2nd radio CQ is active, display these on the other radio
    // unless in S/P on active radio
    int nr = nrig;
    if (activeR2CQ && cqMode[activeRadio]) {
      nr = nr ^ 1;
    }

    labelCountry[nr]->setText(qso[nrig]->country_name);
    if (pp != -1) {
      // prefix ID successful

      // working oneself? Fill in 0 heading and exact sun times
      // (useful shortcut to recall exact sunrise/sunset)
      if (qso[nrig]->call == settings->value(s_call, s_call_def)) {
        clearDisplays(nr);
        sunLabelPtr[nr]->setText(log->mySunTimes());
      } else {
        labelBearing[nr]->setText(QString::number(qso[nr]->bearing) +
                                  QChar(176));
        if (qso[nrig]->distance >= 0) {
          labelCountry[nr]->setText("<font color=#0000FF>" +
                                    QString::number(qso[nrig]->distance) +
                                    " km");
          labelLPBearing[nr]->clear();
        } else {
          labelLPBearing[nr]->setText(
              "<font color=#0000FF>" +
              QString::number(((qso[nrig]->bearing + 180) % 360)) + QChar(176));
        }
        sunLabelPtr[nr]->setText(qso[nrig]->sun);
      }
      log->guessMult(qso[nrig]);
    } else {
      // prefix ID failed, just leave these blank
      clearDisplays(nrig);
    }
    // if dupe option is 2 (no dupe checking), dupes will not be recorded and
    // will be scored
    qso[nrig]->dupe = logPartial(nrig, qso[nrig]->call) &&
                      (csettings->value(c_dupemode, c_dupemode_def).toInt() <
                       NO_DUPE_CHECKING);
    if (csettings->value(c_mastermode, c_mastermode_def).toBool()) {
      superPartial(qso[nrig]->call);
    }
    // prefill previous or guessed exchange if exchange has not been edited by
    // user
    if (!qsy && !lineEditExchange[nr]->isModified()) {
      prefillExch(nr);
    }
    // show/clear dupe messages
    if (qso[nrig]->dupe) {
      // show information from most recent previous qso with dupe message
      if (csettings->value(c_dupemode, c_dupemode_def).toInt() ==
          STRICT_DUPES) {
        So2sdrStatusBar->showMessage("** " + qso[nrig]->call +
                                     " DUPE ** : Ctrl+Enter to log");
      } else if (csettings->value(c_dupemode, c_dupemode_def).toInt() ==
                 WORK_DUPES) {
        So2sdrStatusBar->showMessage("** " + qso[nrig]->call + " DUPE ** ");
      }
      statusBarDupe = true;
    } else {
      So2sdrStatusBar->clearMessage();
      statusBarDupe = false;
    }
    setDupeColor(nrig, qso[nrig]->dupe);
    // if exchange has been edited, should recheck it now
    if (lineEditExchange[nrig]->isModified()) {
      exchCheck(nrig, lineEditExchange[nrig]->text());
    }
    // there is exchange present but qso is marked invalid, recheck it
    if (!qso[nrig]->valid && !lineEditExchange[nrig]->text().isEmpty()) {
      exchCheck(nrig, lineEditExchange[nrig]->text());
    }
  } else {
    if (csettings->value(c_mastermode, c_mastermode_def).toBool()) {
      MasterTextEdit->clear();
    }

    // if 2nd radio CQ and S/P on active radio, display on active radio
    int nr = nrig;
    if (activeR2CQ && cqMode[activeRadio]) {
      nr = nr ^ 1;
    }
    clearDisplays(nr);
    clearWorked(nr);
  }
}

/*!
   update display showing bands a station has been worked on
 */
void So2sdr::updateWorkedDisplay(int nr, unsigned int worked) {
  if (lineEditCall[nr]->text().isEmpty())
    return;

  QString tmp = "<span style=\"font-family: monospace\">Q :";
  for (int j = 0; j < N_BANDS; j++) {
    int i = log->highlightBand(j);
    if (i == BAND_NONE)
      continue;
    if (worked & bits[j]) {
      tmp = tmp + "<font color=#000000>" + log->bandLabel(i) + "</font> ";
    } else {
      tmp = tmp + "<font color=#AAAAAA>" + log->bandLabel(i) + "</font> ";
    }
  }
  tmp = tmp + "</span>";
  qsoWorkedLabel[nr]->setText(tmp);
}

/*!
   clear worked Q/M display on radio nr
 */
void So2sdr::clearWorked(int i) {
  qsoWorkedLabel[i]->setText(
      "<span style=\"font-family: monospace\">Q :</span>");
  for (int j = 0; j < MMAX; j++) {
    multWorkedLabel[i][j]->setText("<span style=\"font-family: monospace\"" +
                                   multNameLabel[j]->text() + "</span>");
  }
}

/*!
   Update band/mult display just above call entry
 */
void So2sdr::updateWorkedMult(int nr) {
  if (lineEditCall[nr]->text().isEmpty())
    return;

  unsigned int worked[NRIG] = {0, 0};
  log->workedMults(qso[nr], worked);
  for (int ii = 0; ii < MMAX; ii++) {
    if (multNameLabel[ii]->text().isEmpty())
      continue;
    QString tmp =
        "<span style=\"font-family: monospace\">" + multNameLabel[ii]->text();

    if (!csettings->value(c_multsband, c_multsband_def).toBool()) {
      if (worked[ii] == (1 + 2 + 4 + 8 + 16 + 32)) {
        // mult already worked
        if (qso[nr]->isamult[ii])
          tmp = tmp + "<font color=#000000>      WORKED      </font>";
      } else {
        // needed
        if (qso[nr]->isamult[ii])
          tmp = tmp + "<font color=#CC0000>      NEEDED      </font>";
      }
    } else {
      for (int i = 0; i < N_BANDS; i++) {
        int j = log->highlightBand(i);
        if (j == BAND_NONE)
          continue;
        if (worked[ii] & bits[i]) {
          tmp = tmp + "<font color=#000000>" + log->bandLabel(j) + "</font> ";
        } else {
          tmp = tmp + "<font color=#AAAAAA>" + log->bandLabel(j) + "</font> ";
        }
      }
    }
    tmp = tmp + "</span>";
    multWorkedLabel[nr][ii]->setText(tmp);
  }
}
/*!
   take entered number and qsy rig, or change mode. These are
parsed in Cty::idPfx

   if ; follows the entered freq or mode 2nd radio is changed
 */
bool So2sdr::enterFreqOrMode() {
  bool retval = false;

  // nr holds the radio the change will apply to, which is 2nd radio if ;
  // present and secondRadioQsy is set
  int nr = activeRadio;
  if (qso[activeRadio]->secondRadioQsy)
    nr = nr ^ 1;

  // check for mode change
  if (qso[activeRadio]->qsyMode != RIG_MODE_NONE) {
    if (nr == 0)
      emit setRigMode1(qso[activeRadio]->qsyMode);
    else
      emit setRigMode2(qso[activeRadio]->qsyMode);
    qso[activeRadio]->qsyFreq = 0;
    if (qso[activeRadio]->qsyMode == RIG_MODE_USB ||
        qso[activeRadio]->qsyMode == RIG_MODE_LSB ||
        qso[activeRadio]->qsyMode == RIG_MODE_FM ||
        qso[activeRadio]->qsyMode == RIG_MODE_AM)
      ssbMessage->switchRadio(nr);
    qso[activeRadio]->qsyMode = RIG_MODE_NONE;
    retval = true;
  }

  // check for frequency change
  if (qso[activeRadio]->qsyFreq > 0) {
    // qsy returns "corrected" rigFreq in event there is no radio CAT connection
    if (cat[nr]) {
      qsy(nr, qso[activeRadio]->qsyFreq, true);

      int b;
      if ((b = getBand(qso[activeRadio]->qsyFreq)) != BAND_NONE) {
        // if band change, update bandmap calls
        if (cat[nr]->band() != BAND_NONE && b != cat[nr]->band() &&
            bandmap->bandmapon(nr)) {
          bandmap->syncCalls(nr, spotList[b]);
        }
        // clear wsjtx calls
        if (wsjtx[nr]->isEnabled())
          wsjtx[nr]->clear();
      }
      if (bandmap->bandmapon(nr)) {
        bandmap->bandmapSetFreq(qso[activeRadio]->qsyFreq, nr);
        bandmap->setAddOffset(cat[nr]->ifFreq(), nr);
      }
      updateBreakdown();
      if (getBand(qso[activeRadio]->qsyFreq) != BAND_NONE) {
        updateMults(nr, getBand(qso[activeRadio]->qsyFreq));
      }
      clearWorked(nr);
      clearDisplays(nr);
      retval = true;
      qso[activeRadio]->qsyFreq = 0;
    }
  }
  qso[activeRadio]->call.clear();
  lineEditCall[activeRadio]->clear();
  lineEditCall[activeRadio]->setFocus();
  if (grab) {
    lineEditCall[activeRadio]->activateWindow();
  }
  grabWidget = lineEditCall[activeRadio];
  lineEditCall[activeRadio]->setModified(false);

  return retval;
}

/*!
   update display of qsos/score
 */
void So2sdr::updateBreakdown() {
  int n = 0;
  for (int i = 0; i < N_BANDS_HF; i++) {
    n += log->columnCount(i);
    qsoLabel[i]->setNum(log->columnCount(i));
  }
  int nm[2] = {0, 0};
  int nb[2] = {0, 0};
  for (int ii = 0;
       ii < csettings->value(c_nmulttypes, c_nmulttypes_def).toInt(); ii++) {
    for (int i = 0; i < N_BANDS; i++) {
      int m = log->nMultsColumn(i, ii);
      nm[ii] += m;
      if (i == N_BANDS_HF)
        break;

      multLabel[ii][i]->setNum(m);
    }

    // for contests where mults are not per-band
    nb[ii] += log->nMultsBWorked(ii, N_BANDS);
    if (csettings->value(c_multsband, c_multsband_def).toBool()) {
      multTotal[ii]->setNum(nm[ii]);
    } else {
      multTotal[ii]->setNum(nb[ii]);
    }
  }
  TotalQsoLabel->setNum(n);
  ScoreLabel->setText(QString::number(log->score()) + " pts");
}

/*!
   update needed mults display for radio ir
 */
void So2sdr::updateMults(int ir, int bandOverride) {
  if (!log)
    return; // do nothing if contest not loaded

  int band;
  if (bandOverride != BAND_NONE)
    band = bandOverride;
  else
    band = cat[ir]->band();

  // in case tuned out of a ham band
  if (band == BAND_NONE)
    return;

  MultTextEdit->clear();
  if (!csettings->value(c_showmults, c_showmults_def).toBool())
    return;

  QList<QByteArray> neededMults;
  QList<QByteArray> mults;
  mults.clear();
  neededMults.clear();
  for (int i = 0; i < log->nMults(multMode); i++) {
    // don't show invalid mults in window
    if (!log->validMult(multMode, i))
      continue;

    bool needed_band, needed;
    QByteArray mult;
    if (csettings->value(c_multsmode, c_multsmode_def).toBool()) {
      // per-mode mults
      mult = log->neededMultNameMode(multMode, band, cat[ir]->modeType(), i,
                                     needed_band, needed);
    } else {
      mult = log->neededMultName(multMode, band, i, needed_band, needed);
    }
    if (excludeMults[multMode].contains(mult))
      continue;
    mults.append(mult);
    if (csettings->value(c_multsband, c_multsband_def).toBool()) {
      if (needed_band) {
        neededMults.append(mult);
      }
    } else {
      if (needed) {
        neededMults.append(mult);
      }
    }
  }
  MultTextEdit->setMults(mults);
  MultTextEdit->setNeededMults(neededMults);
  MultTextEdit->updateMults();
  if (csettings->value(c_multsmode, c_multsmode_def).toBool()) {
    // per-mode mults
    MultGroupBox->setTitle("Mults: Radio " + QString::number(ir + 1) + ": " +
                           bandNames[band] + " " +
                           modeNames[cat[ir]->modeType()]);
  } else {
    MultGroupBox->setTitle("Mults: Radio " + QString::number(ir + 1) + ": " +
                           bandNames[band]);
  }
}

/*!
   update radio frequency/mode display
 */
void So2sdr::updateRadioFreq() {
  static ModeTypes oldModeType[NRIG] = {CWType, CWType};
  static bool init = false;
  static int previousBand[NRIG] = {BAND_NONE, BAND_NONE};

  if (!init) {
    oldModeType[0] = cat[0]->modeType();
    oldModeType[1] = cat[1]->modeType();
    init = true;
  } else {
    // for per-mode multipliers, need to switch mult display if the mode has
    // changed
    if (log && csettings->value(c_multsmode, c_multsmode_def).toBool() &&
        cat[activeRadio]->modeType() != oldModeType[activeRadio]) {
      updateMults(activeRadio);
      oldModeType[activeRadio] = cat[activeRadio]->modeType();
    }
  }

  for (int i = 0; i < 6; i++) {
    bandLabel[i]->setStyleSheet(
        "QLabel { background-color : palette(Background); color : black; }");
  }
  double rigFreq[NRIG];
  for (int i = 0; i < NRIG; i++) {
    rigFreq[i] = cat[i]->getRigFreq();
    if (bandmap->bandmapon(i)) {
      bandmap->bandmapSetFreq(rigFreq[i], i);
      // add additional offset if specified by radio (like K3)
      bandmap->setAddOffset(static_cast<double>(cat[i]->ifFreq()), i);
    }
    // sync bandmap and clear wsjtx if band change
    if (cat[i]->band() != BAND_NONE && cat[i]->band() != previousBand[i]) {
      if (bandmap->bandmapon(i))
        bandmap->syncCalls(i, spotList[cat[i]->band()]);
      if (wsjtx[i]->isEnabled()) {
        wsjtx[i]->clear();
        wsjtx[i]->clearWsjtx();
      }
      if (i == activeRadio) {
        updateMults(i, cat[i]->band());
        clearWorked(i);
        clearDisplays(i);
      }
      previousBand[i] = cat[i]->band();
    }
    double f = rigFreq[i] / 1000.0;
    wsjtx[i]->setFreq(rigFreq[i]);
    if (cat[i]->radioOpen()) {
      rLabelPtr[i]->setText("R" + QString::number(i + 1) + ":ON");
    } else {
      rLabelPtr[i]->setText("<font color=#FF0000>R" + QString::number(i + 1) +
                            ":OFF </font>");
    }
    // highlight the active bands
    int t;
    if (!log) {
      t = cat[i]->band();
    } else if (log->bandLabelEnable(cat[i]->band()) &&
               cat[i]->band() != BAND_NONE) {
      t = log->highlightBand(cat[i]->band(), cat[i]->modeType());
    } else {
      t = BAND_NONE;
    }
    if (t >= 0 && t < 6)
      bandLabel[t]->setStyleSheet(
          "QLabel { background-color : grey; color : white; }");

    if (i == activeRadio) {
      freqDisplayPtr[i]->setText("<b>" + QString::number(f, 'f', 1) + "</b>");
      modeDisplayPtr[i]->setText("<b>" + cat[i]->modeStr() + "</b>");
    } else {
      freqDisplayPtr[i]->setText(QString::number(f, 'f', 1));
      modeDisplayPtr[i]->setText(cat[i]->modeStr());
    }
  }
  winkeyLabel->setText(cw->textStatus());
}

/*!
   Launch speed up: if modifier=ctrl key, apply to 2nd radio
 */
void So2sdr::launch_speedUp(Qt::KeyboardModifiers mod, int nr) {
  switch (mod) {
  case Qt::NoModifier:
    speedUp(nr);
    break;
  case Qt::ControlModifier:
    speedUp(nr ^ 1);
    break;
  default:
    return;
  }
}

/*!
   Launch speed down: if modifier=ctrl key, apply to 2nd radio
 */
void So2sdr::launch_speedDn(Qt::KeyboardModifiers mod, int nr) {
  switch (mod) {
  case Qt::NoModifier:
    speedDn(nr);
    break;
  case Qt::ControlModifier:
    speedDn(nr ^ 1);
    break;
  default:
    return;
  }
}

/*!
   Speed up (page_up)

   increases WPM by 2
 */
void So2sdr::speedUp(int nrig) {
  wpm[nrig] += 2;
  if (wpm[nrig] > 99)
    wpm[nrig] = 99;
  wpmLineEditPtr[nrig]->setText(QString::number(wpm[nrig]));

  // don't actually change speed if we are sending on other radio
  if (nrig != activeTxRadio) {
    return;
  }
  cw->setSpeed(wpm[nrig]);
}

/*!
   Speed down (page_down)

   decreases WPM by 2
 */
void So2sdr::speedDn(int nrig) {
  wpm[nrig] -= 2;
  if (wpm[nrig] < 5)
    wpm[nrig] = 5;
  wpmLineEditPtr[nrig]->setText(QString::number(wpm[nrig]));

  // don't actually change speed if we are sending on other radio
  if (nrig != activeTxRadio) {
    return;
  }
  cw->setSpeed(wpm[nrig]);
}

/*!
   Start entering CW speed
 */
void So2sdr::launch_WPMDialog(int nr) {
  enterCW[nr] = true;
  // save where keyboard focus was
  if (lineEditCall[nr]->hasFocus()) {
    callFocus[nr] = true;
  } else {
    callFocus[nr] = false;
  }
  wpmLineEditPtr[nr]->setReadOnly(false);
  wpmLineEditPtr[nr]->setFocus();
  // if (grab) {
  //    wpmLineEditPtr[nr]->grabKeyboard();
  // }
  grabWidget = wpmLineEditPtr[nr];
  wpmLineEditPtr[nr]->selectAll();
}

/*!
   slot called when cw speed edited, radio 0
 */
void So2sdr::launch_enterCWSpeed0(const QString &text) {
  if (twokeyboard) {
    enterCWSpeed(activeRadio, text);
  } else {
    if (enterCW[0])
      enterCWSpeed(0, text);
  }
}

/*!
   slot called when cw speed edited, radio 1
 */
void So2sdr::launch_enterCWSpeed1(const QString &text) {
  if (twokeyboard) {
    enterCWSpeed(activeRadio, text);
  } else {
    if (enterCW[1])
      enterCWSpeed(1, text);
  }
}

/*!
   Get new CW speed from lineedit
 */
void So2sdr::enterCWSpeed(int nrig, const QString &text) {
  bool ok = false;
  // try to convert to a digit. Ok=false for non-digits
  int w = text.toInt(&ok);

  // must enter exactly 2 digits
  if (text.size() < 2) {
    if (ok) {
      return;
    }
  } else {
    if (ok) {
      wpm[nrig] = w;
      // don't actually change speed if we are sending on other radio
      if (nrig == activeTxRadio) {
        cw->setSpeed(wpm[nrig]);
      }
    }
  }
  enterCW[nrig] = false;

  // reset focus
  if (twokeyboard) {
    wpmLineEditPtr[nrig]->setReadOnly(true);
    if (callFocus[nrig]) {
      lineEditCall[nrig]->setFocus();
      lineEditCall[nrig]->deselect();
      if (grab) {
        //   lineEditCall[activeRadio]->grabKeyboard();
        lineEditCall[activeRadio]->activateWindow();
      }
      grabWidget = lineEditCall[activeRadio];
    } else {
      lineEditExchange[nrig]->setFocus();
      lineEditExchange[nrig]->deselect();
      if (grab) {
        //  lineEditExchange[activeRadio]->grabKeyboard();
        lineEditExchange[activeRadio]->activateWindow();
      }
      grabWidget = lineEditExchange[activeRadio];
    }
  }

  // revert to previous number in case of bad input
  if (!ok) {
    wpmLineEditPtr[nrig]->clear();
    wpmLineEditPtr[nrig]->setText(QString::number(wpm[nrig]));
  }
}

/*!
   Sprint - change mode after SP qso
 */
void So2sdr::sprintMode() {
  if (!cqMode[activeRadio]) {
    if (activeR2CQ) { // clear 2nd radio CQ if S/P QSO is logged
      clearR2CQ(activeRadio ^ 1);
    }
    setCqMode(activeRadio);
  }
}

/*!
   switch radio i to SP mode
 */
void So2sdr::spMode(int i) {
  cqQsoInProgress[i] = false;
  cqMode[i] = false;
  if (!altDActive || i != altDActiveRadio) {
    QPalette palette(lineEditCall[i]->palette());
    palette.setColor(QPalette::Base, SP_COLOR);
    lineEditCall[i]->setPalette(palette);
    lineEditExchange[i]->setPalette(palette);
  }
  if (altDActive != 1 || i != altDActiveRadio) {
    lineEditExchange[i]->setEnabled(true);
  }
}

/*!
   switch radio i to CQ mode
 */
void So2sdr::setCqMode(int i) {
  cqQsoInProgress[i] = false;
  excMode[i] = false;
  exchangeSent[i] = false;
  lineEditExchange[i]->clear();
  lineEditExchange[i]->setEnabled(false);
  cqMode[i] = true;
  nrReserved[i] = 0;
  QPalette palette(lineEditCall[i]->palette());
  palette.setColor(QPalette::Base, CQ_COLOR);
  lineEditCall[i]->setPalette(palette);
  lineEditExchange[i]->setPalette(palette);
}

/*!
   Send a CW message
 */
void So2sdr::send(QByteArray text, bool stopcw) {
  if (cat[activeTxRadio]->modeType() != CWType)
    return;
  if (cw->isSending() && stopcw) {
    cw->cancelcw(); // cancel any cw in progress
  }
  cw->loadbuff(text);
  cw->sendcw();
}

/*!
   clears any 2nd radio cq, resets colors on radio nr
 */
void So2sdr::clearR2CQ(int nr) {
  if (activeR2CQ) {
    activeR2CQ = false;
    lineEditCall[nr]->clear();
    QPalette palette(lineEditCall[nr]->palette());
    if (cqMode[nr]) {
      palette.setColor(QPalette::Base, CQ_COLOR);
    } else {
      palette.setColor(QPalette::Base, SP_COLOR);
    }
    lineEditCall[nr]->setPalette(palette);
    lineEditExchange[nr]->setPalette(palette);
    lineEditExchange[nr]->clear();
    editingExchange[nr] = false;
  }
}

/*!
 * \brief So2sdr::messageFinished
 * slot called when message finishes playing
 */
void So2sdr::messageFinished() { expandMacro("", false, true); }

/*!
 * load messages into outgoing queue. If two keyboard mode is
 * not active, simply call expandMacro2
 *
 * this can be called as a slot when cw manager finishes sending a message;
 * for this case restart is set to true
 */
void So2sdr::expandMacro(QByteArray msg, bool stopcw, bool restart) {
  if (restart) {
    if (!queue.isEmpty()) {
      activeRadio = rqueue.dequeue();
      expandMacro2(queue.dequeue());
    }
    return;
  }
  // if first character of message is '@', send immediately
  if (!settings->value(s_queuemessages, s_queuemessages_def).toBool() ||
      msg.at(0) == '@') {
    if (msg.at(0) == '@')
      msg.replace('@', QByteArray(""));
    expandMacro2(msg, stopcw, true);
    return;
  } else {
    queue.enqueue(msg);
    rqueue.enqueue(activeRadio);
    if (!cw->isSending() && !ssbMessage->isPlaying()) {
      activeRadio = rqueue.dequeue();
      msg = queue.dequeue();
      expandMacro2(msg);
    }
  }
}

/*!
 * \brief So2sdr:: parse messages
 * \param msg Text of message
 */
void So2sdr::expandMacro2(QByteArray msg, bool stopcw, bool instant) {
  int tmp_wpm;
  if (toggleMode) {
    tmp_wpm = wpm[activeTxRadio];
  } else if (autoCQMode && !autoCQModePause) {
    tmp_wpm = wpm[autoCQRadio];
  } else {
    tmp_wpm = wpm[activeRadio];
  }
  QByteArray out = "";
  QByteArray command = "";

  struct macroMsg {
    QByteArray token_name; // macro keyword
    bool instant; // if true, macro can be sent immediately in queued mode
                  // without clearing queue
  };

  // will not overwrite a dupe message that is present
  if (!statusBarDupe)
    So2sdrStatusBar->clearMessage();
  const macroMsg names[] = {{"CALL", false},
                            {"#", false},
                            {"UP", false},
                            {"DN", false},
                            {"R2", false},
                            {"R2CQ", false},
                            {"STATE", false},
                            {"SECTION", false},
                            {"NAME", false},
                            {"CQZ", false},
                            {"ITUZ", false},
                            {"GRID", false},
                            {"CALL_ENTERED", false},
                            {"TOGGLESTEREOPIN", true},
                            {"CQMODE", false},
                            {"SPMODE", false},
                            {"SWAP_RADIOS", false},
                            {"REPEAT_LAST", false},
                            {"REPEAT_NR", false},
                            {"CLEAR_RIT", false},
                            {"RIG_FREQ", false},
                            {"RIG2_FREQ", false},
                            {"BEST_CQ", true},
                            {"BEST_CQ_R2", true},
                            {"CANCEL", false},
                            {"PLAY", false},
                            {"SWITCH_RADIOS", false},
                            {"MCP", true},
                            {"OTRSP", true},
                            {"CAT", true},
                            {"CATR2", true},
                            {"CAT1", true},
                            {"CAT2", true},
                            {"CALL_OK", false},
                            {"SCRIPT", true},
                            {"SCRIPTNR", true},
                            {"PTTON", false},
                            {"PTTON1", false},
                            {"PTTON2", false},
                            {"PTTONR2", false},
                            {"PTTOFF", false},
                            {"PTTOFF1", false},
                            {"PTTOFF2", false},
                            {"PTTOFFR2", false},
                            {"RECORD", false},
                            {"2KBD", true},
                            {"/SCRIPT", true},
                            {"/SCRIPTNR", true},
                            {"EXCH_ENTERED", false}};
  const int n_token_names = 49;

  /*!
     cw/ssb message macros

     - {CALL}    insert callsign
     - {#}       insert qso number
     - {UP}      increase speed by 5
     - {DN}      decrease speed by 5
     - {CANCEL}  cancel any speed change
     - {R2}      send on other radio
     - {R2CQ}    send on other radio, marked as CQ
     - {STATE}   insert state
     - {SECTION} insert ARRL section
     - {NAME}    insert name
     - {CQZ}     insert CQ zone
     - {ITUZ}    insert ITU zone
     - {GRID}    insert grid
     - {CALL_ENTERED} call entered in call window
     - {TOGGLESTEREOPIN} toggle parallel port pin; can't combine with cw
     - {CQMODE}  switch to CQ mode
     - {SPMODE}  switch to SP mode
     - {SWAP_radios} swap frequencies between radios
     - {REPEAT_LAST} repeats previously sent message
     - {REPEAT_NR} if the call entry line is not empty, send current qso #. If
     call entry line is empty, sends number sent for last logged qso
     - {CLEAR_RIT} clears RIT
     - {RIG_FREQ} send current rig frequency in KHz
     - {RIG2_FREQ} send current inactive rig freq in KHz
     - {BEST_CQ} qsy current radio to "best" CQ freq
     - {BEST_CQ_R2} qsy 2nd radio to "best" CQ freq
     - {PLAY}file play an audio message in the file "file.wav"
     - {SWITCH_RADIOS} same as alt-R
     - {MCP}{/MCP} Microham Control Protocol commands
     - {OTRSP}{/OTRSP} OTRSP Control Protocol commands
     - {CAT}{/CAT} raw serial port command to active radio
     - {CATR2}{/CATR2} raw serial port command to second radio
     - {CAT1}{/CAT1} raw serial port command to radio 1
     - {CAT2}{/CAT2} raw serial port command to radio 2
     - {CALL_OK} mark call in entry line as correct, so correct call msg will
     not be sent
     - {SCRIPT}{/SCRIPT} run a script in the /scripts directory
     - {SCRIPTNR}{/SCRIPTNR} run a script in the /scripts directory, where
     #=radio number
     - {PTTON} {PTTOFF} turn active radio PTT on/off
     - {PTTON1} {PTTOFF1} turn radio 1 PTT on/off
     - {PTTON2} {PTTOFF2} turn radio 2 PTT on/off
     - {PTTONR2} {PTTOFFR2} turn inactive radio PTT on/off
     - {RECORD} file  record audio to file "file.wav"
     - {2KBD} toggle two keyboard mode
     - {EXCH_ENTERED} exchange from exchange window
  */
  bool switchradio = true;
  bool first = true;
  bool repeat = false;
  int i1;
  int i2;

  if (msg.isEmpty())
    return; // abort for null length messages
  if ((i1 = msg.indexOf("{")) != -1) {
    int i0 = 0;
    while (i1 != -1) {
      out.append(msg.mid(i0, i1 - i0));
      i2 = msg.indexOf("}", i1);

      QByteArray val = msg.mid(i1 + 1, i2 - i1 - 1);
      val = val.trimmed();
      val = val.toUpper();
      for (int i = 0; i < n_token_names; i++) {
        // macros not marked "instant" will clear the message queue if
        // they are called with instant option ("@" at beginning of macro, see
        // So2sdr::expandMacro)
        if (val == names[i].token_name) {
          if (instant && !names[i].instant) {
            queue.clear();
            rqueue.clear();
          }
          switch (i) {
          case 0: // CALL
            out.append(settings->value(s_call, s_call_def).toByteArray());
            break;
          case 1: // #
            if (nrReserved[activeTxRadio]) {
              out.append(QByteArray::number(nrReserved[activeTxRadio]));
            } else {
              out.append(QByteArray::number(nrSent));
            }
            break;
          case 2: // SPEED UP
            if (tmp_wpm < 95) {
              tmp_wpm += 5;
              out.append(0x1c);
              out.append(static_cast<char>(tmp_wpm));
            }
            break;
          case 3: // SPEED DN
            if (tmp_wpm > 9) {
              tmp_wpm -= 5;
              out.append(0x1c);
              out.append(static_cast<char>(tmp_wpm));
            }
            break;
          case 4: // Radio 2
            switchTransmit(activeRadio ^ 1);
            switchradio = false;
            autoSendPause = true; // prevent autosend from going crazy
            break;
          case 5: // Radio 2 CQ
          {
            autoCQActivate(false);
            switchTransmit(activeRadio ^ 1);
            setCqMode(activeRadio ^ 1);
            activeR2CQ = true;
            QPalette palette(lineEditCall[activeRadio ^ 1]->palette());
            palette.setColor(QPalette::Base, ALTD_COLOR);
            lineEditCall[activeRadio ^ 1]->setPalette(palette);
            lineEditCall[activeRadio ^ 1]->setText("CQCQCQ");
            lineEditExchange[activeRadio ^ 1]->setPalette(palette);
            switchradio = false;
          } break;
          case 6: // State
            out.append(settings->value(s_state, s_state_def).toByteArray());
            break;
          case 7: // ARRL Section
            out.append(settings->value(s_section, s_section_def).toByteArray());
            break;
          case 8: // Name
            out.append(settings->value(s_name, s_name_def).toByteArray());
            break;
          case 9: // CQ zone
            out.append(settings->value(s_cqzone, s_cqzone_def).toByteArray());
            break;
          case 10: // ITU Zone
            out.append(settings->value(s_ituzone, s_ituzone_def).toByteArray());
            break;
          case 11: // grid
            out.append(settings->value(s_grid, s_grid_def).toByteArray());
            break;
          case 12: // call entered
            if (!toggleMode) {
              out.append(qso[activeRadio]->call);
            } else {
              out.append(qso[activeTxRadio]->call);
            }
            break;
          case 13: // togglestereopin
            toggleStereo();
            // return immediately to avoid stopping cw
            return;
          case 14: // cqmode
            setCqMode(activeTxRadio);
            break;
          case 15: // spmode
            spMode(activeTxRadio);
            break;
          case 16: // swap_radios
            swapRadios();
            break;
          case 17: // repeat last
            repeat = true;
            break;
          case 18: // repeat NR
            int nr;
            if (lineEditCall[activeTxRadio]->text().isEmpty()) {
              // send last logged number
              nr = log->lastNr();
            } else {
              if (nrReserved[activeTxRadio]) {
                nr = nrReserved[activeTxRadio];
              } else {
                nr = nrSent;
              }
            }
            if (nr != 0) {
              out.append(QByteArray::number(nr));
            }
            break;
          case 19: // clear RIT
            cat[activeTxRadio]->clearRIT();
            break;
          case 20: // RIG_FREQ
            out.append(QByteArray::number(
                qRound(cat[activeRadio]->getRigFreq() / 1000.0)));
            break;
          case 21: // RIG2_FREQ
            out.append(QByteArray::number(
                qRound(cat[activeRadio ^ 1]->getRigFreq() / 1000.0)));
            break;
          case 22: // best cq
            if (cat[activeRadio]->band() == BAND_NONE)
              break;
            bandmap->setFreqLimits(
                activeRadio,
                settings
                    ->value(s_sdr_cqlimit_low[cat[activeRadio]->band()],
                            cqlimit_default_low[cat[activeRadio]->band()])
                    .toDouble(),
                settings
                    ->value(s_sdr_cqlimit_high[cat[activeRadio]->band()],
                            cqlimit_default_high[cat[activeRadio]->band()])
                    .toDouble());
            bandmap->findFreq(activeRadio);
            break;
          case 23: // best cq radio2
            if (cat[activeRadio ^ 1]->band() == BAND_NONE)
              break;
            bandmap->setFreqLimits(
                activeRadio ^ 1,
                settings
                    ->value(s_sdr_cqlimit_low[cat[activeRadio ^ 1]->band()],
                            cqlimit_default_low[cat[activeRadio]->band()])
                    .toDouble(),
                settings
                    ->value(s_sdr_cqlimit_high[cat[activeRadio ^ 1]->band()],
                            cqlimit_default_high[cat[activeRadio ^ 1]->band()])
                    .toDouble());
            bandmap->findFreq(activeRadio ^ 1);
            // return immediately to avoid stopping cw on current radio
            return;
          case 24: // cancel speed change
            if (cw->cwDevice() == modeWinkey)
              out.append(0x1e);
            break;
          case 25: // play
            command = msg.mid(i2 + 1, msg.indexOf("{", i2) - (i2 + 1));
            ssbMessage->playMessage(activeRadio, command);
            break;
          case 26: // switch radios
            switchRadios();
            break;
          case 27: // MCP
            command = msg.mid(i2 + 1, msg.indexOf("{", i2) - (i2 + 1));
            command.append("\r");
            so2r->sendMicrohamCommand(command);
            i2 = msg.indexOf("}", msg.indexOf("{", i2));
            break;
          case 28: // OTRSP (send to both devices)
            command = msg.mid(i2 + 1, msg.indexOf("{", i2) - (i2 + 1));
            command.append("\r");
            so2r->sendOtrspCommand(command, 0);
            so2r->sendOtrspCommand(command, 1);
            i2 = msg.indexOf("}", msg.indexOf("{", i2));
            break;
          case 29: // CAT
            command = msg.mid(i2 + 1, msg.indexOf("{", i2) - (i2 + 1));
            cat[activeRadio]->sendRaw(command);
            i2 = msg.indexOf("}", msg.indexOf("{", i2));
            break;
          case 30: // CATR2
            command = msg.mid(i2 + 1, msg.indexOf("{", i2) - (i2 + 1));
            cat[activeRadio ^ 1]->sendRaw(command);
            i2 = msg.indexOf("}", msg.indexOf("{", i2));
            break;
          case 31: // CAT1
            command = msg.mid(i2 + 1, msg.indexOf("{", i2) - (i2 + 1));
            cat[0]->sendRaw(command);
            i2 = msg.indexOf("}", msg.indexOf("{", i2));
            break;
          case 32: // CAT2
            command = msg.mid(i2 + 1, msg.indexOf("{", i2) - (i2 + 1));
            cat[1]->sendRaw(command);
            i2 = msg.indexOf("}", msg.indexOf("{", i2));
            break;
          case 33: // CALL_OK
            origCallEntered[activeRadio] = qso[activeRadio]->call;
            break;
          case 34: // SCRIPT
            // text up to first space is script name
            command = msg.mid(i2 + 1, msg.indexOf(" ", i2) - (i2 + 1));
            i2 = msg.indexOf(" ", i2);
            // out will hold command line arguments
            out.append(msg.mid(i2 + 1, msg.indexOf("{", i2) - (i2 + 1)));
            i2 = msg.indexOf("{", i2) - 1;
            switchradio = false;
            break;
          case 35: // SCRIPTNR
            // text up to first space is script name
            command = msg.mid(i2 + 1, msg.indexOf(" ", i2) - (i2 + 1));
            i2 = msg.indexOf(" ", i2);
            // out will hold command line arguments
            out.append(msg.mid(i2 + 1, msg.indexOf("{", i2) - (i2 + 1)));
            i2 = msg.indexOf("{", i2) - 1;
            switchradio = false;
            // replace # with radio number
            if (activeRadio == 0) {
              out.replace('#', '0');
            } else {
              out.replace('#', '1');
            }
            switchradio = false;
            break;
          case 36: // PTTON
            so2r->setPtt(activeRadio, 1);
            switchradio = false;
            break;
          case 37: // PTTON1
            so2r->setPtt(0, 1);
            switchradio = false;
            break;
          case 38: // PTTON2
            so2r->setPtt(1, 1);
            switchradio = false;
            break;
          case 39: // PTTONR2
            so2r->setPtt(activeRadio ^ 1, 1);
            switchradio = false;
            break;
          case 40: // PTTOFF
            so2r->setPtt(activeRadio, 0);
            switchradio = false;
            break;
          case 41: // PTTOFF1
            so2r->setPtt(0, 0);
            switchradio = false;
            break;
          case 42: // PTTOFF2
            so2r->setPtt(1, 0);
            switchradio = false;
            break;
          case 43: // PTTOFFR2
            so2r->setPtt(activeRadio ^ 1, 0);
            switchradio = false;
            break;
          case 44: // RECORD
            command = msg.mid(i2 + 1, msg.indexOf("{", i2) - (i2 + 1));
            ssbMessage->recMessage(command);
            break;
          case 45: // 2KBD
            twokeyboard = !twokeyboard;
            twoKeyboard();
            twokeyboardAction->setChecked(twokeyboard);
            return;
          case 46:
          case 47: // /SCRIPT, /SCRIPTNR
            if (!command.isEmpty()) {
              runScript(command, out.simplified());
              command.clear();
              out.clear();
            }
            switchradio = false;
            break;
          case 48: // EXCH_ENTERED
            if (!toggleMode) {
              out.append(lineEditExchange[activeRadio]->text().toLatin1());
            } else {
              out.append(lineEditExchange[activeTxRadio]->text().toLatin1());
            }
            break;
          }
          break;
        }
      }
      if (first && switchradio) {
        // the first element of most macros resets TX radio and speed
        // TOGGLESTEREOPIN, R2, R2CQ do not
        if (toggleMode) {
          switchTransmit(activeTxRadio,
                         tmp_wpm); // don't switch TX focus, pass speed change
        } else if (autoCQMode && !autoCQModePause) {
          switchTransmit(autoCQRadio, tmp_wpm);
        } else {
          switchTransmit(activeRadio, tmp_wpm);
        }
      }
      first = false;

      i0 = i2 + 1;
      i1 = msg.indexOf("{", i2); // find next one
    }

    // add any remaining text
    if (i2 < (i1 = msg.size())) {
      out.append(msg.right(i1 - i2 - 1));
    }

    // winkey: cancel any buffered speed changes
    if (cw->cwDevice() == modeWinkey)
      out.append(0x1e);
    if (repeat) {
      send(lastMsg, stopcw);
    } else {
      send(out, stopcw);
    }
  } else {
    // no macro present send as-is
    if (instant) {
      queue.clear();
      rqueue.clear();
    }
    if (toggleMode) {
      switchTransmit(activeTxRadio,
                     tmp_wpm); // don't switch TX focus, pass speed change
    } else if (autoCQMode && !autoCQModePause) {
      switchTransmit(autoCQRadio, tmp_wpm);
    } else {
      switchTransmit(activeRadio, tmp_wpm);
    }
    out.append(msg);
    send(out, stopcw);
  }
  lastMsg = out;
}

/*! show message on status bar
 */
void So2sdr::showMessage(QString s) {
  if (!statusBarDupe) {
    So2sdrStatusBar->showMessage(s.simplified());
  }
}

/*!
   Open file dialog
 */
void So2sdr::openFile() {
  // search for files in directory set by contestDirectory
  QDir::setCurrent(contestDirectory);
  fileName = QFileDialog::getOpenFileName(this, tr("Open config file"),
                                          contestDirectory,
                                          tr("Config Files (*.cfg)"));
  if (fileName.isNull()) {
    return;
  }

  // update contest directory
  int i = fileName.lastIndexOf("/");
  if (i != -1) {
    contestDirectory = fileName;
    contestDirectory.truncate(i);
  }
  if (setupContest()) {
    actionOpen->setEnabled(false);
    actionNewContest->setEnabled(false);
    // enable two keyboard mode. It is not enabled before because otherwise
    // file dialog cannot get keyboard input
    if (twokeyboard) {
      twoKeyboard();
    }
  }
}

/*!
   rescore and redupe

 */
void So2sdr::rescore() {
  log->rescore();
  updateBreakdown();
  updateMults(activeRadio);
}

/*!
   fill in sent exchange fields. Two fields are auto-filled: qso # and RST.
   All others are taken from values entered in contest options dialog.
*/
void So2sdr::fillSentExch(Qso *qso, int nr) {
  for (int i = 0; i < log->nExch(); i++) {
    switch (qso->exchange_type[i]) {
    case QsoNumber:
      qso->snt_exch[i] = QByteArray::number(nr);
      break;
    case RST:
      switch (qso->modeType) {
      case CWType:
      case DigiType:
        qso->snt_exch[i] = "599";
        break;
      case PhoneType:
        qso->snt_exch[i] = "59";
      }
      break;
    default:
      switch (i) {
      case 0:
        qso->snt_exch[0] =
            csettings->value(c_sentexch1, c_sentexch1_def).toByteArray();
        break;
      case 1:
        qso->snt_exch[1] =
            csettings->value(c_sentexch2, c_sentexch2_def).toByteArray();
        break;
      case 2:
        qso->snt_exch[2] =
            csettings->value(c_sentexch3, c_sentexch3_def).toByteArray();
        break;
      case 3:
        qso->snt_exch[3] =
            csettings->value(c_sentexch4, c_sentexch4_def).toByteArray();
        break;
      }
    }
  }
}

/*!
   Show help file
 */
void So2sdr::showHelp() {
  help->show();
  help->setFocus();
}

/*!
   Switch display of mult type
 */
void So2sdr::switchMultMode() {
  if (csettings->value(c_nmulttypes, c_nmulttypes_def).toInt() < 2)
    return;
  multMode++;
  multMode = multMode % MMAX;
  updateMults(activeRadio);
}

/*!
   update display of next qso number to send
 */
void So2sdr::updateNrDisplay() {
  // number reserved: show in bold
  for (int i = 0; i < NRIG; i++) {
    if (nrReserved[i]) {
      numLabelPtr[i]->setText("<b>" + QString::number(nrReserved[i]) + "</b>");
    } else {
      numLabelPtr[i]->setText(QString::number(nrSent));
    }
  }
}

/*! check to see if userDirectory() exists. If not, give options to
   create it. Also creates subdirectory for wav files.

   returns true if directory exists or was created; false if program
   should exit.
 */
bool So2sdr::checkUserDirectory() {
  QDir dir;
  dir.setCurrent(userDirectory());
  if (dir.exists(userDirectory())) {
    dir.mkdir("wav");
    if (!QFile::exists(userDirectory() + "/wl_cty.dat")) {
      QFile::copy(dataDirectory() + "wl_cty.dat",
                  userDirectory() + "/wl_cty.dat");
    }
    if (!QFile::exists(userDirectory() + "/wl_cty-ns.dat")) {
      QFile::copy(dataDirectory() + "wl_cty-ns.dat",
                  userDirectory() + "/wl_cty-ns.dat");
    }
    return (true);
  }
  QMessageBox *msg = new QMessageBox(this);
  msg->setWindowTitle("Error");
  msg->setText("User data directory " + userDirectory() + " does not exist.");
  msg->setInformativeText("Create it?");
  msg->setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
  msg->setDefaultButton(QMessageBox::Yes);
  int ret = msg->exec();
  switch (ret) {
  case QMessageBox::Yes:
    // create directory
    if (dir.mkdir(userDirectory())) {
      dir.setCurrent(userDirectory());
      dir.mkdir("wav");
      if (!QFile::exists(userDirectory() + "/wl_cty.dat")) {
        QFile::copy(dataDirectory() + "wl_cty.dat",
                    userDirectory() + "/wl_cty.dat");
      }
      if (!QFile::exists(userDirectory() + "/wl_cty-ns.dat")) {
        QFile::copy(dataDirectory() + "wl_cty-ns.dat",
                    userDirectory() + "/wl_cty-ns.dat");
      }
      msg->deleteLater();
      return (true);
    } else {
      msg->setText("Could not create directory <" + userDirectory() + ">");
      msg->exec();
      msg->deleteLater();
      return (false);
    }
  case QMessageBox::Cancel:
    // this will abort the program (return false)
    break;
  default: // never reached
    break;
  }
  msg->deleteLater();
  return (false);
}

/*! start timers
 */
void So2sdr::startTimers() {
  for (int i = 0; i < N_TIMERS; i++)
    timerId[i] = startTimer(timerSettings[i]);
}

/*! stop timers
 */
void So2sdr::stopTimers() {
  for (int i = 0; i < N_TIMERS; i++) {
    if (timerId[i])
      killTimer(timerId[i]);
  }
}

/*! validates freq entered in Khz, returns freq in Hz
   if exact=true, no validation (freq should be in Hz in this case)
 */
void So2sdr::qsy(int nrig, double &freq, bool exact) {
  if (exact) {
    if (nrig == 0)
      emit qsyExact1(freq);
    else
      emit qsyExact2(freq);
  } else {
    // entered in KHz?
    if (freq > 1799 && freq <= 148000) {
      freq *= 1000;
      if (nrig == 0)
        emit qsyExact1(freq);
      else
        emit qsyExact2(freq);
    }
  }
}

/*!
  logSearch : triggered with ctrl-F

  do callsign-partial search on contents of call entry window, display
  matching calls in log window for editing
  */
void So2sdr::logSearch(int nr) {
  QByteArray searchFrag = lineEditCall[nr]->text().toLatin1();
  So2sdrStatusBar->showMessage("Searching log for \"" + searchFrag + "\"");
  if (!log->logSearch(searchFrag)) {
    So2sdrStatusBar->showMessage("Searching log for \"" + searchFrag +
                                 "\" : NOT FOUND");
  } else {
    LogTableView->scrollToBottom();
    logSearchFlag = true;
  }
}

/*!
  clears any log search in progress and resets SQL filter
  */
void So2sdr::clearLogSearch() {
  if (logSearchFlag) {
    log->logSearchClear();
    LogTableView->scrollToBottom();
    logSearchFlag = false;
    So2sdrStatusBar->clearMessage();
    LogTableView->clearSelection();
  }
}

/*!
  log partial callsign search
  */
void So2sdr::searchPartial(Qso *qso, QByteArray part, QList<QByteArray> &calls,
                           QList<unsigned int> &worked, QList<int> &mult1,
                           QList<int> &mult2) {
  calls.clear();
  worked.clear();
  mult1.clear();
  mult2.clear();

  // only do partial call search if we have >= 2 characters
  const int psz = part.size();
  if (psz < 2) {
    return;
  }
  log->searchPartial(qso, part, calls, worked, mult1, mult2);

  // fill exchange with history if not in log
  if (qso->prefill.simplified().isEmpty() &&
      csettings->value(c_historymode, c_historymode_def).toBool()) {
    history->fillExchange(qso, part);
  }
}

/*! returns true if initialization was successful
 */
bool So2sdr::so2sdrOk() const { return (initialized); }

/*!
  read settings from station .ini file
 */
void So2sdr::readStationSettings() {
  contestDirectory =
      settings->value(s_contestdirectory, s_contestdirectory_def).toString();
  setWindowTitle("SO2SDR:" + settings->value(s_call, s_call_def).toString());
  for (int i = 0; i < NRIG; i++) {
    wpm[i] = settings->value(s_wpm[i], s_wpm_def[i]).toInt();
    wpmLineEditPtr[i]->setText(QString::number(wpm[i]));
  }
}

/*!
  update settings to contest .cfg file
  */
void So2sdr::writeContestSettings() {
  if (!csettings)
    return;

  // log column widths
  csettings->beginWriteArray(c_col_width_group);
  for (int i = 0; i < SQL_N_COL; i++) {
    csettings->setArrayIndex(i);
    csettings->setValue(c_col_width_item, LogTableView->columnWidth(i));
  }
  csettings->endArray();
}

/*!
  update settings to station .ini file
  */
void So2sdr::writeStationSettings() {
  // main window geometry
  settings->setValue(s_contestdirectory, contestDirectory);
  settings->beginGroup("MainWindow");
  settings->setValue("size", size());
  settings->setValue("pos", pos());
  settings->endGroup();

  // dupesheet geometry
  if (dupesheet[0]) {
    settings->beginGroup("DupeSheetWindow1");
    settings->setValue("size", dupesheet[0]->size());
    settings->setValue("pos", dupesheet[0]->pos());
    settings->endGroup();
  }
  if (dupesheet[1]) {
    settings->beginGroup("DupeSheetWindow2");
    settings->setValue("size", dupesheet[1]->size());
    settings->setValue("pos", dupesheet[1]->pos());
    settings->endGroup();
  }
  // wsjtx window geometry
  if (wsjtx[0]->isEnabled()) {
    settings->beginGroup("WsjtxWindow1");
    settings->setValue("size", wsjtx[0]->size());
    settings->setValue("pos", wsjtx[0]->pos());
    settings->endGroup();
  }
  if (wsjtx[1]->isEnabled()) {
    settings->beginGroup("WsjtxWindow2");
    settings->setValue("size", wsjtx[1]->size());
    settings->setValue("pos", wsjtx[1]->pos());
    settings->endGroup();
  }
  settings->setValue(s_wpm[0], wpmLineEditPtr[0]->text());
  settings->setValue(s_wpm[1], wpmLineEditPtr[1]->text());
}

void So2sdr::closeEvent(QCloseEvent *event) {
  for (int i = 0; i < NRIG; i++) {
    bandmap->closeBandmap(i);
  }
  writeContestSettings();
  writeStationSettings();
  event->accept();
}

/*!
   Read in a list of mults from given file. These mults
   will not appear in the on-screen display to save screen space.

   Typically exclude some really rare country mults from being shown.
 */
void So2sdr::readExcludeMults() {
  QDir::setCurrent(dataDirectory());
  for (int ii = 0; ii < MMAX; ii++) {
    QString filename;
    switch (ii) {
    case 0:
      filename =
          csettings->value(c_exclude_mults1, c_exclude_mults1_def).toString();
      break;
    case 1:
      filename =
          csettings->value(c_exclude_mults2, c_exclude_mults2_def).toString();
      break;
    }
    if (filename.isEmpty())
      continue;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qDebug("Can't open %s", filename.toLatin1().data());
      return;
    }
    excludeMults[ii].clear();
    while (!file.atEnd()) {
      QString buffer;
      buffer = file.readLine();
      QStringList list = buffer.split(" ", Qt::SkipEmptyParts);
      for (int i = 0; i < list.size(); i++) {
        excludeMults[ii].append(list[i].toLatin1());
      }
    }
    file.close();
  }
  // return to contest directory
  QDir::setCurrent(contestDirectory);
}

void So2sdr::initPointers() {
  csettings = nullptr;
  settings = nullptr;
  multMode = 0;
  help = nullptr;
  cat[0] = nullptr;
  cat[1] = nullptr;
  cabrillo = nullptr;
  downloader = nullptr;
  log = nullptr;
  cwMessage = nullptr;
  ssbMessage = nullptr;
  newContest = nullptr;
  notes = nullptr;
  options = nullptr;
  radios = nullptr;
  station = nullptr;
  sdr = nullptr;
  cw = nullptr;
  cwDialog = nullptr;
  master = nullptr;
  history = nullptr;
  dupesheet[0] = nullptr;
  dupesheet[1] = nullptr;
  qso[0] = nullptr;
  qso[1] = nullptr;
  bandmap = nullptr;
  scriptProcess = nullptr;
  telnet = nullptr;
  kbdHandler[0] = nullptr;
  kbdHandler[1] = nullptr;
  wpmLineEditPtr[0] = WPMLineEdit;
  wpmLineEditPtr[1] = WPMLineEdit2;
  lineEditCall[0] = lineEditCall1;
  lineEditCall[1] = lineEditCall2;
  lineEditExchange[0] = lineEditExchange1;
  lineEditExchange[1] = lineEditExchange2;
  freqDisplayPtr[0] = FreqDisplay;
  freqDisplayPtr[1] = FreqDisplay2;
  modeDisplayPtr[0] = ModeDisplay;
  modeDisplayPtr[1] = ModeDisplay2;
  labelCountry[0] = labelCountry1;
  labelCountry[1] = labelCountry2;
  labelBearing[0] = labelBearing1;
  labelBearing[1] = labelBearing2;
  labelLPBearing[0] = labelLPBearing1;
  labelLPBearing[1] = labelLPBearing2;
  qsoLabel[0] = qso160Label;
  qsoLabel[1] = qso80Label;
  qsoLabel[2] = qso40Label;
  qsoLabel[3] = qso20Label;
  qsoLabel[4] = qso15Label;
  qsoLabel[5] = qso10Label;
  qsoWorkedLabel[0] = Qso1Label;
  qsoWorkedLabel[1] = Qso2Label;
  multWorkedLabel[0][0] = Mult1Label;
  multWorkedLabel[0][1] = Mult1Label2;
  multWorkedLabel[1][0] = Mult2Label;
  multWorkedLabel[1][1] = Mult2Label2;
  bandLabel[0] = label_160;
  bandLabel[1] = label_80;
  bandLabel[2] = label_40;
  bandLabel[3] = label_20;
  bandLabel[4] = label_15;
  bandLabel[5] = label_10;
  bandQsoLabel[0] = qso160Label;
  bandQsoLabel[1] = qso80Label;
  bandQsoLabel[2] = qso40Label;
  bandQsoLabel[3] = qso20Label;
  bandQsoLabel[4] = qso15Label;
  bandQsoLabel[5] = qso10Label;
  multNameLabel[0] = multName;
  multNameLabel[1] = multName2;
  multLabel[0][0] = mult160Label;
  multLabel[0][1] = mult80Label;
  multLabel[0][2] = mult40Label;
  multLabel[0][3] = mult20Label;
  multLabel[0][4] = mult15Label;
  multLabel[0][5] = mult10Label;
  multLabel[1][0] = mult160Label2;
  multLabel[1][1] = mult80Label2;
  multLabel[1][2] = mult40Label2;
  multLabel[1][3] = mult20Label2;
  multLabel[1][4] = mult15Label2;
  multLabel[1][5] = mult10Label2;
  multTotal[0] = TotalMultLabel;
  multTotal[1] = TotalMultLabel2;
  sunLabelPtr[0] = Sun1Label;
  sunLabelPtr[1] = Sun2Label;
  numLabelPtr[0] = NumLabel;
  numLabelPtr[1] = NumLabel2;
  validLabel[0] = validLabel1;
  validLabel[1] = validLabel2;
}

void So2sdr::initVariables() {
  initialized = true;
  uiEnabled = false;
  labelCountry1->clear();
  labelCountry2->clear();
  labelBearing1->clear();
  labelBearing2->clear();
  labelLPBearing1->clear();
  labelLPBearing2->clear();
  activeRadio = 0;
  activeTxRadio = -1; // force switchTransmit to update SO2R device
  altDActive = 0;
  altDActiveRadio = 0;
  altDOrigMode = 0;
  cqQsoInProgress[0] = false;
  cqQsoInProgress[1] = false;
  for (int i = 0; i < N_BANDS; i++)
    spotList[i].clear();
  spotListPopUp[0] = false;
  spotListPopUp[1] = false;
  callSent[0] = false;
  callSent[1] = false;
  logSearchFlag = false;
  statusBarDupe = false;
  multNameLabel[0]->clear();
  multNameLabel[1]->clear();
  for (int i = 0; i < N_BANDS; i++) {
    if (i == N_BANDS_HF)
      break;
    multLabel[0][i]->clear();
    multLabel[1][i]->clear();
  }
  wpm[0] = 35;
  wpm[1] = 35;
  nrSent = 1;
  nrReserved[0] = 0;
  nrReserved[1] = 0;
  for (int i = 0; i < N_TIMERS; i++)
    timerId[i] = 0;
  ratePtr = 0;
  for (int i = 0; i < 60; i++)
    rateCount[i] = 0;
  telnetOn = false;
  for (int i = 0; i < MMAX; i++) {
    excludeMults[i].clear();
  }
  origCallEntered[0].clear();
  origCallEntered[1].clear();
  fileName.clear();
  for (int i = 0; i < N_BANDS; i++) {
    bandInvert[0][i] = false;
    bandInvert[1][i] = false;
  }
  cqMode[0] = true;
  cqMode[1] = true;
  excMode[0] = false;
  excMode[1] = false;
  callFocus[0] = true;
  callFocus[1] = true;
  activeR2CQ = false;
  exchangeSent[0] = false;
  exchangeSent[1] = false;
  dupeCheckDone = false;
  cqQsoInProgress[0] = false;
  cqQsoInProgress[1] = false;
  grab = false;
  twokeyboard = false;
  editingExchange[0] = false;
  editingExchange[1] = false;

  toggleMode = false;
  autoCQMode = false;
  autoCQModePause = false;
  autoCQModeWait = true;
  duelingCQMode = false;
  duelingCQWait = true;
  duelingCQModePause = false;
  autoSend = false;
  autoSendPause = false;
  autoSendTrigger = false;
  sendLongCQ = true;
  autoSendCall.clear();
  aboutOn = false;
  enterCW[0] = false;
  enterCW[1] = false;
  queue.clear();
  rqueue.clear();
  uiStyleSheet.clear();
  textStyleSheet.clear();
  textEntryStyleSheet.clear();
}

/*!
 * \brief So2sdr::screenShot
 * Take a screenshot of main and bandmap windows
 *
 */
void So2sdr::screenShot() {
  // main window
  QScreen *screen = QGuiApplication::primaryScreen();
  QCoreApplication::processEvents();
  QPixmap p = screen->grabWindow(winId());
  QCoreApplication::processEvents();
  QString format = "png";
  QDir::setCurrent(contestDirectory);
  QString filename = "screenshot-main-" +
                     QDateTime::currentDateTimeUtc().toString(Qt::ISODate) +
                     ".png";
  p.save(filename, format.toLatin1());
  QCoreApplication::processEvents();
  So2sdrStatusBar->showMessage("Saved screenshot", 3000);

  // bandmaps
  for (int i = 0; i < NRIG; i++) {
    if (bandmap->bandmapon(i) && bandmap->winId(i) != 0) {
      p = screen->grabWindow(bandmap->winId(i));
      QCoreApplication::processEvents();
      filename = "screenshot-bandmap-" + QString::number(i + 1) + "-" +
                 QDateTime::currentDateTimeUtc().toString(Qt::ISODate) + ".png";
      p.save(filename, format.toLatin1());
      QCoreApplication::processEvents();
    }
  }
  So2sdrStatusBar->showMessage("Saved screenshot", 3000);
}

void So2sdr::showWsjtx1(bool state) {
  menuWindows->hide();
  if (state) {
    wsjtx[0]->replay();
    wsjtx[0]->show();
  } else {
    wsjtx[0]->hide();
  }
}

void So2sdr::showWsjtx2(bool state) {
  menuWindows->hide();
  if (state) {
    wsjtx[1]->replay();
    wsjtx[1]->show();
  } else {
    wsjtx[1]->hide();
  }
}

void So2sdr::showBandmap1(bool state) {
  menuWindows->hide();
  bandmap->showBandmap(0, state);
}

void So2sdr::showBandmap2(bool state) {
  menuWindows->close();
  bandmap->showBandmap(1, state);
}

void So2sdr::runScript(const QString &cmd, QString args) {
  So2sdrStatusBar->showMessage("Script:" + cmd, 3000);
  QString program = userDirectory() + "/scripts/" + cmd;
  QStringList arglist = args.simplified().split(" ");
  QProcess::execute(program, arglist);
}

void So2sdr::showRecordingStatus(bool b) {
  if (b) {
    So2sdrStatusBar->showMessage("RECORDING AUDIO");
  } else {
    So2sdrStatusBar->clearMessage();
  }
}

void So2sdr::setDupeColor(int nr, bool dupe) {
  if (dupe) {
    if (lineEditCall[nr]->palette().color(QPalette::Base) == DUPE_COLOR) {

    } else {
      QPalette palette(lineEditCall[nr]->palette());
      palette.setColor(QPalette::Base, DUPE_COLOR);
      lineEditCall[nr]->setPalette(palette);
    }
    if (lineEditExchange[nr]->palette().color(QPalette::Base) == DUPE_COLOR) {

    } else {
      QPalette palette(lineEditExchange[nr]->palette());
      palette.setColor(QPalette::Base, DUPE_COLOR);
      lineEditExchange[nr]->setPalette(palette);
    }
  } else {
    if (lineEditCall[nr]->palette().color(QPalette::Base) == DUPE_COLOR) {
      if (!altDActive || nr != altDActiveRadio) {
        QPalette palette(lineEditCall[nr]->palette());
        QPalette palette2(lineEditExchange[nr]->palette());
        if (cqMode[nr]) {
          palette.setColor(QPalette::Base, CQ_COLOR);
          palette2.setColor(QPalette::Base, CQ_COLOR);
        } else {
          palette.setColor(QPalette::Base, SP_COLOR);
          palette2.setColor(QPalette::Base, SP_COLOR);
        }
        lineEditCall[nr]->setPalette(palette);
        lineEditExchange[nr]->setPalette(palette2);
      }
    }
  }
}

/*! start download of CTY file */
void So2sdr::updateCty() {
  if (settings->value(s_cty_url, s_cty_url_def).toString().isEmpty())
    return;
  QUrl url(settings->value(s_cty_url, s_cty_url_def).toString());
  if (!downloader) {
    downloader = new FileDownloader(url, this);
  }
  connect(downloader, SIGNAL(downloaded()), this, SLOT(checkCtyVersion()));
}

/*! Slot called when CTY file has been downloaded. Checks version to see if
 * newer than current version, prompts user for update */
void So2sdr::checkCtyVersion() {
  QByteArray newCty = downloader->downloadedData();
  if (newCty.contains("=VER")) {
    // extract version string
    int indx = newCty.indexOf("=VER");
    if (newCty.mid(indx + 1, 7) == "VERSION") {
      indx = newCty.indexOf("=VER", indx + 1);
    }
    QString newVer = newCty.mid(indx + 4, 8);
    QDate newDate = QDate(newVer.left(4).toInt(), newVer.mid(4, 2).toInt(),
                          newVer.right(2).toInt());

    // get current CTY file version
    QDir dir;
    dir.setCurrent(userDirectory());
    QFileInfo oldInfo("wl_cty.dat");
    bool oldExists = false;
    QDate oldDate;
    if (oldInfo.exists()) {
      QFile oldCty("wl_cty.dat");
      if (!oldCty.open(QIODevice::ReadOnly | QIODevice::Text)) {
        downloader->deleteLater();
        downloader = nullptr;
        return;
      }
      while (!oldCty.atEnd()) {
        QByteArray line = oldCty.readLine();
        int indx = line.indexOf("=VER");
        if (indx == -1 || line.mid(indx + 1, 7) == "VERSION")
          continue;
        QString oldVer = line.mid(indx + 4, 8);
        oldDate = QDate(oldVer.left(4).toInt(), oldVer.mid(4, 2).toInt(),
                        oldVer.right(2).toInt());
        oldExists = true;
      }
      oldCty.close();
    }

    // compare dates
    if (!oldExists || newDate > oldDate) {
      QMessageBox *msg = new QMessageBox(this);
      msg->setText("New CTY file version " + newVer + " is available.");
      msg->setInformativeText("Install it?");
      msg->setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
      msg->setDefaultButton(QMessageBox::Yes);
      msg->setModal(true);
      int ret = msg->exec();
      switch (ret) {
      case QMessageBox::Yes: {
        // save new CTY file
        QFile save("wl_cty.dat");
        if (!save.open(QIODevice::WriteOnly | QIODevice::Text)) {
          downloader->deleteLater();
          downloader = nullptr;
          return;
        }
        save.write(newCty);
        save.close();
        // save new NCCC Sprint CTY file with some countries changed from SA
        // to NA
        newCty.replace("Trinidad & Tobago:        09:  11:  SA",
                       "Trinidad & Tobago:        09:  11:  NA");
        newCty.replace("Aruba:                    09:  11:  SA",
                       "Aruba:                    09:  11:  NA");
        newCty.replace("Curacao:                  09:  11:  SA",
                       "Curacao:                  09:  11:  NA");
        newCty.replace("Bonaire:                  09:  11:  SA",
                       "Bonaire:                  09:  11:  NA");
        save.setFileName("wl_cty-ns.dat");
        if (!save.open(QIODevice::WriteOnly | QIODevice::Text)) {
          downloader->deleteLater();
          downloader = nullptr;
          return;
        }
        save.write(newCty);
        save.close();
        break;
      }
      case QMessageBox::Cancel:
        msg->deleteLater();
        downloader->deleteLater();
        downloader = nullptr;
        return;
      default: // never reached
        break;
      }
      msg->deleteLater();
    }
  } else {
    // CTY download failed (incomplete), missing version string
    downloader->deleteLater();
    downloader = nullptr;
    return;
  }
}

/*! add a wsjtx qso to log.
 *
 */
void So2sdr::logWsjtx(Qso *qso) {
  bool qsy;
  int pp = log->idPfx(qso, qsy);
  Q_UNUSED(pp)
  fillSentExch(qso, nrReserved[qso->nr]);
  qso->externalQso = true;
  qso->valid = log->validateExchange(qso);
  qso->mode = cat[qso->nr]->mode();
  qso->modeType = getAdifModeType(qso->adifMode);
  log->isDupe(qso, log->dupeCheckingByBand(), false);
  addQso(qso);
  rateCount[ratePtr]++;
  nrSent = log->rowCount() + 1;
  updateNrDisplay();
  updateMults(qso->nr, qso->band);
  updateBreakdown();
  if (wsjtx[qso->nr]->isEnabled())
    wsjtx[qso->nr]->redupe();
}

/*! Set fonts and font sizes
 *
 */
void So2sdr::setFontSize() {
  // set default UI font size
  uiStyleSheet =
      "font-family: " + settings->value(s_ui_font, s_ui_font_def).toString() +
      ";font-size: " +
      QString::number(
          settings->value(s_ui_font_size, s_ui_font_size_def).toInt()) +
      "px;";
  setStyleSheet(uiStyleSheet);
  progsettings->setStyleSheet(uiStyleSheet);
  station->setStyleSheet(uiStyleSheet);
  radios->setStyleSheet(uiStyleSheet);
  sdr->setStyleSheet(uiStyleSheet);
  cwMessage->setStyleSheet(uiStyleSheet);
  ssbMessage->setStyleSheet(uiStyleSheet);
  wsjtx[0]->setStyleSheet(uiStyleSheet);
  wsjtx[1]->setStyleSheet(uiStyleSheet);
  if (telnet)
    telnet->setStyleSheet(uiStyleSheet);
  menubar->setStyleSheet(uiStyleSheet);
  QFont font = this->font();
  actionOpen->setFont(font);
  actionADIF->setFont(font);
  actionCabrillo->setFont(font);
  actionHistory->setFont(font);
  actionImport_Cabrillo->setFont(font);
  actionNewContest->setFont(font);
  actionQuit->setFont(font);
  actionCW_Messages->setFont(font);
  actionContestOptions->setFont(font);
  actionRadios->setFont(font);
  actionSDR->setFont(font);
  actionSSB_Messages->setFont(font);
  actionSettings->setFont(font);
  actionSo2r->setFont(font);
  actionStation->setFont(font);
  actionWinkey->setFont(font);
  bandmapAction1->setFont(font);
  bandmapAction2->setFont(font);
  dupesheetAction1->setFont(font);
  dupesheetAction2->setFont(font);
  grabAction->setFont(font);
  telnetAction->setFont(font);
  twokeyboardAction->setFont(font);
  wsjtxAction1->setFont(font);
  wsjtxAction2->setFont(font);
  actionAbout->setFont(font);
  actionHelp->setFont(font);

  // some widgets need transparent background
  WPMLineEdit->setStyleSheet(uiStyleSheet + " background: transparent;");
  WPMLineEdit2->setStyleSheet(uiStyleSheet + " background: transparent;");

  // size of text in mult box, call box, log
  textStyleSheet =
      "font-family: " +
      settings->value(s_text_font, s_text_font_def).toString() +
      ";font-size: " +
      QString::number(
          settings->value(s_text_font_size, s_text_font_size_def).toInt()) +
      "px;";
  MasterTextEdit->setStyleSheet(textStyleSheet);
  MultTextEdit->setStyleSheet(textStyleSheet);
  LogTableView->setStyleSheet(textStyleSheet);
  if (telnet)
    telnet->TelnetTextEdit->setStyleSheet(textStyleSheet);

  // size of text in entry windows
  textEntryStyleSheet =
      "font-family: " +
      settings->value(s_entry_font, s_entry_font_def).toString() +
      ";font-size: " +
      QString::number(
          settings->value(s_entry_font_size, s_entry_font_size_def).toInt()) +
      "px;";
  lineEditCall1->setStyleSheet(textEntryStyleSheet);
  lineEditCall2->setStyleSheet(textEntryStyleSheet);
  lineEditExchange1->setStyleSheet(textEntryStyleSheet);
  lineEditExchange2->setStyleSheet(textEntryStyleSheet);
}
