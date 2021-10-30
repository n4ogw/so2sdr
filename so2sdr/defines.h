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
#ifndef DEFINES_H
#define DEFINES_H
#include <QApplication>
#include <QByteArray>
#include <QColor>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QString>
#include <QtGlobal>
#include <QVariant>
#include "hamlib/rig.h"

// ///////// version ///////////////
const QByteArray Version = "2.5.13";

// //////// colors ////////////////
// all of form (R,G,B)
const QColor CQ_COLOR(255, 255, 255);  // color in CQ mode; white
const QColor SP_COLOR(255, 255, 127);  // color in SP mode; yellow
const QColor DUPE_COLOR(255, 0, 0);  // color for dupe; red
const QColor ALTD_COLOR(255, 0, 255);  // color during alt-D, Sprint SO2R

// //////////// telnet //////////////////
/*!
   Maximum number of characters kept in telnet buffer
 */
const int MAX_TELNET_CHARS=4096;

/////////////// parallel port ////////////
const int defaultParallelPortStereoPin=5;
const int defaultParallelPortAudioPin=4;
const int defaultParallelPortTxPin=3;
#ifdef Q_OS_LINUX
const QString defaultParallelPort = "/dev/parport0";
#endif

// /////////////// winkey //////////////////

/*!
   Winkey parameter structure
 */
typedef struct WinkeyParam
{
    QByteArray device;
    int        WinkeyPotMin;
    int        WinkeyPotMax;
    int        WinkeySidetone;
    int        PaddleMode;
    bool       WinkeySidetonePaddle;
    bool       WinkeyUsePot;
    bool       WinkeyPaddleSwap;
    bool       CTSpace;
    char       padding[4];
} WinkeyParam;
Q_DECLARE_TYPEINFO(WinkeyParam, Q_PRIMITIVE_TYPE);


// /////////// Radio serial communications ///////////////

#ifdef Q_OS_LINUX
const QString defaultSerialPort[3] = {"/dev/ttyS0","/dev/ttyS1","/dev/ttyS2"};
#endif

const int NRIG=2;

// Contest types
typedef enum ContestType {
    Undefined= 0,
    Arrl10_t  = 1,
    Arrl160_t = 2,
    Cq160_t   = 3,
    Cqp_t     = 4,
    Cqww_t    = 5,
    Cwops_t   = 6,
    Dxped_t   = 7,
    Fd_t      = 8,
    Iaru_t    = 9,
    Kqp_t     = 10,
    Naqp_t    = 11,
    Sprint_t  = 12,
    Stew_t    = 13,
    Sweepstakes_t = 14,
    Wpx_t     = 15,
    Arrldx_t  = 16,
    Paqp_t    = 17,
    Msqp_t    = 18,
    JuneVHF_t = 19,
    WWDigi_t  = 20
} ContestType;

// ////////////// Contest/Log/country database

const int MMAX=2;

typedef enum MultTypeDef {
    None        = 0,
    File        = 1,
    ARRLCountry = 2,
    CQCountry   = 3,
    ContNA      = 4,
    ContSA      = 5,
    ContEU      = 6,
    ContAF      = 7,
    ContAS      = 8,
    ContOC      = 9,
    Prefix      = 10,
    CQZone      = 11,
    ITUZone     = 12,
    Special     = 13,
    Uniques     = 14,
    Grids       = 15,
    GridFields  = 16
} MultTypeDef;

/*!
   Continents

 */
typedef enum {
    NA  = 0,
    SA  = 1,
    EU  = 2,
    AF  = 3,
    AS  = 4,
    OC  = 5,
    ALL = 6
} Cont;

/*!
   Country structure

 */
typedef struct Country {
    int               indx;
    int               Zone;
    Cont              Continent;
    float             delta_t;
    int               bearing;
    bool              multipleZones;
    QList<QByteArray> zonePfx;
    QList<int>        zones;
    QString           sun;
    QByteArray        MainPfx;
    QByteArray        name;
} Country;
Q_DECLARE_TYPEINFO(Country, Q_PRIMITIVE_TYPE);

