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
#ifndef SO2SDR_H
#define SO2SDR_H

#include "ui_so2sdr.h"
#include "cabrillodialog.h"
#include "winkeydialog.h"
#include "cwmessagedialog.h"
#include "stationdialog.h"
#include "radiodialog.h"
#include "sdrdialog.h"
#include "contestoptdialog.h"
#include "newcontestdialog.h"
#include "notedialog.h"
#include "dupesheet.h"
#include "bandmap.h"
#include "log.h"
#include "logedit.h"
#include "cty.h"
#include "helpdialog.h"
#include "serial.h"
#include "contest.h"
#include "contest_arrldx.h"
#include "contest_arrl10.h"
#include "contest_arrl160.h"
#include "contest_cq160.h"
#include "contest_cqww.h"
#include "contest_cwops.h"
#include "contest_fd.h"
#include "contest_dxped.h"
#include "contest_iaru.h"
#include "contest_naqp.h"
#include "contest_sprint.h"
#include "contest_stew.h"
#include "contest_sweepstakes.h"
#include "contest_wpx.h"
#include "master.h"
#include "qso.h"
#include "telnet.h"
#include "utils.h"
#include "winkey.h"
#include "detailededit.h"
#include "mytableview.h"
#ifdef Q_OS_WIN
#include "win_pp.h"
#endif
#ifdef Q_OS_LINUX
#include "linux_pp.h"
#endif

class QDir;
class QString;
class QByteArray;
class QThread;
class QTimer;
class QCheckBox;
class QProgessDialog;
class QSettings;
class QWidgetAction;
class QErrorMessage;

/*!
   Main window
 */
class So2sdr : public QMainWindow, public Ui::So2sdr
{
Q_OBJECT

public:
    So2sdr(QStringList args, QWidget *parent = 0);
    ~So2sdr();
    bool so2sdrOk() const;

public slots:
    void addSpot(QByteArray call, int f);
    void addSpot(QByteArray call, int f, bool d);
    void qsyEvent(int nr, int hz);
    void removeSpot(QByteArray call, int band);
    void removeSpotFreq(int f, int band);
    void rescore();
    void setBandmapTxStatus(bool, int);
    void showMessage(QString);
    void stationUpdate();
    void startWinkey();
    void updateRecord(QSqlRecord);

signals:
    void qsyExact(int, int);
    void setRigMode(int, rmode_t, pbwidth_t);

private slots:
    void about();
    void cleanup();
    void clearEditSelection(QWidget *);
    void detailEditDone();
    void editLogDetail(QModelIndex);
    void enterCWSpeed(int nrig, const QString & text);
    void exportADIF();
    void exportCabrillo();
    void importCabrillo();
    void launch_WPMDialog();
    void launch_enterCWSpeed(const QString &text);
    void logEdited(const QModelIndex & topLeft, const QModelIndex & bottomRight);
    void mouseQSYevent(int nr, int hz);
    void openFile();
    void openRadios();
    void prefixCheck1(const QString &call);
    void prefixCheck2(const QString &call);
    void quit();
    void regrab();
    void send(QByteArray text);
    void setGrab(bool);
    void setupNewContest(int result);
    void showBandmap1(int checkboxState);
    void showBandmap2(int checkboxState);
    void showCabrillo();
    void showDupesheet1(int checkboxState);
    void showDupesheet2(int checkboxState);
    void showHelp();
    void showTelnet(int checkboxState);
    void speedDn(int nrig);
    void speedUp(int nrig);
    void startLogEdit();
    void startMaster();
    void switchMultMode();
    void switchRadios(bool switchcw = true);
    void ungrab();
    void updateOptions();
    void updateRadioFreq();
    void windowBorders(bool t);

protected:
    void timerEvent(QTimerEvent *event);
    bool eventFilter(QObject*, QEvent* e);
    void closeEvent(QCloseEvent *event);

private:
    Bandmap              *bandmap[NRIG];
    bool                 aboutActive;
    bool                 activeR2CQ;
    bool                 bandInvert[NRIG][N_BANDS];
    bool                 bandmapOn[NRIG];
    bool                 callFocus[NRIG];
    bool                 callSent[NRIG];
    bool                 cqMode[NRIG];
    bool                 cqQsoInProgress[NRIG];
    bool                 dupeCheckDone;
    bool                 exchangeSent[NRIG];
    bool                 excMode[NRIG];
    bool                 grab;
    bool                 initialized;
	bool                 keyInProgress;
    bool                 logSearchFlag;
    bool                 sendingOtherRadio;
    bool                 spotListPopUp[NRIG];
    bool                 statusBarDupe;
    bool                 telnetOn;
    bool                 uiEnabled;
    CabrilloDialog       *cabrillo;
    Contest              * contest;
    ContestOptionsDialog *options;
    Cty                  *cty;
    CWMessageDialog      *cwMessage;
    DetailedEdit         *detail;
    DupeSheet            *dupesheet[NRIG];
    HelpDialog           *help;
    int                  activeRadio;
    int                  altDActive;
    int                  altDActiveRadio;
    int                  altDOrigMode;
    int                  band[NRIG];
    int                  multMode;
    int                  nDupesheet;
    int                  nqso[N_BANDS];
    int                  nrReserved[NRIG];
    int                  nrSent;
    int                  rateCount[60];
    int                  ratePtr;
    int                  rigFreq[NRIG];
    int                  timerId[N_TIMERS];
    int                  usveIndx[MMAX];
    int                  wpm[NRIG];
    log                  *mylog;
    logDelegate          *logdel;
    Master               *master;
    ModeTypes             modeTypeShown;
    NewDialog            *newContest;
    NoteDialog           *notes;
    ParallelPort         *pport;
    QByteArray           lastMsg;
    QByteArray           origCallEntered[NRIG];
    QCheckBox            *bandmapCheckBox[NRIG];
    QCheckBox            *dupesheetCheckBox[NRIG];
    QCheckBox            *grabCheckBox;
    QCheckBox            *telnetCheckBox;
    QCheckBox            *windowBorderCheckBox;
    QDir                 *directory;
    QErrorMessage        *errorBox;
    QLabel               *freqDisplayPtr[NRIG];
    QLabel               *grabLabel;
    QLabel               *labelBearing[NRIG];
    QLabel               *labelLPBearing[NRIG];
    QLabel               *labelCountry[NRIG];
    QLabel               *modeDisplayPtr[NRIG];
    QLabel               *multLabel[2][N_BANDS];
    QLabel               *multNameLabel[MMAX];
    QLabel               *multTotal[2];
    QLabel               *multWorkedLabel[2][MMAX];
    QLabel               *numLabelPtr[NRIG];
    QLabel               *qsoLabel[N_BANDS];
    QLabel               *qsoWorkedLabel[NRIG];
    QLabel               *rLabelPtr[NRIG];
    QLabel               *sunLabelPtr[NRIG];
    QLabel               *winkeyLabel;
    QLineEdit            *lineEditCall[NRIG];
    QLineEdit            *lineEditExchange[NRIG];
    QLineEdit            *wpmLineEditPtr[NRIG];
    QList<BandmapEntry>  spotList[N_BANDS];
    QList<char>          *dupeCallsKey[NRIG];
    QList<QByteArray>    configFiles;
    QList<QByteArray>    *dupeCalls[NRIG];
    QList<QByteArray>    excludeMults[MMAX];
    QList<int>           searchList;
    Qso                  *qso[NRIG];
    tableModel           *model;
    QSettings            *csettings;
    QSettings            *settings;
    QString              contestDirectory;
    QString              dataDirectory;
    QString              fileName;
    QString              installDirectory;
    QString              settingsFile;
    QThread              catThread;
    QWidgetAction        *bandmapCheckAction[NRIG];
    QWidgetAction        *dupesheetCheckAction[NRIG];
    QWidgetAction        *grabAction;
    QWidgetAction        *telnetCheckAction;
    QWidgetAction        *windowBorderCheckAction;
    QWidget              *grabWidget;
    RadioDialog          *radios;
    RigSerial            *cat;
    SDRDialog            *sdr;
    StationDialog        *station;
    Telnet               *telnet;
    WinkeyDialog         *winkeyDialog;
    Winkey               *winkey;

