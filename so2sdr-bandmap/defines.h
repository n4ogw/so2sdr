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
#ifndef DEFINES_H
#define DEFINES_H
#include <QByteArray>
#include <QColor>
#include <QString>

// //////////////// Bandscope defines //////////////////

// ///////// version //////////////
const QByteArray Version = QByteArrayLiteral("2.6.4");

typedef enum SdrType {
  soundcard_t = 0,
  network_t = 1,
  afedri_t = 2,
  rtl_t = 3
} SdrType;

typedef enum mode { IF = 0, RF = 1, RFauto = 2 } mode;

typedef struct uiSize {
  qreal height;
  qreal width;
  qreal smallHeight;
  qreal smallWidth;
  qreal rad;
} uiSize;
Q_DECLARE_TYPEINFO(uiSize, Q_PRIMITIVE_TYPE);

// number of bands
const int N_BANDS = 28;
const int BAND_NONE = -1;
const int N_BANDS_HF = 6;
const int BAND160 = 0;
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
const int BAND222 = 12;
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

const int N_BANDMAP_TIMERS = 3;

// default frequency for various things (all in milliseconds)
const int timerSettings[] = {
    100,  // freq update
    1000, // UDP beacon
    10000 // IQ plot update
};

// the fit order is currently fixed (4==cubic a0 + a1*x +a2*x*x + a3*x*x*x),
// may want to make this adjustable in the future.
const int FIT_ORDER = 4;

// Signal (peak detection) parameters

/*!  max number of detected signals
 */
const int SIG_MAX = 4096;

/*! minimum delta over background, as a fraction of the background level
 */
const double SIG_DELTA_BG = 0.2;

/*! minimum delta over background, for signals used to calibrate gain/phase
 */
const double SIG_DELTA_CALIB = 5.0;

/*! frequency of measuring calibration data, in units of spectrum sweeps
 */
const int SIG_CALIB_FREQ = 5;

/*! bins near zero freq not used for IQ balance data
 */
const int SIG_CALIB_SKIP = 400;
const int SIG_CALIB_SPACE = 100;

/*! min peak width
 */
const int SIG_PEAK_MIN_WIDTH = 2;

/*! max peak width
 */
const int SIG_PEAK_MAX_WIDTH = 12;

/*! number of scans averaged together before peak detection
 *  at speed 1; for speed 1/2, divide this by 2, 1/4 by 4
 */
const int SIG_N_AVG = 8;

/*!
   min tolerance for signal separation (Hz)
   signals + or - this amount will go in the same bin
 */
const int SIG_MIN_FREQ_DIFF = 65;

/*!
  min tolerance for callsign separation
  */
const int SIG_MIN_SPOT_DIFF = 200;

/*! minimum time in seconds to keep a signal once detected
 */
const double SIG_KEEP_TIME = 5.0;

/*! radius of circles marking signals (in pixels)
 */
const int SIG_SYMBOL_RAD = 4;

/*! pixel location from left edge of field for signal symbols
 */
const int SIG_SYMBOL_X = 5;

/*!  Default timeout of spotted calls (minutes)
 */
const int DEFAULT_SPOT_TIMEOUT = 10;

/*! pixel size of font used
 */
const int BANDMAP_FONT_POINT_SIZE = 10;

/*!  pixel location from left edge of field for calls in bandmap
 */
const int BANDMAP_CALL_X = 15;

/*! timeout to wait after freq change detected before peak detecting (ms)
 */
const int TUNING_TIMEOUT = 1500;

const int MAX_W = 800; // max pixmap width

typedef struct sampleSizes {
  unsigned long
      chunk_size; // FFT length * size of 1 frame (L+R channel samples)
  unsigned long advance_size; // bytes for one advance of spectrum
} sampleSizes;
Q_DECLARE_TYPEINFO(sampleSizes, Q_PRIMITIVE_TYPE);