/*!
   Prefix
 */
typedef struct Pfx {
    QByteArray prefix;
    int        CtyIndx;
    int        Zone;
    bool       zoneOverride;
} Pfx;
Q_DECLARE_TYPEINFO(Pfx, Q_PRIMITIVE_TYPE);

/*!
   Callsign exception in CTY database
 */
typedef struct CtyCall {
    QByteArray call;
    int        CtyIndx;
    int        Zone;
    QString    sun;
} CtyCall;
Q_DECLARE_TYPEINFO(CtyCall, Q_PRIMITIVE_TYPE);

/*!
   Defined multiplier structure. Allows alternate names
 */
typedef struct DomMult {
    QByteArray        name;
    bool              hasAltNames;
    bool              isamult;
    Cont              continent;
    QList<QByteArray> alt_names;
} DomMult;
Q_DECLARE_TYPEINFO(DomMult, Q_PRIMITIVE_TYPE);

typedef struct uiSize {
    qreal height;
    qreal width;
    qreal smallHeight;
    qreal smallWidth;
} uiSize;
Q_DECLARE_TYPEINFO(uiSize, Q_PRIMITIVE_TYPE);

// column numbers in SQL log
const int SQL_COL_NR    =  0;     // ID number (SQL primary key)
const int SQL_COL_TIME  =  1;     // time HHMM  (string)
const int SQL_COL_FREQ  =  2;     // freq in Hz (double)
const int SQL_COL_CALL  =  3;     // call (string)
const int SQL_COL_BAND  =  4;     // band (int)
const int SQL_COL_DATE  =  5;     // date MMddyyyy (string)
const int SQL_COL_MODE  =  6;     // mode (int)
const int SQL_COL_ADIF_MODE = 7;  // ADIF mode (string)
const int SQL_COL_MODE_TYPE = 8;  // mode type (int)
const int SQL_COL_SNT1  =  9;     // sent exchange field 1 (string)
const int SQL_COL_SNT2  =  10;    // sent exchange field 2 (string)
const int SQL_COL_SNT3  =  11;    // sent exchange field 3 (string)
const int SQL_COL_SNT4  =  12;    // sent exchange field 4 (string)
const int SQL_COL_RCV1  =  13;    // rcv exchange field 1 (string)
const int SQL_COL_RCV2  =  14;    // rcv exchange field 2 (string)
const int SQL_COL_RCV3  =  15;    // rcv exchange field 3 (string)
const int SQL_COL_RCV4  =  16;    // rcv exchange field 4 (string)
const int SQL_COL_PTS   =  17;    // qso points (int)
const int SQL_COL_VALID =  18;    // valid flag (int) if 0, qso not exported to cabrillo

const int SQL_N_COL     =  19;    // total number of columns


/*!
   Exchange field types

   - 0  general string; no checking done. Will remember from previous qsos. Use for names, etc.
   - 1  RST; does not need to be entered, assumes 599 unless entered
   - 2  Domestic mult; read list from file
   - 3  Zone number; auto-fill exchange based on callsign
   - 4  QSO number; must be + integer
   - 5  Name (string); takes name from station dialog
   - 6  State (string); takes from station dialog
   - 7  ARRL section (string); takes from station dialog
   - 8  Grid square (string); takes from station dialog
   - 9  Number, not qso number
 */
typedef enum FieldTypes {
    General     = 0,
    RST         = 1,
    DMult       = 2,
    Zone        = 3,
    QsoNumber   = 4,
    Name        = 5,
    State       = 6,
    ARRLSection = 7,
    Grid        = 8,
    Number      = 9
} FieldTypes;

const QString FieldTypesNames[10]={"","RST","Mult","Z","#","Name","St","Sec","Grid","#"};

// //////////////// Bandscope defines //////////////////

