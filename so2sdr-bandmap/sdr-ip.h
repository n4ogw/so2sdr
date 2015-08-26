#ifndef SDRIP_H
#define SDRIP_H

// SDR-IP and Afedri TCP command definitions
//
// copied from "sdr-split" source v.6, http://www.afedri-sdr.com
// not completely sure if some are Afedri-specific

#define SET_CONTROL_ITEM                0
#define REQUEST_CONTROL_ITEM            1
#define REQUEST_CONTROL_ITEM_RANGE      2
#define DATA_ITEM_ACK                   3
#define DATA_ITEM_0                     4
#define DATA_ITEM_1                     5
#define DATA_ITEM_2                     6
#define DATA_ITEM_3                     7
#define CI_TARGET_NAME                  0x0001
#define CI_TARGET_SERIAL_NUMBER         0x0002
#define CI_INTERFACE_VERSION            0x0003
#define CI_HW_FW_VERSION                0x0004
#define CI_STATUS_ERROR_CODE            0x0005
#define CI_PRODUCT_ID                   0x0009
#define CI_RECEIVER_STATE               0x0018
 #define RCV_START   2
 #define RCV_STOP    1
#define CI_FREQUENCY                    0x0020
#define CI_RF_GAIN                      0x0038
#define CI_AF_GAIN                      0x0048
#define CI_RF_FILTER_SELECT             0x0044
#define CI_AD_MODES                     0x008A
#define CI_INPUT_SYNC_MODES             0x00B4
#define CI_DDC_SAMPLE_RATE              0x00B8
#define CI_SAMPLE_RATE_CALIBRATION      0x00B0
#define CI_CALIBRATION_DATA             0x00D0
#define CI_PULSE_OUTPUT_MODE            0x00B6
#define CI_DA_OUTPUT_MODE               0x012A
#define CI_DATA_PACKET_SIZE             0x00C4
#define CI_UDP_IP_PORT                  0x00C5
#define CI_RS232_OPEN                   0x0200
#define CI_RS232_CLOSE                  0x0201

#define CI_RF_ATT_0DB                   0x00
#define CI_RF_ATT_10DB                  0xF6
#define CI_RF_ATT_20DB                  0xEC
#define CI_RF_ATT_30DB                  0xE2

#define SDR_IDLE  0x0B
#define SDR_BUSY  0x0B

#endif // SDRIP_H