// band limits: used for automatic qsy between signals
// right now just CW sub-bands for 160-10m
const double band_limits[N_BANDS][2] = {{1800000, 1900000},
                                        {3500000, 3600000},
                                        {7000000, 7125000},
                                        {14000000, 14150000},
                                        {21000000, 21200000},
                                        {28000000, 28300000},
                                        {5330500, 5403500},
                                        {10100000, 10150000},
                                        {18068000, 18110000},
                                        {24890000, 24930000},
                                        {50000000, 54000000},
                                        {144000000, 148000000},
                                        {420000000, 450000000},
                                        {222000000, 225000000},
                                        {902000000, 928000000},
                                        {1240000000, 1300000000},
                                        {2300000000, 2450000000},
                                        {3300000000, 3500000000},
                                        {5650000000, 5925000000},
                                        {10000000000, 10500000000},
                                        {24000000000, 24250000000},
                                        {47000000000, 47200000000},
                                        {76000000000, 81000000000},
                                        {122250000000, 123000000000},
                                        {134000000000, 141000000000},
                                        {241000000000, 250000000000},
                                        {472000, 479000},
                                        {135700, 137800}};

//////// QSettings key names and default values used in so2sdr-bandmap.ini
//////////

const QString s_sdr_nrig = QStringLiteral("nrig");
const int s_sdr_nrig_def = 0;

const QString s_sdr_bandmap_tcp_port = QStringLiteral("bandmap_tcp_port");
const int s_sdr_bandmap_tcp_port_def = 5001;

const QString s_sdr_bandmap_udp_port = QStringLiteral("bandmap_udp_port");
const int s_sdr_bandmap_udp_port_def = 45454;

const QString s_sdr_type = QStringLiteral("type");
const SdrType s_sdr_type_def = soundcard_t;

const QString s_sdr_bits = QStringLiteral("bits");
const int s_sdr_bits_def = 0;

const QString s_sdr_api = QStringLiteral("api");
const int s_sdr_api_def = 0;

const QString s_sdr_fft = QStringLiteral("fft");
const int s_sdr_fft_def = 4096;

const QString s_sdr_speed = QStringLiteral("speed");
const int s_sdr_speed_def = 1;

const QString s_sdr_sample_freq = QStringLiteral("sample_freq");
const int s_sdr_sample_freq_def = 96000;

const QString s_sdr_mode = QStringLiteral("mode");
const mode s_sdr_mode_def = IF;

const QString s_sdr_txstop = QStringLiteral("txstop");
const bool s_sdr_txstop_def = false;

// sdr-ip SDR

const QString s_sdr_net_speed = QStringLiteral("net_speed");
const int s_sdr_net_speed_def = 1;

const QString s_sdr_net_tcp_ip = QStringLiteral("net_ipaddress");
const QString s_sdr_net_tcp_ip_def = QStringLiteral("127.0.0.1");

const QString s_sdr_net_tcp_port = QStringLiteral("net_tcp_port");
const int s_sdr_net_tcp_port_def = 50000;

const QString s_sdr_net_udp_port = QStringLiteral("net_udp_port");
const int s_sdr_net_udp_port_def = 50000;

const QString s_sdr_offset_network = QStringLiteral("offset_network");
const int s_sdr_offset_network_def = 15000;

const QString s_sdr_swap_network = QStringLiteral("swapiq_network");
const bool s_sdr_swap_network_def = false;

const QString s_sdr_net_sample_freq = QStringLiteral("net_sample_freq");
const int s_sdr_net_sample_freq_def = 100000;

const QString s_sdr_net_if_freq = QStringLiteral("if_freq_rtl");
const int s_sdr_net_if_freq_def = 8225000;

// soundcard SDR

const QString s_sdr_sound_speed = QStringLiteral("soundcard_speed");
const int s_sdr_sound_speed_def = 1;

const QString s_sdr_sound_sample_freq = QStringLiteral("soundcard_sample_freq");
const int s_sdr_sound_sample_freq_def = 96000;

const QString s_sdr_offset_soundcard = QStringLiteral("offset_soundcard");
const int s_sdr_offset_soundcard_def = 0;