typedef struct BandmapShared {
    double     freq;
    bool       tx;
} BandmapShared;
Q_DECLARE_TYPEINFO(BandmapShared, Q_PRIMITIVE_TYPE);


/*!
   min tolerance for signal separation (Hz)
   signals + or - this amount will go in the same bin
 */
const int SIG_MIN_FREQ_DIFF=65;

/*!
  min tolerance for callsign separation
  */
const int SIG_MIN_SPOT_DIFF=200;

/*! minimum time in seconds to keep a signal once detected
 */
const double SIG_KEEP_TIME=5.0;

/*! radius of circles marking signals (in pixels)
 */
const int SIG_SYMBOL_RAD=4;

/*! pixel location from left edge of field for signal symbols
 */
const int SIG_SYMBOL_X=5;

/*!  Default timeout of spotted calls (minutes)
 */
const int DEFAULT_SPOT_TIMEOUT=10;

/*! pixel size of font used
 */
const int BANDMAP_FONT_PIX_SIZE=10;

/*!  pixel location from left edge of field for calls in bandmap
 */
const int BANDMAP_CALL_X=15;


// /////// Misc stuff

// number of F-keys
const int N_FUNC=12;

// timers
const int N_TIMERS=4;

// default frequency for various things (all in milliseconds)
const int timerSettings[]={
    1000,  // clock update
    300, // radio/serial update
    100, // auto-CQ, dueling CQ resolution
    30000 // wsjtx call list
};

// delay in ms between queued messages (two keyboard mode)
const int queueDelay=200;

/*! number of minutes in moving average of rate display
 */
const int RATE_AVG_MINUTES=3;

// powers of 2
const unsigned int bits[] = { 1, 2, 4, 8, 16, 32, 64, 128,
                              256, 512, 1024, 2048, 4096, 8192, 16384, 32768,
                              65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608,
                              16777216, 33554432, 67108864, 134217728, 268435456, 536870912, 1073741824, 2147483648};

// number of keys in two keyboard keymap
const int NKEYS=112;

// maximum number of exchange fields
// NOTE: the SQL fields are currently hard-coded at 4, do not change this!
const int MAX_EXCH_FIELDS=4;

// number of dupesheet columns
const int dsColumns=10;

// dupe mode settings
const int STRICT_DUPES=0;     // will not work dupes during CW or S&P
const int WORK_DUPES=1;       // work dupes during CQ but not during S&P
const int NO_DUPE_CHECKING=2; // no dupe checking

// number of bands
const int N_BANDS=28;

const QString bandNames[N_BANDS] = { "160", "80", "40", "20", "15", "10", "60", "30", "17", "12",
                                     "6", "2","1.25m","70cm","33cm","1.2G","2.3G","3.3G","5.6G",
                                     "10G","24G","47G","76G","122G","134G","241G","630m","2200m"};

const int N_BANDS_HF=6;
const int BAND_NONE=-1;
const int BAND160 =0;
const int BAND80 = 1;
const int BAND40 = 2;
const int BAND20 = 3;
const int BAND15 = 4;
const int BAND10 = 5;
const int BAND60 = 6;
const int BAND30 = 7;
const int BAND17 = 8;
const int BAND12 = 9;
const int BAND6 = 10;
const int BAND2 = 11;
const int BAND222= 12;
const int BAND420 = 13;
const int BAND902 = 14;
const int BAND1240 = 15;
const int BAND2300 = 16;
const int BAND3300 = 17;
const int BAND5650 = 18;
const int BAND10000 = 19;
const int BAND24000 = 20;
const int BAND47000 = 21;
const int BAND76000 = 22;
const int BAND122000 = 23;
const int BAND134000 = 24;
const int BAND241000 = 25;
const int BAND630 = 26;
const int BAND2200 = 27;

// hamlib modes : this has to match the modes defined in hamlib rig.h, enum rmode_t
const int nModes=23;
const QString modes[nModes] = { "NONE", "AM",  "CW",  "USB", "LSB", "RTTY", "FM",  "WFM", "CWR", "RTTYR", "AMS",
                            "PKT",  "PKT", "PKT", "USB", "LSB", "FAX",  "SAM", "SAL", "SAH", "DSB", "FM", "PKT" };

