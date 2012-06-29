/*! Copyright 2010-2012 R. Torsten Clay N4OGW

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
#include <QList>
#include <QMainWindow>
#include <QMessageBox>
#include <QMetaType>
#include <QModelIndex>
#include <QObject>
#include <QPalette>
#include <QProgressDialog>
#include <QRect>
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
#include <QStyleOptionViewItemV4>
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

#ifdef Q_OS_WIN
    // in Windows, keep data files in same directory as executable
    //
    installDirectory = QApplication::applicationDirPath();
    dataDirectory    = installDirectory;
#endif
#ifdef Q_OS_LINUX
    // in Linux, installDirectory is typically /usr/local with executable in
    // /usr/local/bin and data in /usr/local/share/so2sdr
    //
    installDirectory = INSTALL_DIR;
    dataDirectory    = installDirectory + "/share/so2sdr/";
#endif
    // check to see if user directory exists
    initialized = checkUserDirectory();

    settingsFile=userDirectory+"/so2sdr.ini";
    // check for optional command argument giving station config file name
    if (args.size() > 1) {
        settingsFile = args[1];
    }
    settings=new QSettings(settingsFile,QSettings::IniFormat);
    setFocusPolicy(Qt::StrongFocus);
    errorBox           = new QErrorMessage(this);
    setWindowIcon(QIcon(dataDirectory + "/icon24x24.png"));

    // pointers for each radio
    for (int i=0;i<NRIG;i++) {
        wpmLineEditPtr[i]->setFocusPolicy(Qt::NoFocus);
        lineEditCall[i]->setValidator(new UpperValidator(lineEditCall[i]));
        lineEditExchange[i]->setValidator(new UpperValidator(lineEditExchange[i]));
    }
    rLabelPtr[0]      = new QLabel("<font color=#FF0000>R1:OFF /font>");
    rLabelPtr[1]      = new QLabel("<font color=#FF0000>R2:OFF </font>");
    winkeyLabel       = new QLabel("<font color=#FF0000>WK:OFF </font>");
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

    // create hamlib rig interface
    // show a progress dialog if it will take longer than 4 seconds --
    // initial load can take ~30 seconds on Windows
    //
    QProgressDialog progress("Please wait, querying installed hamlib backends", "Abort", 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(4000);
    cat = new RigSerial();
    connect(&progress, SIGNAL(canceled()), cat, SLOT(cancelHamlib()));
    connect(cat, SIGNAL(maxBackends(int)), &progress, SLOT(setMaximum(int)));
    connect(cat, SIGNAL(backendsDone(int)), &progress, SLOT(setValue(int)));
    initialized = cat->initializeHamlib(userDirectory);
    if (!initialized) return;  // in case "abort" clicked
    cat->moveToThread(&catThread);
    connect(&catThread, SIGNAL(started()), cat, SLOT(run()));
    options = new ContestOptionsDialog(this);
    connect(options, SIGNAL(accepted()), this, SLOT(regrab()));
    connect(options, SIGNAL(rejected()), this, SLOT(regrab()));
    connect(options,SIGNAL(rescore()),this,SLOT(rescore()));
    options->hide();
    cabrillo = new CabrilloDialog(this);
    cabrillo->hide();
    station = new StationDialog(settings,this);
    connect(station, SIGNAL(accepted()), this, SLOT(regrab()));
    connect(station, SIGNAL(rejected()), this, SLOT(regrab()));
    connect(station, SIGNAL(stationUpdate()), this, SLOT(stationUpdate()));
    station->hide();
    cwMessage = new CWMessageDialog(this);
    connect(cwMessage, SIGNAL(accepted()), this, SLOT(regrab()));
    connect(cwMessage, SIGNAL(rejected()), this, SLOT(regrab()));
    cwMessage->hide();
    radios = new RadioDialog(settings,cat, this);
    connect(radios, SIGNAL(accepted()), this, SLOT(regrab()));
    connect(radios, SIGNAL(rejected()), this, SLOT(regrab()));
    radios->hide();
    winkeyDialog = new WinkeyDialog(settings,this);
    winkeyDialog->setWinkeyVersionLabel(0);
    connect(winkeyDialog, SIGNAL(accepted()), this, SLOT(regrab()));
    connect(winkeyDialog, SIGNAL(rejected()), this, SLOT(regrab()));
    winkeyDialog->hide();
    directory->setCurrent(dataDirectory);
    sdr = new SDRDialog(settings,this);
    connect(sdr, SIGNAL(accepted()), this, SLOT(regrab()));
    connect(sdr, SIGNAL(rejected()), this, SLOT(regrab()));
    sdr->hide();
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
    connect(actionStation, SIGNAL(triggered()), station, SLOT(show()));
    connect(actionStation, SIGNAL(triggered()), this, SLOT(ungrab()));
    connect(actionRadios, SIGNAL(triggered()), radios, SLOT(show()));
    connect(actionRadios, SIGNAL(triggered()), this, SLOT(ungrab()));
    connect(actionContestOptions, SIGNAL(triggered()), options, SLOT(show()));
    connect(actionContestOptions, SIGNAL(triggered()), this, SLOT(ungrab()));
    connect(actionSDR, SIGNAL(triggered()), sdr, SLOT(show()));
    connect(actionSDR, SIGNAL(triggered()), this, SLOT(ungrab()));
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    /* @todo problem here: with the aboutQt() dialog, there is no way to connect
     to its accepted or rejected signals, so no way to reset the keyboard grab */
    // connect(actionAbout_Qt,SIGNAL(triggered()),this,SLOT(ungrab()));
    connect(actionHelp, SIGNAL(triggered()), this, SLOT(showHelp()));
    connect(actionHelp, SIGNAL(triggered()), this, SLOT(ungrab()));
    connect(lineEditCall1, SIGNAL(textEdited(const QString &)), this, SLOT(prefixCheck1(const QString &)));
    connect(lineEditCall2, SIGNAL(textEdited(const QString &)), this, SLOT(prefixCheck2(const QString &)));
    connect(options, SIGNAL(accepted()), this, SLOT(updateOptions()));
    connect(actionCW_Messages, SIGNAL(triggered()), cwMessage, SLOT(show()));
    initDupeSheet();
    menuWindows->addSeparator();
    bandmapCheckBox[0] = new QCheckBox("Bandmap 1", menuWindows);
    bandmapCheckBox[1] = new QCheckBox("Bandmap 2", menuWindows);
    for (int i = 0; i < NRIG; i++) {
        bandmapCheckAction[i] = new QWidgetAction(menuWindows);
        bandmapCheckAction[i]->setDefaultWidget(bandmapCheckBox[i]);
        menuWindows->addAction(bandmapCheckAction[i]);
    }
    windowBorderCheckBox    = new QCheckBox("Borderless", menuWindows);
    windowBorderCheckAction = new QWidgetAction(menuWindows);
    windowBorderCheckAction->setDefaultWidget(windowBorderCheckBox);
