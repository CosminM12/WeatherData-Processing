#ifndef WEATHER_H
#define WEATHER_H

#include "Format.h"

int read_csv(FILE *file, Weather_t **data, int *size);

void readDates(int *year, int *month, int *day);

int findFirstDateOcc(Weather_t *data, int size, int year, int month, int day);

int findLastDateOcc(Weather_t *data, int size, int year, int month, int day);

void findDailyTemp(Weather_t *data, int size, int year, int month, int day);

void display_statistics(Weather_t *data, int size);

void data_filtering(Weather_t *data, int size);

void show_visuals(Weather_t *data, int size);

#endif