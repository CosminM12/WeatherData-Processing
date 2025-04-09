#ifndef FORMAT_H
#define FORMAT_H

#define MAGIC_NUMBER "WBIN"
#define MAGIC_SIZE 4
#define VERSION 1
#define CITI_NAME "Timisoara"
#define LAT 45.76
#define LONG 21.23

#include <stdint.h>
#include <time.h>

typedef struct {
    char magic[4];
    uint16_t version;
    uint32_t timestamp;
    uint32_t recordCount;
    char city[51];
    double latitude;      
    double longitude;     
} __attribute__((packed)) FileHeader_t;

typedef struct {
    time_t timestamp;
    char dt_iso[30];
    int timezone;
    char city_name[50];
    double lat, lon;
    double temp, visibility, dew_point, feels_like, temp_min, temp_max;
    int pressure, sea_level, grnd_level, humidity;
    double wind_speed;
    int wind_deg;
    double wind_gust, rain_1h, rain_3h, snow_1h, snow_3h;
    int clouds_all, weather_id;
    char weather_main[20];
    char weather_description[50];
    char weather_icon[5];

} Weather_t;

typedef struct {
    uint32_t timestamp;
    float temp, visibility/*, feels_like*/;
    int16_t feels_like;
    // int16_t temp;
    uint16_t pressure;
    uint8_t humidity;
    float wind_speed;
    uint16_t wind_deg;
    uint8_t clouds;
    uint16_t weather_id;
    uint8_t weather_type;
} __attribute__((packed)) WeatherRecord_t;

#endif