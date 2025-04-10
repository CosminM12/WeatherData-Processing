#ifndef BINARY_H
#define BINARY_H

uint32_t readDateToTimestamp();

bool binaryMenu(const char *csvPath, const char *binaryPath, bool *loadedData);

int convertToBinary(const char *csvPath, const char *binaryPath);

void readHeader(const char *binaryPath);

void readRecordByIndex(const char *binaryPath);

bool verifyHeader(const char *binaryPath);

void readRecordInInterval(const char *binaryPath);

unsigned indexReader();

void addRecord(const char *binaryPath, WeatherRecord_t *newRecord);

void updateRecord(const char *binaryPath, WeatherRecord_t* newRecord);

void hardDeleteRecord(const char *binaryPath);


#endif