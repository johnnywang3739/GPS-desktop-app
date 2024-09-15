
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
    char signal_id[4];  // Signal ID as a string
    int signal_type = -1;  // Signal type (L1, L5, etc.)

    // Parse the sentence to identify if it belongs to L1 or L5
    if (strstr(sentence, "GPGSV,") != NULL) {
        // Check if the last character before '*' is '1' for L1 or '8' for L5
        char *checksum_ptr = strchr(start, '*');
        if (checksum_ptr != NULL && *(checksum_ptr - 1) == '1') {
            signal_type = 1;  // L1 Signal
        } else if (checksum_ptr != NULL && *(checksum_ptr - 1) == '8') {
            signal_type = 8;  // L5 Signal
        }
    }

    // Loop through the fields in the sentence
    while (end != NULL) {
        *end = '\0';

        switch (field) {
            case 0:
                // Talker ID (extract first two characters after '$')
                strncpy(data->talker_id, start + 1, 2);
                data->talker_id[2] = '\0';
                break;
            case 1:
                // Total number of sentences in this GSV message
                data->total_num_sentences = atoi(start);
                break;
            case 2:
                // Current sentence number
                data->current_sentence_number = atoi(start);
                break;
            case 3:
                // Total number of satellites in view
                data->total_num_sats_in_view = atoi(start);
                break;
            default:
                // Satellite fields start from field 4 onward
                if ((field - 4) % 4 == 0) {
                    // Parse Satellite ID
                    if (sat_index >= data->num_satellites) {
                        data->num_satellites++;
                        data->satellites = realloc(data->satellites, data->num_satellites * sizeof(SatelliteInfo));
                        if (data->satellites == NULL) {
                            printf("Memory allocation failed\n");
                            return;
                        }
                    }

                    if (*start != '\0') {
                        data->satellites[sat_index].sat_id = atoi(start);
                    } else {
                        data->satellites[sat_index].sat_id = -1;  // Missing sat_id
                    }

                    // Copy talker ID to satellite data
                    strncpy(data->satellites[sat_index].talker_id, data->talker_id, sizeof(data->satellites[sat_index].talker_id));

                } else if ((field - 5) % 4 == 0) {
                    // Parse elevation (or -1 if missing)
                    data->satellites[sat_index].elevation = (*start != '\0') ? atoi(start) : -1;

                } else if ((field - 6) % 4 == 0) {
                    // Parse azimuth (or -1 if missing)
                    data->satellites[sat_index].azimuth = (*start != '\0') ? atoi(start) : -1;

                } else if ((field - 7) % 4 == 0) {
                    // Parse SNR for the signal (L1 or L5)
                    int snr_value = (*start != '\0') ? atoi(start) : -1;

                    // Assign signal ID based on the signal type and GNSS type
                    if (strncmp(data->talker_id, "GP", 2) == 0) {
                        strncpy(signal_id, (signal_type == 1) ? "L1" : "L5", sizeof(signal_id));
                    } else if (strncmp(data->talker_id, "GL", 2) == 0) {
                        strncpy(signal_id, "L1", sizeof(signal_id));  // GLONASS only has L1
                    } else if (strncmp(data->talker_id, "GA", 2) == 0) {
                        strncpy(signal_id, (signal_type == 1) ? "E5a" : "E1", sizeof(signal_id));
                    } else if (strncmp(data->talker_id, "GB", 2) == 0) {
                        strncpy(signal_id, (signal_type == 1) ? "B1I" : "B2a", sizeof(signal_id));
                    }

                    // Find the satellite in the satellite data and update its signal information
                    int found = 0;
                    for (int j = 0; j < satellite_data->num_satellites; j++) {
                        if (satellite_data->satellites[j].sat_id == data->satellites[sat_index].sat_id) {
                            if (satellite_data->satellites[j].num_signals < 2) {
                                // Add signal data to the satellite
                                strncpy(satellite_data->satellites[j].signals[satellite_data->satellites[j].num_signals].signal_id, signal_id, sizeof(satellite_data->satellites[j].signals[satellite_data->satellites[j].num_signals].signal_id));
                                satellite_data->satellites[j].signals[satellite_data->satellites[j].num_signals].snr = snr_value;
                                satellite_data->satellites[j].num_signals++;
                            }
                            found = 1;
                            break;
                        }
                    }

                    if (!found && data->satellites[sat_index].sat_id != -1) {
                        // Add new satellite if it wasn't found
                        satellite_data->num_satellites++;
                        satellite_data->satellites = realloc(satellite_data->satellites, satellite_data->num_satellites * sizeof(SatelliteInfo));
                        if (satellite_data->satellites == NULL) {
                            printf("Memory allocation failed\n");
                            return;
                        }

                        satellite_data->satellites[satellite_data->num_satellites - 1] = data->satellites[sat_index];
                        strncpy(satellite_data->satellites[satellite_data->num_satellites - 1].signals[0].signal_id, signal_id, sizeof(satellite_data->satellites[satellite_data->num_satellites - 1].signals[0].signal_id));
                        satellite_data->satellites[satellite_data->num_satellites - 1].signals[0].snr = snr_value;
                        satellite_data->satellites[satellite_data->num_satellites - 1].num_signals = 1;
                    }

                    sat_index++;  // Move to the next satellite after processing 4 fields
                }
                break;
        }

        start = end + 1;
        end = strchr(start, ',');
        field++;
    }
}


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
void parseGNGSA(const char *sentence, GSA_Talker_Data *data) {
    char sentence_copy[UART_BUFFER_SIZE];
    strncpy(sentence_copy, sentence, UART_BUFFER_SIZE);
    sentence_copy[UART_BUFFER_SIZE - 1] = '\0';  // Ensure null-termination

    memset(data, 0, sizeof(GSA_Talker_Data));  // Clear the structure

    char *start = sentence_copy;
    char *end = strchr(start, ',');
    int field = 0;
    int sat_index = 0;

    while (end != NULL) {
        *end = '\0';  // Split the sentence at each comma
        switch (field) {
            case 1:  // Mode (A=Automatic, M=Manual)
                strncpy(data->mode, start, sizeof(data->mode) - 1);
                break;
            case 2:  // Fix Mode (1=No fix, 2=2D fix, 3=3D fix)
                strncpy(data->fix_mode, start, sizeof(data->fix_mode) - 1);
                break;
            case 3 ... 14:  // Satellite IDs (Up to 12)
                if (*start != '\0') {
                    // Convert satellite ID to integer and store as a string without leading zeros
                    int sat_id = atoi(start); // This will automatically handle leading zeros
                    snprintf(data->sat_ids[sat_index++], sizeof(data->sat_ids[0]), "%d", sat_id);
                    data->active_satellites++;  // Increment the count of active satellites
                }
                break;
            case 15:  // PDOP (Positional Dilution of Precision)
                strncpy(data->pdop, start, sizeof(data->pdop) - 1);
                break;
            case 16:  // HDOP (Horizontal Dilution of Precision)
                strncpy(data->hdop, start, sizeof(data->hdop) - 1);
                break;
            case 17:  // VDOP (Vertical Dilution of Precision)
                strncpy(data->vdop, start, sizeof(data->vdop) - 1);
                break;
        }

        start = end + 1;  // Move to the next field
        end = strchr(start, ',');
        field++;
    }

    char *checksum_start = strchr(start, '*');  // Find the start of the checksum
    if (checksum_start != NULL && checksum_start > start) {
        // The last number before the '*' is the System ID (Talker ID)
        char talker_id = *(checksum_start - 1);  // Get the character before '*'

        // Map the number to the System ID
        switch (talker_id) {
            case '1':  // GPS (GP)
                strncpy(data->system_id, "GP", sizeof(data->system_id) - 1);
                break;
            case '2':  // GLONASS (GL)
                strncpy(data->system_id, "GL", sizeof(data->system_id) - 1);
                break;
            case '3':  // Galileo (GA)
                strncpy(data->system_id, "GA", sizeof(data->system_id) - 1);
                break;
            case '4':  // Beidou (GB)
                strncpy(data->system_id, "GB", sizeof(data->system_id) - 1);
                break;
            default:
                strncpy(data->system_id, "UNKNOWN", sizeof(data->system_id) - 1);
                break;
        }
    }
}


