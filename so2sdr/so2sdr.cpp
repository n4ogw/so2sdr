/*! Copyright 2010-2015 R. Torsten Clay N4OGW

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
#include <QCheckBox>
#include <QColor>
#include <QDataStream>
#include <QDateTime>
#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QErrorMessage>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QHostAddress>
#include <QList>
#include <QMainWindow>
#include <QMessageBox>
#include <QModelIndex>
#include <QObject>
#include <QPixmap>
#include <QPalette>
#include <QProgressDialog>
#include <QSettings>
#include <QScrollBar>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QString>
#include <QStringList>
#include <QStyle>
#include <QThread>
#include <QTimer>
#include <QWidgetAction>
#include "hamlib/rig.h"
#include "defines.h"
#include "so2sdr.h"
#include "utils.h"

So2sdr::So2sdr(QStringList args, QWidget *parent) : QMainWindow(parent)
{
    setupUi(this);
    initPointers();
    initVariables();

    // Register rmode_t, pbwidth_t for connect()
    qRegisterMetaType<rmode_t>("rmode_t");
    qRegisterMetaType<pbwidth_t>("pbwidth_t");

    // check to see if user directory exists
    initialized = checkUserDirectory();

    settingsFile=userDirectory()+"/so2sdr.ini";
    // check for optional command argument giving station config file name
    if (args.size() > 1) {
        settingsFile = args[1];
    }
    settings=new QSettings(settingsFile,QSettings::IniFormat);
    setFocusPolicy(Qt::StrongFocus);
    errorBox           = new QErrorMessage(this);
    setWindowIcon(QIcon(dataDirectory() + "/icon24x24.png"));
    if (!iconValid.load(dataDirectory() + "/check.png")) {
        qDebug("file check.png missing");
    }

    // pointers for each radio
    for (int i=0;i<NRIG;i++) {
        wpmLineEditPtr[i]->setFocusPolicy(Qt::NoFocus);
        lineEditCall[i]->setValidator(new UpperValidator(lineEditCall[i]));
        lineEditExchange[i]->setValidator(new UpperValidator(lineEditExchange[i]));
    }
    rLabelPtr[0]      = new QLabel("<font color=#FF0000>R1:OFF /font>");
    rLabelPtr[1]      = new QLabel("<font color=#FF0000>R2:OFF </font>");
    winkeyLabel       = new QLabel("<font color=#FF0000>WK:OFF </font>");
    grabLabel         = new QLabel("Grab");
    offPtr            = new QLabel("");
    autoCQStatus      = new QLabel("");
    duelingCQStatus   = new QLabel("");
    toggleStatus      = new QLabel("");
    autoSendStatus      = new QLabel("");
    redLED    = "QLabel { background-color : red; border-radius: 4px; }";
    greenLED  = "QLabel { background-color : green; border-radius: 4px; }";
    clearLED  = "QLabel { background-color : none; border-radius: 4px; }";
    if (settings->value(s_settings_focusindicators,s_settings_focusindicators_def).toBool()) {
        switch (activeRadio) {
        case 0:
            RX1->setStyleSheet(redLED);
            RX2->setStyleSheet(clearLED);
            break;
        case 1:
            RX2->setStyleSheet(redLED);
            RX1->setStyleSheet(clearLED);
            break;
        }
        switch (activeTxRadio) {
        case 0:
            TX1->setStyleSheet(redLED);
            TX2->setStyleSheet(clearLED);
            break;
        case 1:
            TX2->setStyleSheet(redLED);
            TX1->setStyleSheet(clearLED);
            break;
        }
    }
    So2sdrStatusBar->addPermanentWidget(offPtr);
    So2sdrStatusBar->addPermanentWidget(grabLabel);
    grabLabel->hide();
    So2sdrStatusBar->addPermanentWidget(autoSendStatus);
    So2sdrStatusBar->addPermanentWidget(duelingCQStatus);
    So2sdrStatusBar->addPermanentWidget(autoCQStatus);
    So2sdrStatusBar->addPermanentWidget(toggleStatus);
    So2sdrStatusBar->addPermanentWidget(rLabelPtr[0]);
    So2sdrStatusBar->addPermanentWidget(rLabelPtr[1]);
    So2sdrStatusBar->addPermanentWidget(winkeyLabel);
    for (int i=0;i<NRIG;i++) {
        clearWorked(i);
    }
    MultTextEdit->setReadOnly(true);
    MultTextEdit->setDisabled(true); // this prevents text from being selectable
    MasterTextEdit->setReadOnly(true);
    MasterTextEdit->setDisabled(true);
    TimeDisplay->setText(QDateTime::currentDateTimeUtc().toString("MM-dd hh:mm:ss"));
    updateNrDisplay();

    cat = new RigSerial(*settings);
    cat->moveToThread(&catThread);
    connect(&catThread, SIGNAL(started()), cat, SLOT(run()));
    connect(cat, SIGNAL(radioError(const QString &)), errorBox, SLOT(showMessage(const QString &)));
    options = new ContestOptionsDialog(this);
    connect(options, SIGNAL(accepted()), this, SLOT(regrab()));
    connect(options, SIGNAL(rejected()), this, SLOT(regrab()));
    connect(options,SIGNAL(rescore()),this,SLOT(rescore()));
    connect(options,SIGNAL(multiModeChanged()),this,SLOT(setSummaryGroupBoxTitle()));
    connect(options,SIGNAL(updateOffTime()),this,SLOT(updateOffTime()));
    options->hide();
    detail=new DetailedEdit();
    connect(detail,SIGNAL(editedRecord(QSqlRecord)),this,SLOT(updateRecord(QSqlRecord)));
    connect(detail,SIGNAL(accepted()),this,SLOT(detailEditDone()));
    connect(detail,SIGNAL(rejected()),this,SLOT(detailEditDone()));
    detail->hide();
    cabrillo = new CabrilloDialog(this);
    cabrillo->hide();
    station = new StationDialog(*settings,this);
    connect(station, SIGNAL(accepted()), this, SLOT(regrab()));
    connect(station, SIGNAL(rejected()), this, SLOT(regrab()));
    connect(station, SIGNAL(stationUpdate()), this, SLOT(stationUpdate()));
    station->hide();
    progsettings = new SettingsDialog(*settings, this);
    connect(progsettings, SIGNAL(accepted()), this, SLOT(regrab()));
    connect(progsettings, SIGNAL(rejected()), this, SLOT(regrab()));
    connect(progsettings, SIGNAL(settingsUpdate()), this, SLOT(settingsUpdate()));
    progsettings->hide();

    cwMessage = new CWMessageDialog(CWType,this);
    connect(cwMessage, SIGNAL(accepted()), this, SLOT(regrab()));
    connect(cwMessage, SIGNAL(rejected()), this, SLOT(regrab()));
    cwMessage->hide();
    ssbMessage = new SSBMessageDialog(this);
    connect(ssbMessage, SIGNAL(accepted()), this, SLOT(regrab()));
    connect(ssbMessage, SIGNAL(rejected()), this, SLOT(regrab()));
    ssbMessage->hide();

    radios = new RadioDialog(*settings,*cat, this);
    connect(radios, SIGNAL(accepted()), this, SLOT(regrab()));
    connect(radios, SIGNAL(rejected()), this, SLOT(regrab()));
    radios->hide();
    winkeyDialog = new WinkeyDialog(*settings,this);
    winkeyDialog->setWinkeyVersionLabel(0);
    connect(winkeyDialog, SIGNAL(accepted()), this, SLOT(regrab()));
    connect(winkeyDialog, SIGNAL(rejected()), this, SLOT(regrab()));
    winkeyDialog->hide();
    directory->setCurrent(dataDirectory());
    sdr = new SDRDialog(*settings,this);
    connect(sdr, SIGNAL(accepted()), this, SLOT(regrab()));
    connect(sdr, SIGNAL(rejected()), this, SLOT(regrab()));
    sdr->hide();

    bandmap=new BandmapInterface(*settings,this);
    connect(bandmap,SIGNAL(removeCall(QByteArray,int)),this,SLOT(removeSpot(QByteArray,int)));
    connect(bandmap,SIGNAL(qsy(int,int)),cat,SLOT(qsyExact(int,int)));
    connect(bandmap,SIGNAL(sendMsg(QString)),So2sdrStatusBar,SLOT(showMessage(QString)));
    notes = new NoteDialog(this);
    connect(notes, SIGNAL(accepted()), this, SLOT(regrab()));
    connect(notes, SIGNAL(rejected()), this, SLOT(regrab()));
    newContest = new NewDialog(this);
    connect(newContest, SIGNAL(accepted()), this, SLOT(regrab()));
    connect(newContest, SIGNAL(rejected()), this, SLOT(regrab()));
    newContest->hide();
    readContestList();
    connect(newContest, SIGNAL(finished(int)), this, SLOT(setupNewContest(int)));
    RateLabel->setText("Rate=0");
    HourRateLabel->setText("0/hr");
    connect(wpmLineEditPtr[0], SIGNAL(textEdited(QString)), this, SLOT(launch_enterCWSpeed(QString)));
    connect(wpmLineEditPtr[1], SIGNAL(textEdited(QString)), this, SLOT(launch_enterCWSpeed(QString)));
    connect(actionOpen, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(actionNewContest, SIGNAL(triggered()), newContest, SLOT(show()));
    connect(actionWinkey, SIGNAL(triggered()), winkeyDialog, SLOT(show()));
    connect(actionWinkey, SIGNAL(triggered()), this, SLOT(ungrab()));
    connect(winkeyDialog,SIGNAL(accepted()),this,SLOT(regrab()));
    connect(winkeyDialog,SIGNAL(rejected()),this,SLOT(regrab()));
    connect(actionStation, SIGNAL(triggered()), station, SLOT(show()));
    connect(actionStation, SIGNAL(triggered()), this, SLOT(ungrab()));
    connect(station,SIGNAL(accepted()),this,SLOT(regrab()));
    connect(station,SIGNAL(rejected()),this,SLOT(regrab()));
    connect(actionSettings, SIGNAL(triggered()), progsettings, SLOT(show()));
    connect(actionSettings, SIGNAL(triggered()), this, SLOT(ungrab()));
    connect(progsettings,SIGNAL(accepted()),this,SLOT(regrab()));
    connect(progsettings,SIGNAL(rejected()),this,SLOT(regrab()));
    connect(actionRadios, SIGNAL(triggered()), radios, SLOT(show()));
    connect(actionRadios, SIGNAL(triggered()), this, SLOT(ungrab()));
    connect(radios,SIGNAL(accepted()),this,SLOT(regrab()));
    connect(radios,SIGNAL(rejected()),this,SLOT(regrab()));
    connect(actionContestOptions, SIGNAL(triggered()), options, SLOT(show()));
    connect(actionContestOptions, SIGNAL(triggered()), this, SLOT(ungrab()));
    connect(options,SIGNAL(accepted()),this,SLOT(regrab()));
    connect(options,SIGNAL(rejected()),this,SLOT(regrab()));
    connect(actionSDR, SIGNAL(triggered()), sdr, SLOT(show()));
    connect(actionSDR, SIGNAL(triggered()), this, SLOT(ungrab()));
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(sdr,SIGNAL(accepted()),this,SLOT(regrab()));
    connect(sdr,SIGNAL(rejected()),this,SLOT(regrab()));

    //RTC 07/11/2012 disabled About Qt unless a solution is found
    // instead added Qt version number to So2sdr About dialog
    //connect(actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    /* @todo problem here: with the aboutQt() dialog, there is no way to connect
     to its accepted or rejected signals, so no way to reset the keyboard grab */

    connect(actionHelp, SIGNAL(triggered()), this, SLOT(showHelp()));
    connect(actionHelp, SIGNAL(triggered()), this, SLOT(ungrab()));
    connect(lineEditCall1, SIGNAL(textEdited(const QString &)), this, SLOT(prefixCheck1(const QString &)));
    connect(lineEditCall2, SIGNAL(textEdited(const QString &)), this, SLOT(prefixCheck2(const QString &)));
    connect(lineEditExchange1, SIGNAL(textEdited(const QString &)), this, SLOT(exchCheck1(const QString &)));
    connect(lineEditExchange2, SIGNAL(textEdited(const QString &)), this, SLOT(exchCheck2(const QString &)));
    connect(options, SIGNAL(accepted()), this, SLOT(updateOptions()));
    connect(actionCW_Messages, SIGNAL(triggered()), cwMessage, SLOT(show()));
    connect(actionCW_Messages, SIGNAL(triggered()), this, SLOT(ungrab()));
    connect(cwMessage,SIGNAL(accepted()),this,SLOT(regrab()));
    connect(cwMessage,SIGNAL(rejected()),this,SLOT(regrab()));
    connect(ssbMessage,SIGNAL(accepted()),this,SLOT(regrab()));
    connect(ssbMessage,SIGNAL(rejected()),this,SLOT(regrab()));
#ifdef DVK_ENABLE
    connect(actionSSB_Messages, SIGNAL(triggered()), ssbMessage, SLOT(show()));
    connect(actionSSB_Messages, SIGNAL(triggered()), this, SLOT(ungrab()));
#else
    actionSSB_Messages->setEnabled(false);
#endif
    initDupeSheet();
    menuWindows->addSeparator();
    bandmapCheckBox[0] = new QCheckBox("Bandmap 1", menuWindows);
    bandmapCheckBox[1] = new QCheckBox("Bandmap 2", menuWindows);
    connect(bandmap,SIGNAL(bandmap1state(bool)),bandmapCheckBox[0],SLOT(setChecked(bool)));
    connect(bandmap,SIGNAL(bandmap2state(bool)),bandmapCheckBox[1],SLOT(setChecked(bool)));
    connect(bandmap,SIGNAL(bandmap1state(bool)),this,SLOT(sendCalls1(bool)));
    connect(bandmap,SIGNAL(bandmap2state(bool)),this,SLOT(sendCalls2(bool)));

    for (int i = 0; i < NRIG; i++) {
        bandmapCheckAction[i] = new QWidgetAction(menuWindows);
        bandmapCheckAction[i]->setDefaultWidget(bandmapCheckBox[i]);
        menuWindows->addAction(bandmapCheckAction[i]);
    }
    grabCheckBox = new QCheckBox("Grab keyboard", menuWindows);
    grabCheckBox->setCheckState(Qt::Unchecked);
    grabAction = new QWidgetAction(menuWindows);
    grabAction->setDefaultWidget(grabCheckBox);
    menuWindows->addAction(grabAction);
    connect(bandmapCheckBox[0], SIGNAL(stateChanged(int)), this, SLOT(showBandmap1(int)));
    connect(bandmapCheckBox[1], SIGNAL(stateChanged(int)), this, SLOT(showBandmap2(int)));
    connect(grabCheckBox, SIGNAL(toggled(bool)), this, SLOT(setGrab(bool)));
    connect(grabCheckBox, SIGNAL(clicked()), menuWindows, SLOT(close()));
    telnetCheckBox    = new QCheckBox("Telnet", menuWindows);
    telnetCheckAction = new QWidgetAction(menuWindows);
    telnetCheckAction->setDefaultWidget(telnetCheckBox);
    menuWindows->addSeparator();
    menuWindows->addAction(telnetCheckAction);

    // ungrab keyboard when menubar menus are open
    connect(SetupMenu,SIGNAL(aboutToShow()),this,SLOT(ungrab()));
    connect(SetupMenu,SIGNAL(aboutToHide()),this,SLOT(regrab()));
    connect(FileMenu,SIGNAL(aboutToShow()),this,SLOT(ungrab()));
    connect(FileMenu,SIGNAL(aboutToHide()),this,SLOT(regrab()));
    connect(menuWindows,SIGNAL(aboutToShow()),this,SLOT(ungrab()));
    connect(menuWindows,SIGNAL(aboutToHide()),this,SLOT(regrab()));
    connect(HelpMenu,SIGNAL(aboutToShow()),this,SLOT(ungrab()));
    connect(HelpMenu,SIGNAL(aboutToHide()),this,SLOT(regrab()));

    connect(telnetCheckBox, SIGNAL(stateChanged(int)), this, SLOT(showTelnet(int)));
    lineEditCall[0]->installEventFilter(this);
    lineEditCall[1]->installEventFilter(this);
    lineEditExchange[0]->installEventFilter(this);
    lineEditExchange[1]->installEventFilter(this);
    const QObjectList clist = children();
    for (int i = 0; i < clist.size(); i++) {
        clist.at(i)->installEventFilter(this);
    }
    contestDirectory = QDir::homePath();
    directory = new QDir(contestDirectory);

    // parallel port
    pport = new ParallelPort(*settings);
    connect(pport, SIGNAL(parallelPortError(const QString &)), errorBox, SLOT(showMessage(const QString &)));
    connect(radios, SIGNAL(setParallelPort()), pport, SLOT(initialize()));
    readStationSettings();
    if (settings->value(s_radios_pport_enabled,s_radios_pport_enabled_def).toBool()) {
        pport->initialize();
    }
    // otrsp device
    otrsp = new OTRSP(*settings);
    connect(radios,SIGNAL(setOTRSP()),otrsp,SLOT(openOTRSP()));
    connect(otrsp, SIGNAL(otrspError(const QString &)), errorBox, SLOT(showMessage(const QString &)));
    if (settings->value(s_otrsp_enabled,s_otrsp_enabled_def).toBool()) {
        otrsp->openOTRSP();
    }
    // microHam device
    microham = new MicroHam(*settings);
    connect(radios,SIGNAL(setMicroHam()),microham,SLOT(openMicroHam()));
    connect(microham, SIGNAL(microhamError(const QString &)), errorBox, SLOT(showMessage(const QString &)));
    if (settings->value(s_microham_enabled,s_microham_enabled_def).toBool()) {
        microham->openMicroHam();
    }

    WPMLineEdit->setReadOnly(true);
    // set background of WPM speed edit box to grey
    QPalette palette(wpmLineEditPtr[0]->palette());
    palette.setColor(QPalette::Base, this->palette().color(QPalette::Background));
    for (int i = 0; i < NRIG; i++) {
        wpmLineEditPtr[i]->setText(QString::number(wpm[i]));
        wpmLineEditPtr[i]->setPalette(palette);
    }

    // start serial comm
    winkey = new Winkey(*settings,this);
    connect(winkey, SIGNAL(version(int)), winkeyDialog, SLOT(setWinkeyVersionLabel(int)));
    connect(winkey, SIGNAL(winkeyTx(bool, int)), bandmap, SLOT(setBandmapTxStatus(bool, int)));
    connect(winkeyDialog, SIGNAL(startWinkey()), this, SLOT(startWinkey()));
    connect(winkey->winkeyPort, SIGNAL(readyRead()), winkey, SLOT(receive()));
    connect(radios, SIGNAL(startRadios()), this, SLOT(openRadios()));
    startWinkey();

    openRadios();
    switchAudio(activeRadio);
    toggleStereo();
    switchTransmit(activeRadio);

    callFocus[activeRadio]=true;
    setEntryFocus();
    for (int i = 0; i < NRIG; i++) {
        lineEditExchange[i]->hide();
        lineEditCall[i]->setEnabled(false);
        lineEditExchange[i]->setEnabled(false);
    }
    setDefaultFreq(0);
    setDefaultFreq(1);
    updateRadioFreq();

    menubar->setNativeMenuBar(false);
    disableUI();
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanup()));

    // restore window geometry
    settings->beginGroup("MainWindow");
    resize(settings->value("size", QSize(720, 579)).toSize());
    move(settings->value("pos", QPoint(200, 200)).toPoint());
    settings->endGroup();

    show();
}

