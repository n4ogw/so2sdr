/*! Copyright 2010-2018 R. Torsten Clay N4OGW

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

#include <QByteArray>
#include <QPixmap>
#include <QProgressDialog>
#include <QSqlRecord>
#include <QString>
#include <QThread>
#include <QTime>

#include "ui_so2sdr.h"
#include "bandmapentry.h"
#include "utils.h"

class BandmapInterface;
class CabrilloDialog;
class ContestOptionsDialog;
class CWMessageDialog;
class DupeSheet;
class FileDownloader;
class HelpDialog;
class History;
class Log;
class Master;
class NewDialog;
class NoteDialog;
class QTimerEevent;
class QSettings;
class QErrorMessage;
class QProcess;
class Qso;
class QWidget;
class RadioDialog;
class RigSerial;
class SettingsDialog;
class SDRDialog;
class So2r;
class SSBMessageDialog;
class StationDialog;
class Telnet;
class UDPReader;
class Winkey;
class WinkeyDialog;

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
    void addSpot(QByteArray call, double f);
    void addSpot(QByteArray call, double f, bool d);
    void bandChange(int nr,int band);
    void expandMacro(QByteArray msg, bool stopcw = true);
    void removeSpot(QByteArray call, int band);
    void removeSpotFreq(double f, int band);
    void rescore();
    void setEntryFocus();
    void settingsUpdate();
    void showMessage(QString);
    void stationUpdate();
    void startWinkey();
    void updateOffTime();
    void updateSpotlistEdit(QSqlRecord origRecord, QSqlRecord r);

signals:
    void contestReady();
    void qsyExact1(double);
    void qsyExact2(double);
    void setRigMode1(rmode_t, pbwidth_t);
    void setRigMode2(rmode_t, pbwidth_t);

private slots:
    void about();
    void checkCtyVersion();
    void cleanup();
    void clearEditSelection(QWidget *);
    void enterCWSpeed(int nrig, const QString & text);
    void exchCheck1(const QString &exch);
    void exchCheck2(const QString &exch);
    void exportADIF();
    void exportCabrillo();
    void importCabrillo();
    void launch_WPMDialog();
    void launch_enterCWSpeed(const QString &text);
    void logWsjtx(Qso *qso);
    void openFile();
    void openRadios();
    void prefixCheck1(const QString &call);
    void prefixCheck2(const QString &call);
    void quit();
    void regrab();
    void screenShot();
    void send(QByteArray text, bool stopcw = true);
    void sendCalls1(bool);
    void sendCalls2(bool);
    void setGrab(bool);
    void setSummaryGroupBoxTitle();
    void setupNewContest(int result);
    void showBandmap1(bool);
    void showBandmap2(bool);
    void showCabrillo();
    void showDupesheet1(bool checkboxState);
    void showDupesheet2(bool checkboxState);
    void showHelp();
    void showRecordingStatus(bool);
    void showTelnet(bool checkboxState);
    void speedDn(int nrig);
    void speedUp(int nrig);
    void startLogEdit();
    void startMaster();
    void switchAudio(int r);
    void switchMultMode();
    void switchRadios(bool switchcw = true);
    void switchTransmit(int r, int CWspeed = 0);
    void toggleStereo();
    void ungrab();
    void updateCty();
    void updateHistory();
    void updateOptions();
    void updateRadioFreq();

protected:
    void timerEvent(QTimerEvent *event);
    bool eventFilter(QObject*, QEvent* e);
    void closeEvent(QCloseEvent *event);

private:
    bool                 activeR2CQ;
    bool                 autoCQMode;
    bool                 autoCQModeWait;
    bool                 autoCQModePause;
    bool                 autoSend;
    bool                 autoSendPause;
    bool                 autoSendTrigger;
    bool                 bandInvert[NRIG][N_BANDS];
    bool                 callFocus[NRIG];
    bool                 callSent[NRIG];
    bool                 cqMode[NRIG];
    bool                 cqQsoInProgress[NRIG];
    bool                 duelingCQMode;
    bool                 duelingCQWait;
    bool                 duelingCQModePause;
    bool                 dupeCheckDone;
    bool                 exchangeSent[NRIG];
    bool                 excMode[NRIG];
    bool                 grab;
    bool                 grabbing;
    bool                 initialized;
    bool                 logSearchFlag;
    bool                 editingExchange[NRIG];
    bool                 sendLongCQ;
    bool                 spotListPopUp[NRIG];
    bool                 statusBarDupe;
    bool                 telnetOn;
    bool                 toggleMode;
    bool                 uiEnabled;
    CabrilloDialog       *cabrillo;
    ContestOptionsDialog *options;
    CWMessageDialog      *cwMessage;
    SSBMessageDialog     *ssbMessage;
    DupeSheet            *dupesheet[NRIG];
    HelpDialog           *help;
    int                  activeRadio;
    int                  activeTxRadio;
    int                  altDActive;
    int                  altDActiveRadio;
    int                  altDOrigMode;
    int                  autoCQRadio;
    int                  multMode;
    int                  nrReserved[NRIG];
    int                  nrSent;
    int                  rateCount[60];
    int                  ratePtr;
    int                  timerId[N_TIMERS];
    int                  wpm[NRIG];
    FileDownloader       *downloader;
    Log                  *log;
    Master               *master;
    ModeTypes             modeTypeShown;
    NewDialog            *newContest;
    NoteDialog           *notes;
    So2r                 *so2r;
    QByteArray           lastMsg;
    QByteArray           origCallEntered[NRIG];
    QErrorMessage        *errorBox;
    QPixmap              iconValid;
    QLabel               *autoCQStatus;
    QLabel               *autoSendStatus;
    QLabel               *bandLabel[6];
    QLabel               *bandQsoLabel[6];
    QLabel               *bandMult1Label[6];
    QLabel               *bandMult2Label[6];
    QLabel               *duelingCQStatus;
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
    QLabel               *offPtr;
    QLabel               *qsoLabel[N_BANDS];
    QLabel               *qsoWorkedLabel[NRIG];
    QLabel               *rLabelPtr[NRIG];
    QLabel               *sunLabelPtr[NRIG];
    QLabel               *toggleStatus;
    QLabel               *validLabel[NRIG];
    QLabel               *winkeyLabel;
    QLineEdit            *lineEditCall[NRIG];
    QLineEdit            *lineEditExchange[NRIG];
    QLineEdit            *wpmLineEditPtr[NRIG];
    QList<BandmapEntry>  spotList[N_BANDS];
    QList<QByteArray>    excludeMults[MMAX];
    QProcess             *scriptProcess;
    Qso                  *qso[NRIG];
    BandmapInterface     *bandmap;
    QSettings            *csettings;
    QSettings            *settings;
    History              *history;
    QProgressDialog      progress;
    QString              contestDirectory;
    QString              fileName;
    QString              settingsFile;
    QString              autoSendCall;
    QThread              catThread[NRIG];
    QTime                cqTimer;
    QWidget              *grabWidget;
    RadioDialog          *radios;
    RigSerial            *cat[NRIG];
    SDRDialog            *sdr;
    SettingsDialog       *progsettings;
    StationDialog        *station;
    Telnet               *telnet;
    uiSize               sizes;
    UDPReader            *wsjtxUDP;
    WinkeyDialog         *winkeyDialog;
    Winkey               *winkey;

    void bandmapSetFreq(double f,int nr);
    void addQso(Qso *qso);
    void altd();
    void altDEnter(int level, Qt::KeyboardModifiers mod);
    void backSlash();
    void keyCtrlDn();
    void keyCtrlUp();
    void checkSpot(int nr);
    bool checkUserDirectory();
    void clearDisplays(int nr);
    void clearLogSearch();
    void clearWorked(int i);
    void clearR2CQ(int nr);
    void controlE();
    void setCqMode(int i);
    void decaySpots();
    void disableUI();
    void down();
    void enableUI();
    void enter(Qt::KeyboardModifiers);
    bool enterFreqOrMode();
    void esc(Qt::KeyboardModifiers);
    void exchCheck(int nr,const QString &exch);
    void fillSentExch(Qso *qso,int nr);
    void initDupeSheet();
    void initLogView();
    void initPointers();
    void initVariables();
    bool isaSpot(double f, int band);
    void launch_speedUp(Qt::KeyboardModifiers);
    void launch_speedDn(Qt::KeyboardModifiers);
    void loadSpots();
    bool logPartial(int nrig, QByteArray partial);
    void logSearch();
    void markDupe(int nrig);
    int nDupesheet() const;
    void populateDupesheet();
    void prefixCheck(int nrig, const QString &call);
    void prefillExch(int nr);
    void qsy(int nrig, double &freq, bool exact);
    void readStationSettings();
    void readExcludeMults();
    void runScript(QByteArray cmd);
    void saveSpots();
    void searchPartial(Qso *qso, QByteArray part, QList<QByteArray>& calls, QList<unsigned int>& worked, QList<int>& mult1, QList<int>& mult2);
    void selectContest(QByteArray name);
    void selectContest2();
    void sendCalls(int);
    void sendFunc(int i, Qt::KeyboardModifiers mode);
    void setDupeColor(int nr,bool dupe);
    void setUiSize();
    bool setupContest();
    void showDupesheet(int nr, bool checkboxState);
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
    void updateDupesheet(QByteArray call,int nr);
    void updateMults(int ir, int bandOverride=-1);
    void updateNrDisplay();
    void updateRate();
    void updateWorkedDisplay(int nr,unsigned int worked);
    void updateWorkedMult(int nr);
    void writeContestSettings();
    void writeNote();
    void writeStationSettings();
    void autoCQ();
    void duelingCQ();
    void duelingCQActivate(bool state = false);
    void autoSendExch();
    void autoSendExch_exch();
    void autoSendActivate(bool state = false);
    void autoCQdelay(bool incr = true);
    void autoCQActivate(bool state = false);
};

#endif