#ifdef Q_OS_LINUX
    // this option only works on Linux/X11
    menuWindows->addAction(windowBorderCheckAction);
#endif
    grabCheckBox = new QCheckBox("Grab keyboard", menuWindows);
    grabCheckBox->setCheckState(Qt::Unchecked);
    grabAction = new QWidgetAction(menuWindows);
    grabAction->setDefaultWidget(grabCheckBox);
    menuWindows->addAction(grabAction);
    connect(bandmapCheckBox[0], SIGNAL(stateChanged(int)), this, SLOT(showBandmap1(int)));
    connect(bandmapCheckBox[1], SIGNAL(stateChanged(int)), this, SLOT(showBandmap2(int)));
    connect(windowBorderCheckBox, SIGNAL(toggled(bool)), this, SLOT(windowBorders(bool)));
    connect(grabCheckBox, SIGNAL(toggled(bool)), this, SLOT(setGrab(bool)));
    connect(grabCheckBox, SIGNAL(clicked()), menuWindows, SLOT(close()));
    telnetCheckBox    = new QCheckBox("Telnet", menuWindows);
    telnetCheckAction = new QWidgetAction(menuWindows);
    telnetCheckAction->setDefaultWidget(telnetCheckBox);
    menuWindows->addSeparator();
    menuWindows->addAction(telnetCheckAction);
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
    pport = new ParallelPort(settings);
    connect(pport, SIGNAL(parallelPortError(const QString &)), errorBox, SLOT(showMessage(const QString &)));
    connect(radios, SIGNAL(setParallelPort()), pport, SLOT(initialize()));
    readStationSettings();
    pport->initialize();
    WPMLineEdit->setReadOnly(true);
    // set background of WPM speed edit box to grey
    QPalette palette(wpmLineEditPtr[0]->palette());
    palette.setColor(QPalette::Base, this->palette().color(QPalette::Background));
    for (int i = 0; i < NRIG; i++) {
        wpmLineEditPtr[i]->setText(QString::number(wpm[i]));
        wpmLineEditPtr[i]->setPalette(palette);
    }

    // start serial comm
    winkey = new Winkey(settings,this);
    connect(winkey, SIGNAL(version(int)), winkeyDialog, SLOT(setWinkeyVersionLabel(int)));
    connect(winkeyDialog, SIGNAL(startWinkey()), this, SLOT(startWinkey()));
    connect(winkey->winkeyPort, SIGNAL(readyRead()), winkey, SLOT(receive()));
    connect(radios, SIGNAL(startRadios()), this, SLOT(openRadios()));
    startWinkey();

    openRadios();

    lineEditCall[activeRadio]->setFocus();
    grabWidget = lineEditCall[activeRadio];
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
    resize(settings->value("size", QSize(700, 560)).toSize());
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
    delete mylog;
    if (model) {
        QSqlDatabase::removeDatabase("QSQLITE");
    }
    // stop hamlib thread
    if (catThread.isRunning()) {
        catThread.quit();
        catThread.wait();
    }
    delete cat;
    if (bandmapOn[0]) {
        bandmap[0]->close();
    }
    if (bandmapOn[1]) {
        bandmap[1]->close();
    }
    delete bandmap[0];
    delete bandmap[1];
    delete cabrillo;
    delete radios;
    delete cwMessage;
    delete errorBox;
    delete winkeyDialog;
    delete station;
    delete sdr;
    delete options;
    delete notes;
    delete newContest;
    delete rLabelPtr[0];
    delete rLabelPtr[1];
    delete winkeyLabel;
    delete master;
    delete contest;
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
    delete windowBorderCheckBox;
    delete windowBorderCheckAction;
    delete grabCheckBox;
    delete grabAction;
    delete telnet;
    delete telnetCheckBox;
    delete telnetCheckAction;
    delete directory;
    delete winkey;
    delete pport;
    delete qso[0];
    delete qso[1];
    settings->sync();
    delete settings;
    delete csettings;
}

/*!
   add a new qso to the log
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
    newqso.setValue(SQL_COL_CALL, QVariant(qso->call));
    newqso.setValue(SQL_COL_PTS, QVariant(qso->pts));
    newqso.setValue(SQL_COL_FREQ, QVariant(qso->freq));
    newqso.setValue(SQL_COL_BAND, QVariant(qso->band));
    newqso.setValue(SQL_COL_MODE, QVariant(qso->mode));
    for (int i = 0; i < 4; i++) {
        if (i < contest->nExchange()) {
            switch (i) {
            case 0:
                newqso.setValue(SQL_COL_SNT1, QVariant(qso->snt_exch[0]));
                newqso.setValue(SQL_COL_RCV1, QVariant(qso->rcv_exch[0]));
                break;
            case 1:
                newqso.setValue(SQL_COL_SNT2, QVariant(qso->snt_exch[1]));
                newqso.setValue(SQL_COL_RCV2, QVariant(qso->rcv_exch[1]));
                break;
            case 2:
                newqso.setValue(SQL_COL_SNT3, QVariant(qso->snt_exch[2]));
                newqso.setValue(SQL_COL_RCV3, QVariant(qso->rcv_exch[2]));
                break;
            case 3:
                newqso.setValue(SQL_COL_SNT4, QVariant(qso->snt_exch[3]));
                newqso.setValue(SQL_COL_RCV4, QVariant(qso->rcv_exch[3]));
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


/*! this will have clean-up code
 */
void So2sdr::cleanup()
{
    saveSpots();
    if (bandmapOn[0]) bandmap[0]->closeIQ();
    if (bandmapOn[1]) bandmap[1]->closeIQ();
    quit();
}

/*!
   called when Winkey started or restarted
 */
void So2sdr::startWinkey()
{
    winkey->openWinkey();
    winkey->setSpeed(wpm[activeRadio]);
    winkey->setOutput(activeRadio);
}


/*! regrab keyboard to last widget to have grab
 */
void So2sdr::regrab()
{
    if (grab) {
        grabWidget->setFocus();
        grabWidget->grabKeyboard();
    }
}

/*! change grab status
 */
void So2sdr::setGrab(bool s)
{
    if (s) {
        grab = true;
        grabWidget->setFocus();
        grabWidget->grabKeyboard();
    } else {
        grab = false;
        grabWidget->releaseKeyboard();
    }
}

void So2sdr::ungrab()
{
    if (grab) {
        grabWidget->releaseKeyboard();
    }
}