So2sdr::~So2sdr()
{
    if (model) {
        model->clear();
        delete model;
    }
    if (history) delete history;
    if (mylog) delete mylog;
    if (model) {
        QSqlDatabase::removeDatabase("QSQLITE");
    }
    // stop hamlib thread
    if (catThread.isRunning()) {
        catThread.quit();
        catThread.wait();
    }
    delete cat;
    delete cabrillo;
    delete detail;
    delete radios;
    delete cwMessage;
    delete ssbMessage;
    delete errorBox;
    delete winkeyDialog;
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
    if (master) delete master;
    if (contest) delete contest;
    delete dupesheetCheckBox[0];
    delete dupesheetCheckBox[1];
    delete dupesheetCheckAction[0];
    delete dupesheetCheckAction[1];
    delete [] dupeCalls[0];
    delete [] dupeCalls[1];
    delete [] dupeCallsKey[0];
    delete [] dupeCallsKey[1];
    delete dupesheet[0];
    delete dupesheet[1];
    delete bandmapCheckBox[0];
    delete bandmapCheckBox[1];
    delete bandmapCheckAction[0];
    delete bandmapCheckAction[1];
    delete grabCheckBox;
    delete grabAction;
    delete telnet;
    delete telnetCheckBox;
    delete telnetCheckAction;
    delete directory;
    delete winkey;
    delete pport;
    delete otrsp;
    delete bandmap;

    if (qso[0]) delete qso[0];
    if (qso[1]) delete qso[1];
    settings->sync();
    delete settings;
    delete csettings;
}

/*!
   add a new qso to the log

   @todo Fix bytearray/string conversion correctly
 */
void So2sdr::addQso(const Qso *qso)
{
    QSqlRecord newqso;
    newqso.append(QSqlField("nr", QVariant::Int));
    newqso.append(QSqlField("time", QVariant::String));
    newqso.append(QSqlField("freq", QVariant::Int));
    newqso.append(QSqlField("call", QVariant::String));
    newqso.append(QSqlField("band", QVariant::Int));
    newqso.append(QSqlField("date", QVariant::String));
    newqso.append(QSqlField("mode", QVariant::Int));
    newqso.append(QSqlField("snt1", QVariant::String));
    newqso.append(QSqlField("snt2", QVariant::String));
    newqso.append(QSqlField("snt3", QVariant::String));
    newqso.append(QSqlField("snt4", QVariant::String));
    newqso.append(QSqlField("rcv1", QVariant::String));
    newqso.append(QSqlField("rcv2", QVariant::String));
    newqso.append(QSqlField("rcv3", QVariant::String));
    newqso.append(QSqlField("rcv4", QVariant::String));
    newqso.append(QSqlField("pts", QVariant::Int));
    newqso.append(QSqlField("valid", QVariant::Int));
    for (int i = 0; i < SQL_N_COL; i++) newqso.setGenerated(i, true);
    newqso.setValue(SQL_COL_NR, QVariant(model->rowCount() + 1));
    newqso.setValue(SQL_COL_CALL, QVariant(qso->call).toString());
    newqso.setValue(SQL_COL_PTS, QVariant(qso->pts));
    newqso.setValue(SQL_COL_FREQ, QVariant(qso->freq));
    newqso.setValue(SQL_COL_BAND, QVariant(qso->band));
    newqso.setValue(SQL_COL_MODE, QVariant(qso->mode));
    for (int i = 0; i < 4; i++) {
        if (i < contest->nExchange()) {
            switch (i) {
            case 0:
                newqso.setValue(SQL_COL_SNT1, QVariant(qso->snt_exch[0]).toString());
                newqso.setValue(SQL_COL_RCV1, QVariant(qso->rcv_exch[0]).toString());
                break;
            case 1:
                newqso.setValue(SQL_COL_SNT2, QVariant(qso->snt_exch[1]).toString());
                newqso.setValue(SQL_COL_RCV2, QVariant(qso->rcv_exch[1]).toString());
                break;
            case 2:
                newqso.setValue(SQL_COL_SNT3, QVariant(qso->snt_exch[2]).toString());
                newqso.setValue(SQL_COL_RCV3, QVariant(qso->rcv_exch[2]).toString());
                break;
            case 3:
                newqso.setValue(SQL_COL_SNT4, QVariant(qso->snt_exch[3]).toString());
                newqso.setValue(SQL_COL_RCV4, QVariant(qso->rcv_exch[3]).toString());
                break;
            }
        } else {
            switch (i) {
            case 0:
                newqso.setNull(SQL_COL_SNT1);
                newqso.setNull(SQL_COL_RCV1);
                break;
            case 1:
                newqso.setNull(SQL_COL_SNT2);
                newqso.setNull(SQL_COL_RCV2);
                break;
            case 2:
                newqso.setNull(SQL_COL_SNT3);
                newqso.setNull(SQL_COL_RCV3);
                break;
            case 3:
                newqso.setNull(SQL_COL_SNT4);
                newqso.setNull(SQL_COL_RCV4);
                break;
            }
        }
    }
    newqso.setValue(SQL_COL_DATE, QVariant(qso->time.toUTC().toString("MMddyyyy")));
    newqso.setValue(SQL_COL_TIME, QVariant(qso->time.toUTC().toString("hhmm")));
    newqso.setValue(SQL_COL_VALID, QVariant(qso->valid));

    if (csettings->value(c_historyupdate,c_historyupdate_def).toBool()) {
        history->addQso(qso);
    }


    model->insertRecord(-1, newqso);
    model->submitAll();
    while (model->canFetchMore()) {
        model->fetchMore();
    }
    LogTableView->scrollToBottom();
}

/*! updates things depending on contents of Station dialog
 */
void So2sdr::stationUpdate()
{
    // in case called before contest loaded
    if (!contest) {
        setWindowTitle("SO2SDR:" + settings->value(s_call,s_call_def).toString());
        return;
    }

    // zone
    if (contest->zoneType() == 0) {
        contest->setMyZone(settings->value(s_cqzone,s_cqzone_def).toInt());
    } else {
        contest->setMyZone(settings->value(s_ituzone,s_ituzone_def).toInt());
    }

    // callsign
    if (csettings) {
        QString name=csettings->value(c_contestname,c_contestname_def).toString().toUpper();
        int     indx = fileName.lastIndexOf("/");
        QString tmp  = fileName.mid(indx + 1, fileName.size() - indx);
        setWindowTitle(settings->value(s_call,s_call_def).toString() + " : " + tmp + " : " +
                       csettings->value(c_contestname_displayed,name).toString());
    }
    Qso  tmp(2);
    tmp.call = settings->value(s_call,s_call_def).toString().toAscii();
    bool b;
    contest->setCountry(cty->idPfx(&tmp, b));
    contest->setContinent(tmp.continent);
}

/*!
    Update labels for General Settings changes
 */
void So2sdr::settingsUpdate()
{
    switchAudio(activeRadio);
    switchTransmit(activeRadio);
    if (autoSend) {
        if (settings->value(s_settings_autosend_mode,s_settings_autosend_mode_def).toInt() == 0
                || csettings->value(c_sprintmode,c_sprintmode_def).toBool() ) { // semi-auto
            autoSendStatus->setText("<font color=#006699>AutoSend(ESM)</font>");
        } else {
            autoSendStatus->setText("<font color=#006699>AutoSend("
                + QString::number(settings->value(s_settings_autosend,s_settings_autosend_def).toInt()) + ")</font>");
        }
    }
    if (autoCQMode) {
        autoCQStatus->setText("<font color=#5200CC>AutoCQ ("
           + QString::number(settings->value(s_settings_cqrepeat,s_settings_cqrepeat_def).toFloat(),'f',1) + "s)</font>");
    }
}

/*! this will have clean-up code
 */
void So2sdr::cleanup()
{
    saveSpots();
    quit();
}

/*!
   called when Winkey started or restarted
 */
void So2sdr::startWinkey()
{
    winkey->openWinkey();
    winkey->setSpeed(wpm[activeRadio]);
    winkey->switchTransmit(activeRadio);
}


/*! regrab keyboard to last widget to have grab
 */
void So2sdr::regrab()
{
    if (grab) {
        grabLabel->show();
        grabWidget->setFocus();
        grabWidget->activateWindow();
        grabWidget->grabKeyboard();
        grabbing=true;
    }
}

/*! change grab status
 */
void So2sdr::setGrab(bool s)
{
    if (s) {
        grab = true;
        grabLabel->show();
        grabWidget->grabKeyboard();
        grabbing=true;
        grabWidget->setFocus();
        grabWidget->activateWindow();
    } else {
        grab = false;
        grabWidget->releaseKeyboard();
        grabbing=false;
        grabLabel->hide();
    }
}

void So2sdr::ungrab()
{
    if (grab) {
        grabWidget->releaseKeyboard();
        grabLabel->hide();
        grabbing=false;
    }
}

/*!
 * \brief So2sdr::setEntryFocus
 * Sets focus and grab status to current line edit
 */
void So2sdr::setEntryFocus()
{
    if (callFocus[activeRadio]) {
        lineEditCall[activeRadio]->setFocus();
        if (grabbing) {
            lineEditCall[activeRadio]->grabKeyboard();
            lineEditCall[activeRadio]->activateWindow();
            raise();
        }
        grabWidget = lineEditCall[activeRadio];
    } else {
        lineEditExchange[activeRadio]->setFocus();
        if (grabbing) {
            lineEditExchange[activeRadio]->grabKeyboard();
            lineEditExchange[activeRadio]->activateWindow();
            raise();
        }
        grabWidget = lineEditExchange[activeRadio];
    }
}

void So2sdr::quit()
{
    for (int i = 0; i < NRIG; i++) {
        if (telnetOn) {
            telnet->close();
            telnetOn=false;
        }
    }
    close();
}


void So2sdr::openRadios()
{
    if (catThread.isRunning()) {
        catThread.quit();
        catThread.wait(100); // needed to wait for the thread to stop
    }
    cat->initialize();
    // Connect signals from functions in this class with slots in RigSerial class
    connect(this, SIGNAL(qsyExact(int, int)), cat, SLOT(qsyExact(int, int)));
    connect(this, SIGNAL(setRigMode(int, rmode_t, pbwidth_t)), cat, SLOT(setRigMode(int, rmode_t, pbwidth_t)));
    cat->openRig();
    catThread.start();
    for (int i = 0; i < N_BANDS; i++) {
        bandInvert[0][i] = false;
        bandInvert[1][i] = false;
    }
    for (int i = 0; i < NRIG; i++) {
        /* @todo fix this
        if (radios->model(i) == 221) {
            // K2 (hamlib #221) has inverted IF on 21/28 MHz.
            bandInvert[i][5] = true;
            bandInvert[i][4] = true;
        }
        */
    }
    stopTimers();
    startTimers();
}

/*!
   Enter a log note
 */
void So2sdr::writeNote()
{
    ungrab();
    notes->enterNote(fileName, contestDirectory, TimeDisplay->text(), grab);
}


/*!
   turn off UI elements that should not be used until a
   contest has been selected
 */
void So2sdr::disableUI()
{
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
    grabCheckBox->setEnabled(false);
    actionImport_Cabrillo->setEnabled(false);
    dupesheetCheckBox[0]->setEnabled(false);
    dupesheetCheckBox[1]->setEnabled(false);
    telnetCheckBox->setEnabled(false);
    bandmapCheckBox[0]->setEnabled(false);
    bandmapCheckBox[1]->setEnabled(false);
    uiEnabled = false;
}

/*!
   Enable UI
 */
void So2sdr::enableUI()
{
    for (int i = 0; i < NRIG; i++) {
        lineEditCall[i]->setEnabled(true);
        lineEditExchange[i]->setEnabled(true);
        bandmapCheckBox[i]->setEnabled(true);
        dupesheetCheckBox[i]->setEnabled(true);
    }
    cwMessage->setEnabled(true);
#ifdef DVK_ENABLE
    ssbMessage->setEnabled(true);
    actionSSB_Messages->setEnabled(true);
#endif
    options->setEnabled(true);
    actionCW_Messages->setEnabled(true);
    actionContestOptions->setEnabled(true);
    actionSave->setEnabled(true);
    actionADIF->setEnabled(true);
    actionCabrillo->setEnabled(true);
    grabCheckBox->setEnabled(true);
    actionImport_Cabrillo->setEnabled(true);
    telnetCheckBox->setEnabled(true);
    uiEnabled = true;
    callFocus[activeRadio]=true;
    setEntryFocus();
}


/*!
   set up new contest
 */
void So2sdr::setupNewContest(int result)
{
    if (!result) return;
    int indx = newContest->newIndx();

    // set the standard config file for this contest
    QString fname = configFiles[indx];

    // set an initial name
    QString tmp = contestDirectory + "/" + QString(fname);
    tmp.chop(4); // remove ".cfg"
    tmp = tmp + QDateTime::currentDateTimeUtc().toString("MMddyy") + ".cfg";
    QString newfname;
    newfname.clear();
    newfname = QFileDialog::getSaveFileName(this, "New config file", tmp, "Config Files (*.cfg)");

    // if user presses cancel
    if (newfname.isEmpty()) return;

    //add .cfg at end of filename if it is not there
    newfname=newfname.trimmed();
    if (newfname.right(4).toUpper()!=".CFG") {
        newfname=newfname+".cfg";
    }

    // if file already exists, remove it first
    QFileInfo oldfileInfo(newfname);
    if (oldfileInfo.exists()) {
        // remove cfg file
        QFile tmp(newfname);
        tmp.remove();
        // remove log file if it exists
        QString s=newfname;
        s.chop(4);
        s=s+".log";
        QFileInfo oldLogfileInfo(s);
        if (oldLogfileInfo.exists()) {
            QFile tmp(s);
            tmp.remove();
        }
    }
    // copy standard config to contest directory
    // check for local copy
    QFileInfo userStdFile(userDirectory()+"/"+fname);
    QFile stdFile;
    if (userStdFile.exists()) {
        stdFile.setFileName(userDirectory()+"/"+fname);
    } else {
        stdFile.setFileName(dataDirectory()+"/"+fname);
    }
    stdFile.copy(newfname);
    fileName=newfname;
    if (setupContest()) {
        // disable option to start another new contest: must close and restart program
        actionNewContest->setDisabled(true);
        actionOpen->setDisabled(true);
    }
    int i = fileName.lastIndexOf("/");
    if (i != -1) {
        // set contest directory based on where user wants to save
        contestDirectory = fileName;
        contestDirectory.truncate(i);
    }
    directory->setCurrent(contestDirectory);
}

/*!
   update when changes made in Contest Options dialog
 */
void So2sdr::updateOptions()
{
    if (csettings->value(c_showmode,c_showmode_def).toBool()) {
        LogTableView->setColumnHidden(SQL_COL_MODE, false);
    } else {
        LogTableView->setColumnHidden(SQL_COL_MODE, true);
    }
    updateBreakdown();
    updateMults(activeRadio);
    setEntryFocus();
    startMaster();
    if (csettings->value(c_historymode,c_historymode_def).toBool()) {
        history->startHistory();
        if (history->isOpen()) {
            actionHistory->setEnabled(true);
            actionHistory->setText("Update History (" + csettings->value(c_historyfile,c_historyfile_def).toString() + ") from Log");
        } else {
            actionHistory->setEnabled(false);
            actionHistory->setText("Update History from Log");
        }
    } else {
        history->stopHistory();
        actionHistory->setEnabled(false);
        actionHistory->setText("Update History from Log");
    }
}


/*!
   open contest cfg file and set up program
 */