// mode types
typedef enum {
    CWType = 0,
    PhoneType = 1,
    DigiType = 2
} ModeTypes;
const int NModeTypes=3;

// mode type names
const QString modeNames[NModeTypes]={"CW","Phone","Digital"};

// maximum number of Cabrillo fields
const int MAX_CAB_FIELDS=7;

//////// QSettings key names and default values ///////

// s_xxx are used in station config file (so2sdr.ini)
// c_xxx are used in contest config file

const QString s_twokeyboard_enable="twokeyboard/enable";
const bool s_twokeyboard_enable_def=false;

const QString s_twokeyboard_device[NRIG]={"twokeyboard/device1","twokeyboard/device2"};
const QString s_twokeyboard_device_def[NRIG]={"",""};

const QString s_twokeyboard_dev1="twokeyboard/device1";
const QString s_twokeyboard_dev1_def="";

const QString s_queuemessages="twokeyboard/queuemessages";
const bool s_queuemessages_def=false;

const QString s_contestdirectory="main/contestdirectory";
const QString s_contestdirectory_def=QDir::homePath();

const QString s_call="station/call";
const QString s_call_def="";

const QString s_name="station/name";
const QString s_name_def="";

const QString s_state="station/state";
const QString s_state_def="";

const QString s_section="station/arrlsection";
const QString s_section_def="";

const QString s_grid="station/grid";
const QString s_grid_def="";

const QString s_cqzone="station/cqzone";
const int s_cqzone_def=0;

const QString s_ituzone="station/ituzone";
const int s_ituzone_def=0;

const QString s_cab_address="cabrillo/address";
const QString s_cab_address_def="";

const QString s_cab_city="cabrillo/city";
const QString s_cab_city_def="";

const QString s_cab_state="cabrillo/state";
const QString s_cab_state_def="";

const QString s_cab_country="cabrillo/country";
const QString s_cab_country_def="";

const QString s_cab_zip="cabrillo/zip";
const QString s_cab_zip_def="";

const QString s_cab_name="cabrillo/name";
const QString s_cab_name_def="";

const QString c_cab_club="cabrillo/club";
const QString c_cab_club_def="";

const QString s_otrsp_enabled[NRIG]={"otrsp/enable1","otrsp/enable2"};
const bool s_otrsp_enabled_def=false;

const QString s_otrsp_device[NRIG]={"otrsp/device1","otrsp/device2"};
const QString s_otrsp_device_def="";

const QString s_otrsp_baud[NRIG]={"otrsp/baud1","otrsp/baud2"};
const int s_otrsp_baud_def=9600;

const QString s_otrsp_databits[NRIG]={"otrsp/databits1","otrsp/databits2"};
const int s_otrsp_databits_def=8;

const QString s_otrsp_parity[NRIG]={"otrsp/parity1","otrsp/parity2"};
const bool s_otrsp_parity_def=false;

const QString s_otrsp_stopbits[NRIG]={"otrsp/stopbits1","otrsp/stopbits2"};
const int s_otrsp_stopbits_def=1;

const QString s_otrsp_focus[NRIG]={"otrsp/focus1","otrsp/focus2"};
const bool s_otrsp_focus_def=true;

const QString s_microham_enabled="microham/enable";
const bool s_microham_enabled_def=false;

const QString s_microham_device="microham/device";
const QString s_microham_device_def="";

const QString s_winkey_device="winkey/device";
const QString s_winkey_device_def="";

const QString s_winkey_cwon="winkey/cwon";
const bool s_winkey_cwon_def=true;

const QString s_winkey_ctspace="winkey/ctspace";
const bool s_winkey_ctspace_def=true;

const QString s_winkey_paddle_mode="winkey/paddlemode";
const int s_winkey_paddle_mode_def=0;

