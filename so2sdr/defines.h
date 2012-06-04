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
#ifndef DEFINES_H
#define DEFINES_H
#include <QByteArray>
#include <QColor>
#include <QDateTime>
#include <QDir>
#include <QString>
#include <QtGlobal>
#include <QVariant>
#include "fftw3.h"
#include "hamlib/rig.h"

// ///////// version ///////////////
const QByteArray Version = "1.2.2git";

// directory where station config, hamlib cache, iqdata, etc are stored
#ifdef Q_OS_WIN
const QString userDirectory = QDir::homePath() + "/so2sdr";
#endif
#ifdef Q_OS_LINUX
const QString userDirectory = QDir::homePath() + "/.so2sdr";
#endif

#ifdef Q_OS_WIN
// for some reason, can't access this variable in MSVC
const QByteArray so2sdr_hamlib_version="1.2.14";
#endif


// //////// colors ////////////////
// all of form (R,G,B)
const QColor CQ_COLOR(255, 255, 255);  // color in CQ mode; white
const QColor SP_COLOR(255, 255, 127);  // color in SP mode; yellow
const QColor ALTD_COLOR(255, 0, 255);  // color during alt-D, Sprint SO2R

// //////////// telnet //////////////////
/*!
   Maximum number of characters kept in telnet buffer
 */
const int MAX_TELNET_CHARS=4096;

/////////////// parallel port ////////////
const int defaultParallelPortStereoPin=5;
const int defaultParallelPortRadioPin=4;
#ifdef Q_OS_LINUX
const QString defaultParallelPort = "/dev/parport0";
#endif
#ifdef Q_OS_WIN
const QString defaultParallelPort = "0x378";
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
} WinkeyParam;
Q_DECLARE_TYPEINFO(WinkeyParam, Q_PRIMITIVE_TYPE);


// /////////// Radio serial communications ///////////////

#ifdef Q_OS_LINUX
const QString defaultSerialPort[3] = {"/dev/ttyS0","/dev/ttyS1","/dev/ttyS2"};
#endif
#ifdef Q_OS_WIN
const QString defaultSerialPort[3] = {"COM1","COM2","COM3"};
#endif

const int NRIG=2;

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
    Uniques     = 14
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

   @todo Make this a class to make memory management easier

 */