bool So2sdr::setupContest()
{
    QFileInfo info(fileName);
    if (!info.exists()) return false;
    csettings=new QSettings(fileName,QSettings::IniFormat);

    if (!QFile::exists(fileName)) return(false);

    // construct dialogs that depend on contest ini file settings
    QString cname=csettings->value(c_contestname,c_contestname_def).toString().toUpper();
    if (cname.isEmpty()) return(false);
    selectContest(cname.toAscii());
    cwMessage->initialize(csettings);
#ifdef DVK_ENABLE
    ssbMessage->initialize(csettings);
#endif
    options->initialize(csettings);
    cty = new Cty(*csettings);
    connect(cty, SIGNAL(ctyError(const QString &)), errorBox, SLOT(showMessage(const QString &)));
    if (contest->zoneType() == 0) {
        contest->setMyZone(settings->value(s_cqzone,s_cqzone_def).toInt());
    } else {
        contest->setMyZone(settings->value(s_ituzone,s_ituzone_def).toInt());
    }
    cty->initialize(station->lat(), station->lon(), contest->zoneType());

    /* @todo disable changing the grid square
      need to figure work-around */
    station->GridLineEdit->setEnabled(false);

    // fill in local sunrise/set times
    QString sTime;
    cty->mySunTimes(sTime);
    station->SunLabel->setText("Sunrise/Sunset: " +sTime + " z");

    // now in stationupdate
    Qso  tmp(2);
    tmp.call = settings->value(s_call,s_call_def).toString().toAscii();
    bool b;
    contest->setCountry(cty->idPfx(&tmp, b));
    contest->setContinent(tmp.continent);
    master = new Master();
    connect(master, SIGNAL(masterError(const QString &)), errorBox, SLOT(showMessage(const QString &)));
    startMaster();
    history = new History(*csettings,this);
    connect(history,SIGNAL(message(const QString&,int)),So2sdrStatusBar,SLOT(showMessage(const QString&,int)));

    if (csettings->value(c_historymode,c_historymode_def).toBool()) {
        history->startHistory();
        if (history->isOpen()) {
            actionHistory->setEnabled(true);
            actionHistory->setText("Update History (" + csettings->value(c_historyfile,c_historyfile_def).toString() + ") from Log");
        } else {
            actionHistory->setEnabled(false);
            actionHistory->setText("Update History from Log");
        }
    }
    mylog = new log(*csettings,contest->nExchange(), this);
    connect(contest,SIGNAL(mobileDupeCheck(Qso*)),mylog,SLOT(mobileDupeCheck(Qso*)));
    connect(contest,SIGNAL(clearDupe()),So2sdrStatusBar,SLOT(clearMessage()));
    mylog->setRstField(contest->rstField());
    mylog->setupQsoNumbers(contest->numberField());
    mylog->setFieldsShown(contest->sntFieldShown(), contest->rcvFieldShown());

    // additional contest-specific configs
    selectContest2();
    contest->initialize(settings,csettings,cty);

    for (int i = 0; i < N_BANDS; i++) nqso[i] = 0;
    directory->setCurrent(contestDirectory);
    mylog->openLogFile(fileName,false);
    initLogView();
    loadSpots();
    rescore();

    connect(actionADIF, SIGNAL(triggered()), this, SLOT(exportADIF()));
    connect(actionCabrillo, SIGNAL(triggered()), this, SLOT(showCabrillo()));
    connect(actionCabrillo,SIGNAL(triggered()),this,SLOT(ungrab()));
    connect(cabrillo, SIGNAL(accepted()), this, SLOT(exportCabrillo()));
    connect(actionImport_Cabrillo, SIGNAL(triggered()), this, SLOT(importCabrillo()));
    connect(cabrillo,SIGNAL(accepted()),this,SLOT(regrab()));
    connect(cabrillo,SIGNAL(rejected()),this,SLOT(regrab()));
    connect(actionHistory, SIGNAL(triggered()), this, SLOT(updateHistory()));
    nrSent = model->rowCount() + 1;
    updateNrDisplay();
    updateBreakdown();
    updateMults(activeRadio);
    clearWorked(0);
    clearWorked(1);
    enableUI();
    So2sdrStatusBar->showMessage("Read " + fileName, 3000);
    setSummaryGroupBoxTitle();
    if (csettings->value(c_off_time_enable,c_off_time_enable_def).toBool()) updateOffTime();
    return(true);
}

/*!
   Select contest
 */
void So2sdr::selectContest(QByteArray name)
{
    if (name == "ARRLDX") {
        // from US/VE
        contest = new ARRLDX(true);
    }
    if (name == "ARRLDX-DX") {
        // from DX
        contest = new ARRLDX(false);
    }
    if (name == "ARRL10") {
        contest = new ARRL10;
    }
    if (name == "ARRL160") {
        contest = new ARRL160(true);
    }
    if (name == "ARRL160-DX") {
        contest = new ARRL160(false);
    }
    if (name == "CQP-CA") {
        contest = new CQP();
        static_cast<CQP*>(contest)->setWithinState(true);
    }
    if (name == "CQP") {
        contest = new CQP();
        static_cast<CQP*>(contest)->setWithinState(false);
    }
    if (name == "CQ160") {
        contest = new CQ160();
    }
    if (name == "CQWW") {
        contest = new CQWW;
    }
    if (name == "CWOPS") {
        contest = new Cwops;
        // disable changing mults by band status
        options->MultsByBandCheckBox->setDisabled(true);
    }
    if (name == "DXPED") {
        contest = new Dxped;
    }
    if (name == "FD") {
        contest = new FD;
    }
    if (name == "IARU") {
        contest = new IARU;
    }
    if (name == "KQP-KS") {
        contest = new KQP;
        static_cast<KQP*>(contest)->setWithinState(true);
    }
    if (name == "KQP") {
        contest = new KQP;
        static_cast<KQP*>(contest)->setWithinState(false);
    }
    if (name == "NAQP") {
        contest = new Naqp;
    }
    if (name == "SPRINT") {
        contest = new Sprint;
    }
    if (name == "STEW") {
        contest = new Stew;
    }
    if (name == "SWEEPSTAKES") {
        contest = new Sweepstakes;
    }
    if (name == "WPX") {
        contest = new WPX;
        // disable changing mults by band status
        options->MultsByBandCheckBox->setDisabled(true);
    }
    if (name == "PAQP-PA") {
        contest = new PAQP();
        static_cast<PAQP*>(contest)->setWithinState(true);
    }
    if (name == "PAQP") {
        contest = new PAQP();
        static_cast<PAQP*>(contest)->setWithinState(false);
    }
    if (contest) {
        int sz=csettings->beginReadArray(c_qso_type1);
        for (int i=0;i<sz;i++) {
            csettings->setArrayIndex(i);
            QByteArray tmp=csettings->value("pfx","").toByteArray();
            contest->addQsoType(tmp,0);
        }
        csettings->endArray();
        sz=csettings->beginReadArray(c_qso_type2);
        for (int i=0;i<sz;i++) {
            csettings->setArrayIndex(i);
            QByteArray tmp=csettings->value("pfx","").toByteArray();
            contest->addQsoType(tmp,1);
        }
        csettings->endArray();
        // make extra exchange fields inactive/hidden
        for (int i=contest->nExchange();i<4;i++) {
            options->sent[i]->setEnabled(false);
            options->sent[i]->hide();
            cabrillo->sent[i]->setEnabled(false);
            cabrillo->sent[i]->hide();
        }

        contest->setContestName(name);
        int     indx = fileName.lastIndexOf("/");
        QString tmp  = fileName.mid(indx + 1, fileName.size() - indx);
        setWindowTitle(settings->value(s_call,s_call_def).toString() + " : " + tmp + " : " +
                       csettings->value(c_contestname_displayed,name).toString());
        multNameLabel[0]->setText(csettings->value(c_mult_name1,c_mult_name1_def).toString());
        multNameLabel[1]->setText(csettings->value(c_mult_name2,c_mult_name2_def).toString());
        readExcludeMults();
        for (int i = 0; i < NRIG; i++) {
            lineEditCall[i]->setEnabled(true);
            lineEditExchange[i]->setEnabled(true);
        }
        callFocus[activeRadio]=true;
        setEntryFocus();
    }
}

/*!
   contains additional initializations for specific cases

 */
void So2sdr::selectContest2()
{
    // for several ARRL contests need to know whether station is US/VE or DX
    Qso  tmp(1);
    tmp.call = settings->value(s_call,s_call_def).toString().toAscii();
    QString snt_exch[MAX_EXCH_FIELDS];
    for (int i=0;i<contest->nExchange();i++) {
        snt_exch[i].clear();
    }
    QByteArray name=contest->contestName();
    if (name == "ARRLDX") {
        mylog->setQsoPtsField(true);
        snt_exch[0] = "599";
        snt_exch[1]=settings->value(s_state,s_state_def).toString();
    }
    if (name == "ARRLDX-DX") {
        mylog->setQsoPtsField(true);
        snt_exch[0] = "599";
    }
    if (name == "ARRL10") {
        mylog->setQsoPtsField(true);
        snt_exch[0] = "599";
        snt_exch[1]=settings->value(s_state,s_state_def).toString();
    }
    if (name == "ARRL160") {
        mylog->setQsoPtsField(true);
        snt_exch[0] = "599";
        snt_exch[1]=settings->value(s_section,s_section_def).toString();
    }
    if (name == "SWEEPSTAKES") {
        snt_exch[0]="#";
        snt_exch[3]=settings->value(s_section,s_section_def).toString();
    }
    if (name == "CQP") {
        mylog->setQsoPtsField(true);
        snt_exch[0]="#";
        snt_exch[1]=settings->value(s_state,s_state_def).toString();
    }
    if (name == "CQP-CA") {
        mylog->setQsoPtsField(true);
        snt_exch[0]="#";
    }
    if (name == "CQ160") {
        mylog->setQsoPtsField(true);
        snt_exch[0] = "599";
        snt_exch[1]=settings->value(s_state,s_state_def).toString();
    }
    if (name == "CQWW") {
        mylog->setQsoPtsField(true);
        snt_exch[0] = "599";
        snt_exch[1]=settings->value(s_cqzone,s_cqzone_def).toString();
    }
    if (name == "CWOPS") {
        snt_exch[0]=settings->value(s_name,s_name_def).toString();
    }
    if (name == "DXPED") {
        snt_exch[0] = "599";
    }
    if (name == "FD") {
        mylog->setQsoPtsField(true);
        snt_exch[1]=settings->value(s_section,s_section_def).toString();
    }
    if (name == "GeneralQSO") {
        mylog->setQsoPtsField(false);
        snt_exch[1] = "599";
    }
    if (name == "IARU") {
        mylog->setQsoPtsField(true);
        snt_exch[0] = "599";
        snt_exch[1]=settings->value(s_ituzone,s_ituzone_def).toString();
    }
    if (name == "KQP") {
        mylog->setQsoPtsField(true);
        snt_exch[0]="599";
        snt_exch[1]=settings->value(s_state,s_state_def).toString();
    }
    if (name == "KQP-KS") {
        mylog->setQsoPtsField(true);
        snt_exch[0]="599";
    }
    if (name == "NAQP") {
        snt_exch[0]=settings->value(s_name,s_name_def).toString();
        snt_exch[1]=settings->value(s_state,s_state_def).toString();
    }
    if (name == "SPRINT") {
        snt_exch[0]="#";
        snt_exch[1]=settings->value(s_name,s_name_def).toString();
        snt_exch[2]=settings->value(s_state,s_state_def).toString();
    }
    if (name == "STEW") {
        mylog->setQsoPtsField(true);
        snt_exch[0]=settings->value(s_grid,s_grid_def).toString();
    }
    if (name == "WPX") {
        snt_exch[0] = "599";
        snt_exch[1] = "#";
        mylog->setQsoPtsField(true);
    }
    if (name == "PAQP") {
        mylog->setQsoPtsField(true);
        snt_exch[0]="#";
        snt_exch[1]=settings->value(s_state,s_state_def).toString();
    }
    if (name == "PAQP-PA") {
        mylog->setQsoPtsField(true);
        snt_exch[0]="#";
    }
    // fill in sent exchange fields with typical values if they are
    // not already entered
    for (int i=0;i<contest->nExchange();i++) {
        if (options->sent[i]->text().isEmpty()) {
            options->sent[i]->setText(snt_exch[i]);
            switch (i) {
            case 0:
                csettings->setValue(c_sentexch1,snt_exch[0]);
                break;
            case 1:
                csettings->setValue(c_sentexch2,snt_exch[1]);
                break;
            case 2:
                csettings->setValue(c_sentexch3,snt_exch[2]);
                break;
            case 3:
                csettings->setValue(c_sentexch4,snt_exch[3]);
                break;
            }
        }
    }
    // set which fields get prefilled from previous qso's in log
    for (int i = 0; i < contest->nExchange(); i++) {
        if (contest->logPrefill(i)) mylog->setPrefill(i);
    }

    qso[0] = new Qso(contest->nExchange());
    qso[1] = new Qso(contest->nExchange());
    for (int i = 0; i < contest->nExchange(); i++) {
        qso[0]->setExchangeType(i, contest->exchType(i));
        qso[1]->setExchangeType(i, contest->exchType(i));
    }
}



/*!
   initialize supercheck partial
   slot called when master checkbox clicked
 */
void So2sdr::startMaster()
{
    if (csettings->value(c_mastermode,c_mastermode_def).toBool()) {
        directory->setCurrent(dataDirectory());
        QString filename=csettings->value(c_masterfile,c_masterfile_def).toString();
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly)) {
            master->initialize(file);
        } else {
            errorBox->showMessage("ERROR: can't open file " + filename);
        }
    }
}

/*!
 * \brief So2sdr::updateHistory
 * update the exchange history. RTC: why is this so slow?
 *
 */