const QString s_winkey_paddle_swap="winkey/paddleswap";
const int s_winkey_paddle_swap_def=false;

const QString s_winkey_potmin="winkey/potmin";
const int s_winkey_potmin_def=15;

const QString s_winkey_potmax="winkey/potmax";
const int s_winkey_potmax_def=50;

const QString s_winkey_sidetone="winkey/sidetone";
const int s_winkey_sidetone_def=7;

const QString s_winkey_usepot="winkey/usepot";
const bool s_winkey_usepot_def=false;

const QString s_winkey_sidetonepaddle="winkey/sidetonepaddle";
const bool s_winkey_sidetonepaddle_def=true;

const QString s_wpm[NRIG]={"winkey/wpm1","winkey/wpm2"};
const int s_wpm_def[NRIG]={35,35};

const QString s_cty_url="main/ctyurl";
const QString s_cty_url_def="http://www.country-files.com/cty/wl_cty.dat";

//each of these is an array of variable length
const QString c_cab[7]={"cabrillo/cab1","cabrillo/cab2","cabrillo/cab3","cabrillo/cab4",
                        "cabrillo/cab5","cabrillo/cab6","cabrillo/cab7"};

const QString c_cab_contestname="cabrillo/contestname";
const QString c_cab_contestname_def="";

const QString c_cab_location="cabrillo/location";
const QString c_cab_location_def="";

const QString c_cab_version="cabrillo/version";
const QString c_cab_version_def="3.0";

const QString c_col_width_group="column";
const QString c_col_width_item="width";
const int c_col_width_def[SQL_N_COL]={5,5,7,9,0,0,0,5,5,5,5,5,5,5,5,2,2};

const QString s_sdr_path[NRIG]={"sdr/path1","sdr/path2"};

const QString s_wsjtx_enable[NRIG]={"wsjtx/enable1","wsjtx/enable2"};
const bool s_wsjtx_enable_def=false;

const QString s_wsjtx_udp[NRIG]={"wsjtx/udp1","wsjtx/udp2"};
const int s_wsjtx_udp_def[NRIG]={2237,2238};

const QString s_wsjtx_hide_dupes[NRIG]={"wsjtx/dupes1","wsjtx/dupes2"};
const bool s_wsjtx_hide_dupes_def=true;

const QString s_sdr_config[NRIG]={"sdr/config1","sdr/config2"};
#ifdef Q_OS_LINUX
const QString s_sdr_config_def[NRIG]={QDir::homePath()+"/.so2sdr/so2sdr-bandmap1.ini",QDir::homePath()+"/.so2sdr/so2sdr-bandmap2.ini"};
#endif

const QString s_sdr_ip[NRIG]={"sdr/ip1","sdr/ip2"};
const QString s_sdr_ip_def[NRIG]={"localhost","localhost"};

const QString s_sdr_port[NRIG]={"sdr/port1","sdr/port2"};
const int s_sdr_port_def[NRIG]={5001,5002};

const QString s_sdr_udp="sdr/udp";
const int s_sdr_udp_def=45454;

const QString s_sdr_changeclick="sdr/changeclick";
const bool s_sdr_changeclick_def=false;

const QString s_sdr_spotcalls="sdr/spotcalls";
const bool s_sdr_spotcalls_def=true;

const QString s_sdr_spottime="sdr/spottime";
const int s_sdr_spottime_def=1800;

const QString s_sdr_cqlimit_low[N_BANDS]={"sdr/160low","sdr/80low","sdr/40low","sdr/20low","sdr/15low","sdr/10low","sdr/60low",
                                          "sdr/30low","sdr/17low","sdr/12low","sdr/6low","sdr/2low","sdr/420low","sdr/222low","sdr/902low",
                                          "sdr/1240low","sdr/620low","sdr/2200low"};
const double cqlimit_default_low[N_BANDS]={1805000,3505000,7005000,14005000,21005000,28005000, 5330500, 10100000,
                                       18068000, 24890000, 50000000, 144000000, 420000000,222000000,902000000,1240000000,
                                          472000,135700};