void So2sdr::quit()
{
    for (int i = 0; i < NRIG; i++) {
        if (bandmapOn[i]) {
            bandmap[i]->close();
            bandmapOn[i]=false;
        }
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
    }

    cat->initialize(settings);

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
    options->setEnabled(false);
    actionCW_Messages->setEnabled(false);
    actionContestOptions->setEnabled(false);
    actionSave->setEnabled(false);
    actionADIF->setEnabled(false);
    actionCabrillo->setEnabled(false);
    grabCheckBox->setEnabled(false);
    actionImport_Cabrillo->setEnabled(false);
    dupesheetCheckBox[0]->setEnabled(false);
    dupesheetCheckBox[1]->setEnabled(false);
    windowBorderCheckBox->setEnabled(false);
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
    }
    cwMessage->setEnabled(true);
    options->setEnabled(true);
    actionCW_Messages->setEnabled(true);
    actionContestOptions->setEnabled(true);
    actionSave->setEnabled(true);
    actionADIF->setEnabled(true);
    actionCabrillo->setEnabled(true);
    grabCheckBox->setEnabled(true);
    actionImport_Cabrillo->setEnabled(true);
    dupesheetCheckBox[0]->setEnabled(true);
    telnetCheckBox->setEnabled(true);
    windowBorderCheckBox->setEnabled(true);

    // dupesheetCheckBox[1]->setEnabled(true); // @todo currently support only 1 dupesheet
    if (sdr->checkBox->isChecked()) bandmapCheckBox[0]->setEnabled(true);
    if (sdr->checkBox_2->isChecked()) bandmapCheckBox[1]->setEnabled(true);
    uiEnabled = true;
    lineEditCall[activeRadio]->setFocus();
    if (grab) {
        lineEditCall[activeRadio]->grabKeyboard();
    }
    grabWidget = lineEditCall[activeRadio];
    connect(sdr->checkBox, SIGNAL(toggled(bool)), bandmapCheckBox[0], SLOT(setEnabled(bool)));
    connect(sdr->checkBox_2, SIGNAL(toggled(bool)), bandmapCheckBox[1], SLOT(setEnabled(bool)));
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
    QFile stdFile(dataDirectory+"/"+fname);
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
    if (callFocus[activeRadio]) {
        lineEditCall[activeRadio]->setFocus();
        if (grab) {
            lineEditCall[activeRadio]->grabKeyboard();
        }
        grabWidget = lineEditCall[activeRadio];
    } else {
        lineEditExchange[activeRadio]->setFocus();
        if (grab) {
            lineEditExchange[activeRadio]->grabKeyboard();
        }
        grabWidget = lineEditExchange[activeRadio];
    }
    startMaster();
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
    options->initialize(csettings);
    cty = new Cty();
    connect(cty, SIGNAL(ctyError(const QString &)), errorBox, SLOT(showMessage(const QString &)));
    if (contest->zoneType() == 0) {
        contest->setMyZone(settings->value(s_cqzone,s_cqzone_def).toInt());
    } else {
        contest->setMyZone(settings->value(s_ituzone,s_ituzone_def).toInt());
    }
    cty->initialize(csettings,dataDirectory,station->lat(), station->lon(), contest->zoneType());

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
    mylog = new log(contest->nExchange(), this);
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
    connect(cabrillo, SIGNAL(accepted()), this, SLOT(exportCabrillo()));
    connect(actionImport_Cabrillo, SIGNAL(triggered()), this, SLOT(importCabrillo()));
    nrSent = model->rowCount() + 1;
    updateNrDisplay();
    updateBreakdown();
    updateMults(activeRadio);
    clearWorked(0);
    clearWorked(1);
    enableUI();
    So2sdrStatusBar->showMessage("Read " + fileName, 3000);
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
        contest->setDataDirectory(dataDirectory);
        for (int i = 0; i < NRIG; i++) {
            lineEditCall[i]->setEnabled(true);
            lineEditExchange[i]->setEnabled(true);
        }
        lineEditCall[activeRadio]->setFocus();
        if (grab) {
            lineEditCall[activeRadio]->grabKeyboard();
        }
        grabWidget = lineEditCall[activeRadio];
    }
}

/*!
   contains additional initializations for specific cases

 */
void So2sdr::selectContest2()
{
    // for several ARRL contests need to know whether station is US/VE or DX
    bool isDx;
    Qso  tmp(1);
    tmp.call = settings->value(s_call,s_call_def).toString().toAscii();
    bool b;
    int  pfx = cty->idPfx(&tmp, b);
    if (pfx == usveIndx[0] || pfx == usveIndx[1]) {
        isDx = false;
    } else {
        isDx = true;
    }
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
        directory->setCurrent(dataDirectory);
        QString filename=csettings->value(c_masterfile,c_masterfile_def).toString();
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly)) {
            master->initialize(file);
        } else {
            errorBox->showMessage("ERROR: can't open file " + filename);
        }
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
 */
