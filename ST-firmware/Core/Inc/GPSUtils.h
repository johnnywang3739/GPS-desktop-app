#ifndef INC_GPSUTILS_H_
#define INC_GPSUTILS_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define UART_BUFFER_SIZE 2048

typedef struct {
    char utc_time[11];
    char latitude[12];
    char ns_indicator[2];
    char longitude[13];
    char ew_indicator[2];
    char quality[2];
    char num_satellites[3];
    char hdop[6];
    char altitude[8];
    char alt_units[2];
    char geoidal_separation[8];
    char sep_units[2];
    char diff_age[10];
    char diff_station[10];
    char checksum[3];
} GNGGA_Data;

typedef struct {
    char utc_time[11];
    char status[2];
    char latitude[12];
    char ns_indicator[2];
    char longitude[13];
    char ew_indicator[2];
    char speed_over_ground[7];
    char course_over_ground[7];
    char date[7];
    char mag_variation[7];
    char mag_var_direction[2];
    char mode_indicator[2];
    char nav_status[2];
    char checksum[3];
} GNRMC_Data;

typedef struct {
    char latitude[12];
    char ns_indicator[2];
    char longitude[13];
    char ew_indicator[2];
    char utc_time[11];
    char status[2];
    char mode_indicator[2];
    char checksum[3];
} GNGLL_Data;

typedef struct {
    char cogt[7];
    char cogt_indicator[2];
    char cogm[7];
    char cogm_indicator[2];
    char sogn[7];
    char sogn_indicator[2];
    char sogk[7];
    char sogk_indicator[2];
    char mode_indicator[2];
    char checksum[3];
} GNVTG_Data;
typedef struct {
    char signal_id[4];  // Signal type as a string (e.g., "L1", "L5", etc.)
    int snr;            // Signal-to-noise ratio for this signal type
} SatelliteSignal;

typedef struct {
    int sat_id;                     // Satellite ID (e.g., 10, 15, etc.)
    int elevation;                  // Satellite elevation
    int azimuth;                    // Satellite azimuth
    SatelliteSignal signals[2];     // Up to 2 signal types per satellite (e.g., L1 and L5)
    int num_signals;                // Number of signals for this satellite (up to 2)
    char talker_id[3];              // Talker ID (e.g., GP for GPS)
} SatelliteInfo;

typedef struct {
    SatelliteInfo *satellites;  // Dynamically allocated array of satellites
    int num_satellites;         // Number of unique satellites
} SatelliteData;

typedef struct {
    char talker_id[3];
    int total_num_sentences;
    int current_sentence_number;
    int total_num_sats_in_view;
    SatelliteInfo *satellites;  // Pointer to dynamically allocated array of satellites
    int num_satellites;         // Number of satellites in the current sentence
} GSV_Data;

typedef struct {
    char mode[2];               // Mode (A=Automatic, M=Manual)
    char fix_mode[2];           // Fix Mode (1=No fix, 2=2D fix, 3=3D fix)
    char system_id[3];          // System ID (GP, GL, GA, etc.), updated to 3 characters
    char sat_ids[12][4];        // Satellite IDs used in the solution (up to 12)
    char pdop[6];               // PDOP (Positional Dilution of Precision)
    char hdop[6];               // HDOP (Horizontal Dilution of Precision)
    char vdop[6];               // VDOP (Vertical Dilution of Precision)
    int active_satellites;       // Count of active satellites for this talker
} GSA_Talker_Data;

extern SatelliteData satellite_data;

void parseGNGGA(const char *sentence, GNGGA_Data *data);
void parseGNRMC(const char *sentence, GNRMC_Data *data);
void parseGNGLL(const char *sentence, GNGLL_Data *data);
void parseGNVTG(const char *sentence, GNVTG_Data *data);
void parseGSV(const char *sentence, GSV_Data *data, SatelliteData *satellite_data);
void parseGNGSA(const char *sentence, GSA_Talker_Data *data);

#endif /* INC_GPSUTILS_H_ */