const QString s_sdr_cqlimit_high[N_BANDS]={"sdr/160high","sdr/80high","sdr/40high","sdr/20high","sdr/15high","sdr/10high","sdr/60high",
                                           "sdr/30high","sdr/17high","sdr/12high","sdr/6high","sdr/2high","sdr/420high","sdr/222high",
                                           "sdr/902high","sdr/1240high","sdr/620high","sdr/2200high"};
const double cqlimit_default_high[N_BANDS]={1845000,3555000,7550000,14055000,21055000,28055000, 5403500,10150000,
                                        18110000, 24930000,50100000,144100000,450000000,225000000,928000000,1300000000,
                                           479000,137800};

const QString s_radios_rigctld_enable[NRIG]={"radios/rigctldenable1","radios/rigctldenable2"};
const bool s_radios_rigctld_enable_def[NRIG]={false,false};

const QString s_radios_rigctld_ip[NRIG]={"radios/rigctldip1","radios/rigctldip2"};
const QString s_radios_rigctld_ip_def[NRIG]={"localhost","localhost"};

const QString s_radios_rigctld_port[NRIG]={"radios/rigctldport1","radios/rigctldport2"};
const int s_radios_rigctld_port_def[NRIG]={4532,4534};

const QString s_radios_rig[NRIG]={"radios/hamlibmodel1","radios/hamlibmodel2"};
const int s_radios_rig_def[NRIG]={1,1};

const QString s_radios_port[NRIG]={"radios/port1","radios/port2"};
const QString s_radios_port_def[NRIG]={"",""};

const QString s_radios_baud[NRIG]={"radios/baud1","radios/baud2"};
const int s_radios_baud_def[NRIG]={4800,4800};

const QString s_radios_poll[NRIG]={"radios/poll1","radios/poll2"};
const int s_radios_poll_def[NRIG]={500,500};

const QString s_radios_if[NRIG]={"radios/if1","radios/if2"};
const double s_radios_if_def[NRIG]={8215000,8215000}; // default for Elecraft K3

const QString s_radios_pport_enabled="radios/pport_enabled";
const bool s_radios_pport_enabled_def=false;

const QString s_radios_pport="radios/pport";
const QString s_radios_pport_def=defaultParallelPort;

const QString s_radios_focus="radios/focuspin";
const int s_radios_focus_def=defaultParallelPortAudioPin;

const QString s_radios_focusinvert="radios/focusinvert";
const bool s_radios_focusinvert_def=false;

const QString s_radios_txfocus="radios/txfocuspin";
const int s_radios_txfocus_def=defaultParallelPortTxPin;

const QString s_radios_txfocusinvert="radios/txfocusinvert";
const bool s_radios_txfocusinvert_def=false;

const QString s_radios_stereo="radios/stereopin";
const int s_radios_stereo_def=defaultParallelPortStereoPin;

const QString s_telnet_addresses="telnet/addresses";
const QString s_telnet_addresses_def="";

const QString s_settings_qsyfocus="main/qsyfocus";
const bool s_settings_qsyfocus_def=false;

const QString s_settings_focusindicators="main/focusindicators";
const bool s_settings_focusindicators_def=false;

const QString s_settings_exchangelogs="main/exchangelogs";
const bool s_settings_exchangelogs_def=false;

const QString s_settings_cqrepeat="main/cqrepeat";
const double s_settings_cqrepeat_def=3000;

const QString s_settings_duelingcqdelay="main/duelingcqdelay";
const double s_settings_duelingcqdelay_def=4.0;

const QString s_settings_autosend_mode="main/autosendmode";
const int s_settings_autosend_mode_def=0;

const QString s_settings_autosend="main/autosend";
const int s_settings_autosend_def=3;

const QString c_multsband="contest/multsband";
const bool c_multsband_def=true;

