
/*
 * GPSUtils.c
 *
 * Created on: Jul 24, 2024
 * Author: JohnnyWang
 */

#include "GPSUtils.h"
#include <stdio.h>


void parseGSV(const char *sentence, GSV_Data *data, SatelliteData *satellite_data) {
    char sentence_copy[UART_BUFFER_SIZE];
    strncpy(sentence_copy, sentence, UART_BUFFER_SIZE);
    sentence_copy[UART_BUFFER_SIZE - 1] = '\0';

    char *start = sentence_copy;
    char *end = strchr(start, ',');
    int field = 0;
    int sat_index = 0;

    // Resetting the number of satellites to 0
    data->num_satellites = 0;

    while (end != NULL) {
        *end = '\0';

        switch (field) {
            case 0:
                strncpy(data->talker_id, start + 1, 2);
                data->talker_id[2] = '\0';
                break;
            case 1:
                data->total_num_sentences = atoi(start);
                break;
            case 2:
                data->current_sentence_number = atoi(start);
                break;
            case 3:
                data->total_num_sats_in_view = atoi(start);
                break;
            default:
                // Satellite fields start from field 4 onward
                if ((field - 4) % 4 == 0) {
                    // Parsing SatID
                    if (sat_index >= data->num_satellites) {
                        data->num_satellites++;
                        data->satellites = realloc(data->satellites, data->num_satellites * sizeof(SatelliteInfo));
                    }
                    data->satellites[sat_index].sat_id = atoi(start);
                    strncpy(data->satellites[sat_index].talker_id, data->talker_id, sizeof(data->satellites[sat_index].talker_id));
                } else if ((field - 5) % 4 == 0) {
                    // Parsing Elevation
                    data->satellites[sat_index].elevation = atoi(start);
                } else if ((field - 6) % 4 == 0) {
                    // Parsing Azimuth
                    data->satellites[sat_index].azimuth = atoi(start);
                } else if ((field - 7) % 4 == 0) {
                    // Parsing SNR
                    data->satellites[sat_index].snr = atoi(start);
                    sat_index++;
                }
                break;
        }

        start = end + 1;
        end = strchr(start, ',');
        field++;
    }

//
//    printf("GSV Parsing Debug:\r\n");
//    printf("Total Sentences: %d\r\n", data->total_num_sentences);
//    printf("Current Sentence: %d\r\n", data->current_sentence_number);
//    printf("Total Satellites in View: %d\r\n", data->total_num_sats_in_view);
//    printf("Parsed Satellites: %d\r\n", data->num_satellites);

//    for (int i = 0; i < data->num_satellites; i++) {
//        printf("%s Satellite %d - ID: %d, Elevation: %d, Azimuth: %d, SNR: %d\r\n",
//        		data->satellites[i].talker_id, i, data->satellites[i].sat_id, data->satellites[i].elevation,
//               data->satellites[i].azimuth, data->satellites[i].snr);
//    }

    // Update the satellite data structure
    if (data->num_satellites > 0) {
        for (int i = 0; i < data->num_satellites; i++) {
            int found = 0;
            for (int j = 0; j < satellite_data->num_satellites; j++) {
                if (satellite_data->satellites[j].sat_id == data->satellites[i].sat_id) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                satellite_data->num_satellites++;
                satellite_data->satellites = realloc(satellite_data->satellites, satellite_data->num_satellites * sizeof(SatelliteInfo));
                satellite_data->satellites[satellite_data->num_satellites - 1] = data->satellites[i];
            }
        }
    }
}
//