void So2sdr::importCabrillo()
{
    // if currently have qsos logged, ask whether to delete them or not
    int n=0;
    for (int i=0;i<N_BANDS;i++) n+=nqso[i];
    // search for files in directory set by contestDirectory
    directory->setCurrent(contestDirectory);

    // get filename
    QString CabFile = QFileDialog::getOpenFileName(this, tr("Import Cabrillo log"), contestDirectory, tr("Cabrillo Files (*.*)"));
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

    QDataStream s(&file);
    int         cnt = 0;
    progress.setValue(0);
    for (int i = 0; i < N_BANDS; i++) nqso[i] = 0;
    Qso *qso;
    qso = new Qso(contest->nExchange());
    while (!file.atEnd() && !progress.wasCanceled()) {
        QString buffer;
        buffer = file.readLine();
        buffer = buffer.trimmed();
        if (!buffer.contains("QSO:") && !buffer.contains("qso:")) continue;  // ignore header data
        QStringList field = buffer.split(" ", QString::SkipEmptyParts);
        int         nf    = field.size();

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

        // Field1 = frequency in KHz
        int f = field[1].toInt() * 1000;
        newqso.setValue(SQL_COL_FREQ, QVariant(f));

        int b = getBand(f);
        newqso.setValue(SQL_COL_BAND, QVariant(b));
        qso->band = b;

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
        qso->mode = (rmode_t) m;
        newqso.setValue(SQL_COL_MODE, QVariant(m));
        cnt++;
        newqso.setValue(SQL_COL_NR, QVariant(cnt));

        // Field3 = date
        QDateTime time;
        time.setTimeSpec(Qt::UTC);
        int       y = field[3].mid(0, 4).toInt();
        m = field[3].mid(5, 2).toInt();
        int       d = field[3].mid(8, 2).toInt();
        time.setDate(QDate(y, m, d));
        newqso.setValue(SQL_COL_DATE, QVariant(time.toString("MMddyyyy").toAscii()));

        // Field4=time
        newqso.setValue(SQL_COL_TIME, QVariant(field[4]));

        // Field5=station call. ignore this

        // Field6+
        // next fields are sent exchange
        int i, j;
        for (i = 6, j = 0; i < (6 + contest->nExchange()); i++, j++) {
            switch (j) {
            case 0:
                newqso.setValue(SQL_COL_SNT1, QVariant(field.at(i).toUpper()));
                qso->snt_exch[0]=field.at(i).toAscii().toUpper();
                break;
            case 1:
                newqso.setValue(SQL_COL_SNT2, QVariant(field.at(i).toUpper()));
                qso->snt_exch[1]=field.at(i).toAscii().toUpper();
                break;
            case 2:
                newqso.setValue(SQL_COL_SNT3, QVariant(field.at(i).toUpper()));
                qso->snt_exch[2]=field.at(i).toAscii().toUpper();
                break;
            case 3:
                newqso.setValue(SQL_COL_SNT4, QVariant(field.at(i).toUpper()));
                qso->snt_exch[3]=field.at(i).toAscii().toUpper();
                break;
            }
        }

        // next field=call worked
        newqso.setValue(SQL_COL_CALL, QVariant(field.at(6 + contest->nExchange()).toUpper()));
        qso->call = field.at(6 + contest->nExchange()).toAscii().toUpper();
        bool bb;
        qso->country = cty->idPfx(qso, bb);

        // next received report
        qso->exch.clear();
        for (i = 7 + contest->nExchange(), j = 0; i < (7 + 2 * contest->nExchange()); i++, j++) {
            // some fields may be empty (flaw in Cabrillo spec?)
            if (i >= nf) {
                continue;
            }
            switch (j) {
            case 0:
                newqso.setValue(SQL_COL_RCV1, QVariant(field.at(i).toUpper()));
                qso->exch = qso->exch + field.at(i).toAscii().toUpper();
                qso->rcv_exch[0]=field.at(i).toAscii().toUpper();
                break;
            case 1:
                newqso.setValue(SQL_COL_RCV2, QVariant(field.at(i).toUpper()));
                qso->exch = qso->exch + " " + field.at(i).toAscii().toUpper();
                qso->rcv_exch[1]=field.at(i).toAscii().toUpper();
                break;
            case 2:
                newqso.setValue(SQL_COL_RCV3, QVariant(field.at(i).toUpper()));
                qso->exch = qso->exch + " " + field.at(i).toAscii().toUpper();
                qso->rcv_exch[2]=field.at(i).toAscii().toUpper();
                break;
            case 3:
                newqso.setValue(SQL_COL_RCV4, QVariant(field.at(i).toUpper()));
                qso->exch = qso->exch + " " + field.at(i).toAscii().toUpper();
                qso->rcv_exch[3]=field.at(i).toAscii().toUpper();
                break;
            }
        }
        qso->dupe = mylog->isDupe(qso, contest->dupeCheckingByBand(), false);
        if (contest->validateExchange(qso)) {
            contest->addQso(qso);
            if (!qso->dupe) nqso[qso->band]++;
        } else {
            // validate failed: flag as a dupe with zero points
            qso->dupe=true;
            qso->pts=0;
            qso->mult[0]=-1;
            qso->mult[1]=-1;
            qso->newmult[0]=-1;
            qso->newmult[1]=-1;
            contest->addQso(qso);
        }
        newqso.setValue(SQL_COL_PTS, QVariant(qso->pts));
        newqso.setValue(SQL_COL_VALID, QVariant(true)); // set to valid
        model->insertRecord(-1, newqso);
        model->submitAll();
        progress.setValue(cnt);
    }
    while (model->canFetchMore()) {
        model->fetchMore();
    }
    LogTableView->scrollToBottom();

    updateBreakdown();
    updateMults(activeRadio);
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
}


/*!
   initialize log view
 */
void So2sdr::initLogView()
{
    LogTableView->setShowGrid(true);
    LogTableView->horizontalHeader()->hide();
    LogTableView->verticalHeader()->hide();
    LogTableView->verticalHeader()->setDefaultSectionSize(16);

    model = new tableModel(this,*mylog->db);
    model->setTable("log");
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->select();
    connect(model, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(logEdited(QModelIndex, QModelIndex)));
    while (model->canFetchMore()) {
        model->fetchMore();
    }
    LogTableView->setModel(model);
    LogTableView->setColumnWidth(SQL_COL_NR, 42); // NR
    LogTableView->setColumnWidth(SQL_COL_TIME, 43); // UTC
    LogTableView->setColumnWidth(SQL_COL_FREQ, 52); // FREQ
    LogTableView->setColumnWidth(SQL_COL_MODE, 35); // MODE
    LogTableView->setColumnWidth(SQL_COL_CALL, 67); // CALL
    LogTableView->setColumnWidth(SQL_COL_VALID, 20); // valid
    LogTableView->setItemDelegate(new logDelegate(this,contest,&logSearchFlag,&searchList));
    for (int i = 0; i < SQL_N_COL; i++) {
        LogTableView->setColumnHidden(i, true);
    }
    LogTableView->setDragEnabled(false);

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
    LogTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    LogTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    LogTableView->scrollToBottom();
    LogTableView->show();
}

/*!
  subclass of QSqlTableModel for log data
  */
tableModel::tableModel(QObject * parent, QSqlDatabase db) : QSqlTableModel(parent,db)
{

}

/*!
  returns appropriate flags for each log column
  */
Qt::ItemFlags tableModel::flags ( const QModelIndex & index ) const
{
    Qt::ItemFlags f;
    f=Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    // set flags appropriate for each column
    // this defines which columns are editable
    switch (index.column()) {
    case SQL_COL_CALL:
    case SQL_COL_TIME:
    case SQL_COL_SNT1:
    case SQL_COL_SNT2:
    case SQL_COL_SNT3:
    case SQL_COL_SNT4:
    case SQL_COL_RCV1:
    case SQL_COL_RCV2:
    case SQL_COL_RCV3:
    case SQL_COL_RCV4:
        f=f | Qt::ItemIsEditable;
        break;
    case SQL_COL_VALID:
        f=f | Qt::ItemIsUserCheckable;
        break;
    }
    return f;
}

/*!
  only SQL_COL_VALID needs a special value here: return a Qt::CheckState
  */
QVariant tableModel::data( const QModelIndex& index, int role ) const
{
    if (index.column()==SQL_COL_VALID && role==Qt::CheckStateRole) {
        Qt::CheckState state;
        if (index.data().toBool()) {
            state=Qt::Checked;
        } else {
            state=Qt::Unchecked;
        }
        return(QVariant(state));
    }
    return QSqlTableModel::data(index,role);
}

/*!
  only SQL_COL_VALID is a special case: translate CheckState into integer 0/1
  */
bool tableModel::setData( const QModelIndex& index, const QVariant&value, int role )
{
    if (index.column()==SQL_COL_VALID && role == Qt::CheckStateRole ) {
        QVariant newValue;
        if (value.toBool()) {
            newValue=QVariant(true);
        } else {
            newValue=QVariant(false);
        }
        return QSqlTableModel::setData(index,newValue,Qt::EditRole);
    }
    return QSqlTableModel::setData(index,value,role);
}

