#include "gps.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

double convert_to_decimal(char *nmea_coord, char indicator) {
    double raw_coord = atof(nmea_coord);
    int degrees = (int)(raw_coord / 100);
    double minutes = raw_coord - (degrees * 100);
    double decimal_coord = degrees + (minutes / 60.0);
    if (indicator == 'S' || indicator == 'W') {
        decimal_coord *= -1;
    }
    return decimal_coord;
}

void format_time(char *nmea_time, char *formatted_time) {
    int hours, minutes, seconds;
    sscanf(nmea_time, "%2d%2d%2d", &hours, &minutes, &seconds);
    sprintf(formatted_time, "%02d:%02d:%02d", hours, minutes, seconds);
}

bool parse_nmea_sentence(char *nmea_sentence, gps_data_t *data) {
    if (strstr(nmea_sentence, "$GPGGA") != NULL) {
        char time[11] = {0}, latitude[11] = {0}, ns = '\0', longitude[12] = {0}, ew = '\0', quality = '\0', satellites[3] = {0}, hdop[5] = {0}, altitude[7] = {0}, altitude_unit[2] = {0}, geoid_sep[7] = {0}, geoid_sep_unit[2] = {0}, diff_age[7] = {0}, diff_station[5] = {0};

        int result = sscanf(nmea_sentence, "$GPGGA,%10[^,],%10[^,],%c,%11[^,],%c,%c,%2[^,],%4[^,],%6[^,],%1[^,],%6[^,],%1[^,],%6[^,],%4[^,]",
                            time, latitude, &ns, longitude, &ew, &quality, satellites, hdop, altitude, altitude_unit, geoid_sep, geoid_sep_unit, diff_age, diff_station);

        if (result >= 10) {
            format_time(time, data->time);
            data->latitude = convert_to_decimal(latitude, ns);
            data->longitude = convert_to_decimal(longitude, ew);
            data->fix_quality = quality;
            strcpy(data->satellites, satellites);
            strcpy(data->hdop, hdop);
            strcpy(data->altitude, altitude);
            strcpy(data->altitude_unit, altitude_unit);
            if (result >= 12) {
                strcpy(data->geoid_sep, geoid_sep);
                strcpy(data->geoid_sep_unit, geoid_sep_unit);
            }
            return true;
        } else {
            return false;
        }
    }
    return false;
}

void init_uart() {
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD2, RXD2, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void init_gps() {
    init_uart();
}

bool read_data(gps_data_t *data) {
    uint8_t buffer[BUF_SIZE];
    int len = uart_read_bytes(UART_NUM_1, buffer, BUF_SIZE, 100 / portTICK_PERIOD_MS);
    if (len > 0) {
        for (int i = 0; i < len; ++i) {
            if (buffer[i] == '$') {
                char nmea_sentence[82] = {0};
                int j = 0;
                while (i < len && buffer[i] != '\n' && j < sizeof(nmea_sentence) - 1) {
                    nmea_sentence[j++] = buffer[i++];
                }
                nmea_sentence[j] = '\0';
                if(parse_nmea_sentence(nmea_sentence, data)){
                    return true;
                }
            }
        }
        return false;
    }
    return false;
}
