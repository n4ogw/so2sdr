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
#ifndef DEFINES_H
#define DEFINES_H
#include <QByteArray>
#include <QColor>
#include <QString>

// //////////////// Bandscope defines //////////////////

// ///////// version ///////////////
const QByteArray Version = "2.0.1";

typedef enum SdrType {
    soundcard_t=0,
    network_t=1,
    afedri_t=2
} SdrType;

// number of bands
const int N_BANDS=16;

const int BAND_NONE=-1;
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

const int N_BANDMAP_TIMERS=3;

// default frequency for various things (all in milliseconds)
const int timerSettings[]={
    100,  // freq update
    1000,  // UDP beacon
    10000  // IQ plot update
};

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
 *  at speed 1; for speed 1/2, divide this by 2, 1/4 by 4
 */
const int SIG_N_AVG=8;

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

const int MAX_W=800; // max pixmap width

typedef struct sampleSizes {
    unsigned long chunk_size;    // FFT length * size of 1 frame (L+R channel samples)
    unsigned long advance_size;  // bytes for one advance of spectrum
} sampleSizes;
Q_DECLARE_TYPEINFO(sampleSizes, Q_PRIMITIVE_TYPE);

// powers of 2
const unsigned int bits[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768 };

// band limits: used for automatic qsy between signals
const int band_limits[N_BANDS][2] = {{   1800000,  1900000 }, {  3500000,  3600000 }, { 7000000, 7125000 }, { 14000000, 14150000 },
                                      { 21000000, 21200000 }, { 28000000, 28300000 }, { 5330500, 5403500 }, { 10100000, 10150000 },
                                      { 18068000, 18110000 }, { 24890000, 24930000 }, {50000000,54000000 }, {144000000, 148000000 },
                                      {420000000, 450000000}, {222000000,225000000},{902000000,928000000},{1240000000,1300000000}};

//////// QSettings key names and default values used in so2sdr-bandmap.ini  ///////

const QString s_sdr_nrig="nrig";
const QString s_sdr_nrig_def=0;

const QString s_sdr_bandmap_tcp_port="bandmap_tcp_port";
const int s_sdr_bandmap_tcp_port_def=5001;

const QString s_sdr_bandmap_udp_port="bandmap_udp_port";
const int s_sdr_bandmap_udp_port_def=45454;

const QString s_sdr_type="type";
const SdrType s_sdr_type_def=soundcard_t;

const QString s_sdr_bits="bits";
const int s_sdr_bits_def=0;

const QString s_sdr_api="api";
const int s_sdr_api_def=0;

const QString s_sdr_fft="fft";
const int s_sdr_fft_def=4096;

const QString s_sdr_speed="speed";
const int s_sdr_speed_def=1;

const QString s_sdr_sound_speed="soundcard_speed";
const int s_sdr_sound_speed_def=1;

const QString s_sdr_net_speed="net_speed";
const int s_sdr_net_speed_def=1;

const QString s_sdr_afedri_speed="afedri_speed";
const int s_sdr_afedri_speed_def=1;

const QString s_sdr_sample_freq="sample_freq";
const int s_sdr_sample_freq_def=96000;

const QString s_sdr_net_sample_freq="net_sample_freq";
const int s_sdr_net_sample_freq_def=96000;

const QString s_sdr_sound_sample_freq="soundcard_sample_freq";
const int s_sdr_sound_sample_freq_def=96000;

const QString s_sdr_afedri_sample_freq="afedri_sample_freq";
const int s_sdr_afedri_sample_freq_def=100000;

const QString s_sdr_tcp_ip="ipaddress";
const QString s_sdr_tcp_ip_def="127.0.0.1";

const QString s_sdr_udp_ip="udp_ipaddress";
const QString s_sdr_udp_ip_def="127.0.0.1";

const QString s_sdr_tcp_port="tcp_port";
const int s_sdr_tcp_port_def=50000;

const QString s_sdr_udp_port="udp_port";
const int s_sdr_udp_port_def=50000;