/*! logDelegate:

   Controls display of sent exchange, received exchange, and callsign in displayed log. Changes color
   of entries that are new multipliers

   needs pointer to contest object to get score/multiplier information
 */
logDelegate::logDelegate(QObject *parent, const Contest *c, bool *e, QList<int> *l) : QStyledItemDelegate(parent)
{
    contest = c;
    logSearchFlag = e;
    searchList = l;
}

/*!
  creates editors for each column.

  certain columns (id #, freq, qso pts) are not editable, return NULL in these cases
  other columns use a QLineEdit restricted to upper case
  */
QWidget* logDelegate::createEditor ( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)


    QLineEdit *le=new QLineEdit(parent);
    le->setFocusPolicy(Qt::StrongFocus);
    if (index.column()==SQL_COL_TIME) {
        // validator for time column
        le->setValidator(new TimeValidator(le));
    } else {
        // edit in upper case
        le->setValidator(new UpperValidator(le));
    }
    return(le);
}

/*! paints data from log into exchange columns on screen.

   Columns that are a new multiplier are highlighted in red.
   Use the points from contest->score rather than in the SQL log
 */
void logDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // for qso valid column, use default implementation
    if (index.column()==SQL_COL_VALID) {
        return QStyledItemDelegate::paint(painter,option,index);
    }
    // get the real row for this qso. When a log search is performed, index.row() gives the
    // row within the restricted filter, and not the true row. The true row is needed to
    // display newmult and valid status
    int realRow;
    if (*logSearchFlag) {
        realRow=(*searchList).at(index.row());
    } else {
        realRow=index.row();
    }

    QStyleOptionViewItemV4 opt = option;
    initStyleOption(&opt, index);
    QString s         = index.model()->data(index).toString();

    // display frequency in KHz
    // (this is the reason editing the frequency column is a problem)
    if (index.column()==SQL_COL_FREQ) {
        int f_khz=index.model()->data(index).toInt();
        f_khz = qRound(f_khz / 1000.0);
        s = QString::number(f_khz, 10);
    }

    if (index.column() == SQL_COL_MODE) {
        rmode_t m = (rmode_t)index.model()->data(index).toInt();

        switch(m) {
        case RIG_MODE_CW:
            s = "CW";
            break;
        case RIG_MODE_CWR:
            s = "CWR";
            break;
        case RIG_MODE_LSB:
            s = "LSB";
            break;
        case RIG_MODE_USB:
            s = "USB";
            break;
        case RIG_MODE_FM:
            s = "FM";
            break;
        case RIG_MODE_AM:
            s = "AM";
            break;
        default:
            break;  // Just show the mode number otherwise--fix later, ha!
        }
    }

    // 0 = regular text
    // 1 = red (new multiplier)
    // 2 = grey (dupe)
    int    highlight = 0;

    // get qso points from contest object instead of sql database
    if (index.column() == SQL_COL_PTS) {
        s = QString::number(contest->points(realRow));
    }

    // check to see if a column is a new multiplier and needs highlighting
    if ((contest->newMult(realRow, 0)) == index.column()) {
        highlight = 1;
    }
    if ((contest->newMult(realRow, 1)) == index.column()) {
        highlight = 1;
    }

    // dupes are grayed out
    if (contest->dupe(realRow) || !contest->valid(realRow)) highlight = 2;

    // draw correct background
    opt.text = "";
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    QRect                rect = opt.rect;
    QPalette::ColorGroup cg   = opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
    }

    // set pen color
    if (opt.state & QStyle::State_Selected) {
        painter->setPen(opt.palette.color(cg, QPalette::HighlightedText));
    } else {
        switch (highlight) {
        case 1:
            painter->setPen(Qt::red); // new multiplier: red
            break;
        case 2:
            painter->setPen(Qt::lightGray); // dupes and invalids: light grey
            break;
        default:
            painter->setPen(opt.palette.color(cg, QPalette::Text)); // regular text: black
            break;
        }
    }

    // draw text
    painter->drawText(QRect(rect.left(), rect.top(), rect.width(), rect.height()), opt.displayAlignment, s);
}

void So2sdr::about()
{
    ungrab();
    QMessageBox::about(this, "SO2SDR", "<p>SO2SDR " + Version + " Copyright 2010-2012 R.T. Clay N4OGW</p>"
                       + "<br><hr>Credits:<ul><li>FFTW http://fftw.org"
#ifdef Q_OS_WIN
                       + "<li>hamlib http://www.hamlib.org " + so2sdr_hamlib_version
#endif
#ifdef Q_OS_LINUX
                       + "<li>hamlib http://www.hamlib.org " + hamlib_version
#endif
                       + "<li>qextserialport http://code.google.com/p/qextserialport/"
                       + "<li>QtSolutions_Telnet http://qt.nokia.com/products/appdev/add-on-products/catalog/4/Utilities/qttelnet/"
                       + "<li>PortAudio http://portaudio.com"
                       + "<li>Windows parallel port:  Inpout32.dll http://logix4u.net/"
                       + "<li>MASTER.DTA algorithm, IQ balancing: Alex Shovkoplyas VE3NEA, http://www.dxatlas.com</ul>"
                       + "<hr><p>SO2SDR is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License "
                       + "as published by the Free Software Foundation, either version 3 of the License, or any later version, http://www.gnu.org/licenses/</p>");
    regrab();
}

/*! update rate display. Called by timerId[2] every 60 seconds
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
}

/*!
   process timer events
 */
void So2sdr::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timerId[1]) {
        // every 300 ms
        // radio comm update
        updateRadioFreq();

        // check bandmap
        if (bandmapOn[0] && !keyInProgress) {
            checkSpot(0);
        }
        if (bandmapOn[1] && !keyInProgress) {
            checkSpot(1);
        }
    } else if (event->timerId() == timerId[0]) {
        // clock update; every second
        TimeDisplay->setText(QDateTime::currentDateTimeUtc().toString("MM-dd hh:mm:ss"));
    } else if (event->timerId() == timerId[2]) {
        // these happen every 60 seconds
        updateRate();
        decaySpots();
    } else if (event->timerId() == timerId[3]) {
        // update IQ plot every 10 seconds
        for (int i=0;i<NRIG;i++) {
            if (bandmapOn[i]) {
                bandmap[i]->calc();
            }
        }
    }
}

/*!
     swap freqs between radios
 */
void So2sdr::swapRadios()
{
    int old_f[NRIG];
    old_f[0] = rigFreq[0];
    old_f[1] = rigFreq[1];
    band[1]  = getBand(old_f[0]);
    band[0]  = getBand(old_f[1]);
    if (!cat) return;
    qsy(0, old_f[1], true);
    qsy(1, old_f[0], true);
}