void parseGNGGA(const char *sentence, GNGGA_Data *data) {

    char sentence_copy[UART_BUFFER_SIZE];
    strncpy(sentence_copy, sentence, UART_BUFFER_SIZE);
    sentence_copy[UART_BUFFER_SIZE - 1] = '\0';
    memset(data, 0, sizeof(GNGGA_Data));
    strncpy(data->utc_time, " ", sizeof(data->utc_time));
    strncpy(data->latitude, "  ", sizeof(data->latitude));
    strncpy(data->ns_indicator, " ", sizeof(data->ns_indicator));
    strncpy(data->longitude, "  ", sizeof(data->longitude));
    strncpy(data->ew_indicator, " ", sizeof(data->ew_indicator));
    strncpy(data->quality, " ", sizeof(data->quality));
    strncpy(data->num_satellites, " ", sizeof(data->num_satellites));
    strncpy(data->hdop, "     ", sizeof(data->hdop));
    strncpy(data->altitude, " ", sizeof(data->altitude));
    strncpy(data->alt_units, " ", sizeof(data->alt_units));
    strncpy(data->geoidal_separation, " ", sizeof(data->geoidal_separation));
    strncpy(data->sep_units, " ", sizeof(data->sep_units));
    strncpy(data->diff_age, "  ", sizeof(data->diff_age));
    strncpy(data->diff_station, "  ", sizeof(data->diff_station));
    strncpy(data->checksum, "  ", sizeof(data->checksum));

    char *start = sentence_copy;
    char *end = strchr(start, ',');
    int field = 0;

    while (end != NULL) {
        *end = '\0';
        switch (field) {
            case 1:
                strncpy(data->utc_time, start, sizeof(data->utc_time) - 1);
                break;
            case 2:
                strncpy(data->latitude, start, sizeof(data->latitude) - 1);
                break;
            case 3:
                strncpy(data->ns_indicator, start, sizeof(data->ns_indicator) - 1);
                break;
            case 4:
                strncpy(data->longitude, start, sizeof(data->longitude) - 1);
                break;
            case 5:
                strncpy(data->ew_indicator, start, sizeof(data->ew_indicator) - 1);
                break;
            case 6:
                strncpy(data->quality, start, sizeof(data->quality) - 1);
                break;
            case 7:
                strncpy(data->num_satellites, start, sizeof(data->num_satellites) - 1);
                break;
            case 8:
                strncpy(data->hdop, start, sizeof(data->hdop) - 1);
                break;
            case 9:
                strncpy(data->altitude, start, sizeof(data->altitude) - 1);
                break;
            case 10:
                strncpy(data->alt_units, start, sizeof(data->alt_units) - 1);
                break;
            case 11:
                strncpy(data->geoidal_separation, start, sizeof(data->geoidal_separation) - 1);
                break;
            case 12:
                strncpy(data->sep_units, start, sizeof(data->sep_units) - 1);
                break;
            case 13:
                strncpy(data->diff_age, start, sizeof(data->diff_age) - 1);
                break;
            case 14:
                strncpy(data->diff_station, start, sizeof(data->diff_station) - 1);
                break;
        }
        start = end + 1;
        end = strchr(start, ',');
        field++;
    }

    if (*start == '*') {
        start++;
        strncpy(data->checksum, start, 2);
    }
}

void parseGNRMC(const char *sentence, GNRMC_Data *data) {
    char sentence_copy[UART_BUFFER_SIZE];
    strncpy(sentence_copy, sentence, UART_BUFFER_SIZE);
    sentence_copy[UART_BUFFER_SIZE - 1] = '\0';

    memset(data, 0, sizeof(GNRMC_Data));
    strncpy(data->utc_time, " ", sizeof(data->utc_time));
    strncpy(data->status, " ", sizeof(data->status));
    strncpy(data->latitude, "  ", sizeof(data->latitude));
    strncpy(data->ns_indicator, " ", sizeof(data->ns_indicator));
    strncpy(data->longitude, "  ", sizeof(data->longitude));
    strncpy(data->ew_indicator, " ", sizeof(data->ew_indicator));
    strncpy(data->speed_over_ground, " ", sizeof(data->speed_over_ground));
    strncpy(data->course_over_ground, " ", sizeof(data->course_over_ground));
    strncpy(data->date, "      ", sizeof(data->date));
    strncpy(data->mag_variation, " ", sizeof(data->mag_variation));
    strncpy(data->mag_var_direction, " ", sizeof(data->mag_var_direction));
    strncpy(data->mode_indicator, " ", sizeof(data->mode_indicator));
    strncpy(data->nav_status, " ", sizeof(data->nav_status));
    strncpy(data->checksum, "00", sizeof(data->checksum));

    char *start = sentence_copy;
    char *end = strchr(start, ',');
    int field = 0;
    while (end != NULL) {
        *end = '\0';
        switch (field) {
            case 1:
                strncpy(data->utc_time, start, sizeof(data->utc_time) - 1);
                break;
            case 2:
                strncpy(data->status, start, sizeof(data->status) - 1);
                break;
            case 3:
                strncpy(data->latitude, start, sizeof(data->latitude) - 1);
                break;
            case 4:
                strncpy(data->ns_indicator, start, sizeof(data->ns_indicator) - 1);
                break;
            case 5:
                strncpy(data->longitude, start, sizeof(data->longitude) - 1);
                break;
            case 6:
                strncpy(data->ew_indicator, start, sizeof(data->ew_indicator) - 1);
                break;
            case 7:
                strncpy(data->speed_over_ground, start, sizeof(data->speed_over_ground) - 1);
                break;
            case 8:
                strncpy(data->course_over_ground, start, sizeof(data->course_over_ground) - 1);
                break;
            case 9:
                strncpy(data->date, start, sizeof(data->date) - 1);
                break;
            case 10:
                strncpy(data->mag_variation, start, sizeof(data->mag_variation) - 1);
                break;
            case 11:
                strncpy(data->mag_var_direction, start, sizeof(data->mag_var_direction) - 1);
                break;
            case 12:
                strncpy(data->mode_indicator, start, sizeof(data->mode_indicator) - 1);
                break;
            case 13:
                strncpy(data->nav_status, start, sizeof(data->nav_status) - 1);
                break;
        }
        start = end + 1;
        end = strchr(start, ',');
        field++;
    }

    char *checksum_ptr = strchr(start, '*');
    if (checksum_ptr) {
        *checksum_ptr = '\0';
        strncpy(data->nav_status, start, sizeof(data->nav_status) - 1);
        checksum_ptr++;
        strncpy(data->checksum, checksum_ptr, 2);
    }
}