const QString s_sdr_swap_soundcard = QStringLiteral("swapiq_soundcard");
const bool s_sdr_swap_soundcard_def = false;

// afedri SDR

const QString s_sdr_afedri_speed = QStringLiteral("afedri_speed");
const int s_sdr_afedri_speed_def = 1;

const QString s_sdr_afedri_sample_freq = QStringLiteral("afedri_sample_freq");
const int s_sdr_afedri_sample_freq_def = 100000;

const QString s_sdr_afedri_real_sample_freq =
    QStringLiteral("afedri_sample_freq");
const int s_sdr_afedri_real_sample_freq_def = 0;

const QString s_sdr_offset_afedri = QStringLiteral("offset_afedri");
const int s_sdr_offset_afedri_def = 15000;

const QString s_sdr_afedri_tcp_ip = QStringLiteral("tcp_address_afedri");
const QString s_sdr_afedri_tcp_ip_def = QStringLiteral("127.0.0.1");

const QString s_sdr_afedri_tcp_port = QStringLiteral("tcp_port_afedri");
const int s_sdr_afedri_tcp_port_def = 50000;

const QString s_sdr_afedri_udp_port = QStringLiteral("udp_port_afedri");
const int s_sdr_afedri_udp_port_def = 50000;

const QString s_sdr_afedri_channel = QStringLiteral("afedri_channel");
const int s_sdr_afedri_channel_def = 0;

const QString s_sdr_afedri_multi = QStringLiteral("afedri_mode");
const int s_sdr_afedri_multi_def = 0;

const QString s_sdr_afedri_bcast = QStringLiteral("afedri_bcast");
const int s_sdr_afedri_bcast_def = 0;

const QString s_sdr_afedri_freq = QStringLiteral("afedri_freq");
// this is the IF frequency needed for an Elecraft K3
const int s_sdr_afedri_freq_def = 8225000;

const QString s_sdr_swap_afedri = QStringLiteral("swapiq_afedri");
const bool s_sdr_swap_afedri_def = false;

// RTL-SDR

const QString s_sdr_rtl_bits = QStringLiteral("rtl_bits");
const int s_sdr_rtl_bits_def = 3;

const QString s_sdr_rtl_speed = QStringLiteral("rtl_speed");
const int s_sdr_rtl_speed_def = 1;

const QString s_sdr_rtl_sample_freq = QStringLiteral("rtl_sample_freq");
const int s_sdr_rtl_sample_freq_def = 262144;

const QString s_sdr_offset_rtl = QStringLiteral("offset_rtl");
const int s_sdr_offset_rtl_def = 15000;

const QString s_sdr_swap_rtl = QStringLiteral("swapiq_rtl");
const bool s_sdr_swap_rtl_def = false;

const QString s_sdr_rtl_tcp_ip = QStringLiteral("tcp_address_rtl");
const QString s_sdr_rtl_tcp_ip_def = QStringLiteral("127.0.0.1");

const QString s_sdr_rtl_tcp_port = QStringLiteral("tcp_port_rtl");
const int s_sdr_rtl_tcp_port_def = 1234;

const QString s_sdr_rtl_dev_index = QStringLiteral("dev_index_rtl");
const int s_sdr_rtl_dev_index_def = 0;

const QString s_sdr_rtl_if_freq = QStringLiteral("if_freq_rtl");
const int s_sdr_rtl_if_freq_def = 8225000;

const QString s_sdr_rtl_tuner_gain = QStringLiteral("tuner_gain_rtl");
const int s_sdr_rtl_tuner_gain_def = 0;

const QString s_sdr_rtl_direct = QStringLiteral("direct_rtl");
const bool s_sdr_rtl_direct_def = false;

const QString s_sdr_tcp_ip = QStringLiteral("ipaddress");
const QString s_sdr_tcp_ip_def = QStringLiteral("127.0.0.1");