/*!
   Switch active radio

   Alt+R

   switchcw (default=true) controls whether cw is switched or not
 */
void So2sdr::switchRadios(bool switchcw)
{
    activeRadio = activeRadio ^ 1;
    clearR2CQ(activeRadio);
    if (switchcw) {
        winkey->cancelcw();
        winkey->setOutput(activeRadio);
        winkey->setSpeed(wpm[activeRadio]);
    }
    if (callFocus[activeRadio]) {
        lineEditCall[activeRadio]->setFocus();
        if (grab) {
            lineEditCall[activeRadio]->grabKeyboard();
        }
        grabWidget = lineEditCall[activeRadio];
        if (qso[activeRadio]->call.isEmpty()) {
            lineEditCall[activeRadio]->setCursorPosition(0);
        }
    } else {
        lineEditExchange[activeRadio]->setFocus();
        if (grab) {
            lineEditExchange[activeRadio]->grabKeyboard();
        }
        grabWidget = lineEditExchange[activeRadio];
        if (qso[activeRadio]->exch.isEmpty()) {
            lineEditExchange[activeRadio]->setCursorPosition(0);
        }
    }
    pport->switchRadios(activeRadio);
    MasterTextEdit->clear();
    updateRadioFreq();
    updateBreakdown();
    /*!
       @todo Implement option of mults display following non-active radio
     */
    updateMults(activeRadio);
    if (nDupesheet) populateDupesheet();
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

    QScrollBar *sb = MasterTextEdit->verticalScrollBar();
    sb->setValue(sb->minimum());
}

/*!
   log partial checking. Returns true if the partial/call is a dupe
 */