const QString s_sdr_net_tcp_ip="net_ipaddress";
const QString s_sdr_net_tcp_ip_def="127.0.0.1";

const QString s_sdr_net_tcp_port="net_tcp_port";
const int s_sdr_net_tcp_port_def=50000;

const QString s_sdr_net_udp_port="net_udp_port";
const int s_sdr_net_udp_port_def=50000;

const QString s_sdr_device="device";
const int s_sdr_device_def=0;

const QString s_sdr_deviceindx="deviceindx";
const int s_sdr_deviceindx_def=0;

const QString s_sdr_offset_soundcard="offset_soundcard";
const int s_sdr_offset_soundcard_def=0;

const QString s_sdr_offset_afedri="offset_afedri";
const int s_sdr_offset_afedri_def=15000;

const QString s_sdr_afedri_tcp_ip="tcp_address_afedri";
const QString s_sdr_afedri_tcp_ip_def="127.0.0.1";

const QString s_sdr_afedri_tcp_port="tcp_port_afedri";
const int s_sdr_afedri_tcp_port_def=50000;

const QString s_sdr_afedri_udp_port="udp_port_afedri";
const int s_sdr_afedri_udp_port_def=50000;

const QString s_sdr_offset_network="offset_network";
const int s_sdr_offset_network_def=15000;

const QString s_sdr_afedri_channel="afedri_channel";
const int s_sdr_afedri_channel_def=0;

const QString s_sdr_afedri_multi="afedri_mode";
const int s_sdr_afedri_multi_def=0;

const QString s_sdr_afedri_bcast="afedri_bcast";
const int s_sdr_afedri_bcast_def=0;

const QString s_sdr_afedri_freq1="afedri_freq1";
// this is the IF frequency needed for a Elecraft K3
const int s_sdr_afedri_freq1_def=8225000;

const QString s_sdr_afedri_freq2="afedri_freq2";
const int s_sdr_afedri_freq2_def=8225000;

const QString s_sdr_afedri_freq3="afedri_freq3";
const int s_sdr_afedri_freq3_def=0;

const QString s_sdr_afedri_freq4="afedri_freq4";
const int s_sdr_afedri_freq4_def=0;

const QString s_sdr_offset="offset";
const int s_sdr_offset_def=0;

const QString s_sdr_swapiq="swapiq";
const bool s_sdr_swapiq_def=false;

const QString s_sdr_swap_afedri="swapiq_afedri";
const bool s_sdr_swap_afedri_def=false;

const QString s_sdr_swap_network="swapiq_network";
const bool s_sdr_swap_network_def=false;

const QString s_sdr_swap_soundcard="swapiq_soundcard";
const bool s_sdr_swap_soundcard_def=false;

const QString s_sdr_scale="scale";
const int s_sdr_scale_def=1;

const QString s_sdr_peakdetect="peakdetect";
const bool s_sdr_peakdetect_def=true;

const QString s_sdr_iqdata="iqdata";
const bool s_sdr_iqdata_def=true;

const QString s_sdr_iqcorrect="iqcorrect";
const bool s_sdr_iqcorrect_def=true;

const QString s_sdr_cqtime="cqtime";
const int s_sdr_cqtime_def=15;

const QString s_sdr_level="level";
const int s_sdr_level_def=100;

const QString s_sdr_n1mm="n1mm";
const bool s_sdr_n1mm_def=false;

const QString s_sdr_n1mm_port="n1mm_port";
const int s_sdr_n1mm_port_def=12060;

const int cqlimit_default_low[N_BANDS]={1805000,3505000,7005000,14005000,21005000,28005000, 5330500, 10100000,
                                       18068000, 24890000, 50000000, 144000000, 420000000,222000000,902000000,1240000000};

const int cqlimit_default_high[N_BANDS]={1845000,3555000,7550000,14055000,21055000,28055000, 5403500,10150000,
                                        18110000, 24930000,50100000,144100000,450000000,225000000,928000000,1300000000};

#endif // DEFINES_H