const QString c_multsmode="contest/multsmode";
const bool c_multsmode_def=false;

const QString c_mastermode="contest/usemaster";
const bool c_mastermode_def=true;

const QString c_masterfile="contest/masterfile";
const QString c_masterfile_def="MASTER.DTA";

const QString c_historyupdate="contest/historyupdate";
const bool c_historyupdate_def=false;

const QString c_historymode="contest/usehistory";
const bool c_historymode_def=false;

const QString c_historyfile="contest/historyfile";
const QString c_historyfile_def="";

const QString c_sprintmode="contest/sprintmode";
const bool c_sprintmode_def=false;

const QString c_showmode="contest/showmode";
const bool c_showmode_def=false;

const QString c_showmults="contest/showmults";
const bool c_showmults_def=true;

const QString c_dupemode="contest/dupemode";
const int c_dupemode_def=WORK_DUPES;

const QString c_sentexch1="contest/sentexch1";
const QString c_sentexch1_def="";

const QString c_sentexch2="contest/sentexch2";
const QString c_sentexch2_def="";

const QString c_sentexch3="contest/sentexch3";
const QString c_sentexch3_def="";

const QString c_sentexch4="contest/sentexch4";
const QString c_sentexch4_def="";

const QString c_exchname1="contest/sentexchname1";
const QString c_exchname1_def="";

const QString c_exchname2="contest/sentexchname2";
const QString c_exchname2_def="";

const QString c_exchname3="contest/sentexchname3";
const QString c_exchname3_def="";

const QString c_exchname4="contest/sentexchname4";
const QString c_exchname4_def="";

const QString c_contestname="contest/contest";
const QString c_contestname_def="";

const QString c_contestname_displayed="contest/contestname_displayed";
const QString c_contestname_displayed_def="";

const QString c_multfile1="contest/multfile1";
const QString c_multfile1_def="";

const QString c_multfile2="contest/multfile2";
const QString c_multfile2_def="";

const QString c_nmulttypes="contest/nmulttypes";
const int c_nmulttypes_def=0;

const QString c_mult_name1="contest/mult_name1";
const QString c_mult_name1_def="";

const QString c_mult_name2="contest/mult_name2";
const QString c_mult_name2_def="";

const QString c_exclude_mults1="contest/exclude_mults_file1";
const QString c_exclude_mults1_def="";

const QString c_exclude_mults2="contest/exclude_mults_file2";
const QString c_exclude_mults2_def="";

const QString c_qso_type1="contest/qsotype1";
const QString c_qso_type1_def="";

const QString c_qso_type2="contest/qsotype2";
const QString c_qso_type2_def="";

const QString c_cq_func[2]={"keys/cq","keys_phone/cq"};
const QString c_cq_func_def[2]={"",""};

const QString c_cq_rec_func="keys_phone_rec/cq";
const QString c_cq_rec_func_def="";

const QString c_ex_func[2]={"keys/ex","keys_phone/ex"};
const QString c_ex_func_def[2]={"",""};

const QString c_ex_rec_func="keys_phone_rec/ex";
const QString c_ex_rec_func_def="";

const QString c_ctrl_func[2]={"keys/ctrl","keys_phone/ctrl"};
const QString c_ctrl_func_def[2]={"",""};

const QString c_shift_func[2]={"keys/shift","keys_phone/shift"};
const QString c_shift_func_def[2]={"",""};

const QString c_sp_exc[2]={"keys/sp_exch","keys_phone/sp_exch"};
const QString c_sp_exc_def[2]={"",""};

const QString c_sp_exc_rec="keys_phone/sp_exch_rec";
const QString c_sp_exc_rec_def="";

const QString c_cq_exc[2]={"keys/cq_exch","keys_phone/cq_exch"};
const QString c_cq_exc_def[2]={"",""};

const QString c_cq_exc_rec="keys_phone/cq_exch_rec";
const QString c_cq_exc_rec_def="";