typedef struct Country {
    int               indx;
    QByteArray        name;
    int               Zone;
    Cont              Continent;
    float             delta_t;
    int               bearing;
    QByteArray        MainPfx;
    bool              multipleZones;
    QList<QByteArray> zonePfx;
    QList<int>        zones;
    QString           sun;
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

// //////////////// Bandscope defines //////////////////

// the fit order is currently fixed (4==cubic a0 + a1*x +a2*x*x + a3*x*x*x),
// may want to make this adjustable in the future.
const int FIT_ORDER=4;

// Signal (peak detection) parameters

/*!  max number of detected signals
 */
const int SIG_MAX=4096;

/*! minimum delta over background, as a fraction of the background level
 */
const double SIG_DELTA_BG=0.2;

/*! minimum delta over background, for signals used to calibrate gain/phase
 */
const double SIG_DELTA_CALIB=5.0;

/*! frequency of measuring calibration data, in units of spectrum sweeps
 */
const int SIG_CALIB_FREQ=5;

/*! bins near zero freq not used for IQ balance data
 */
const int SIG_CALIB_SKIP=400;
const int SIG_CALIB_SPACE=100;

/*! min peak width
 */
const int SIG_PEAK_MIN_WIDTH=2;

/*! max peak width
 */
const int SIG_PEAK_MAX_WIDTH=12;

/*! number of scans averaged together before peak detection
 */
const int SIG_N_AVG=10;

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

const int MAX_H=2049; // max pixmap height
const int MAX_W=800; // max pixmap width
const int SAMPLE_FREQ=96; // Sampling frequency in Khz

/*! ALSA period size in units of frames. This sets the frequency of interrupts to
   the audio device.

   1 frame = left + right samples, 4 bytes for 16 bit audio,
   8 bytes for 24(padded) or 32 bit
 */
const int ALSA_PERIOD_SIZE=2048;

typedef struct sampleSizes {
    int           sample_length; // FFT length
    unsigned long chunk_size;    // FFT length * size of 1 frame (L+R channel samples)
    unsigned long advance_size;  // bytes for one advance of spectrum
    int           spec_length;   // size in bytes of one displayed scanline
    int           display_length;
    int           spec_buff;     // total buffer size, displayed spectrum
} sampleSizes;
Q_DECLARE_TYPEINFO(sampleSizes, Q_PRIMITIVE_TYPE);

/*! finished spectrum scans buffer:
   total number of spectrum scans to keep */
const int N_SPEC=100;

#ifdef Q_OS_WIN
/*! Windows FFTW function pointers */
typedef void* (*fftw_malloc_ptr)(int n);
typedef void* (*fftw_free_ptr)(void* ptr);
typedef fftw_plan (*fftw_plan_dft_1d_ptr)(int n, void *ptr_in, void* ptr_out, int i1, int i2);
typedef void (*fftw_destroy_plan_ptr)(fftw_plan plan);
#endif

// /////// Misc stuff

// timers
const int N_TIMERS =4;

// default frequency for various things (all in milliseconds)
const int timerSettings[]={
    1000,  // clock update
    300, // radio/serial update
    60000, // rate display update, band spot update
    10000 // IQ plot update
};

/*! number of minutes in moving average of rate display
 */
const int RATE_AVG_MINUTES=3;

// powers of 2
const unsigned int bits[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768 };

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
const int N_BANDS=16;

const int BAND160 =0;
const int BAND80 = 1;
const int BAND40 = 2;
const int BAND20 = 3;
const int BAND15 = 4;
const int BAND10 = 5;

// only traditional contest bands used for score calculations
const int N_BANDS_SCORED=6;
const int BAND60 = 6;
const int BAND30 = 7;
const int BAND17 = 8;
const int BAND12 = 9;
const int BAND6 = 10;
const int BAND2 = 11;
const int BAND420 = 12; // 420 MHz
const int BAND222= 13; // 222 MHz
const int BAND902 = 14; // 902 MHz
const int BAND1240 = 15; // 1240 MHz

const QString bandName[N_BANDS] = { "160", "80", "40", "20", "15", "10", "60", "30", "17", "12", "6M", "2M",
                                    "70cm","1.25M","33cm","23cm"};

// band limits: used for automatic qsy between signals
// right now just CW sub-bands for 160-10m
const int band_limits[N_BANDS][2] = {{   1800000,  1900000 }, {  3500000,  3600000 }, { 7000000, 7125000 }, { 14000000, 14150000 },
                                      { 21000000, 21200000 }, { 28000000, 28300000 }, { 5330500, 5403500 }, { 10100000, 10150000 },
                                      { 18068000, 18110000 }, { 24890000, 24930000 }, {50000000,54000000 }, {144000000, 148000000 },
                                      {420000000, 450000000}, {222000000,225000000},{902000000,928000000},{1240000000,1300000000}};

// maximum number of Cabrillo fields
const int MAX_CAB_FIELDS=7;

//////// QSettings key names and default values ///////

// s_xxx are used in station config file (so2sdr.ini)
// c_xxx are used in contest config file

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

//each of these is an array of variable length
const QString c_cab[7]={"cabrillo/cab1","cabrillo/cab2","cabrillo/cab3","cabrillo/cab4",
                        "cabrillo/cab5","cabrillo/cab6","cabrillo/cab7"};

const QString c_cab_contestname="cabrillo/contestname";
const QString c_cab_contestname_def="";

const QString c_cab_location="cabrillo/location";
const QString c_cab_location_def="";

const QString c_cab_version="cabrillo/version";
const QString c_cab_version_def="3.0";

const QString s_sdr_changeclick="sdr/changeclick";
const bool s_sdr_changeclick_def=false;

const QString s_sdr_enabled[NRIG]={"sdr/enabled1","sdr/enabled2"};
const bool s_sdr_enabled_def[NRIG]={false,false};

const QString s_sdr_spotcalls="sdr/spotcalls";
const bool s_sdr_spotcalls_def=true;

const QString s_sdr_bits[NRIG]={"sdr/bits1","sdr/bits2"};
const int s_sdr_bits_def[NRIG]={24,24};

const QString s_sdr_api[NRIG]={"sdr/api1","sdr/api2"};
const int s_sdr_api_def[NRIG]={0,0};

const QString s_sdr_device[NRIG]={"sdr/device1","sdr/device2"};
const int s_sdr_device_def[NRIG]={0,0};

const QString s_sdr_offset[NRIG]={"sdr/offset1","sdr/offset2"};
const int s_sdr_offset_def[NRIG]={0,0};

const QString s_sdr_scale[NRIG]={"sdr/scale1","sdr/scale2"};
const int s_sdr_scale_def[NRIG]={1,1};

const QString s_sdr_mark="sdr/mark";
const bool s_sdr_mark_def=true;

const QString s_sdr_peakdetect[NRIG]={"sdr/peakdetect1","sdr/peakdetect2"};
const bool s_sdr_peakdetect_def[NRIG]={true,true};

const QString s_sdr_iqdata[NRIG]={"sdr/iqdata1","sdr/iqdata2"};
const bool s_sdr_iqdata_def[NRIG]={true,true};

const QString s_sdr_iqcorrect[NRIG]={"sdr/iqcorrect1","sdr/iqcorrect2"};
const bool s_sdr_iqcorrect_def[NRIG]={true,true};

const QString s_sdr_click[NRIG]={"sdr/click1","sdr/click2"};
const bool s_sdr_click_def[NRIG]={false,false};

const QString s_sdr_cqtime="sdr/cqtime";
const int s_sdr_cqtime_def=15;

const QString s_sdr_spottime="sdr/spottime";
const int s_sdr_spottime_def=30;

const QString s_sdr_cqlimit_low[N_BANDS]={"sdr/160low","sdr/80low","sdr/40low","sdr/20low","sdr/15low","sdr/10low","sdr/60low",
                                          "sdr/30low","sdr/17low","sdr/12low","sdr/6low","sdr/2low","sdr/420low","sdr/222low","sdr/902low",
                                          "sdr/1240low"};
const int cqlimit_default_low[N_BANDS]={1805000,3505000,705000,14005000,21005000,28005000, 5330500, 10100000,
                                       18068000, 24890000, 50000000, 144000000, 420000000,222000000,902000000,1240000000};

const QString s_sdr_cqlimit_high[N_BANDS]={"sdr/160high","sdr/80high","sdr/40high","sdr/20high","sdr/15high","sdr/10high","sdr/60high",
                                           "sdr/30high","sdr/17high","sdr/12high","sdr/6high","sdr/2high","sdr/420high","sdr/222high",
                                           "sdr/902high","sdr/1240high"};
const int cqlimit_default_high[N_BANDS]={1845000,3555000,755000,14055000,21055000,28055000, 5403500,10150000,
                                        18110000, 24930000,50100000,144100000,450000000,225000000,928000000,1300000000};

const QString s_radios_rig[NRIG]={"radios/hamlibmodel1","radios/hamlibmodel2"};
const int s_radios_rig_def[NRIG]={1,1};

const QString s_radios_port[NRIG]={"radios/port1","radios/port2"};
const QString s_radios_port_def[NRIG]={"",""};

const QString s_radios_baud[NRIG]={"radios/baud1","radios/baud2"};
const int s_radios_baud_def[NRIG]={1200,1200};

const QString s_radios_poll[NRIG]={"radios/poll1","radios/poll2"};
const int s_radios_poll_def[NRIG]={500,500};

const QString s_radios_pport="radios/pport";
const QString s_radios_pport_def=defaultParallelPort;

const QString s_radios_focus="radios/focuspin";
const int s_radios_focus_def=defaultParallelPortRadioPin;

const QString s_radios_focusinvert="radios/focusinvert";
const bool s_radios_focusinvert_def=false;

const QString s_radios_stereo="radios/stereopin";
const int s_radios_stereo_def=defaultParallelPortStereoPin;

const QString s_telnet_addresses="telnet/addresses";
const QString s_telnet_addresses_def="";

const QString c_multsband="contest/multsband";
const bool c_multsband_def=true;

const QString c_mastermode="contest/usemaster";
const bool c_mastermode_def=true;

const QString c_masterfile="contest/masterfile";
const QString c_masterfile_def="MASTER.DTA";

const QString c_sprintmode="contest/sprintmode";
const bool c_sprintmode_def=false;

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

const QString c_cq_func="keys/cq";
const QString c_cq_func_def="";

const QString c_ex_func="keys/ex";
const QString c_ex_func_def="";

const QString c_ctrl_func="keys/ctrl";
const QString c_ctrl_func_def="";

const QString c_shift_func="keys/shift";
const QString c_shift_func_def="";

const QString c_sp_exc="keys/sp_exch";
const QString c_sp_exc_def="";

const QString c_cq_exc="keys/cq_exch";
const QString c_cq_exc_def="";

const QString c_qsl_msg="keys/qsl_msg";
const QString c_qsl_msg_def="";

const QString c_qsl_msg_updated="keys/qsl_msg_updated";
const QString c_qsl_msg_updated_def="";

const QString c_qqsl_msg="keys/qqsl_msg";
const QString c_qqsl_msg_def="";

const QString c_dupe_msg="keys/dupe_msg";
const QString c_dupe_msg_def="";

const QString c_cty="contest/cty";
const QString c_cty_def="wl_cty.dat";
#endif // DEFINES_H