bool So2sdr::logPartial(int nrig, QByteArray partial)
{
    bool                dupe = false;
    QList<QByteArray>   calls;
    QList<unsigned int> worked;
    QString             txt;
    QList<int>          mults1;
    QList<int>          mults2;
    qso[nrig]->worked = 0;
    searchPartial(qso[nrig], partial, calls, worked, mults1, mults2);
    txt           = "<font color=#0000FF>";
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
                txt.append("<font color=#CCCCCC>" + calls.at(i) + " <font color=#000000>");
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

    // check/supercheck partial callsign fragment
    // don't do anything unless at least 2 chars entered
    if (qso[nrig]->call.size() > 1) {
        qso[nrig]->prefill.clear();
        qso[nrig]->dupe = false;
        qso[nrig]->mode = cat->mode(nrig);
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
        int nr = nrig;
        if (activeR2CQ) {
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
    } else {
        if (csettings->value(c_mastermode,c_mastermode_def).toBool()) {
            MasterTextEdit->clear();
        }

        // if Alt-D or 2nd radio CQ is active, display these on the other radio
        int nr = nrig;
        if (altDActive || activeR2CQ) {
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
            tmp = tmp + "<font color=#CCCCCC>" + bandName[i] + "</font> ";
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
    unsigned int worked[2];
    worked[0] = 0;
    worked[1] = 0;
    contest->workedMults(qso[nr], worked);
    for (int ii = 0; ii < MMAX; ii++) {
        QString tmp = multNameLabel[ii]->text();
        if (tmp.isEmpty()) continue;

        if (!csettings->value(c_multsband,c_multsband_def).toBool()) {
            if (worked[ii] == (1 + 2 + 4 + 8 + 16 + 32)) {
                // mult already worked
                tmp = tmp + "<font color=#000000>160 80 40 20 15 10</font>";
            } else {
                // needed
                tmp = tmp + "<font color=#CCCCCC>160 80 40 20 15 10</font>";
            }
        } else {
            for (int i = 0; i < N_BANDS; i++) {
                if (i==N_BANDS_SCORED) break;

                if (worked[ii] & bits[i]) {
                    tmp = tmp + "<font color=#000000>" + bandName[i] + "</font> ";
                } else {
                    tmp = tmp + "<font color=#CCCCCC>" + bandName[i] + "</font> ";
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
            band[nr] = b;
        }

        if (bandmapOn[nr]) {
            bandmap[nr]->setFreq(f, band[nr], spotList[band[nr]]);
            bandmap[nr]->setWindowTitle("Bandmap:" + bandName[band[nr]]);

            // invert spectrum if needed
            bandmap[nr]->setInvert(bandInvert[nr][band[nr]] ^ (cat->mode(nr) == RIG_MODE_CWR));
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

        if (rx.cap(1) == "CWR") {
            emit setRigMode(nr, RIG_MODE_CWR, pb);
        } else if (rx.cap(1) == "CW") {
            emit setRigMode(nr, RIG_MODE_CW, pb);
        } else if (rx.cap(1) == "LSB") {
            emit setRigMode(nr, RIG_MODE_LSB, pb);
        } else if (rx.cap(1) == "USB") {
            emit setRigMode(nr, RIG_MODE_USB, pb);
        } else if (rx.cap(1) == "FM") {
            emit setRigMode(nr, RIG_MODE_FM, pb);
        } else if (rx.cap(1) == "AM") {
            emit setRigMode(nr, RIG_MODE_AM, pb);
        }

    } else {
        // Incomplete frequency or mode entered
        return false;
    }


    qso[activeRadio]->call.clear();
    lineEditCall[activeRadio]->clear();
    lineEditCall[activeRadio]->setFocus();

    if (grab) {
        lineEditCall[activeRadio]->grabKeyboard();
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

    return(true);
}


/*!
   update display of qsos/score
 */
void So2sdr::updateBreakdown()
{
    int n = 0;
    int nm[2];
    nm[0] = 0;
    nm[1] = 0;
    int nb[2];
    nb[0] = 0;
    nb[1] = 0;
    for (int i = 0; i < N_BANDS; i++) {
        n += nqso[i];
        if (i==N_BANDS_SCORED) break;

        qsoLabel[i]->setNum(nqso[i]);
    }
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

    QByteArray tmp = "";
    QByteArray mult;
    bool       needed_band, needed;
    for (int i = 0; i < contest->nMults(multMode); i++) {
        mult = contest->neededMultName(multMode, band[ir], i, needed_band, needed);
        if (excludeMults[multMode].contains(mult)) continue;
        if (csettings->value(c_multsband,c_multsband_def).toBool()) {
            if (needed_band) {
                tmp = tmp + "<font color=#FF0000>" + mult + "</font> "; // red=needed
            } else {
                tmp = tmp + "<font color=#CCCCCC>" + mult + "</font> "; // grey=worked
            }
        } else {
            if (needed) {
                tmp = tmp + "<font color=#FF0000>" + mult + "</font> "; // red=needed
            } else {
                tmp = tmp + "<font color=#CCCCCC>" + mult + "</font> "; // grey=worked
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
    int           tmp[2];

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
            band[i] = b;
        }

        // band change event
        if (contest && tmp[i] != band[i]) {
            if (i == activeRadio) {
                updateMults(i);
                if (nDupesheet) {
                    populateDupesheet();
                }
            }
            if (bandmapOn[i]) {
                bandmap[i]->setWindowTitle("Bandmap:" + bandName[band[i]]);
            }
        }
        if (bandmapOn[i]) {
            //add additional offset if specified by radio (like K3)
            bandmap[i]->setAddOffset(cat->ifFreq(i));
            bandmap[i]->setFreq(rigFreq[i], band[i], spotList[band[i]]);

            // invert spectrum if needed
            bandmap[i]->setInvert(bandInvert[i][band[i]] ^ (cat->mode(i) == RIG_MODE_CWR));
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
   Launch speed up
 */
void So2sdr::launch_speedUp(int mod)
{
    switch (mod) {
    case 0:
        speedUp(activeRadio);
        break;
    case 1:
        speedUp(activeRadio ^ 1);
        break;
    default:
        return;
    }
}

/*!
   Launch speed down
 */
void So2sdr::launch_speedDn(int mod)
{
    switch (mod) {
    case 0:
        speedDn(activeRadio);
        break;
    case 1:
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
    QByteArray out = "";
    wpm[nrig] += 2;
    if (wpm[nrig] > 99) wpm[nrig] = 99;
    if (!(sendingOtherRadio && winkey->isSending() && nrig == activeRadio)) {
        // don't actually change speed if we are sending on other radio
        winkey->setSpeed(wpm[nrig]);
    }
    wpmLineEditPtr[nrig]->setText(QString::number(wpm[nrig]));
}

/*!
   Speed down (page_down)

   decreases WPM by 2
 */
void So2sdr::speedDn(int nrig)
{
    QByteArray out = "";
    wpm[nrig] -= 2;
    if (wpm[nrig] < 5) wpm[nrig] = 5;
    if (!(sendingOtherRadio && winkey->isSending() && nrig == activeRadio)) {
        // don't actually change speed if we are sending on other radio
        winkey->setSpeed(wpm[nrig]);
    }
    wpmLineEditPtr[nrig]->setText(QString::number(wpm[nrig]));
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

    // must enter exactly 2 digits
    if (text.size() < 2) {
        return;
    } else {
        int w = text.toInt(&ok);
        if (ok) {
            wpm[nrig] = w;
            if (!(sendingOtherRadio && winkey->isSending())) {
                // don't actually change speed if we are sending on other radio
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
        }
        grabWidget = lineEditCall[activeRadio];
    } else {
        lineEditExchange[nrig]->setFocus();
        if (grab) {
            lineEditExchange[activeRadio]->grabKeyboard();
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
    if (altDActive != 2) {
        QPalette palette(lineEditCall[i]->palette());
        palette.setColor(QPalette::Base, SP_COLOR);
        lineEditCall[i]->setPalette(palette);
        lineEditExchange[i]->setPalette(palette);
    }
    lineEditExchange[i]->show();
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
void So2sdr::send(QByteArray text)
{
    if (!settings->value(s_winkey_cwon,s_winkey_cwon_def).toBool()) return;

    if (winkey->isSending()) {
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
   expands macros in msg and sends it to Winkey
 */
void So2sdr::expandMacro(QByteArray msg)
{
    int        i0;
    int        i1;
    int        i2;
    int        tmp_wpm = wpm[activeRadio];
    QByteArray out     = "";
    QByteArray val;
    QByteArray txt = QByteArray::number(activeRadio + 1) + ":";

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
                                       "CANCEL" };
    const int        n_token_names = 25;

    /*!
       cw message macros

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
    */
    QPalette palette(lineEditCall[activeRadio ^ 1]->palette());
    palette.setColor(QPalette::Base, ALTD_COLOR);
    sendingOtherRadio = false;
    winkey->setSpeed(tmp_wpm);
    winkey->setOutput(activeRadio);
    bool repeat = false;
    if ((i1 = msg.indexOf("{")) != -1) {
        i0 = 0;
        while (i1 != -1) {
            out.append(msg.mid(i0, i1 - i0));
            txt.append(msg.mid(i0, i1 - i0));
            i2  = msg.indexOf("}", i1);
            val = msg.mid(i1 + 1, i2 - i1 - 1);
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
                        if (nrReserved[activeRadio]) {
                            out.append(QString::number(nrReserved[activeRadio]));
                            txt.append(QString::number(nrReserved[activeRadio]));
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
                        winkey->setSpeed(wpm[activeRadio ^ 1]);
                        winkey->setOutput(activeRadio ^ 1);
                        txt = txt.right(txt.size() - 2);
                        txt.prepend(QByteArray::number((activeRadio ^ 1) + 1) + ":");
                        sendingOtherRadio = true;
                        break;
                    case 5: // Radio 2 CQ
                        winkey->setSpeed(wpm[activeRadio ^ 1]);
                        winkey->setOutput(activeRadio ^ 1);
                        txt = txt.right(txt.size() - 2);
                        txt.prepend(QByteArray::number((activeRadio ^ 1) + 1) + ":");
                        setCqMode(activeRadio ^ 1);
                        activeR2CQ = true;
                        lineEditCall[activeRadio ^ 1]->setPalette(palette);
                        lineEditCall[activeRadio ^ 1]->setText("CQCQCQ");
                        lineEditExchange[activeRadio ^ 1]->setPalette(palette);
                        sendingOtherRadio = true;
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
                        out.append(qso[activeRadio]->call);
                        txt.append(qso[activeRadio]->call);
                        break;
                    case 13: // togglestereopin
                        pport->toggleStereoPin();

                        // return immediately to avoid stopping cw
                        return;
                        break;
                    case 14: // cqmode
                        setCqMode(activeRadio);
                        break;
                    case 15: // spmode
                        spMode(activeRadio);
                        break;
                    case 16: // swap_radios
                        swapRadios();
                        break;
                    case 17:  // repeat last
                        repeat = true;
                        break;
                    case 18: // repeat NR
                        int nr;
                        if (lineEditCall[activeRadio]->text().isEmpty()) {
                            // send last logged number
                            nr = mylog->lastNr();
                        } else {
                            if (nrReserved[activeRadio]) {
                                nr = nrReserved[activeRadio];
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
                        cat->clearRIT(activeRadio);
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
                        if (bandmapOn[activeRadio]) {
                            bandmap[activeRadio]->getCQFreq();
                        }
                        break;
                    case 23: // best cq radio2
                        if (bandmapOn[activeRadio ^ 1]) {
                            bandmap[activeRadio ^ 1]->getCQFreq();
                        }

                        // return immediately to avoid stopping cw on current radio
                        return;
                        break;
                    case 24: // cancel speed change
                        out.append(0x1e);
                        break;
                    }
                    break;
                }
            }
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
            send(lastMsg);
            if (!statusBarDupe && lastMsg.size() > 2) So2sdrStatusBar->showMessage(lastMsg.simplified());
        } else {
            send(out);
            if (!statusBarDupe && txt.size() > 2) So2sdrStatusBar->showMessage(txt.simplified());
        }
    } else {
        // no macro present send as-is
        out.append(msg);
        txt.append(msg);
        send(out);
        if (!statusBarDupe && txt.size() > 2) So2sdrStatusBar->showMessage(txt.simplified());
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
    Qso            *tmpqso = new Qso(contest->nExchange());
    QSqlQueryModel m;
    m.setQuery("SELECT * FROM log", *mylog->db);
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
        tmpqso->call    = m.record(i).value("call").toString().toAscii();

        // run prefix check on call: need to check for /MM, etc
        tmpqso->country = cty->idPfx(tmpqso, b);
        tmpqso->exch.clear();
        QByteArray tmp[4];
        tmp[0] = m.record(i).value("rcv1").toString().toAscii();
        tmp[1] = m.record(i).value("rcv2").toString().toAscii();
        tmp[2] = m.record(i).value("rcv3").toString().toAscii();
        tmp[3] = m.record(i).value("rcv4").toString().toAscii();

        for (int j = 0; j < contest->nExchange(); j++) {
            tmpqso->exch = tmpqso->exch + tmp[j] + " ";
        }
        tmpqso->mode = (rmode_t) m.record(i).value("mode").toInt();
        tmpqso->band = m.record(i).value("band").toInt();
        tmpqso->pts  = m.record(i).value("pts").toInt();

        // valid can be changed to ways:
        // 1) when user unchecks checkbox
        // 2) if program can't parse the exchange
        //
        // first check for user changing check status
        bool userValid=m.record(i).value("valid").toBool();

        // next check exchange
        bool exchValid=contest->validateExchange(tmpqso);
        tmpqso->valid=userValid & exchValid;

        // dupe check
        // qsos marked invalid are excluded from log and dupe check
        tmpqso->dupe = false;
        if (tmpqso->valid) {
            if (csettings->value(c_dupemode,c_dupemode_def).toInt()!=NO_DUPE_CHECKING) {
                // can work station on other bands, just check this one
                if (contest->dupeCheckingByBand()) {
                    if (dupes[tmpqso->band].contains(tmpqso->call)) {
                        tmpqso->dupe = true;
                    }
                } else {
                    // qsos count only once on any band
                    for (int j = 0; j < N_BANDS; j++) {
                        if (dupes[j].contains(tmpqso->call)) {
                            tmpqso->dupe = true;
                        }
                    }
                }
                dupes[tmpqso->band].append(tmpqso->call);
            }
            if (!tmpqso->dupe) nqso[tmpqso->band]++;
        } else {
            tmpqso->pts=0;
            tmpqso->mult[0]=-1;
            tmpqso->mult[1]=-1;
            tmpqso->newmult[0]=-1;
            tmpqso->newmult[1]=-1;
        }
        contest->addQso(tmpqso);
    }
    updateBreakdown();
    updateMults(activeRadio);
    delete tmpqso;
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
            qso[nr]->snt_exch[i] = QByteArray::number(nrReserved[activeRadio]);
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
        directory->setCurrent(dataDirectory);
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

/*! check to see if userDirectory exists. If not, give options to
   create it.

   returns true if directory exists or was created; false if program
   should exit.
 */
bool So2sdr::checkUserDirectory()
{
    QDir dir;
    if (dir.exists(userDirectory)) return(true);

    QMessageBox *msg;
    msg = new QMessageBox(this);
    msg->setWindowTitle("Error");
    msg->setText("User data directory " + userDirectory + " does not exist.");
    msg->setInformativeText("Create it?");
    msg->setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msg->setDefaultButton(QMessageBox::Yes);
    int ret = msg->exec();
    switch (ret) {
    case QMessageBox::Yes:

        // create directory
        if (dir.mkdir(userDirectory)) {
            msg->deleteLater();
            return(true);
        } else {
            msg->setText("Could not create directory <" + userDirectory + ">");
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

    logSearchFlag=true;
    So2sdrStatusBar->showMessage("Searching log for \""+searchFrag+"\"");
    model->setFilter("CALL LIKE '%"+searchFrag+"%'");
    model->select();
    while (model->canFetchMore()) {
        model->fetchMore();
    }
    // save a list of rows found by the search
    searchList.clear();
    for (int i=0;i<model->rowCount();i++) {
        searchList.append(model->record(i).value(SQL_COL_NR).toInt()-1);
    }
    LogTableView->scrollToBottom();
    if (model->rowCount()==0) {
        So2sdrStatusBar->showMessage("Searching log for \""+searchFrag+"\" : NOT FOUND");
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
    m.setQuery("SELECT * FROM log WHERE VALID<>0 AND (CALL LIKE'%" + part + "%' )", *mylog->db);
    while (m.canFetchMore()) {
        m.fetchMore();
    }
    for (int i = 0; i < m.rowCount(); i++) {
        QByteArray tmp = m.record(i).value(SQL_COL_CALL).toString().toAscii();
        //
        /* @todo  the following could probably be simplified  using SQL commands... */
        //
        // see if this call is already in list
        int j;
        if ((j = calls.indexOf(tmp)) != -1) {
            // add this band
            worked[j] = worked[j] | bits[m.record(i).value(SQL_COL_BAND).toInt()];
        } else {
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
    directory->setCurrent(dataDirectory);
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
    directory->setCurrent(dataDirectory);
    QFile file("contest_list.dat");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug("can't open %s/contest_list.dat", dataDirectory.toAscii().data());
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
    newContest    = 0;
    notes         = 0;
    options       = 0;
    radios        = 0;
    station       = 0;
    sdr           = 0;
    winkey        = 0;
    winkeyDialog  = 0;
    model         = 0;
    dupesheet[0] = 0;
    dupesheet[1] = 0;
    bandmap[0]   = 0;
    bandmap[1]   = 0;
    dupesheetCheckBox[0] = 0;
    dupesheetCheckBox[1] = 0;
    dupesheetCheckAction[0] = 0;
    dupesheetCheckAction[1] = 0;
    bandmapCheckBox[0] = 0;
    bandmapCheckBox[1] = 0;
    grabCheckBox = 0;
    telnetCheckBox = 0;
    windowBorderCheckBox = 0;
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
    aboutActive    = false;
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
    for (int i = 0; i < 4; i++) timerId[i] = 0;
    ratePtr = 0;
    for (int i = 0; i < 60; i++) rateCount[i] = 0;
    nDupesheet   = 0;
    bandmapOn[0] = false;
    bandmapOn[1] = false;
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
    sendingOtherRadio  = false;
    grab               = false;
    keyInProgress=false;
}