const QString c_qsl_msg[2]={"keys/qsl_msg","keys_phone/qsl_msg"};
const QString c_qsl_msg_def[2]={"",""};

const QString c_call_msg="keys_phone/call";
const QString c_call_msg_def="";

const QString c_call_msg_rec="keys_phone/call_rec";
const QString c_call_msg_rec_def="";

const QString c_qsl_msg_rec="keys_phone/qsl_msg_rec";
const QString c_qsl_msg_rec_def="";

const QString c_qsl_msg_updated[2]={"keys/qsl_msg_updated","keys_phone/qsl_msg_updated"};
const QString c_qsl_msg_updated_def[2]={"",""};

const QString c_qsl_msg_updated_rec="keys_phone/qsl_msg_updated_rec";
const QString c_qsl_msg_updated_rec_def="";

const QString c_qqsl_msg[2]={"keys/qqsl_msg","keys_phone/qqsl_msg"};
const QString c_qqsl_msg_def[2]={"",""};

const QString c_qqsl_msg_rec="keys_phone/qqsl_msg_rec";
const QString c_qqsl_msg_rec_def="";

const QString c_dupe_msg[2]={"keys/dupe_msg","keys_phone/dupe_msg"};
const QString c_dupe_msg_def[2]={"",""};

const QString c_dupe_msg_rec="keys_phone/dupe_msg_rec";
const QString c_dupe_msg_rec_def="";

const QString c_ssb_cancel="keys_phone/cancel";
const QString c_ssb_cancel_def="";

const QString c_cty="contest/cty";
const QString c_cty_def="wl_cty.dat";

const QString c_mobile_dupes="contest/mobile_dupes";
const bool c_mobile_dupes_def=false;

const QString c_mobile_dupes_col="contest/mobile_dupes_column";
const int c_mobile_dupes_col_def=1;

const QString c_mult1_displayonly="contest/mult1_displayonly";
const bool c_mult1_displayonly_def=false;

const QString c_mult2_displayonly="contest/mult2_displayonly";
const bool c_mult2_displayonly_def=false;

const QString c_multimode="contest/multimode";
const bool c_multimode_def=false;

const QString c_multimode_cw="contest/multimode_cw";
const bool c_multimode_cw_def=true;

const QString c_multimode_phone="contest/multimode_phone";
const bool c_multimode_phone_def=true;

const QString c_multimode_digital="contest/multimode_digital";
const bool c_multimode_digital_def=false;

const QString c_off_time_start="contest/offtime_start";
const QDateTime c_off_time_start_def=QDateTime::currentDateTimeUtc();

const QString c_off_time_end="contest/offtime_end";
const QDateTime c_off_time_end_def=QDateTime::currentDateTimeUtc();

const QString c_off_time_min="contest/offtime_min";
const int c_off_time_min_def=30;

const QString c_off_time_enable="contest/offtime_enable";
const bool c_off_time_enable_def=false;

const QString s_play_command[2]={"play_command1","play_command2"};
const QString s_play_command_def="gst-launch-1.0 -q filesrc location=$.wav ! wavparse ! audioconvert ! pulsesink";

const QString s_switch_command[2]={"switch_command1","switch_command2"};
const QString s_switch_command_def="";

const QString s_rec_command="rec_command";
const QString s_rec_command_def="gst-launch-1.0 -q pulsesrc ! wavenc ! filesink location=$.wav";

const QString s_before_rec="before_rec";
const QString s_before_rec_def="pactl set-source-mute 1 0";

const QString s_after_rec="after_rec";
const QString s_after_rec_def="";

const QString s_before_play[2]={"before_play1","before_play2"};
const QString s_before_play_def="pactl set-source-mute 1 1";

const QString s_after_play[2]={"after_play1","after_play2"};
const QString s_after_play_def="pactl set-source-mute 1 0";

const QString s_radios_ptt_type[NRIG]={"radios/ptt1_type","radios/ptt2_type"};
const int s_radios_ptt_type_def=0;
#endif // DEFINES_H