void parseGNGLL(const char *sentence, GNGLL_Data *data) {
    char sentence_copy[UART_BUFFER_SIZE];
    strncpy(sentence_copy, sentence, UART_BUFFER_SIZE);
    sentence_copy[UART_BUFFER_SIZE - 1] = '\0';

    memset(data, 0, sizeof(GNGLL_Data));
    strncpy(data->latitude, " ", sizeof(data->latitude));
    strncpy(data->ns_indicator, " ", sizeof(data->ns_indicator));
    strncpy(data->longitude, " ", sizeof(data->longitude));
    strncpy(data->ew_indicator, " ", sizeof(data->ew_indicator));
    strncpy(data->utc_time, " ", sizeof(data->utc_time));
    strncpy(data->status, " ", sizeof(data->status));
    strncpy(data->mode_indicator, " ", sizeof(data->mode_indicator));
    strncpy(data->checksum, "00", sizeof(data->checksum));

    char *start = sentence_copy;
    char *end = strchr(start, ',');
    int field = 0;

    while (end != NULL) {
        *end = '\0';
        switch (field) {
            case 1:
                strncpy(data->latitude, start, sizeof(data->latitude) - 1);
                break;
            case 2:
                strncpy(data->ns_indicator, start, sizeof(data->ns_indicator) - 1);
                break;
            case 3:
                strncpy(data->longitude, start, sizeof(data->longitude) - 1);
                break;
            case 4:
                strncpy(data->ew_indicator, start, sizeof(data->ew_indicator) - 1);
                break;
            case 5:
                strncpy(data->utc_time, start, sizeof(data->utc_time) - 1);
                break;
            case 6:
                strncpy(data->status, start, sizeof(data->status) - 1);
                break;
            case 7:
                strncpy(data->mode_indicator, start, sizeof(data->mode_indicator) - 1);
                break;
        }
        start = end + 1;
        end = strchr(start, ',');
        field++;
    }
    char *checksum_ptr = strchr(start, '*');
    if (checksum_ptr) {
        *checksum_ptr = '\0';
        strncpy(data->mode_indicator, start, sizeof(data->mode_indicator) - 1);
        checksum_ptr++;
        strncpy(data->checksum, checksum_ptr, 2);
    }
}

void parseGNVTG(const char *sentence, GNVTG_Data *data) {
    char sentence_copy[UART_BUFFER_SIZE];
    strncpy(sentence_copy, sentence, UART_BUFFER_SIZE);
    sentence_copy[UART_BUFFER_SIZE - 1] = '\0';

    memset(data, 0, sizeof(GNVTG_Data));
    strncpy(data->cogt, " ", sizeof(data->cogt));
    strncpy(data->cogt_indicator, " ", sizeof(data->cogt_indicator));
    strncpy(data->cogm, " ", sizeof(data->cogm));
    strncpy(data->cogm_indicator, " ", sizeof(data->cogm_indicator));
    strncpy(data->sogn, " ", sizeof(data->sogn));
    strncpy(data->sogn_indicator, " ", sizeof(data->sogn_indicator));
    strncpy(data->sogk, " ", sizeof(data->sogk));
    strncpy(data->sogk_indicator, " ", sizeof(data->sogk_indicator));
    strncpy(data->mode_indicator, " ", sizeof(data->mode_indicator));
    strncpy(data->checksum, "00", sizeof(data->checksum));

    char *start = sentence_copy;
    char *end = strchr(start, ',');
    int field = 0;

    while (end != NULL) {
        *end = '\0';
        switch (field) {
            case 1:
                strncpy(data->cogt, start, sizeof(data->cogt) - 1);
                break;
            case 2:
                strncpy(data->cogt_indicator, start, sizeof(data->cogt_indicator) - 1);
                break;
            case 3:
                strncpy(data->cogm, start, sizeof(data->cogm) - 1);
                break;
            case 4:
                strncpy(data->cogm_indicator, start, sizeof(data->cogm_indicator) - 1);
                break;
            case 5:
                strncpy(data->sogn, start, sizeof(data->sogn) - 1);
                break;
            case 6:
                strncpy(data->sogn_indicator, start, sizeof(data->sogn_indicator) - 1);
                break;
            case 7:
                strncpy(data->sogk, start, sizeof(data->sogk) - 1);
                break;
            case 8:
                strncpy(data->sogk_indicator, start, sizeof(data->sogk_indicator) - 1);
                break;
            case 9:
                strncpy(data->mode_indicator, start, sizeof(data->mode_indicator) - 1);
                break;
        }
        start = end + 1;
        end = strchr(start, ',');
        field++;
    }
    char *checksum_ptr = strchr(start, '*');
    if (checksum_ptr) {
        *checksum_ptr = '\0';
        strncpy(data->mode_indicator, start, sizeof(data->mode_indicator) - 1);
        checksum_ptr++;
        strncpy(data->checksum, checksum_ptr, 2);
    }
}
