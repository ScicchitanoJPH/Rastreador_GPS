#ifndef GPS_H
#define GPS_H

#include "driver/uart.h"

#define RXD2 16
#define TXD2 17
#define BUF_SIZE (1024)

typedef struct {
    char time[9];
    double latitude;
    double longitude;
    char fix_quality;
    char satellites[3];
    char hdop[5];
    char altitude[7];
    char altitude_unit[2];
    char geoid_sep[7];
    char geoid_sep_unit[2];
} gps_data_t;

void init_gps();
bool read_data(gps_data_t *data);

#endif // GPS_H