void So2sdr::updateHistory() {
    if (csettings->value(c_historymode,c_historymode_def).toBool()) {
        QSqlQueryModel log;
        QString query_log = "SELECT call,rcv1,rcv2,rcv3,rcv4 from log where valid='true'";
        log.setQuery(query_log, mylog->db);
        while (log.canFetchMore()) log.fetchMore();
        QProgressDialog progress("Updating history file (" + csettings->value(c_historyfile,c_historyfile_def).toString() + ") from current log" , "Cancel", 0, log.rowCount(), this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setMinimumDuration(0);
        Qso tmpqso(contest->nExchange());
        for (int i = 0; i < contest->nExchange(); i++) {
            tmpqso.setExchangeType(i, contest->exchType(i));
        }
        for (int row = 0; row < log.rowCount() && !progress.wasCanceled(); row++) {
            tmpqso.call=log.record(row).value("Call").toString().toAscii();
            for (int i = 0; i < contest->nExchange(); i++) {
                tmpqso.rcv_exch[i]=log.record(row).value(i+1).toString().toAscii();
            }
            history->addQso(&tmpqso);
            progress.setValue(row);
        }
        progress.setValue(log.rowCount());
    }
}

void So2sdr::exportADIF()
{
    if (fileName.isEmpty()) return;
    QString afname = fileName.remove(".cfg") + ".adi";
    QFile adifFile(afname);

    // check for overwriting existing file
    if (adifFile.exists()) {
        QMessageBox *msg;
        msg = new QMessageBox(this);
        msg->setWindowTitle("Warning");
        msg->setText("File "+afname+" exists.");
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
            break;
        default:
            break;
        }
        msg->deleteLater();
    }
    if (!adifFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    if (mylog->exportADIF(&adifFile)) {
        So2sdrStatusBar->showMessage("Saved ADIF " + afname, 3000);
    } else {
        So2sdrStatusBar->showMessage("error creating ADIF " + afname, 3000);
    }
}

/*! Fill and show Cabrillo entry form

  First check if sent exchange has been entered
 */
void So2sdr::showCabrillo()
{
    bool ok=true;
    for (int i=0;i<contest->nExchange();i++) {
        if (options->sent[i]->text().isEmpty()) {
            ok=false;
        }
    }
    if (!ok) {
        QMessageBox msgBox;
        msgBox.setText("Enter sent exchange in Config->Contest Options first.");
        msgBox.exec();
        return;
    }
    cabrillo->initialize(settings,csettings);
    cabrillo->show();
}

/*! call cabrillo export in log module
 */
void So2sdr::exportCabrillo()
{
    QString cfname = fileName;
    cfname = cfname.remove(".cfg") + ".cbr";
    QFile   cbrFile(cfname);

    // check for overwriting existing file
    if (cbrFile.exists()) {
        QMessageBox *msg;
        msg = new QMessageBox(this);
        msg->setWindowTitle("Warning");
        msg->setText("File "+cfname+" exists.");
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
    cabrillo->writeHeader(&cbrFile,contest->Score());
    mylog->exportCabrillo(&cbrFile,settings->value(s_call,s_call_def).toString(),
                          csettings->value(c_sentexch1,c_sentexch1_def).toString(),
                          csettings->value(c_sentexch2,c_sentexch2_def).toString(),
                          csettings->value(c_sentexch3,c_sentexch3_def).toString(),
                          csettings->value(c_sentexch4,c_sentexch4_def).toString());
    So2sdrStatusBar->showMessage("Saved Cabrillo " + cfname, 3000);
}

/*! import a Cabrillo log

   note: the number of exchange fields must be correct for this contest, otherwise
   bad things happen. @todo check for this and abort if wrong number of fields

   @todo investigate qprogressdialog more. I could not get it to show if the event
   duration was less than the default (4 seconds). Adding qApp->processEvents() causes
   crash.
   @todo give option to delete previous qso's or append to them
 */
void So2sdr::importCabrillo()
{
    int n=0;
    for (int i=0;i<N_BANDS;i++) n+=nqso[i];
    if (n) {
         errorBox->showMessage("ERROR: log must be empty to import cabrillo");
         return;
    }
    // search for files in directory set by contestDirectory
    directory->setCurrent(contestDirectory);

    // get filename
    QString CabFile = QFileDialog::getOpenFileName(this, tr("Import Cabrillo log"), contestDirectory, tr("Cabrillo Files (*.cbr)"));
    if (CabFile.isEmpty()) return;

    // open the file
    QFile file(CabFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    // count number of lines in file to get estimate of time needed to read
    int maxLines = 0;
    while (!file.atEnd()) {
        QString buffer;
        buffer = file.readLine();
        maxLines++;
    }
    file.close();
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QProgressDialog progress("Importing cabrillo", "Cancel", 0, maxLines, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(1000);
    QDataStream s(&file);
    int         cnt = 0;
    progress.setValue(0);
    for (int i = 0; i < N_BANDS; i++) nqso[i] = 0;
    Qso qso(contest->nExchange());
    contest->zeroScore();
    model->database().transaction();

    QString buffer;
    QStringList field;
    QSqlRecord  newqso;
    newqso.append(QSqlField("nr", QVariant::Int));
    newqso.append(QSqlField("time", QVariant::String));
    newqso.append(QSqlField("freq", QVariant::Int));
    newqso.append(QSqlField("call", QVariant::String));
    newqso.append(QSqlField("band", QVariant::Int));
    newqso.append(QSqlField("date", QVariant::String));
    newqso.append(QSqlField("mode", QVariant::Int));
    newqso.append(QSqlField("snt1", QVariant::String));
    newqso.append(QSqlField("snt2", QVariant::String));
    newqso.append(QSqlField("snt3", QVariant::String));
    newqso.append(QSqlField("snt4", QVariant::String));
    newqso.append(QSqlField("rcv1", QVariant::String));
    newqso.append(QSqlField("rcv2", QVariant::String));
    newqso.append(QSqlField("rcv3", QVariant::String));
    newqso.append(QSqlField("rcv4", QVariant::String));
    newqso.append(QSqlField("pts", QVariant::Int));
    newqso.append(QSqlField("valid", QVariant::Int));
    for (int i = 0; i < SQL_N_COL; i++) newqso.setGenerated(i, true);

    while (!file.atEnd() && !progress.wasCanceled()) {
        buffer = file.readLine();
        buffer = buffer.trimmed();
        if (!buffer.contains("QSO:") && !buffer.contains("qso:")) {
            continue;  // ignore header data
        }
        field = buffer.split(" ", QString::SkipEmptyParts);
        int         nf    = field.size();

        // Field1 = frequency in KHz
        int f = field.at(1).toInt() * 1000;
        newqso.setValue(SQL_COL_FREQ, QVariant(f));

        int b = getBand(f);
        newqso.setValue(SQL_COL_BAND, QVariant(b));
        qso.band = b;

        // Field2 = mode
        int m;
        if (field.at(2).toUpper() == "CW") {
            m = RIG_MODE_CW;
        } else if (field.at(2).toUpper() == "RY") {
            m = RIG_MODE_RTTY;
        } else if (field.at(2).toUpper() == "PH") {
            // Cabrillo doesn't store LSB/USB?
            if (f < 14000000) {
                m = RIG_MODE_LSB;
            } else {
                m = RIG_MODE_USB;
            }
        } else if (field.at(2).toUpper() == "FM") {
            m = RIG_MODE_FM;
        } else {
            m = RIG_MODE_CW;
        }
        qso.mode = (rmode_t) m;
        qso.modeType=cat->getModeType(qso.mode);
        newqso.setValue(SQL_COL_MODE, QVariant(m));
        cnt++;
        newqso.setValue(SQL_COL_NR, QVariant(cnt));

        // Field3 = date
        QDateTime time;
        time.setTimeSpec(Qt::UTC);
        int       y = field.at(3).mid(0, 4).toInt();
        m = field[3].mid(5, 2).toInt();
        int       d = field.at(3).mid(8, 2).toInt();
        time.setDate(QDate(y, m, d));
        newqso.setValue(SQL_COL_DATE, QVariant(time.toString("MMddyyyy").toAscii()));

        // Field4=time
        newqso.setValue(SQL_COL_TIME, QVariant(field.at(4)));

        // Field5=station call. ignore this

        // Field6+
        // next fields are sent exchange
        int i, j;
        for (i = 6, j = 0; i < (6 + contest->nExchange()); i++, j++) {
            switch (j) {
            case 0:
                newqso.setValue(SQL_COL_SNT1, QVariant(field.at(i).toUpper()));
                qso.snt_exch[0]=field.at(i).toAscii().toUpper();
                break;
            case 1:
                newqso.setValue(SQL_COL_SNT2, QVariant(field.at(i).toUpper()));
                qso.snt_exch[1]=field.at(i).toAscii().toUpper();
                break;
            case 2:
                newqso.setValue(SQL_COL_SNT3, QVariant(field.at(i).toUpper()));
                qso.snt_exch[2]=field.at(i).toAscii().toUpper();
                break;
            case 3:
                newqso.setValue(SQL_COL_SNT4, QVariant(field.at(i).toUpper()));
                qso.snt_exch[3]=field.at(i).toAscii().toUpper();
                break;
            }
        }

        // next field=call worked
        newqso.setValue(SQL_COL_CALL, QVariant(field.at(6 + contest->nExchange()).toUpper()));
        qso.call = field.at(6 + contest->nExchange()).toAscii().toUpper();
        bool bb;
        qso.country = cty->idPfx(&qso, bb);

        // next received report
        qso.exch.clear();
        for (i = 7 + contest->nExchange(), j = 0; i < (7 + 2 * contest->nExchange()); i++, j++) {
            // some fields may be empty (flaw in Cabrillo spec?)
            if (i >= nf) {
                continue;
            }
            switch (j) {
            case 0:
                newqso.setValue(SQL_COL_RCV1, QVariant(field.at(i).toUpper()));
                qso.exch = qso.exch + field.at(i).toAscii().toUpper();
                qso.rcv_exch[0]=field.at(i).toAscii().toUpper();
                break;
            case 1:
                newqso.setValue(SQL_COL_RCV2, QVariant(field.at(i).toUpper()));
                qso.exch = qso.exch + " " + field.at(i).toAscii().toUpper();
                qso.rcv_exch[1]=field.at(i).toAscii().toUpper();
                break;
            case 2:
                newqso.setValue(SQL_COL_RCV3, QVariant(field.at(i).toUpper()));
                qso.exch = qso.exch + " " + field.at(i).toAscii().toUpper();
                qso.rcv_exch[2]=field.at(i).toAscii().toUpper();
                break;
            case 3:
                newqso.setValue(SQL_COL_RCV4, QVariant(field.at(i).toUpper()));
                qso.exch = qso.exch + " " + field.at(i).toAscii().toUpper();
                qso.rcv_exch[3]=field.at(i).toAscii().toUpper();
                break;
            }
        }
        newqso.setValue(SQL_COL_PTS, QVariant(qso.pts));
        newqso.setValue(SQL_COL_VALID, QVariant(true)); // set to valid
        contest->addQso(&qso);
        model->insertRecord(-1, newqso);
        progress.setValue(cnt);
    }
    model->submitAll();
    model->database().commit();
    rescore();

    while (model->canFetchMore()) {
        model->fetchMore();
    }

    LogTableView->scrollToBottom();
    nrSent = model->rowCount() + 1;
    progress.setValue(maxLines);
}

/*! triggered after log edit. Rescore the log and scroll to the bottom
 */
void So2sdr::logEdited(const QModelIndex & topLeft, const QModelIndex & bottomRight)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);

    rescore();
    while (model->canFetchMore()) {
        model->fetchMore();
    }
    LogTableView->scrollToBottom();
    regrab();
}

/*!
Clear any selected cells in log and remove log editing message from status bar
*/
void So2sdr::clearEditSelection(QWidget *editor)
{
    Q_UNUSED(editor);
    So2sdrStatusBar->clearMessage();
    LogTableView->clearSelection();
}

/*!
Show status bar message when log edit starts
 */
void So2sdr::startLogEdit()
{
    So2sdrStatusBar->showMessage("EDITING LOG: PRESS ESC to abort");
}

/*!
 start detail qso editor. Connected to signal from LogDelegate
 */
void So2sdr::editLogDetail(QModelIndex index)
{
    QSqlRecord rec = model->record(index.row());
    origRecord = model->record(index.row());
    detail->loadRecord(rec,contest->nExchange());
    detail->show();
    detail->callLineEdit->setFocus();
    detail->callLineEdit->deselect();
    if (grab) {
        ungrab();
    }
}

/*!
Clean up after detailed qso edit
*/
void So2sdr::detailEditDone()
{
    LogTableView->clearSelection();
    if (grab) {
        regrab();
    }
}

/*!
 update a qso from detailed qso edit
 */
void So2sdr::updateRecord(QSqlRecord r)
{
    if (!model->setRecord(r.value(SQL_COL_NR).toInt()-1,r)) {
        qDebug("setRecord failed"); /*! @todo how should this be handled? */
    }
    model->submitAll();
    rescore();
    while (model->canFetchMore()) {
        model->fetchMore();
    }
    // update spot list; replace orig call/freq/dupe status
    int b=origRecord.value(SQL_COL_BAND).toInt();
    QByteArray call=origRecord.value(SQL_COL_CALL).toByteArray();
    for (int i=0;i<spotList[b].size();i++) {
        if (call==spotList[b].at(i).call) {
            spotList[b][i].call=r.value(SQL_COL_CALL).toByteArray();
            spotList[b][i].f=r.value(SQL_COL_FREQ).toInt();
            /*! @todo dupe status */
        }
    }

}

/*!
   initialize log view
 */
void So2sdr::initLogView()
{
    LogTableView->setShowGrid(true);
    LogTableView->verticalHeader()->hide();
    LogTableView->verticalHeader()->setDefaultAlignment(Qt::AlignLeft);
    LogTableView->verticalHeader()->setClickable(false);
    LogTableView->verticalHeader()->setDefaultSectionSize(16);

    model = new tableModel(this,mylog->db);
    model->setTable("log");
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->select();
    connect(model, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(logEdited(QModelIndex, QModelIndex)));
    while (model->canFetchMore()) {
        model->fetchMore();
    }
    LogTableView->setModel(model);
    int ncol=csettings->beginReadArray(c_col_width_group);
    for (int i=0;i<SQL_N_COL;i++) {
        if (ncol) csettings->setArrayIndex(i);
        LogTableView->setColumnWidth(i,settings->value(c_col_width_item,c_col_width_def[i]).toInt());
    }
    csettings->endArray();

    logdel=new logDelegate(this,contest,&logSearchFlag,&searchList);
    connect(logdel,SIGNAL(startLogEdit()),this,SLOT(ungrab()));
    connect(logdel,SIGNAL(startLogEdit()),this,SLOT(startLogEdit()));
    connect(logdel,SIGNAL(closeEditor(QWidget*)),this,SLOT(clearEditSelection(QWidget*)));
    connect(logdel,SIGNAL(editLogRow(QModelIndex)),this,SLOT(editLogDetail(QModelIndex)));
    for (int i=0;i < SQL_N_COL;i++) {
        LogTableView->setItemDelegateForColumn(i,logdel);
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
    if (csettings->value(c_showmode,c_showmode_def).toBool()) {
        LogTableView->setColumnHidden(SQL_COL_MODE, false);
    }
    LogTableView->setColumnHidden(SQL_COL_VALID, false);

    // columns 6+ are contest-specific
    // first are sent data fields
    unsigned f   = contest->sntFieldShown();
    int      cnt = 0;
    if (f & 1) {
        LogTableView->setColumnHidden(SQL_COL_SNT1, false);
        LogTableView->setColumnWidth(SQL_COL_SNT1, contest->fieldWidth(cnt));
        cnt++;
    }
    if (f & 2) {
        LogTableView->setColumnHidden(SQL_COL_SNT2, false);
        LogTableView->setColumnWidth(SQL_COL_SNT2, contest->fieldWidth(cnt));
        cnt++;
    }
    if (f & 4) {
        LogTableView->setColumnHidden(SQL_COL_SNT3, false);
        LogTableView->setColumnWidth(SQL_COL_SNT3, contest->fieldWidth(cnt));
        cnt++;
    }
    if (f & 8) {
        LogTableView->setColumnHidden(SQL_COL_SNT4, false);
        LogTableView->setColumnWidth(SQL_COL_SNT4, contest->fieldWidth(cnt));
        cnt++;
    }
    f = contest->rcvFieldShown();
    if (f & 1) {
        LogTableView->setColumnHidden(SQL_COL_RCV1, false);
        LogTableView->setColumnWidth(SQL_COL_RCV1, contest->fieldWidth(cnt));
        cnt++;
    }
    if (f & 2) {
        LogTableView->setColumnHidden(SQL_COL_RCV2, false);
        LogTableView->setColumnWidth(SQL_COL_RCV2, contest->fieldWidth(cnt));
        cnt++;
    }
    if (f & 4) {
        LogTableView->setColumnHidden(SQL_COL_RCV3, false);
        LogTableView->setColumnWidth(SQL_COL_RCV3, contest->fieldWidth(cnt));
        cnt++;
    }
    if (f & 8) {
        LogTableView->setColumnHidden(SQL_COL_RCV4, false);
        LogTableView->setColumnWidth(SQL_COL_RCV4, contest->fieldWidth(cnt));
    }
    if (mylog->qsoPtsField()) {
        LogTableView->setColumnHidden(SQL_COL_PTS, false);
        LogTableView->setColumnWidth(SQL_COL_PTS, 30);
    }
    for (int i=0;i<SQL_N_COL;i++) {
        model->setHeaderData(i,Qt::Horizontal,contest->columnName(i),Qt::DisplayRole);
        model->setHeaderData(i,Qt::Horizontal,Qt::AlignLeft,Qt::TextAlignmentRole);
    }
    LogTableView->horizontalHeader()->setStretchLastSection(true);
    LogTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    LogTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    LogTableView->scrollToBottom();
    LogTableView->show();
}



void So2sdr::about()
{
    ungrab();
    QMessageBox::about(this, "SO2SDR", "<p>SO2SDR " + Version + " Copyright 2010-2015 R.T. Clay N4OGW</p>"
                       +"  Qt library version: "+qVersion()+
                       + "<li>hamlib http://www.hamlib.org " + hamlib_version
                       + "<li>qextserialport https://github.com/qextserialport/qextserialport"
                       + "<li>QtSolutions_Telnet 2.1"
#ifdef Q_OS_WIN
                       + "<li>Parallel port access:  InpOut32.dll http://www.highrez.co.uk/"
#endif
                       + "<li>MASTER.DTA algorithm, IQ balancing: Alex Shovkoplyas VE3NEA, http://www.dxatlas.com</ul>"
                       + "<hr><p>SO2SDR is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License "
                       + "as published by the Free Software Foundation, either version 3 of the License, or any later version, http://www.gnu.org/licenses/</p>");
    regrab();
}

/*! update rate display.
 */
void So2sdr::updateRate()
{
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
        if (j < 0) j = 59;
    }
    ratePtr++;
    ratePtr = ratePtr % 60;

    // zero out next bin
    rateCount[ratePtr] = 0;
    RateLabel->setText("Rate=" + QString::number(qRound(sum * 60.0 / RATE_AVG_MINUTES)));

    updateOffTime();
}

/*!
 * \brief So2sdr::updateOffTime calculate and update off-time display
 */
void So2sdr::updateOffTime() {
    if (csettings->value(c_off_time_enable,c_off_time_enable_def).toBool()) {
        offPtr->setText(mylog->offTime(options->offMinimumLineEdit->text().toInt(),options->startDateTimeEdit->dateTime(),
                       options->endDateTimeEdit->dateTime()));
    } else {
        offPtr->clear();
    }
}

/*!
   process timer events

   The timer frequencies are define in defines.h, timerSettings
 */
void So2sdr::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timerId[2]) {
        // auto-CQ, dueling CQ, autoSend triggers, 100 ms resolution
        if (autoCQMode) autoCQ();
        if (duelingCQMode) duelingCQ();
        if (autoSend) autoSendExch();
    } else if (event->timerId() == timerId[1]) {
        // radio updates; every 300 mS
        // (the actual serial poll time is set in serial.cpp and may be different)
        updateRadioFreq();

        // check bandmap
        checkSpot(0);
        checkSpot(1);

        // update bandmap calls
        decaySpots();
    } else if (event->timerId() == timerId[0]) {
        // clock update; every 1000 mS
        TimeDisplay->setText(QDateTime::currentDateTimeUtc().toString("MM-dd hh:mm:ss"));
        // update rate display at beginning of minute
        if (QDateTime::currentDateTimeUtc().time().second()==0 && contest) {
            updateRate();
        }
        // check bandmap tcp connection
        bandmap->connectTcp();
    }
}

void So2sdr::autoCQActivate (bool state) {
    autoCQMode = state;
    if (autoCQMode) {
        autoCQRadio = activeRadio;
        if (duelingCQMode) duelingCQActivate(false);
        activeR2CQ = false;
        clearR2CQ(activeRadio ^ 1);
        if (activeTxRadio != activeRadio) {
            switchTransmit(activeRadio);
        }
        autoCQStatus->setText("<font color=#5200CC>AutoCQ ("
           + QString::number(settings->value(s_settings_cqrepeat,s_settings_cqrepeat_def).toFloat(),'f',1) + "s)</font>");
        autoCQModePause = false;
    } else {
        autoCQStatus->clear();
    }
}

void So2sdr::autoSendActivate (bool state) {
    autoSend = state;
    if (autoSend) {
        if (settings->value(s_settings_autosend_mode,s_settings_autosend_mode_def).toInt() == 0
            || csettings->value(c_sprintmode,c_sprintmode_def).toBool() ) { // semi-auto
            autoSendStatus->setText("<font color=#006699>AutoSend(ESM)</font>");
        } else {
            autoSendStatus->setText("<font color=#006699>AutoSend("
                + QString::number(settings->value(s_settings_autosend,s_settings_autosend_def).toInt()) + ")</font>");
        }
    } else {
        autoSendStatus->clear();
    }
}

void So2sdr::duelingCQActivate (bool state) {
    duelingCQMode = state;
    if (duelingCQMode) {
        autoCQActivate(false);
        autoSendStatus->hide();
        activeR2CQ = false;
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
    } else {
        duelingCQStatus->clear();
        toggleMode = false;
        toggleStatus->clear();
        if (activeTxRadio != activeRadio) {
            switchTransmit(activeRadio);
        }
        autoSendStatus->show();
    }
}

/*!
 automatically send call and exchange after user defined characters
 toggles with Alt/-
 Buffers and sends call letters until CW catches up
 Backspaced changes before CW catches up to buffer results in stopped CW
 */
void So2sdr::autoSendExch() {

    if (autoSendTrigger && !autoSendPause) {
        if (!activeR2CQ && !duelingCQMode && !toggleMode) {

            cqQsoInProgress[activeTxRadio] = true; // set here so F2 sprint message correct
            if (activeTxRadio == activeRadio && !exchangeSent[activeRadio] && cqMode[activeRadio]
                    && !(altDActive && altDActiveRadio == activeRadio) ) {

                int comp = QString::compare(tmpCall, lineEditCall[activeRadio]->text(), Qt::CaseInsensitive);
                if ( comp < 0) {
                    int cindx = lineEditCall[activeRadio]->text().length() - tmpCall.length();
                    tmpCall = lineEditCall[activeRadio]->text();
                    QString callDiff = lineEditCall[activeRadio]->text().right(cindx);
                    send(callDiff.toAscii(), false);
                } else if (comp > 0) {
                    winkey->cancelcw();
                    //send(QByteArray("?"));
                    autoSendPause = true;
                    tmpCall.clear();
                } else { // calls equal
                    autoSendExch_exch();
                }

            } else if (activeTxRadio != activeRadio && !exchangeSent[activeTxRadio] && cqMode[activeTxRadio]
                       && !(altDActive && altDActiveRadio == activeTxRadio)) {
                // focused other radio, no more call changes
                autoSendExch_exch();

            } else {
                tmpCall.clear();
            }

        } else {
            tmpCall.clear();
        }

    } else {
        if (settings->value(s_settings_autosend_mode,s_settings_autosend_mode_def).toInt() == 1
                && !csettings->value(c_sprintmode,c_sprintmode_def).toBool()
                && lineEditCall[activeRadio]->text().length() >= settings->value(s_settings_autosend,s_settings_autosend_def).toInt()
                && !autoSendPause && !lineEditCall[activeRadio]->text().contains("?")
           )
        {
            autoSendTrigger = true;
        }
    }
}

/*!
 AutoSend exchange handler
 */
