/*
 * GPSUtils.h
 *
 *  Created on: Jul 16, 2024
 *      Author: JohnnyWang
 */

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
} GNGGA_Data; // please refer to the data sheet, this is the same as data sheet.


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
} GNRMC_Data; // please refer to the data sheet, this is the same as data sheet.

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
    int sat_id;
    int elevation;
    int azimuth;
    int snr;
    char talker_id[3];
} SatelliteInfo;

typedef struct {
    char talker_id[3];
    int total_num_sentences;
    int current_sentence_number;
    int total_num_sats_in_view;
    SatelliteInfo *satellites;  // Pointer to dynamically allocated array of satellites
    int num_satellites;         // Number of satellites in the current sentence
} GSV_Data;

typedef struct {
    SatelliteInfo *satellites;  // Pointer to dynamically allocated array of satellites
    int num_satellites;         // Number of unique satellites in the PPS
} SatelliteData;

extern SatelliteData satellite_data;

//void parseGSV(const char *sentence, GSV_Data *data);
void parseGNGGA(const char *sentence, GNGGA_Data *data);
void parseGNRMC(const char *sentence, GNRMC_Data *data);
void parseGNGLL(const char *sentence, GNGLL_Data *data);
void parseGNVTG(const char *sentence, GNVTG_Data *data);
void parseGSV(const char *sentence, GSV_Data *data, SatelliteData *satellite_data);
//parseGSV
#endif /* INC_GPSUTILS_H_ */