    void addQso(const Qso *qso);
    void altd();
    void altDEnter(int level, int mod);
    void backSlash();
    void keyCtrlDn();
    void keyCtrlUp();
    void checkSpot(int nr);
    bool checkUserDirectory();
    void clearLogSearch();
    void clearWorked(int i);
    void clearR2CQ(int nr);
    void controlE();
    void setCqMode(int i);
    void decaySpots();
    void disableUI();
    void down();
    void enableUI();
    void enter(int);
    bool enterFreqOrMode();
    void esc();
    void expandMacro(QByteArray msg);
    void fillSentExch(int nr);
    void initDupeSheet();
    void initLogView();
    void initPointers();
    void initVariables();
    bool isaSpot(int f, int band);
    void launch_speedUp(int);
    void launch_speedDn(int);
    void loadSpots();
    bool logPartial(int nrig, QByteArray partial);
    void logSearch();
    void markDupe(int nrig);
    void populateDupesheet();
    void prefixCheck(int nrig, const QString &call);
    void prefillExch(int nr);
    void qsy(int nrig, int &freq, bool exact);
    bool readContestList();
    void readStationSettings();
    void readExcludeMults();
    void saveSpots();
    void searchPartial(Qso *qso, QByteArray part, QList<QByteArray>& calls, QList<unsigned int>& worked, QList<int>& mult1, QList<int>& mult2);
    void selectContest(QByteArray name);
    void selectContest2();
    void sendFunc(int i, int mode);
    bool setupContest();
    void setDefaultFreq(int nrig);
    void showBandmap(int nr, int checkboxState);
    void showDupesheet(int nr, int checkboxState);
    void spaceAltD();
    void spaceBar();
    void spaceSP(int nrig);
    void spaceSprint();
    void spMode(int i);
    void sprintMode();
    void startTimers();
    void stopTimers();
    void superPartial(QByteArray partial);
    void swapRadios();
    void tab();
    void up();
    void updateBandmapDupes(const Qso *qso);
    void updateBreakdown();
    void updateDupesheet(QByteArray call);
    void updateMults(int ir);
    void updateNrDisplay();
    void updateRate();
    void updateWorkedDisplay(int nr,unsigned int worked);
    void updateWorkedMult(int nr);
    void writeNote();
    void writeStationSettings();
};

#endif