void So2sdr::autoSendExch_exch() {
    if (!winkey->isSending()) { // wait until complete call is sent, then start exchange
        autoSendTrigger = false;
        autoSendPause = false;
        // duplicated from pieces of So2sdr::enter, {CALL_ENTERED} removed in Exch message macro
        if (nrSent == nrReserved[activeTxRadio ^ 1]) {
            nrReserved[activeTxRadio] = nrSent + 1;
        } else {
            nrReserved[activeTxRadio] = nrSent;
        }
        int m=(int)cat->modeType(activeTxRadio);
        QByteArray tmpExch;
        if (qso[activeTxRadio]->dupe && csettings->value(c_dupemode,c_dupemode_def).toInt() == STRICT_DUPES) {
            tmpExch = csettings->value(c_dupe_msg[m],c_dupe_msg_def[m]).toByteArray();
        } else {
            tmpExch = csettings->value(c_cq_exc[m],c_cq_exc_def[m]).toByteArray();
            tmpExch.replace("{CALL_ENTERED}", "");
        }
        if (activeRadio != activeTxRadio)
            tmpExch.prepend("{R2}");
        expandMacro(tmpExch,-1,false,false);
        exchangeSent[activeTxRadio]    = true;
        callSent[activeTxRadio]        = true; // set this true as well in case mode switched to S&P
        origCallEntered[activeTxRadio] = qso[activeTxRadio]->call;
        updateNrDisplay();
        cqQsoInProgress[activeTxRadio] = true;
        excMode[activeTxRadio] = true;
        lineEditExchange[activeTxRadio]->show();
        prefillExch(activeTxRadio);
        if (activeRadio == activeTxRadio) {
            lineEditExchange[activeTxRadio]->setFocus();
            if (grab) {
                lineEditExchange[activeTxRadio]->grabKeyboard();
                lineEditExchange[activeTxRadio]->activateWindow();
            }
            grabWidget = lineEditExchange[activeTxRadio];
        }
        callFocus[activeTxRadio] = false;
        if (lineEditExchange[activeTxRadio]->text().simplified().isEmpty()) {
            lineEditExchange[activeTxRadio]->clear();
        } else {
            lineEditExchange[activeTxRadio]->setText(lineEditExchange[activeTxRadio]->text().simplified() + " ");
        }
        tmpCall.clear();
    }
}

/*!
 Automatic repeating CQ, user-defined delay
 */
void So2sdr::autoCQ () {
    qint64 delay = (long long) (settings->value(s_settings_cqrepeat,s_settings_cqrepeat_def).toDouble() * 1000.0D);

    if (activeR2CQ) {
        activeR2CQ = false;
        clearR2CQ(activeRadio ^ 1);
        switchTransmit(activeRadio);
    }
    if (!cqMode[activeRadio] && !(altDActive && altDActiveRadio == activeRadio)) {
        setCqMode(activeRadio);
    } else if (!cqMode[activeRadio ^ 1] && altDActive && altDActiveRadio == activeRadio) {
        setCqMode(activeRadio ^ 1);
    }
    if (autoCQModePause) {
        cqTimer.restart();
        autoCQStatus->setText("<font color=#5200CC>AutoCQ (SLEEP)</font>");
    } else if ((!lineEditCall[activeRadio]->text().isEmpty() && !(altDActive && altDActiveRadio == activeRadio))
            || (!lineEditCall[activeRadio ^ 1]->text().isEmpty() && altDActive && altDActiveRadio == activeRadio)
            || (altDActive > 2 && altDActiveRadio == activeRadio)) {
        if (!autoCQModePause) autoCQModePause = true;
        cqTimer.restart();
    } else if (winkey->isSending()) {
        cqTimer.restart();
        autoCQStatus->setText("<font color=#5200CC>AutoCQ ("
           + QString::number(settings->value(s_settings_cqrepeat,s_settings_cqrepeat_def).toFloat(),'f',1) + "s)</font>");
    } else if (cqTimer.elapsed() >= delay) {
        cqTimer.restart();
        if (altDActive && altDActiveRadio == activeRadio) {
            switchTransmit(altDActiveRadio ^ 1);
        } else if (activeTxRadio != activeRadio) {
            switchTransmit(activeRadio);
        }
        switch (cat->modeType(activeTxRadio)) {
        case CWType:case DigiType:
            if (sendLongCQ) {
                expandMacro(cwMessage->cqF[0],0,false);
            } else {
                expandMacro(cwMessage->cqF[1],0,false);
            }
            break;
        case PhoneType:
            if (sendLongCQ) {
                expandMacro(ssbMessage->cqF[0],0,false);
            } else {
                expandMacro(ssbMessage->cqF[1],0,false);
            }
            break;
        }
    } else {
        autoCQStatus->setText("<font color=#5200CC>AutoCQ ("
           + QString::number(settings->value(s_settings_cqrepeat,s_settings_cqrepeat_def).toFloat() - ((float) cqTimer.elapsed() / 1000.0),'f',1) + "s)</font>");
    }
}

/*!
  Increment AutoCQ +/- 0.1 sec: alt-PgUP / alt-PgDN
 */
void So2sdr::autoCQdelay (bool incr) {
    if (incr) {
        settings->setValue(s_settings_cqrepeat,settings->value(s_settings_cqrepeat,s_settings_cqrepeat_def).toDouble() + 0.1D);
    } else {
        settings->setValue(s_settings_cqrepeat,settings->value(s_settings_cqrepeat,s_settings_cqrepeat_def).toDouble() - 0.1D);
        if (settings->value(s_settings_cqrepeat,s_settings_cqrepeat_def).toDouble() < 0) {
            settings->setValue(s_settings_cqrepeat, 0.0D);
        }
    }
    progsettings->CQRepeatLineEdit->setText(settings->value(s_settings_cqrepeat,s_settings_cqrepeat_def).toString());
    settings->sync();
    if (autoCQMode && winkey->isSending()) {
        autoCQStatus->setText("<font color=#5200CC>AutoCQ ("
           + QString::number(settings->value(s_settings_cqrepeat,s_settings_cqrepeat_def).toFloat(),'f',1) + "s)</font>");
    } else {
        So2sdrStatusBar->showMessage("CQ DELAY: " +
             QString::number(settings->value(s_settings_cqrepeat,s_settings_cqrepeat_def).toFloat(),'f',1), 2000);
    }
}

/*!
 Dueling CQ, user-defined delay
 */
void So2sdr::duelingCQ () {
    qint64 delay = (long long) (settings->value(s_settings_duelingcqdelay,s_settings_duelingcqdelay_def).toDouble() * 1000.0D);

    //toggleMode = true;
    activeR2CQ = false;
    if (!cqMode[activeRadio]) setCqMode(activeRadio);
    if (!cqMode[activeRadio ^ 1]) setCqMode(activeRadio ^ 1);

    if (duelingCQWait) {
        if (winkey->isSending()) { // prevent switching hysteresis
            duelingCQWait = false;
        }
    } else if (!lineEditCall[activeRadio]->text().isEmpty() || !lineEditCall[activeRadio ^ 1]->text().isEmpty()) {
        duelingCQStatus->setText("<font color=#006B00>DuelingCQ (ESM) </font>");
    } else if (winkey->isSending()) {
        cqTimer.restart();
    } else if (cqTimer.elapsed() >= delay) {
        duelingCQWait = true;
        duelingCQStatus->setText("<font color=#006B00>DuelingCQ (AUTO)</font>");
        toggleEnter(Qt::NoModifier);
    }
}

/*!
     swap freqs between radios
 */
void So2sdr::swapRadios()
{
    int old_f[NRIG]={rigFreq[0],rigFreq[1]};
    band[1]  = getBand(old_f[0]);
    band[0]  = getBand(old_f[1]);
    updateMults(0);
    updateMults(1);
    if (!cat) return;
    qsy(0, old_f[1], true);
    qsy(1, old_f[0], true);
}

void So2sdr::toggleStereo() {
    if (settings->value(s_radios_pport_enabled,s_radios_pport_enabled_def).toBool()) {
        pport->toggleStereoPin();
    }
    if (settings->value(s_otrsp_enabled,s_otrsp_enabled_def).toBool()) {
        otrsp->toggleStereo(activeRadio);
    }
    if (settings->value(s_microham_enabled,s_microham_enabled_def).toBool()) {
        microham->toggleStereo(activeRadio);
    }
    switchAudio(activeRadio); //update indicators
}

/*!
 Switch Audio
 */
void So2sdr::switchAudio(int r)
{
    if (settings->value(s_radios_pport_enabled,s_radios_pport_enabled_def).toBool()) {
        pport->switchAudio(r);
    }
    if (settings->value(s_otrsp_enabled,s_otrsp_enabled_def).toBool()) {
        otrsp->switchAudio(r);
    }
    if (settings->value(s_microham_enabled,s_microham_enabled_def).toBool()) {
        microham->switchAudio(r);
    }
    if (settings->value(s_settings_focusindicators,s_settings_focusindicators_def).toBool()) {
        bool stereo = false;
        if (settings->value(s_radios_pport_enabled,s_radios_pport_enabled_def).toBool()) {
            stereo = pport->stereoActive();
        }
        if (settings->value(s_otrsp_enabled,s_otrsp_enabled_def).toBool()) {
            stereo = otrsp->stereoActive();
        }
        if (settings->value(s_microham_enabled,s_microham_enabled_def).toBool()) {
            stereo = microham->stereoActive();
        }
        if (stereo) {
            RX1->setStyleSheet(greenLED);
            RX2->setStyleSheet(greenLED);
        } else {
            if (activeRadio) {
                RX1->setStyleSheet(clearLED);
                RX2->setStyleSheet(greenLED);
            } else {
                RX1->setStyleSheet(greenLED);
                RX2->setStyleSheet(clearLED);
            }
        }
    } else {
        RX1->setStyleSheet(clearLED);
        RX2->setStyleSheet(clearLED);
    }
}

/*!
  Switch transmit focus
 */
void So2sdr::switchTransmit(int r, int CWspeed)
{
    winkey->cancelcw();
    if (CWspeed) {
        winkey->setSpeed(CWspeed);
    } else {
        winkey->setSpeed(wpm[r]);
    }
    if (r != activeTxRadio) {
        autoSendTrigger=false;
        autoSendPause=false;
        winkey->switchTransmit(r);

        if (settings->value(s_radios_pport_enabled,s_radios_pport_enabled_def).toBool()) {
            pport->switchTransmit(r);
        }
        if (settings->value(s_otrsp_enabled,s_otrsp_enabled_def).toBool()) {
            otrsp->switchTransmit(r);
        }
        if (settings->value(s_microham_enabled,s_microham_enabled_def).toBool()) {
            microham->switchTransmit(r);
        }
    }
    if (settings->value(s_settings_focusindicators,s_settings_focusindicators_def).toBool()) {
        if (r) {
            TX1->setStyleSheet(clearLED);
            TX2->setStyleSheet(redLED);
        } else {
            TX1->setStyleSheet(redLED);
            TX2->setStyleSheet(clearLED);
        }
    }    else {
        TX1->setStyleSheet(clearLED);
        TX2->setStyleSheet(clearLED);
    }
    activeTxRadio = r;
}

/*!
   Switch active radio

   Alt+R

   switchcw (default=true) controls whether cw/ssb is switched or not
 */
void So2sdr::switchRadios(bool switchcw)
{
    if (switchcw && !altDActive) autoCQActivate(false);
    activeRadio = activeRadio ^ 1;
    clearR2CQ(activeRadio);
    switchAudio(activeRadio);
    if (switchcw) {
        switchTransmit(activeRadio);
    }
    setEntryFocus();
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

    // if only using 1 dupesheet, repopulate it so that it follows the active radio
    if (nDupesheet==1) populateDupesheet();
}

/*!
   Start prefix check on radio 1
 */
void So2sdr::prefixCheck1(const QString &call)
{
    prefixCheck(0, call);
}

/*!
   Start prefix check on radio 2
 */
void So2sdr::prefixCheck2(const QString &call)
{
    prefixCheck(1, call);
}

/*!
   Start exchange check on radio 1
 */
void So2sdr::exchCheck1(const QString &exch)
{
    exchCheck(0,exch);
}

/*!
   Start exchange check on radio 2
 */
void So2sdr::exchCheck2(const QString &exch)
{
    exchCheck(1,exch);
}

/*!
 Check exchange of radio nr, show multiplier status
 */
void So2sdr::exchCheck(int nr,const QString &exch)
{
    if (qso[nr]->call.isEmpty()) {
        qso[nr]->valid=false;
        return; // do nothing unless we have a callsign
    }
    // recopy call; needed in case call was changed by a previous
    // exchange edit
    qso[nr]->call=lineEditCall[nr]->text().toAscii();
    qso[nr]->exch=exch.toAscii();
    qso[nr]->valid=contest->validateExchange(qso[nr]);
    if (qso[nr]->valid) {
        updateWorkedMult(nr);
        validLabel[nr]->setPixmap(iconValid);
    } else {
        validLabel[nr]->clear();
    }
}

/*!
   Supercheck partial a partial call
 */