const QString s_sdr_udp_ip = QStringLiteral("udp_ipaddress");
const QString s_sdr_udp_ip_def = QStringLiteral("127.0.0.1");

const QString s_sdr_tcp_port = QStringLiteral("tcp_port");
const int s_sdr_tcp_port_def = 50000;

const QString s_sdr_udp_port = QStringLiteral("udp_port");
const int s_sdr_udp_port_def = 50000;

const QString s_sdr_device = QStringLiteral("device");
const int s_sdr_device_def = 0;

const QString s_sdr_deviceindx = QStringLiteral("deviceindx");
const int s_sdr_deviceindx_def = 0;

const QString s_sdr_offset = QStringLiteral("offset");
const int s_sdr_offset_def = 0;

const QString s_sdr_swapiq = QStringLiteral("swapiq");
const bool s_sdr_swapiq_def = false;

const QString s_sdr_scale = QStringLiteral("scale");
const int s_sdr_scale_def = 1;

const QString s_sdr_peakdetect = QStringLiteral("peakdetect");
const bool s_sdr_peakdetect_def = true;

const QString s_sdr_iqdata = QStringLiteral("iqdata");
const bool s_sdr_iqdata_def = false;

const QString s_sdr_iqcorrect = QStringLiteral("iqcorrect");
const bool s_sdr_iqcorrect_def = false;

const QString s_sdr_cqtime = QStringLiteral("cqtime");
const int s_sdr_cqtime_def = 15;

const QString s_sdr_cq_finder_calls = QStringLiteral("cq_finder_calls");
const bool s_sdr_cq_finder_calls_def = true;

const QString s_sdr_level = QStringLiteral("level");
const int s_sdr_level_def = 100;

const QString s_sdr_n1mm = QStringLiteral("n1mm");
const bool s_sdr_n1mm_def = false;

const QString s_sdr_n1mm_port = QStringLiteral("n1mm_port");
const int s_sdr_n1mm_port_def = 12060;

const QString s_sdr_reverse_scroll = QStringLiteral("reverse_scroll");
const bool s_sdr_reverse_scroll_def = false;

const double cqlimit_default_low[N_BANDS] = {
    1805000,      3505000,     7005000,     14005000,     21005000,
    28005000,     5330500,     10100000,    18068000,     24890000,
    50000000,     144000000,   420000000,   222000000,    902000000,
    1240000000,   2300000000,  3300000000,  5650000000,   10000000000,
    24000000000,  47000000000, 76000000000, 122250000000, 134000000000,
    241000000000, 472000,      135700};

const double cqlimit_default_high[N_BANDS] = {
    1845000,      3555000,     7550000,     14055000,     21055000,
    28055000,     5403500,     10150000,    18110000,     24930000,
    50100000,     144100000,   450000000,   225000000,    928000000,
    1300000000,   2450000000,  3500000000,  5925000000,   10500000000,
    24250000000,  47200000000, 81000000000, 123000000000, 141000000000,
    250000000000, 479000,      137800};

const QString bandName[N_BANDS] = {
    QStringLiteral("160"),   QStringLiteral("80"),   QStringLiteral("40"),
    QStringLiteral("20"),    QStringLiteral("15"),   QStringLiteral("10"),
    QStringLiteral("60"),    QStringLiteral("30"),   QStringLiteral("17"),
    QStringLiteral("12"),    QStringLiteral("6M"),   QStringLiteral("2M"),
    QStringLiteral("1.25M"), QStringLiteral("70cm"), QStringLiteral("33cm"),
    QStringLiteral("23cm"),  QStringLiteral("13cm"), QStringLiteral("9cm"),
    QStringLiteral("6cm"),   QStringLiteral("3cm"),  QStringLiteral("1.25cm"),
    QStringLiteral("6mm"),   QStringLiteral("4mm"),  QStringLiteral("2.5mm"),
    QStringLiteral("2mm"),   QStringLiteral("1mm"),  QStringLiteral("630"),
    QStringLiteral("2200")};

#endif // DEFINES_H