void So2sdr::superPartial(QByteArray partial)
{
    static QByteArray lastcalls = "";
    QByteArray        calls;
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
 */
bool So2sdr::logPartial(int nrig, QByteArray partial)
{
    bool                dupe = false;
    QList<QByteArray>   calls;
    QList<unsigned int> worked;
    QString             txt("<font color=#0000FF>");
    QList<int>          mults1;
    QList<int>          mults2;
    qso[nrig]->worked = 0;
    searchPartial(qso[nrig], partial, calls, worked, mults1, mults2);
    dupeCheckDone = false; // flag to indicate when a dupe check has been done
    const int nc = calls.size();
    for (int i = 0; i < nc; i++) {
        // for contests where stations can be worked only once on any band
        if (!contest->dupeCheckingByBand()) {
            // all are dupes; show in red
            if (calls.at(i) == partial) {
                dupe               = true;
                dupeCheckDone      = true;
                qso[nrig]->mult[0] = mults1.at(i);
                qso[nrig]->mult[1] = mults2.at(i);
                qso[nrig]->worked  = worked[i];
            }
            txt.append("<font color=#FF0000>" + calls.at(i) + " <font color=#000000>");
        } else {
            // show dupe with current band in grey, available calls from other bands in blue
            if (bits[band[nrig]] & worked[i]) {
                txt.append("<font color=#AAAAAA>" + calls.at(i) + " <font color=#000000>");
                if (calls.at(i) == partial) {
                    dupe          = true;
                    dupeCheckDone = true;
                }
            } else {
                txt.append("<font color=#0000FF>" + calls.at(i) + " <font color=#000000>");
            }
            if (calls.at(i) == partial) {
                qso[nrig]->mult[0] = mults1.at(i);
                qso[nrig]->mult[1] = mults2.at(i);
                qso[nrig]->worked  = worked[i];
            }
        }
    }
    MasterTextEdit->setText(txt);
    updateWorkedDisplay(nrig,qso[nrig]->worked);
    updateWorkedMult(nrig);
    return(dupe);
}

/*!
   callsign entry window edited
 */
void So2sdr::prefixCheck(int nrig, const QString &call)
{
    qso[nrig]->call = call.toAscii();
    qso[nrig]->call = qso[nrig]->call.toUpper();
    for (int ii = 0; ii < MMAX; ii++) qso[nrig]->mult[ii] = -1;
    qso[nrig]->valid = false;
    // check/supercheck partial callsign fragment
    // don't do anything unless at least 2 chars entered
    if (qso[nrig]->call.size() > 1) {
        qso[nrig]->prefill.clear();
        qso[nrig]->dupe = false;
        qso[nrig]->mode = cat->mode(nrig);
        qso[nrig]->modeType = cat->modeType(nrig);
        qso[nrig]->freq = rigFreq[nrig];
        qso[nrig]->band = band[nrig];
        qso[nrig]->time = QDateTime::currentDateTimeUtc();
        qso[nrig]->mult_name.clear();
        for (int i = 0; i < MMAX; i++) {
            qso[nrig]->isamult[i] = false;
            qso[nrig]->mult[i]    = -1;
            qso[nrig]->newmult[i] = -1;
        }
        qso[nrig]->zone = 0;
        qso[nrig]->pts  = 0;
        bool qsy;
        int  pp = cty->idPfx(qso[nrig], qsy);

        // 2nd radio CQ is active, display these on the other radio
        // unless in S/P on active radio
        int nr = nrig;
        if (activeR2CQ && cqMode[activeRadio]) {
            nr = nr ^ 1;
        }
        // WPX contest: check the callsign prefix
        // @todo create a generic callsign "pre-check" for contest.cpp, have contest_wpx check the prefix there
        if (csettings->value(c_contestname,c_contestname_def).toString().toUpper()=="WPX") {
            ((WPX*) contest)->wpxPrefix(qso[nrig]->call, qso[nrig]->mult_name);
            qso[nrig]->isamult[nrig]=true;
            contest->multIndx(qso[nrig]);
        }
        labelCountry[nr]->setText(qso[nrig]->country_name);
        if (pp != -1) {
            // prefix ID successful

            // working oneself? Fill in 0 heading and exact sun times
            // (useful shortcut to recall exact sunrise/sunset)
            if (qso[nrig]->call==settings->value(s_call,s_call_def)) {
                labelBearing[nr]->setText("0");
                labelLPBearing[nr]->setText("<font color=#0000FF>0");
                QString sun;
                cty->mySunTimes(sun);
                sunLabelPtr[nr]->setText(sun);
            } else {
                labelBearing[nr]->setNum(qso[nrig]->bearing);
                labelLPBearing[nr]->setText("<font color=#0000FF>"+QString::number(((qso[nrig]->bearing+180)%360)));
                sunLabelPtr[nr]->setText(qso[nrig]->sun);
            }
            contest->guessMult(qso[nrig]);
        } else {
            // prefix ID failed, just leave these blank
            labelBearing[nr]->clear();
            labelLPBearing[nr]->clear();
            sunLabelPtr[nr]->clear();
        }
        // if dupe option is 2 (no dupe checking), dupes will not be recorded and will
        // be scored
        qso[nrig]->dupe = logPartial(nrig, qso[nrig]->call) && (csettings->value(c_dupemode,c_dupemode_def).toInt() < NO_DUPE_CHECKING);
        if (csettings->value(c_mastermode,c_mastermode_def).toBool()) {
            superPartial(qso[nrig]->call);
        }
        // prefill previous or guessed exchange if exchange has not
        // been edited by user
        // only S&P mode handled here; CQ mode prefill handled in enter()
        if (!cqMode[nr] && !qsy) {
            prefillExch(nr);
        }
        // show/clear dupe messages
        if (qso[nrig]->dupe) {
            // show information from most recent previous qso with dupe message
            if (csettings->value(c_dupemode,c_dupemode_def).toInt()==STRICT_DUPES) {
                So2sdrStatusBar->showMessage("** " + qso[nrig]->call + " DUPE ** : " +
                                             qso[nrig]->logInfo +
                                             " Ctrl+Enter to log");
            } else if (csettings->value(c_dupemode,c_dupemode_def).toInt()==WORK_DUPES) {
                So2sdrStatusBar->showMessage("** " + qso[nrig]->call + " DUPE ** : " +
                                             qso[nrig]->logInfo);
            }
            statusBarDupe = true;
        } else {
            So2sdrStatusBar->clearMessage();
            statusBarDupe = false;
        }
        // if exchange isn't empty, should recheck it now
        if (!lineEditExchange[nrig]->text().isEmpty()) {
            exchCheck(nrig,lineEditExchange[nrig]->text());
        }
    } else {
        if (csettings->value(c_mastermode,c_mastermode_def).toBool()) {
            MasterTextEdit->clear();
        }

        // if 2nd radio CQ and S/P on active radio, display on active radio
        int nr = nrig;
        if (activeR2CQ && cqMode[activeRadio]) {
            nr = nr ^ 1;
        }
        labelCountry[nr]->clear();
        labelBearing[nr]->clear();
        labelLPBearing[nr]->clear();
        sunLabelPtr[nr]->clear();
        clearWorked(nr);
    }
}

/*!
   update display showing bands a station has been worked on
 */
void So2sdr::updateWorkedDisplay(int nr,unsigned int worked)
{
    QString tmp = "Q :";
    for (int i = 0; i < N_BANDS; i++) {
        if (i==N_BANDS_SCORED) break;
        if (worked & bits[i]) {
            tmp = tmp + "<font color=#000000>" + bandName[i] + "</font> ";
        } else {
            tmp = tmp + "<font color=#AAAAAA>" + bandName[i] + "</font> ";
        }
    }
    if (nr == 0) {
        Qso1Label->setText(tmp);
    } else {
        Qso2Label->setText(tmp);
    }
}

/*!
   clear worked Q/M display on radio i
 */
void So2sdr::clearWorked(int i)
{
    qsoWorkedLabel[i]->setText("Q :");
    for (int j = 0; j < MMAX; j++) {
        multWorkedLabel[i][j]->setText(multNameLabel[j]->text());
    }
}

/*!
   Update band/mult display just above call entry
 */
void So2sdr::updateWorkedMult(int nr)
{
    unsigned int worked[2]={0,0};
    contest->workedMults(qso[nr], worked);
    for (int ii = 0; ii < MMAX; ii++) {
        QString tmp = multNameLabel[ii]->text();
        if (tmp.isEmpty()) continue;

        if (!csettings->value(c_multsband,c_multsband_def).toBool()) {
            if (worked[ii] == (1 + 2 + 4 + 8 + 16 + 32)) {
                // mult already worked
                if (qso[nr]->isamult[nr])
                    tmp = tmp + "<font color=#000000>      WORKED      </font>";
                else
                    tmp.clear();
            } else {
                // needed
                if (qso[nr]->isamult[nr])
                    tmp = tmp + "<font color=#CC0000>      NEEDED      </font>";
                else
                    tmp.clear();
            }
        } else {
            for (int i = 0; i < N_BANDS; i++) {
                if (i==N_BANDS_SCORED) break;

                if (worked[ii] & bits[i]) {
                    tmp = tmp + "<font color=#000000>" + bandName[i] + "</font> ";
                } else {
                    tmp = tmp + "<font color=#AAAAAA>" + bandName[i] + "</font> ";
                }
            }
        }
        multWorkedLabel[nr][ii]->setText(tmp);
    }
}
/*!
   take entered number and qsy rig

   if ; follows the entered freq, 2nd radio is qsyed
 */
bool So2sdr::enterFreqOrMode()
{
    // check for 2nd radio flag ";"
    int s  = qso[activeRadio]->call.size();
    int nr = activeRadio;
    if (s > 1 && qso[activeRadio]->call.at(s - 1) == ';') {
        nr = nr ^ 1;
        qso[activeRadio]->call.chop(1);
    }


    // Entered mode command string is optionally followed by a
    // passband width integer in Hz. e.g. "USB" or "USB1800".
    // String must start at the beginning (index 0) of the string.
    QRegExp rx("^(CWR|CW|LSB|USB|FM|AM)(\\d{2,5})?$");

    // Allow the UI to receive values in kHz down to the Hz
    // i.e. "14250.340" will become 14250340 Hz
    bool ok = false;
    int  f  = (int)(double)(1000 * qso[activeRadio]->call.toDouble(&ok));

    // validate we have a positive integer
    if (f > 0 && ok) {
        // qsy returns "corrected" rigFreq in event there is no radio CAT connection
        if (cat) {
            qsy(nr, f, true);
        }

        int b;
        if ((b = getBand(f)) != -1) {
            // if band change, update bandmap calls
            if (b!=band[nr] && bandmap->bandmapon(nr)) {
                bandmap->syncCalls(nr,spotList[b]);
            }
            band[nr] = b;
        }
        if (bandmap->bandmapon(nr)) {
            bandmap->bandmapSetFreq(f,nr);
            bandmap->setAddOffset(cat->ifFreq(nr),nr);
            // invert spectrum if needed
            // bandmap[nr]->setInvert(bandInvert[nr][band[nr]] ^ (cat->mode(nr) == RIG_MODE_CWR));
        }

    } else if (rx.indexIn(qso[activeRadio]->call) == 0) {
        pbwidth_t pb = RIG_PASSBAND_NORMAL;
        QString pass = rx.cap(2);

        if (pass.length() > 1) {
            pb = pass.toLong();
        }

        // 0 Hz not valid!  Hamlib backends should deal with negative values
        if (!pb)
            pb = RIG_PASSBAND_NORMAL;

        /*! @todo RTC: this will have to handle digital modes eventually as well
         */
        if (rx.cap(1) == "CWR") {
            modeTypeShown=CWType;
            emit setRigMode(nr, RIG_MODE_CWR, pb);
        } else if (rx.cap(1) == "CW") {
            modeTypeShown=CWType;
            emit setRigMode(nr, RIG_MODE_CW, pb);
        } else if (rx.cap(1) == "LSB") {
            modeTypeShown=PhoneType;
            emit setRigMode(nr, RIG_MODE_LSB, pb);
        } else if (rx.cap(1) == "USB") {
            modeTypeShown=PhoneType;
            emit setRigMode(nr, RIG_MODE_USB, pb);
        } else if (rx.cap(1) == "FM") {
            modeTypeShown=PhoneType;
            emit setRigMode(nr, RIG_MODE_FM, pb);
        } else if (rx.cap(1) == "AM") {
            modeTypeShown=PhoneType;
            emit setRigMode(nr, RIG_MODE_AM, pb);
        }
        setSummaryGroupBoxTitle();

    } else {
        // Incomplete frequency or mode entered
        return false;
    }


    qso[activeRadio]->call.clear();
    lineEditCall[activeRadio]->clear();
    lineEditCall[activeRadio]->setFocus();

    if (grab) {
        lineEditCall[activeRadio]->grabKeyboard();
        lineEditCall[activeRadio]->activateWindow();
    }

    grabWidget = lineEditCall[activeRadio];
    lineEditCall[activeRadio]->setModified(false);
    updateBreakdown();
    updateMults(activeRadio);

    if (nDupesheet) {
        populateDupesheet();
    }

    clearWorked(activeRadio);
    labelCountry[activeRadio]->clear();
    labelBearing[activeRadio]->clear();
    labelLPBearing[activeRadio]->clear();
    sunLabelPtr[activeRadio]->clear();

    return true;
}


/*!
   update display of qsos/score
 */
void So2sdr::updateBreakdown()
{
    int n = 0;
    for (int i = 0; i < N_BANDS; i++) {
        n += nqso[i];
        if (i==N_BANDS_SCORED) break;

        qsoLabel[i]->setNum(nqso[i]);
    }
    int nm[2]={0,0};
    int nb[2]={0,0};
    for (int ii = 0; ii < csettings->value(c_nmulttypes,c_nmulttypes_def).toInt(); ii++) {
        for (int i = 0; i < N_BANDS; i++) {
            int m = contest->nMultsBWorked(ii, i);
            nm[ii] += m;
            if (i==N_BANDS_SCORED) break;

            multLabel[ii][i]->setNum(m);
        }

        // for contests where mults are not per-band
        nb[ii] += contest->nMultsBWorked(ii, N_BANDS);
        if (csettings->value(c_multsband,c_multsband_def).toBool()) {
            multTotal[ii]->setNum(nm[ii]);
        } else {
            multTotal[ii]->setNum(nb[ii]);
        }
    }
    TotalQsoLabel->setNum(n);
    ScoreLabel->setText(QString::number(contest->Score()) + " pts");
}


/*!
   update needed mults display for radio ir
 */
void So2sdr::updateMults(int ir)
{
    MultTextEdit->clear();
    if (!csettings->value(c_showmults,c_showmults_def).toBool()) return;

    QByteArray tmp;
    tmp.clear();
    for (int i = 0; i < contest->nMults(multMode); i++) {
        bool       needed_band, needed;
        QByteArray mult = contest->neededMultName(multMode, band[ir], i, needed_band, needed);
        if (excludeMults[multMode].contains(mult)) continue;
        if (csettings->value(c_multsband,c_multsband_def).toBool()) {
            if (needed_band) {
                tmp = tmp + "<font color=#FF0000>" + mult + "</font> "; // red=needed
            } else {
                tmp = tmp + "<font color=#AAAAAA>" + mult + "</font> "; // grey=worked
            }
        } else {
            if (needed) {
                tmp = tmp + "<font color=#FF0000>" + mult + "</font> "; // red=needed
            } else {
                tmp = tmp + "<font color=#AAAAAA>" + mult + "</font> "; // grey=worked
            }
        }
    }
    MultTextEdit->setHtml(tmp);
    MultGroupBox->setTitle("Mults: Radio " + QString::number(ir + 1) + ": " + bandName[band[ir]]);
}

/*!
   update radio frequency/mode display
 */
void So2sdr::updateRadioFreq()
{
    int           tmp[NRIG];
    label_160->setStyleSheet("QLabel { background-color : palette(Background); color : black; }");
    label_80->setStyleSheet("QLabel { background-color : palette(Background); color : black; }");
    label_40->setStyleSheet("QLabel { background-color : palette(Background); color : black; }");
    label_20->setStyleSheet("QLabel { background-color : palette(Background); color : black; }");
    label_15->setStyleSheet("QLabel { background-color : palette(Background); color : black; }");
    label_10->setStyleSheet("QLabel { background-color : palette(Background); color : black; }");
    for (int i = 0; i < NRIG; i++) {

        rigFreq[i] = cat->getRigFreq(i);
        tmp[i]     = band[i];
        int b = getBand(rigFreq[i]);

        if (b != -1) {
            if (b!=band[i] && bandmap->bandmapon(i)) {
                bandmap->syncCalls(i,spotList[band[i]]);
            }
            band[i] = b;
        }

        // band change event
        // note: entering the frequency from the keyboard will not
        // register as a band change here, but in enterFreqOrMode
        if (contest && tmp[i] != band[i]) {
            if (i == activeRadio) {
                updateMults(i);
                if (nDupesheet) {
                    populateDupesheet();
                }
            }

        }
        if (bandmap->bandmapon(i)) {
            bandmap->bandmapSetFreq(rigFreq[i],i);
            //add additional offset if specified by radio (like K3)
            bandmap->setAddOffset(cat->ifFreq(i),i);
        }
        double f = rigFreq[i] / 1000.0;
        if (cat->radioOpen(i)) {
            rLabelPtr[i]->setText("R" + QString::number(i + 1) + ":ON");
        } else {
            rLabelPtr[i]->setText("<font color=#FF0000>R" + QString::number(i + 1) + ":OFF </font>");
        }
        switch (band[i]) {
        case BAND160:
            label_160->setStyleSheet("QLabel { background-color : grey; color : white; }");
            break;
        case BAND80:
            label_80->setStyleSheet("QLabel { background-color : grey; color : white; }");
            break;
        case BAND40:
            label_40->setStyleSheet("QLabel { background-color : grey; color : white; }");
            break;
        case BAND20:
            label_20->setStyleSheet("QLabel { background-color : grey; color : white; }");
            break;
        case BAND15:
            label_15->setStyleSheet("QLabel { background-color : grey; color : white; }");
            break;
        case BAND10:
            label_10->setStyleSheet("QLabel { background-color : grey; color : white; }");
            break;
        }
        if (i == activeRadio) {
            freqDisplayPtr[i]->setText("<b>" + QString::number(f, 'f', 1) + "</b>");
            modeDisplayPtr[i]->setText("<b>" + cat->modeStr(i) + "</b>");
        } else {
            freqDisplayPtr[i]->setText(QString::number(f, 'f', 1));
            modeDisplayPtr[i]->setText(cat->modeStr(i));
        }
    }
    if (winkey->winkeyIsOpen()) {
        winkeyLabel->setText("WK:ON");
    } else {
        winkeyLabel->setText("<font color=#FF0000>WK:OFF </font>");
    }
}

/*!
   set a default freq based on band if there is no radio serial connection
 */
void So2sdr::setDefaultFreq(int nrig)
{
    switch (band[nrig]) {
    case BAND160: rigFreq[nrig] = 1800000; break;
    case BAND80: rigFreq[nrig] = 3500000; break;
    case BAND40: rigFreq[nrig] = 7000000; break;
    case BAND20: rigFreq[nrig] = 14000000; break;
    case BAND15: rigFreq[nrig] = 21000000; break;
    case BAND10: rigFreq[nrig] = 28000000; break;
    case BAND60: rigFreq[nrig] = 5900000; break;
    case BAND30: rigFreq[nrig] = 10100000; break;
    case BAND17: rigFreq[nrig] = 18068000; break;
    case BAND12: rigFreq[nrig] = 24890000;break;
    case BAND6: rigFreq[nrig] = 50000000; break;
    case BAND2: rigFreq[nrig] = 144000000; break;
    }
}

/*!
   Launch speed up: if modifier=ctrl key, apply to 2nd radio
 */
void So2sdr::launch_speedUp(Qt::KeyboardModifiers mod)
{
    switch (mod) {
    case Qt::NoModifier:
        speedUp(activeRadio);
        break;
    case Qt::ControlModifier:
        speedUp(activeRadio ^ 1);
        break;
    default:
        return;
    }
}

/*!
   Launch speed down: if modifier=ctrl key, apply to 2nd radio
 */
void So2sdr::launch_speedDn(Qt::KeyboardModifiers mod)
{
    switch (mod) {
    case Qt::NoModifier:
        speedDn(activeRadio);
        break;
    case Qt::ControlModifier:
        speedDn(activeRadio ^ 1);
        break;
    default:
        return;
    }
}

/*!
   Speed up (page_up)

   increases WPM by 2
 */
void So2sdr::speedUp(int nrig)
{
    wpm[nrig] += 2;
    if (wpm[nrig] > 99) wpm[nrig] = 99;
    wpmLineEditPtr[nrig]->setText(QString::number(wpm[nrig]));

    // don't actually change speed if we are sending on other radio
    if (nrig != activeTxRadio)
    {
        return;
    }
    winkey->setSpeed(wpm[nrig]);
}

/*!
   Speed down (page_down)

   decreases WPM by 2
 */
void So2sdr::speedDn(int nrig)
{
    wpm[nrig] -= 2;
    if (wpm[nrig] < 5) wpm[nrig] = 5;
    wpmLineEditPtr[nrig]->setText(QString::number(wpm[nrig]));

    // don't actually change speed if we are sending on other radio
    if (nrig != activeTxRadio)
    {
        return;
    }
    winkey->setSpeed(wpm[nrig]);
}

/*!
   Start entering CW speed
 */
void So2sdr::launch_WPMDialog()
{
    // save where keyboard focus was
    if (lineEditCall[activeRadio]->hasFocus()) {
        callFocus[activeRadio] = true;
    } else {
        callFocus[activeRadio] = false;
    }
    wpmLineEditPtr[activeRadio]->setReadOnly(false);
    wpmLineEditPtr[activeRadio]->setFocus();
    if (grab) {
        wpmLineEditPtr[activeRadio]->grabKeyboard();
    }
    grabWidget = wpmLineEditPtr[activeRadio];
    wpmLineEditPtr[activeRadio]->selectAll();
}

/*!
   slot called when cw speed edited
 */
void So2sdr::launch_enterCWSpeed(const QString & text)
{
    enterCWSpeed(activeRadio, text);
}

/*!
   Get new CW speed from lineedit
 */
void So2sdr::enterCWSpeed(int nrig, const QString & text)
{
    bool ok = false;

    // try to convert to a digit. Ok=false for non-digits
    int w = text.toInt(&ok);

    // must enter exactly 2 digits
    if (text.size() < 2) {
        if (ok) return;
    } else {
        if (ok) {
            wpm[nrig] = w;
            // don't actually change speed if we are sending on other radio
            if (nrig == activeTxRadio) {
                winkey->setSpeed(wpm[nrig]);
            }
        }
    }

    // reset focus
    wpmLineEditPtr[nrig]->setReadOnly(true);
    if (callFocus[nrig]) {
        lineEditCall[nrig]->setFocus();
        if (grab) {
            lineEditCall[activeRadio]->grabKeyboard();
            lineEditCall[activeRadio]->activateWindow();
        }
        grabWidget = lineEditCall[activeRadio];
    } else {
        lineEditExchange[nrig]->setFocus();
        if (grab) {
            lineEditExchange[activeRadio]->grabKeyboard();
            lineEditExchange[activeRadio]->activateWindow();
        }
        grabWidget = lineEditExchange[activeRadio];
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
void So2sdr::sprintMode()
{
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
void So2sdr::spMode(int i)
{
    cqQsoInProgress[i] = false;
    cqMode[i]          = false;
    if (!altDActive || i!=altDActiveRadio) {
        QPalette palette(lineEditCall[i]->palette());
        palette.setColor(QPalette::Base, SP_COLOR);
        lineEditCall[i]->setPalette(palette);
        lineEditExchange[i]->setPalette(palette);
    }
    if (altDActive!=1 || i!=altDActiveRadio) {
        lineEditExchange[i]->show();
    }
}

/*!
   switch radio i to CQ mode
 */
void So2sdr::setCqMode(int i)
{
    cqQsoInProgress[i] = false;
    excMode[i]         = false;
    exchangeSent[i]    = false;
    lineEditExchange[i]->clear();
    lineEditExchange[i]->hide();
    cqMode[i]     = true;
    nrReserved[i] = 0;
    QPalette palette(lineEditCall[i]->palette());
    palette.setColor(QPalette::Base, CQ_COLOR);
    lineEditCall[i]->setPalette(palette);
    lineEditExchange[i]->setPalette(palette);
}

/*!
   Send a CW message
 */
void So2sdr::send(QByteArray text, bool stopcw)
{
    if (cat->modeType(activeTxRadio)!=CWType) return;
    if (!settings->value(s_winkey_cwon,s_winkey_cwon_def).toBool()) return;

    if (winkey->isSending() && stopcw) {
        winkey->cancelcw();       // cancel any cw in progress
    }
    winkey->loadbuff(text);
    winkey->sendcw();
}

/*!
   clears any 2nd radio cq, resets colors on radio nr
 */
void So2sdr::clearR2CQ(int nr)
{
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
    }
}

/*!
 * \brief So2sdr::expandMacro parse messages
 * \param msg Text of message
 * \param ssbnr If ssbnr >=0, activate {AUDIO} to play or record an audio message with number ssbnr
 * \param ssbRecord If true, record this message
 */
void So2sdr::expandMacro(QByteArray msg,int ssbnr,bool ssbRecord, bool stopcw)
{
#ifndef DVK_ENABLE
    Q_UNUSED(ssbnr);
    Q_UNUSED(ssbRecord);
#endif
    int        tmp_wpm = wpm[activeTxRadio];
    QByteArray out     = "";
    QByteArray txt = "";
    QByteArray command = "";

    // will not overwrite a dupe message that is present
    if (!statusBarDupe) So2sdrStatusBar->clearMessage();
    const QByteArray token_names[] = { "CALL",
                                       "#",
                                       "UP",
                                       "DN",
                                       "R2",
                                       "R2CQ",
                                       "STATE",
                                       "SECTION",
                                       "NAME",
                                       "CQZ",
                                       "ITUZ",
                                       "GRID",
                                       "CALL_ENTERED",
                                       "TOGGLESTEREOPIN",
                                       "CQMODE",
                                       "SPMODE",
                                       "SWAP_RADIOS",
                                       "REPEAT_LAST",
                                       "REPEAT_NR",
                                       "CLEAR_RIT",
                                       "RIG_FREQ",
                                       "RIG2_FREQ",
                                       "BEST_CQ",
                                       "BEST_CQ_R2",
                                       "CANCEL",
                                       "AUDIO",
                                       "SWITCH_RADIOS",
                                       "MCP",
                                       "OTRSP",
                                       "CAT",
                                       "CATR2",
                                       "CAT1",
                                       "CAT2",
                                       "CALL_OK"};
    const int        n_token_names = 34;

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
       - {REPEAT_NR} if the call entry line is not empty, send current qso #. If call entry
                   line is empty, sends number sent for last logged qso
       - {CLEAR_RIT} clears RIT
       - {RIG_FREQ} send current rig frequency in KHz
       - {RIG2_FREQ} send current inactive rig freq in KHz
       - {BEST_CQ} qsy current radio to "best" CQ freq
       - {BEST_CQ_R2} qsy 2nd radio to "best" CQ freq
       - {AUDIO} message contains DVK audio
       - {SWITCH_RADIOS} same as alt-R
       - {MCP}{/MCP} Microham Control Protocol commands
       - {OTRSP}{/OTRSP} OTRSP Control Protocol commands
       - {CAT}{/CAT} raw serial port command to active radio
       - {CATR2}{/CATR2} raw serial port command to second radio
       - {CAT1}{/CAT1} raw serial port command to radio 1
       - {CAT2}{/CAT2} raw serial port command to radio 2
       - {CALL_OK} mark call in entry line as correct, so correct call msg will not be sent
    */
    bool switchradio=true;
    bool first=true;
    bool repeat = false;
    int        i1;
    int        i2;
    if ((i1 = msg.indexOf("{")) != -1) {
        int i0 = 0;
        while (i1 != -1) {
            out.append(msg.mid(i0, i1 - i0));
            txt.append(msg.mid(i0, i1 - i0));
            i2  = msg.indexOf("}", i1);
            QByteArray val = msg.mid(i1 + 1, i2 - i1 - 1);
            val = val.trimmed();
            val = val.toUpper();
            for (int i = 0; i < n_token_names; i++) {
                if (val == token_names[i]) {
                    switch (i) {
                    case 0:  // CALL
                        out.append(settings->value(s_call,s_call_def).toByteArray());
                        txt.append(settings->value(s_call,s_call_def).toByteArray());
                        break;
                    case 1:  // #
                        if (nrReserved[activeTxRadio]) {
                            out.append(QString::number(nrReserved[activeTxRadio]));
                            txt.append(QString::number(nrReserved[activeTxRadio]));
                        } else {
                            out.append(QString::number(nrSent));
                            txt.append(QString::number(nrSent));
                        }
                        break;
                    case 2:  // SPEED UP
                        if (tmp_wpm < 95) {
                            tmp_wpm += 5;
                            out.append(0x1c);
                            out.append((char) tmp_wpm);
                        }
                        break;
                    case 3:  // SPEED DN
                        if (tmp_wpm > 9) {
                            tmp_wpm -= 5;
                            out.append(0x1c);
                            out.append((char) tmp_wpm);
                        }
                        break;
                    case 4: // Radio 2
                        switchTransmit(activeRadio ^ 1);
                        switchradio=false;
                        break;
                    case 5: // Radio 2 CQ
                    {
                        switchTransmit(activeRadio ^ 1);
                        setCqMode(activeRadio ^ 1);
                        activeR2CQ = true;
                        QPalette palette(lineEditCall[activeRadio ^ 1]->palette());
                        palette.setColor(QPalette::Base, ALTD_COLOR);
                        lineEditCall[activeRadio ^ 1]->setPalette(palette);
                        lineEditCall[activeRadio ^ 1]->setText("CQCQCQ");
                        lineEditExchange[activeRadio ^ 1]->setPalette(palette);
                        switchradio=false;
                    }
                        break;
                    case 6: // State
                        out.append(settings->value(s_state,s_state_def).toByteArray());
                        txt.append(settings->value(s_state,s_state_def).toByteArray());
                        break;
                    case 7: // ARRL Section
                        out.append(settings->value(s_section,s_section_def).toByteArray());
                        txt.append(settings->value(s_section,s_section_def).toByteArray());
                        break;
                    case 8: // Name
                        out.append(settings->value(s_name,s_name_def).toByteArray());
                        txt.append(settings->value(s_name,s_name_def).toByteArray());
                        break;
                    case 9: // CQ zone
                        out.append(settings->value(s_cqzone,s_cqzone_def).toByteArray());
                        txt.append(settings->value(s_cqzone,s_cqzone_def).toByteArray());
                        break;
                    case 10: // ITU Zone
                        out.append(settings->value(s_ituzone,s_ituzone_def).toByteArray());
                        txt.append(settings->value(s_ituzone,s_ituzone_def).toByteArray());
                        break;
                    case 11: // grid
                        out.append(settings->value(s_grid,s_grid_def).toByteArray());
                        txt.append(settings->value(s_grid,s_grid_def).toByteArray());
                        break;
                    case 12: // call entered
                        if (!toggleMode) {
                            out.append(qso[activeRadio]->call);
                            txt.append(qso[activeRadio]->call);
                        } else {
                            out.append(qso[activeTxRadio]->call);
                            txt.append(qso[activeTxRadio]->call);
                        }
                        break;
                    case 13: // togglestereopin
                        toggleStereo();
                        // return immediately to avoid stopping cw
                        return;
                        break;
                    case 14: // cqmode
                        setCqMode(activeTxRadio);
                        break;
                    case 15: // spmode
                        spMode(activeTxRadio);
                        break;
                    case 16: // swap_radios
                        swapRadios();
                        break;
                    case 17:  // repeat last
                        repeat = true;
                        break;
                    case 18: // repeat NR
                        int nr;
                        if (lineEditCall[activeTxRadio]->text().isEmpty()) {
                            // send last logged number
                            nr = mylog->lastNr();
                        } else {
                            if (nrReserved[activeTxRadio]) {
                                nr = nrReserved[activeTxRadio];
                            } else {
                                nr = nrSent;
                            }
                        }
                        if (nr != 0) {
                            out.append(QString::number(nr));
                            txt.append(QString::number(nr));
                        }
                        break;
                    case 19: // clear RIT
                        cat->clearRIT(activeTxRadio);
                        break;
                    case 20: // RIG_FREQ
                        out.append(QByteArray::number(qRound(rigFreq[activeRadio] / 1000.0)));
                        txt.append(QByteArray::number(qRound(rigFreq[activeRadio] / 1000.0)));
                        break;
                    case 21: // RIG2_FREQ
                        out.append(QByteArray::number(qRound(rigFreq[activeRadio ^ 1] / 1000.0)));
                        txt.append(QByteArray::number(qRound(rigFreq[activeRadio ^ 1] / 1000.0)));
                        break;
                    case 22: // best cq
                        bandmap->setFreqLimits(activeRadio,settings->value(s_sdr_cqlimit_low[band[activeRadio]],
                                               cqlimit_default_low[band[activeRadio]]).toInt(),
                                settings->value(s_sdr_cqlimit_high[band[activeRadio]],
                                cqlimit_default_high[band[activeRadio]]).toInt());
                        bandmap->findFreq(activeRadio);
                        break;
                    case 23: // best cq radio2
                        bandmap->setFreqLimits(activeRadio^1,settings->value(s_sdr_cqlimit_low[band[activeRadio^1]],
                                cqlimit_default_low[band[activeRadio]]).toInt(),
                                settings->value(s_sdr_cqlimit_high[band[activeRadio^1]],
                                cqlimit_default_high[band[activeRadio^1]]).toInt());
                        bandmap->findFreq(activeRadio ^ 1);
                        // return immediately to avoid stopping cw on current radio
                        return;
                        break;
                    case 24: // cancel speed change
                        out.append(0x1e);
                        break;
                    case 25: // play/record audio
#ifdef DVK_ENABLE
                        if (ssbRecord) {
                            emit(recordDvk(ssbnr));
                        } else {
                            emit(playDvk(ssbnr,activeRadio));
                        }
#endif
                        break;
                    case 26: // switch radios
                        switchRadios();
                        break;
                    case 27: // MCP
                        command = msg.mid(i2 + 1, msg.indexOf("{", i2) - (i2 + 1));
                        command.append("\r");
                        microham->sendCommand(command);
                        i2 = msg.indexOf("}", msg.indexOf("{", i2));
                        break;
                    case 28: // OTRSP
                        command = msg.mid(i2 + 1, msg.indexOf("{", i2) - (i2 + 1));
                        command.append("\r");
                        otrsp->sendCommand(command);
                        i2 = msg.indexOf("}", msg.indexOf("{", i2));
                        break;
                    case 29: // CAT
                        command = msg.mid(i2 + 1, msg.indexOf("{", i2) - (i2 + 1));
                        cat->sendRaw(activeRadio,command);
                        i2 = msg.indexOf("}", msg.indexOf("{", i2));
                        break;
                    case 30: // CATR2
                        command = msg.mid(i2 + 1, msg.indexOf("{", i2) - (i2 + 1));
                        cat->sendRaw(activeRadio ^ 1,command);
                        i2 = msg.indexOf("}", msg.indexOf("{", i2));
                        break;
                    case 31: // CAT1
                        command = msg.mid(i2 + 1, msg.indexOf("{", i2) - (i2 + 1));
                        cat->sendRaw(0,command);
                        i2 = msg.indexOf("}", msg.indexOf("{", i2));
                        break;
                    case 32: // CAT2
                        command = msg.mid(i2 + 1, msg.indexOf("{", i2) - (i2 + 1));
                        cat->sendRaw(1,command);
                        i2 = msg.indexOf("}", msg.indexOf("{", i2));
                        break;
                    case 33: // CALL_OK
                        origCallEntered[activeRadio]=qso[activeRadio]->call;
                        break;
                    }
                    break;
                }
            }
            if (first && switchradio) {
                // the first element of most macros resets TX radio and speed
                // TOGGLESTEREOPIN, R2, R2CQ do not
                if (toggleMode) {
                    switchTransmit(activeTxRadio, tmp_wpm); // don't switch TX focus, pass speed change
                } else if (autoCQMode) {
                    if (altDActive && activeRadio == altDActiveRadio && autoCQModePause) {
                        switchTransmit(altDActiveRadio, tmp_wpm);
                    } else {
                        switchTransmit(activeTxRadio, tmp_wpm);
                    }
                } else {
                    switchTransmit(activeRadio, tmp_wpm);
                }
            }
            first=false;

            i0 = i2 + 1;
            i1 = msg.indexOf("{", i2); // find next one
        }

        // add any remaining text
        if (i2 < (i1 = msg.size())) {
            out.append(msg.right(i1 - i2 - 1));
            txt.append(msg.right(i1 - i2 - 1));
        }

        // cancel any buffered speed changes
        out.append(0x1e);
        if (repeat) {
            send(lastMsg,stopcw);
            if (!statusBarDupe && lastMsg.size()) So2sdrStatusBar->showMessage(lastMsg.simplified());
        } else {
            send(out,stopcw);
            if (!statusBarDupe && txt.size()) {
                txt.prepend(QByteArray::number(activeTxRadio + 1) + ":");
                So2sdrStatusBar->showMessage(txt.simplified());
            }
        }
    } else {
        // no macro present send as-is
        if (toggleMode) {
            switchTransmit(activeTxRadio, tmp_wpm); // don't switch TX focus, pass speed change
        } else if (autoCQMode) {
            if (altDActive && activeRadio == altDActiveRadio && autoCQModePause) {
                switchTransmit(altDActiveRadio, tmp_wpm);
            } else {
                switchTransmit(activeTxRadio, tmp_wpm);
            }
        } else {
            switchTransmit(activeRadio, tmp_wpm);
        }
        out.append(msg);
        txt.append(msg);
        send(out,stopcw);
        if (!statusBarDupe && txt.size()) {
            txt.prepend(QByteArray::number(activeTxRadio + 1) + ":");
            So2sdrStatusBar->showMessage(txt.simplified());
        }
    }
    lastMsg = out;
}


/*! show message on status bar
 */
void So2sdr::showMessage(QString s)
{
    if (!statusBarDupe) {
        So2sdrStatusBar->showMessage(s.simplified());
    }
}

/*!
   Open file dialog
 */
void So2sdr::openFile()
{
    // search for files in directory set by contestDirectory
    directory->setCurrent(contestDirectory);
    fileName = QFileDialog::getOpenFileName(this, tr("Open config file"), contestDirectory, tr("Config Files (*.cfg)"));
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
        regrab();
    }
}

/*!
   rescore and redupe

 */
void So2sdr::rescore()
{
    Qso tmpqso(contest->nExchange());
    QSqlQueryModel m;
    m.setQuery("SELECT * FROM log", mylog->db);
    while (m.canFetchMore()) {
        m.fetchMore();
    }
    bool              b;
    QList<QByteArray> dupes[N_BANDS];
    contest->zeroScore();
    for (int i = 0; i < N_BANDS; i++) {
        nqso[i] = 0;
        dupes[i].clear();
    }
    for (int i = 0; i < m.rowCount(); i++) {
        tmpqso.call    = m.record(i).value("call").toString().toAscii();
        // run prefix check on call: need to check for /MM, etc
        tmpqso.country = cty->idPfx(&tmpqso, b);
        tmpqso.exch.clear();
        QByteArray tmp[4];
        tmp[0] = m.record(i).value("rcv1").toString().toAscii();
        tmp[1] = m.record(i).value("rcv2").toString().toAscii();
        tmp[2] = m.record(i).value("rcv3").toString().toAscii();
        tmp[3] = m.record(i).value("rcv4").toString().toAscii();
        tmpqso.nr=m.record(i).value("nr").toInt();

        for (int j = 0; j < contest->nExchange(); j++) {
            tmpqso.exch = tmpqso.exch + tmp[j] + " ";
        }
        tmpqso.mode = (rmode_t) m.record(i).value("mode").toInt();
        tmpqso.modeType = cat->getModeType(tmpqso.mode);
        tmpqso.band = m.record(i).value("band").toInt();
        tmpqso.pts  = m.record(i).value("pts").toInt();

        // valid can be changed to ways:
        // 1) when user unchecks checkbox
        // 2) if program can't parse the exchange
        //
        // first check for user changing check status
        bool userValid=m.record(i).value("valid").toBool();
        tmpqso.valid=userValid;

        // dupe check
        // qsos marked invalid are excluded from log and dupe check
        tmpqso.dupe = false;
        if (tmpqso.valid) {
            if (csettings->value(c_dupemode,c_dupemode_def).toInt()!=NO_DUPE_CHECKING) {
                // can work station on other bands, just check this one
                QByteArray check=tmpqso.call;
                // multi-mode contest: append a mode index to the call
                if (csettings->value(c_multimode,c_multimode_def).toBool()) {
                    check=check+QByteArray::number((int)tmpqso.modeType);
                }
                if (contest->dupeCheckingByBand()) {
                    if (dupes[tmpqso.band].contains(check)) {
                        tmpqso.dupe = true;
                    }
                } else {
                    // qsos count only once on any band
                    for (int j = 0; j < N_BANDS; j++) {
                        if (dupes[j].contains(check)) {
                            tmpqso.dupe = true;
                        }
                    }
                }
                dupes[tmpqso.band].append(check);
            }
        } else {
            tmpqso.pts=0;
            tmpqso.mult[0]=-1;
            tmpqso.mult[1]=-1;
            tmpqso.newmult[0]=-1;
            tmpqso.newmult[1]=-1;
        }
        // next check exchange
        // in the case of mobiles, this might change dupe status!
        bool exchValid=contest->validateExchange(&tmpqso);
        tmpqso.valid=userValid & exchValid;

        if (!tmpqso.dupe && tmpqso.valid) nqso[tmpqso.band]++;
        if (!tmpqso.valid || tmpqso.dupe) {
            tmpqso.pts=0;
            tmpqso.mult[0]=-1;
            tmpqso.mult[1]=-1;
            tmpqso.newmult[0]=-1;
            tmpqso.newmult[1]=-1;
        }
        contest->addQso(&tmpqso);
    }
    updateBreakdown();
    updateMults(activeRadio);
    if (nDupesheet) populateDupesheet();
}


/*!
   fill in sent exchange fields
 */
void So2sdr::fillSentExch(int nr)
{
    for (int i = 0; i < contest->nExchange(); i++) {
        switch (i) {
        case 0:
            qso[nr]->snt_exch[0]=csettings->value(c_sentexch1,c_sentexch1_def).toByteArray();
            break;
        case 1:
            qso[nr]->snt_exch[1]=csettings->value(c_sentexch2,c_sentexch2_def).toByteArray();
            break;
        case 2:
            qso[nr]->snt_exch[2]=csettings->value(c_sentexch3,c_sentexch3_def).toByteArray();
            break;
        case 3:
            qso[nr]->snt_exch[3]=csettings->value(c_sentexch4,c_sentexch4_def).toByteArray();
            break;
        }
        // put in qso number
        if (contest->exchType(i) == QsoNumber) {
            //qso[nr]->snt_exch[i] = QByteArray::number(nrReserved[activeRadio]);
            qso[nr]->snt_exch[i] = QByteArray::number(nrReserved[nr]);
        }
    }
}

/*!
   Show help file
 */
void So2sdr::showHelp()
{
    if (help == 0) {
        // open help file and display it
        directory->setCurrent(dataDirectory()+"/help");
        help = new HelpDialog("so2sdrhelp.html", this);
        connect(help, SIGNAL(accepted()), this, SLOT(regrab()));
        connect(help, SIGNAL(rejected()), this, SLOT(regrab()));
    }
    help->show();
    help->setFocus();
}

/*!
   Switch display of mult type
 */
void So2sdr::switchMultMode()
{
    if (csettings->value(c_nmulttypes,c_nmulttypes_def).toInt() < 2) return;
    multMode++;
    multMode = multMode % MMAX;
    updateMults(activeRadio);
}

/*!
   update display of next qso number to send
 */
void So2sdr::updateNrDisplay()
{
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
   create it.

   returns true if directory exists or was created; false if program
   should exit.
 */
bool So2sdr::checkUserDirectory()
{
    QDir dir;
    if (dir.exists(userDirectory())) return(true);

    QMessageBox *msg= new QMessageBox(this);
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
            msg->deleteLater();
            return(true);
        } else {
            msg->setText("Could not create directory <" + userDirectory() + ">");
            msg->exec();
            msg->deleteLater();
            return(false);
        }
        break;
    case QMessageBox::Cancel:

        // this will abort the program (return false)
        break;
    default:  // never reached
        break;
    }
    msg->deleteLater();
    return(false);
}

/*! start timers
 */
void So2sdr::startTimers()
{
    for (int i=0;i<N_TIMERS;i++) timerId[i]=startTimer(timerSettings[i]);
}

/*! stop timers
 */
void So2sdr::stopTimers()
{
    for (int i=0;i<N_TIMERS;i++) {
        if (timerId[i]) killTimer(timerId[i]);
    }
}


/*! validates freq entered in Khz, returns freq in Hz
   if exact=true, no validation (freq should be in Hz in this case)
 */
void So2sdr::qsy(int nrig, int &freq, bool exact)
{
    if (exact) {
        emit qsyExact(nrig, freq);
    } else {
        // entered in KHz?
        if (freq > 1799 && freq <= 148000) {
            freq *= 1000;
            emit qsyExact(nrig, freq);
        }
    }
}

/*!
  logSearch : triggered with ctrl-F

  do callsign-partial search on contents of call entry window, display matching calls in log window
  for editing
  */
void So2sdr::logSearch()
{
    QByteArray searchFrag=lineEditCall[activeRadio]->text().toAscii();
    if (searchFrag.size()<2) return;

    So2sdrStatusBar->showMessage("Searching log for \""+searchFrag+"\"");
    model->setFilter("CALL LIKE '%"+searchFrag+"%'");
    model->select();
    while (model->canFetchMore()) {
        model->fetchMore();
    }
    if (model->rowCount()==0) {
        So2sdrStatusBar->showMessage("Searching log for \""+searchFrag+"\" : NOT FOUND");
        model->setFilter("");
        model->select();
        while (model->canFetchMore())
            model->fetchMore();
    } else {
        // save a list of rows found by the search
        searchList.clear();
        for (int i=0;i<model->rowCount();i++) {
            searchList.append(model->record(i).value(SQL_COL_NR).toInt()-1);
        }
        LogTableView->scrollToBottom();
        logSearchFlag=true;
    }
}

/*!
  clears any log search in progress and resets SQL filter
  */
void So2sdr::clearLogSearch()
{
    if (logSearchFlag) {
        model->setFilter("");
        model->select();
        while (model->canFetchMore())
            model->fetchMore();
        LogTableView->scrollToBottom();
        logSearchFlag=false;
        So2sdrStatusBar->clearMessage();
    }
}

/*!
  log partial callsign search
  */
void So2sdr::searchPartial(Qso *qso, QByteArray part, QList<QByteArray>& calls, QList<unsigned int>& worked,
                           QList<int>& mult1, QList<int>& mult2)
{
    calls.clear();
    worked.clear();
    mult1.clear();
    mult2.clear();

    clearLogSearch();

    // only do partial call search if we have >= 2 characters
    const int psz = part.size();
    if (psz < 2) {
        return;
    }
    // query for partial fragment
    QSqlQueryModel m;
    m.setQuery("SELECT * FROM log WHERE VALID='true' AND (CALL LIKE'%" + part + "%' )", mylog->db);
    while (m.canFetchMore()) {
        m.fetchMore();
    }
    for (int i = 0; i < m.rowCount(); i++) {
        // if multi-mode contest, check for matching mode
        if (csettings->value(c_multimode).toBool(),c_multimode_def) {
            if (cat->getModeType((rmode_t)m.record(i).value(SQL_COL_MODE).toInt())!=qso->modeType) {
                continue;
            }
        }
        QByteArray tmp = m.record(i).value(SQL_COL_CALL).toString().toAscii();
        //
        /* @todo  the following could probably be simplified  using SQL commands... */
        //
        // see if this call is already in list
        int j=calls.indexOf(tmp);
        if (j != -1) {
            // add this band
            worked[j] = worked[j] | bits[m.record(i).value(SQL_COL_BAND).toInt()];
        } else {
            // new call fragment, or same call and different mode
            // insert call so list is sorted
            int isrt = 0;
            for (int k = 0; k < calls.size(); k++) {
                if (tmp < calls.at(k)) break;
                isrt++;
            }
            calls.insert(isrt, tmp);
            unsigned int w = bits[m.record(i).value(SQL_COL_BAND).toInt()];
            worked.insert(isrt, w);
            int          row = m.record(i).value(SQL_COL_NR).toInt() - 1;
            mult1.insert(isrt, contest->mult(row, 0));
            mult2.insert(isrt, contest->mult(row, 1));
        }
    }

    // if call matches, prefill most recent exchange and mult information from log
    for (int i = m.rowCount() - 1; i >= 0; i--) {
        QByteArray tmp = m.record(i).value(SQL_COL_CALL).toString().toAscii();
        if (tmp == qso->call) {
            qso->prefill.clear();
            int row = m.record(i).value(SQL_COL_NR).toInt() - 1;
            qso->mult[0] = contest->mult(row, 0);
            qso->mult[1] = contest->mult(row, 1);
            qso->prefill = m.record(i).value(SQL_COL_RCV1).toString().toAscii() + " " +
                           m.record(i).value(SQL_COL_RCV2).toString().toAscii() + " " +
                           m.record(i).value(SQL_COL_RCV3).toString().toAscii() + " " +
                           m.record(i).value(SQL_COL_RCV4).toString().toAscii();
            break;
        }
    }

    // fill exchange with history if not in log
    if (qso->prefill.simplified().isEmpty() && csettings->value(c_historymode,c_historymode_def).toBool()) {
        history->fillExchange(qso,part);
    }
}

/*! returns true if initialization was successful
 */
bool So2sdr::so2sdrOk() const
{
    return(initialized);
}

/*!
  read settings from station .ini file
 */
void So2sdr::readStationSettings()
{
    contestDirectory=settings->value(s_contestdirectory,s_contestdirectory_def).toString();
    setWindowTitle("SO2SDR:" + settings->value(s_call,s_call_def).toString());
    for (int i=0;i<NRIG;i++) {
        wpm[i] = settings->value(s_wpm[i],s_wpm_def[i]).toInt();
        wpmLineEditPtr[i]->setText(QString::number(wpm[i]));
    }
}

/*!
  update settings to contest .cfg file
  */
void So2sdr::writeContestSettings()
{
    if (!csettings) return;

    // log column widths
    csettings->beginWriteArray(c_col_width_group);
    for (int i=0;i<SQL_N_COL;i++) {
        csettings->setArrayIndex(i);
        csettings->setValue(c_col_width_item,LogTableView->columnWidth(i));
    }
    csettings->endArray();
}

/*!
  update settings to station .ini file
  */
void So2sdr::writeStationSettings()
{
    // main window geometry
    settings->setValue(s_contestdirectory,contestDirectory);
    settings->beginGroup("MainWindow");
    settings->setValue("size", size());
    settings->setValue("pos", pos());
    settings->endGroup();

    // dupesheet geometry
    if (dupesheet[0]) {
        settings->beginGroup("DupeSheetWindow1");
        settings->setValue("size",dupesheet[0]->size());
        settings->setValue("pos", dupesheet[0]->pos());
        settings->endGroup();
    }
    if (dupesheet[1]) {
        settings->beginGroup("DupeSheetWindow2");
        settings->setValue("size",dupesheet[1]->size());
        settings->setValue("pos", dupesheet[1]->pos());
        settings->endGroup();
    }

    settings->setValue(s_wpm[0],wpmLineEditPtr[0]->text());
    settings->setValue(s_wpm[1],wpmLineEditPtr[1]->text());
}

void So2sdr::closeEvent(QCloseEvent *event)
{
    for (int i=0;i<NRIG;i++) {
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
void So2sdr::readExcludeMults()
{
    directory->setCurrent(dataDirectory());
    for (int ii=0;ii<MMAX;ii++) {
        QString filename;
        switch (ii) {
        case 0:
            filename=csettings->value(c_exclude_mults1,c_exclude_mults1_def).toString();
            break;
        case 1:
            filename=csettings->value(c_exclude_mults2,c_exclude_mults2_def).toString();
            break;
        }
        if (filename.isEmpty()) continue;

        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug("Can't open %s", filename.toAscii().data());
            return;
        }
        excludeMults[ii].clear();
        while (!file.atEnd()) {
            QString     buffer;
            buffer = file.readLine();
            QStringList list = buffer.split(" ", QString::SkipEmptyParts);
            for (int i = 0; i < list.size(); i++) {
                excludeMults[ii].append(list[i].toAscii());
            }
        }
        file.close();
    }
    // return to contest directory
    directory->setCurrent(contestDirectory);
}

/*!
   reads list of known contests/config files
 */
bool So2sdr::readContestList()
{
    directory->setCurrent(dataDirectory());
    QFile file("contest_list.dat");
 	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug("can't open %s/contest_list.dat", dataDirectory().toAscii().data());
        return(false);
    }
    while (!file.atEnd()) {
        QByteArray buffer = file.readLine();
        int        i      = buffer.indexOf(",");
        QByteArray name   = buffer.mid(0, i).trimmed();
        newContest->addContest(name);
        configFiles.append(buffer.right(buffer.size() - i - 1).trimmed());
    }
    file.close();
    return(true);
}

void So2sdr::initPointers()
{
    csettings=0;
    settings=0;
    multMode      = 0;
    help          = 0;
    cty           = 0;
    contest       = 0;
    cat           = 0;
    cabrillo      = 0;
    mylog         = 0;
    cwMessage     = 0;
    ssbMessage    = 0;
    newContest    = 0;
    notes         = 0;
    options       = 0;
    radios        = 0;
    station       = 0;
    sdr           = 0;
    winkey        = 0;
    winkeyDialog  = 0;
    master        = 0;
    model         = 0;
    history       = 0;
    dupesheet[0] = 0;
    dupesheet[1] = 0;
    qso[0]       = 0;
    qso[1]       = 0;
    bandmap      = 0;
    dupesheetCheckBox[0] = 0;
    dupesheetCheckBox[1] = 0;
    dupesheetCheckAction[0] = 0;
    dupesheetCheckAction[1] = 0;
    bandmapCheckBox[0] = 0;
    bandmapCheckBox[1] = 0;
    grabCheckBox = 0;
    telnetCheckBox = 0;
    dupeCalls[0]=0;
    dupeCalls[1]=0;
    dupeCallsKey[0]=0;
    dupeCallsKey[1]=0;
    telnet            = 0;
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
    labelCountry[0]       = labelCountry1;
    labelCountry[1]       = labelCountry2;
    labelBearing[0]       = labelBearing1;
    labelBearing[1]       = labelBearing2;
    labelLPBearing[0]       = labelLPBearing1;
    labelLPBearing[1]       = labelLPBearing2;
    qsoLabel[0]           = qso160Label;
    qsoLabel[1]           = qso80Label;
    qsoLabel[2]           = qso40Label;
    qsoLabel[3]           = qso20Label;
    qsoLabel[4]           = qso15Label;
    qsoLabel[5]           = qso10Label;
    qsoWorkedLabel[0]     = Qso1Label;
    qsoWorkedLabel[1]     = Qso2Label;
    multWorkedLabel[0][0] = Mult1Label;
    multWorkedLabel[0][1] = Mult1Label2;
    multWorkedLabel[1][0] = Mult2Label;
    multWorkedLabel[1][1] = Mult2Label2;
    //bandmapProcess[0]     = 0;
    //bandmapProcess[1]     = 0;

    // the following is needed to get a monospace font under Windows
    for (int i=0;i<NRIG;i++) {
        QFont font("Monospace");
        font.setStyleHint(QFont::TypeWriter);
        qsoWorkedLabel[i]->setFont(font);
        multWorkedLabel[i][0]->setFont(font);
        multWorkedLabel[i][1]->setFont(font);
    }
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
    multTotal[0]    = TotalMultLabel;
    multTotal[1]    = TotalMultLabel2;
    sunLabelPtr[0]  = Sun1Label;
    sunLabelPtr[1]  = Sun2Label;
    numLabelPtr[0] = NumLabel;
    numLabelPtr[1] = NumLabel2;
    validLabel[0]  = validLabel1;
    validLabel[1]  = validLabel2;
}

void So2sdr::initVariables()
{
    initialized = true;
    uiEnabled   = false;
    usveIndx[0] = 0;
    usveIndx[1] = 0;
    labelCountry1->clear();
    labelCountry2->clear();
    labelBearing1->clear();
    labelBearing2->clear();
    labelLPBearing1->clear();
    labelLPBearing2->clear();
    activeRadio = 0;
    activeTxRadio = activeRadio;
    altDActive         = 0;
    altDActiveRadio    = 0;
    altDOrigMode       = 0;
    cqQsoInProgress[0] = false;
    cqQsoInProgress[1] = false;
    for (int i = 0; i < N_BANDS; i++) spotList[i].clear();
    spotListPopUp[0] = false;
    spotListPopUp[1] = false;
    callSent[0]    = false;
    callSent[1]    = false;
    logSearchFlag  = false;
    statusBarDupe = false;
    multNameLabel[0]->clear();
    multNameLabel[1]->clear();
    for (int i = 0; i < N_BANDS; i++) {
        if (i==N_BANDS_SCORED) break;
        multLabel[0][i]->clear();
        multLabel[1][i]->clear();
    }
    wpm[0]        = 35;
    wpm[1]        = 35;
    nrSent        = 1;
    nrReserved[0] = 0;
    nrReserved[1] = 0;
    for (int i = 0; i < N_TIMERS; i++) timerId[i] = 0;
    ratePtr = 0;
    for (int i = 0; i < 60; i++) rateCount[i] = 0;
    nDupesheet   = 0;
    telnetOn          = false;
    for (int i = 0; i < MMAX; i++) {
        excludeMults[i].clear();
    }
    origCallEntered[0].clear();
    origCallEntered[1].clear();
    fileName.clear();
    band[0]    = 3;
    rigFreq[0] = 14000000;
    band[1]    = 2;
    rigFreq[1] = 7000000;
    for (int i = 0; i < N_BANDS; i++) {
        bandInvert[0][i] = false;
        bandInvert[1][i] = false;
    }
    cqMode[0]          = true;
    cqMode[1]          = true;
    excMode[0]         = false;
    excMode[1]         = false;
    callFocus[0]       = true;
    callFocus[1]       = true;
    activeR2CQ         = false;
    exchangeSent[0]    = false;
    exchangeSent[1]    = false;
    dupeCheckDone      = false;
    cqQsoInProgress[0] = false;
    cqQsoInProgress[1] = false;
    grab               = false;
    grabbing           = false;

    cqTimer.start();
    toggleMode = false;
    autoCQMode = false;
    autoCQModePause = false;
    duelingCQMode = false;
    duelingCQWait = false;
    autoSend = false;
    autoSendPause = false;
    autoSendTrigger = false;
    sendLongCQ = true;
    tmpCall.clear();
    toggleFxKey = -1;
    toggleFxKeyMod = Qt::NoModifier;
    toggleFxKeySend = false;
}

/*!
 * \brief So2sdr::screenShot
 * Take a screenshot of the bandmap windows
 *
 */
void So2sdr::screenShot()
{
    // main window
    QCoreApplication::processEvents();
    QPixmap p=QPixmap::grabWindow(winId());
    QCoreApplication::processEvents();
    QString format = "png";
    directory->setCurrent(contestDirectory);
    QString filename="screenshot-main-"+QDateTime::currentDateTimeUtc().toString(Qt::ISODate)+".png";
    p.save(filename,format.toAscii());
    QCoreApplication::processEvents();

    // bandmap windows
    // this could be handled by sending a TCP message to the
    // bandmap process telling it to take a screenshot
    /*
    for (int i=0;i<NRIG;i++) {
        if (bandmap->bandmapon(i)) {

        }
    }
    */
    So2sdrStatusBar->showMessage("Saved screenshot", 3000);
}


void So2sdr::showBandmap1(int checkboxState)
{
    menuWindows->hide();
    bandmap->showBandmap(0,checkboxState);
}

void So2sdr::showBandmap2(int checkboxState)
{
    menuWindows->close();
    bandmap->showBandmap(1,checkboxState);
}
