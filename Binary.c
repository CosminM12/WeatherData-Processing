#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>

#include "Format.h"
#include "Binary.h"

uint32_t readDateToTimestamp() {
    int hour, day, month, year;

    struct tm readDate = {0};
    time_t timestamp;

    struct tm min_tm = {.tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 79};
    struct tm max_tm = {.tm_sec = 0, .tm_min = 0, .tm_hour = 23, .tm_mday = 23, .tm_mon = 10, .tm_year = 124};

    time_t min_timestamp = mktime(&min_tm);
    time_t max_timestamp = mktime(&max_tm);

    char buffer[20];
    bool valid = false;

    while(!valid) {
        printf("Enter a date (dd/mm/yyyy hh): ");
        if(!fgets(buffer, sizeof(buffer), stdin)) {
            printf("Invalid input. Please try again!\n");
            continue;
        }

        if(sscanf(buffer, "%d/%d/%d %d", &day, &month, &year, &hour) != 4) {
            printf("Invalid format! Please try again!\n");
            continue;
        }

        if(day < 1 || day > 31 || month < 1 || month > 12 || year < 1979 || year > 2024 || hour < 0 || hour > 23) {
            printf("Invalid date values! Please try again!\n");
            continue;
        }

        readDate.tm_mday = day;
        readDate.tm_mon = month - 1;
        readDate.tm_year = year - 1900;
        readDate.tm_sec = 0;
        readDate.tm_min = 0;
        readDate.tm_hour = hour;
        readDate.tm_isdst = -1;

        timestamp = mktime(&readDate);

        if(timestamp < min_timestamp || timestamp > max_timestamp) {
            printf("Date must be between 01/01/1979 00:00:00 and 23/11/2024 23:00:00.\n");
            continue;
        }
        
        valid = true;
    }

    return (uint32_t)timestamp;
}


bool binaryMenu(const char *csvPath, const char *binaryPath, bool *loadedData) {
    char input[20];
    int option = 0;
    bool valid = false;
    
    WeatherRecord_t newRecord = {.timestamp=290307600, .temp=10.87, .visibility=-1, .feels_like=-6, .pressure=1008, .humidity=69, .wind_speed=2.09, .wind_deg=170, .clouds=39, .weather_id=802, .weather_type=1};

    printf("\nWelcome to the binary functions menu. Choose an option:\n");

    while(!valid) {
        printf("1. Convert csv file to binary\n");
        printf("2. Read and display the file header\n");
        printf("3. Read record by index\n");
        printf("4. Verify the file's integrity\n");
        printf("5. Find records by date/time range\n");
        printf("6. Find records by weather condition\n");
        printf("7. Add new record\n");
        printf("8. Update existing record\n");
        printf("9. Delete record\n");
        printf("10. Exit\n");
        printf("Anwer: ");

        if(!fgets(input, sizeof(input), stdin)) {
            printf("Invalid input. Please try again!\n");
            continue;
        }

        input[strcspn(input, "\n")] = 0;

        char *ptr;
        option = strtol(input, &ptr, 10);

        if(*ptr != '\0') {
            printf("Invalid format. Please try again!\n");
            continue;
        }

        if(option < 1 || option > 10) {
            printf("Number must be between 1 and 12. Please try again!\n");
            continue;
        }
        if(!(*loadedData) && (option > 1 && option < 10)) {
            printf("You need to load the data to the binary file first!\n");
            continue;
        }

        valid = true;
    }

    switch(option) {
        case 1:
            int resp = convertToBinary(csvPath, binaryPath);
            if(resp == 1) {
                *loadedData = true;
            }
            break;
        case 2:
            readHeader(binaryPath);
            break;
        case 3:
            readRecordByIndex(binaryPath);
            break;
        case 4:
            verifyHeader(binaryPath);
            break;
        case 5:
            readRecordInInterval(binaryPath);
            break;
        case 7:
            addRecord(binaryPath, &newRecord);
            break;
        case 8:
            updateRecord(binaryPath, &newRecord);
            break;
        case 9:
            hardDeleteRecord(binaryPath);
            break;
        case 10:
            break;
    }

    if(option != 10) {
        return true;
    }
    return false;
    
}


int convertToBinary(const char *csvPath, const char *binaryPath) {
    FILE *csvFile = fopen(csvPath, "r");
    if(!csvFile) {
        perror("Error opening the csv file!\n");
        return -1;
    }

    FILE *binaryFile = fopen(binaryPath, "wb");
    if(!binaryFile) {
        perror("Error opening the binary file!\n");
        return -1;
    }

    FileHeader_t header;
    memcpy(header.magic, MAGIC_NUMBER, MAGIC_SIZE);
    // strcpy(header.magic, MAGIC_NUMBER);
    header.version = VERSION;
    header.timestamp = (uint32_t)time(NULL);
    header.recordCount = 0;
    strncpy(header.city, CITI_NAME, 51);
    header.latitude = LAT;
    header.longitude = LONG;

    if(fwrite(&header, sizeof(FileHeader_t), 1, binaryFile) != 1) {
        perror("Error writing binary header!\n");
        fclose(csvFile);
        fclose(binaryFile);
        return -1;
    }


    Weather_t record;
    int count = 0;
    char line[2048];
    

    while (fgets(line, sizeof(line), csvFile) != NULL) {
        char *token = line;
        int column = 0;

        while (column < 28) {
            char *next_token = strchr(token, ',');
            if (next_token) {
                //Replace comma with '\0' and get the position of the next char(next filed start)
                *next_token = '\0'; 
                next_token++;
            }
            
            // Handle empty fields
            switch (column) {
                case 0:  record.timestamp = *token ? atol(token) : -1; break;
                case 1:  strncpy(record.dt_iso, *token ? token : "", sizeof(record.dt_iso) - 1); break;
                case 2:  record.timezone = *token ? atoi(token) : -1; break;
                case 3:  strncpy(record.city_name, *token ? token : "", sizeof(record.city_name) - 1); break;
                case 4:  record.lat = *token ? atof(token) : -1; break;
                case 5:  record.lon = *token ? atof(token) : -1; break;
                case 6:  record.temp = *token ? atof(token) : -1; break;
                case 7:  record.visibility = *token ? atof(token) : -1; break;
                case 8:  record.dew_point = *token ? atof(token) : -1; break;
                case 9:  record.feels_like = *token ? atof(token) : -1; break;
                case 10: record.temp_min = *token ? atof(token) : -1; break;
                case 11: record.temp_max = *token ? atof(token) : -1; break;
                case 12: record.pressure = *token ? atoi(token) : -1; break;
                case 13: record.sea_level = *token ? atoi(token) : -1; break;
                case 14: record.grnd_level = *token ? atoi(token) : -1; break;
                case 15: record.humidity = *token ? atoi(token) : -1; break;
                case 16: record.wind_speed = *token ? atof(token) : -1; break;
                case 17: record.wind_deg = *token ? atoi(token) : -1; break;
                case 18: record.wind_gust = *token ? atof(token) : -1; break;
                case 19: record.rain_1h = *token ? atof(token) : -1; break;
                case 20: record.rain_3h = *token ? atof(token) : -1; break;
                case 21: record.snow_1h = *token ? atof(token) : -1; break;
                case 22: record.snow_3h = *token ? atof(token) : -1; break;
                case 23: record.clouds_all = *token ? atoi(token) : -1; break;
                case 24: record.weather_id = *token ? atoi(token) : -1; break;
                case 25: strncpy(record.weather_main, *token ? token : "Unknown", sizeof(record.weather_main) - 1); break;
                case 26: strncpy(record.weather_description, *token ? token : "Unknown", sizeof(record.weather_description) - 1); break;
                case 27: strncpy(record.weather_icon, *token ? token : "N/A", sizeof(record.weather_icon) - 1); break;
            }

            //Go to the next column
            token = next_token;
            column++;
        }

        WeatherRecord_t optimizedRecord;

        //Copy integer values
        optimizedRecord.timestamp = record.timestamp;
        optimizedRecord.pressure = record.pressure;
        optimizedRecord.humidity = record.humidity;
        optimizedRecord.wind_deg = record.wind_deg;
        optimizedRecord.clouds = record.clouds_all;

        //Copy floats
        optimizedRecord.temp = record.temp;
        optimizedRecord.visibility = record.visibility;
        // optimizedRecord.feels_like = record.feels_like;

        double realToFeel = (record.feels_like - record.temp)*10;
        optimizedRecord.feels_like = (uint16_t)realToFeel;


        optimizedRecord.wind_speed = record.wind_speed;

        //Compact string
        if(strcmp(record.weather_main, "Clear") == 0) {
            optimizedRecord.weather_type = 0;
        }
        else if(strcmp(record.weather_main, "Clouds") == 0) {
            optimizedRecord.weather_type = 1;
        }
        else if(strcmp(record.weather_main, "Mist") == 0) {
            optimizedRecord.weather_type = 2;
        }
        else if(strcmp(record.weather_main, "Snow") == 0) {
            optimizedRecord.weather_type = 3;
        }
        else {
            optimizedRecord.weather_type = 4;
        }


        fwrite(&optimizedRecord, sizeof(WeatherRecord_t), 1, binaryFile);
        
        //Display percentage
        if(count % 4141 == 0) {
            printf("%d%% done\n", count/4141+1);
        }
        count++;
    }

    //Replace record count from beginning
    fseek(binaryFile, offsetof(FileHeader_t, recordCount), SEEK_SET);
    fwrite(&count, sizeof(uint32_t), 1, binaryFile);

    fclose(binaryFile);
    return 1;
}


void readHeader(const char *binaryPath) {
    FILE *binaryFile = fopen(binaryPath, "rb");
    if(!binaryFile) {
        perror("Error opening the binary file!\n");
        return;
    }

    FileHeader_t header;

    if(fread(&header, sizeof(FileHeader_t), 1, binaryFile) != 1) {
        perror("Error reading the header of the binary file!\n");
        fclose(binaryFile);
        return;
    }

    printf("==========\n");
    printf("The header of the file:\nMagic: %.4s\nVersion: %d\nTimestamp: %u\nRecord count: %d\nCity: %s\nLat: %lf\nLong: %lf\n",
    header.magic, header.version, header.timestamp, header.recordCount, header.city, header.latitude, header.longitude);
    printf("==========\n");
}

void readRecordByIndex(const char *binaryPath) {
    //Read index
    bool valid = false;
    unsigned recordIndex;
    while(!valid) {
        char input[20];
        printf("\nEnter the index of the record: ");
        if(!fgets(input, sizeof(input), stdin)) {
            printf("Invalid input. Please try again!\n");
            continue;
        }

        input[strcspn(input, "\n")] = 0;

        char *ptr;
        recordIndex = strtol(input, &ptr, 10);
        // printf("\nat: %u\n", recordIndex);
        if(*ptr != '\0') {
            printf("Invalid format. Please try again!\n");
            continue;
        }

        valid = true;
    }
    
    
    FILE *binaryFile = fopen(binaryPath, "rb");
    if(!binaryFile) {
        perror("Error opening the binary file!\n");
        return;
    }

    fseek(binaryFile, sizeof(FileHeader_t) + recordIndex*sizeof(WeatherRecord_t), SEEK_SET);
    
    WeatherRecord_t record;
    if(fread(&record, sizeof(WeatherRecord_t), 1, binaryFile) != 1) {
        perror("Error reading record\n");
        fclose(binaryFile);
        return;
    }

    printf("%u, %.2f, %.2f, %.2f, %u, %u, %.2f, %u, %u, %u, %u\n",
    record.timestamp, record.temp, record.visibility, ((float)record.feels_like/10)+record.temp,
    record.pressure, record.humidity, record.wind_speed, record.wind_deg, record.clouds, record.weather_id, record.weather_type);
}

bool verifyHeader(const char *binaryPath) {
    FILE *binaryFile = fopen(binaryPath, "rb");
    if(!binaryFile) {
        perror("Error opening the binary file!\n");
        return 0;
    }

    FileHeader_t header;
    if(fread(&header, sizeof(FileHeader_t), 1, binaryFile) != 1) {
        perror("Error reading the header of the binary file!\n");
        fclose(binaryFile);
        return 0;
    }

    if(strncmp(header.magic, MAGIC_NUMBER, MAGIC_SIZE) != 0) {
        printf("Invalid magin number: expected: %s, got: %s\n", MAGIC_NUMBER, header.magic);
        fclose(binaryFile);
        return false;
    }

    if(header.version != VERSION) {
        printf("Unsupporte file version: %d (expected %d)\n", header.version, VERSION);
        fclose(binaryFile);
        return false;
    }

    if(header.recordCount == 0) {
        printf("File doesn't contain any records\n");
        fclose(binaryFile);
        return false;
    }

    printf("Binary file verified successfully!\n");
    printf("Magic: %.4s | Version: %u | Records: %u\n", header.magic, header.version, header.recordCount);
    return true;
}


void readRecordInInterval(const char *binaryPath) {
    uint32_t startTimestamp = readDateToTimestamp();
    uint32_t endTimestamp = readDateToTimestamp();

    int index=-1;

    FILE *binaryFile = fopen(binaryPath, "rb");
    if(!binaryFile) {
        perror("Error opening the binary file!\n");
        return;
    }

    FileHeader_t header;
    if(fread(&header, sizeof(FileHeader_t), 1, binaryFile) != 1) {
        perror("Error reading the header of the binary file!\n");
        fclose(binaryFile);
        return;
    }


    int left = 1;
    int right = header.recordCount;
    while(left < right) {
        int mid = (left + right) / 2;
        printf("left: %d, right: %d, mid: %d\n", left, right, mid);
        fseek(binaryFile, sizeof(FileHeader_t) + (mid-1)*sizeof(WeatherRecord_t), SEEK_SET);
        uint32_t midTimestamp;
        if(fread(&midTimestamp, sizeof(uint32_t), 1, binaryFile) != 1) {
            perror("Error reading record\n");
            fclose(binaryFile);
            return;
        }

        if(startTimestamp < midTimestamp) {
            right = mid - 1;
        }
        else if(startTimestamp > midTimestamp) {
            left = mid + 1;
        }
        else {
            index = mid;
            break;
        }
    }
    if(index == -1) {
        index = left;
    }

    WeatherRecord_t record;
    fseek(binaryFile, sizeof(FileHeader_t) + (index-1)*sizeof(WeatherRecord_t), SEEK_SET);
    while(endTimestamp >= record.timestamp) {
        if(fread(&record, sizeof(WeatherRecord_t), 1, binaryFile) != 1) {
            perror("Error reading the header of the binary file!\n");
            fclose(binaryFile);
            return;
        }
        printf("%u, %.2f, %.2f, %.2f, %u, %u, %.2f, %u, %u, %u, %u\n",
        record.timestamp, record.temp, record.visibility, ((float)record.feels_like/10)+record.temp,
        record.pressure, record.humidity, record.wind_speed, record.wind_deg, record.clouds, record.weather_id, record.weather_type);

    }
}

void addRecord(const char *binaryPath, WeatherRecord_t *newRecord) {
    FILE *binaryFile = fopen(binaryPath, "r+b");
    if(!binaryFile) {
        perror("Error opening the binary file!\n");
        return;
    }

    FileHeader_t header;
    if(fread(&header, sizeof(FileHeader_t), 1, binaryFile) != 1) {
        perror("Error reading the header of the binary file!\n");
        fclose(binaryFile);
        return;
    }

    fseek(binaryFile, 0, SEEK_END);
    if(fwrite(newRecord, sizeof(WeatherRecord_t), 1, binaryFile) != 1) {
        perror("Error adding the new record!\n");
        fclose(binaryFile);
        return;
    }

    header.recordCount++;
    fseek(binaryFile, 0, SEEK_SET);
    if(fwrite(&header, sizeof(FileHeader_t), 1, binaryFile) != 1) {
        perror("Error modifying the header!\n");
        return;
    }

    printf("Record added successfully!\n");
    fclose(binaryFile);
}

void updateRecord(const char *binaryPath, WeatherRecord_t* newRecord) {
    //Read index
    bool valid = false;
    unsigned index;
    while(!valid) {
        char input[20];
        printf("\nEnter the index of the record: ");
        if(!fgets(input, sizeof(input), stdin)) {
            printf("Invalid input. Please try again!\n");
            continue;
        }

        input[strcspn(input, "\n")] = 0;

        char *ptr;
        index = strtol(input, &ptr, 10);
        if(*ptr != '\0') {
            printf("Invalid format. Please try again!\n");
            continue;
        }

        valid = true;
    }

    FILE *binaryFile = fopen(binaryPath, "r+b");
    if(!binaryFile) {
        perror("Error opening the binary file!\n");
        return;
    }

    FileHeader_t header;
    if(fread(&header, sizeof(FileHeader_t), 1, binaryFile) != 1) {
        perror("Error reading the header of the binary file!\n");
        fclose(binaryFile);
        return;
    }

    fseek(binaryFile, sizeof(FileHeader_t) + (index-1)*sizeof(WeatherRecord_t), SEEK_SET);
    if(fwrite(newRecord, sizeof(WeatherRecord_t), 1, binaryFile) != 1) {
        perror("Error updating record at index");
        fclose(binaryFile);
        return; 
    }

    printf("Successfully modified record at index: %d\n", index);
    fclose(binaryFile);

}

void hardDeleteRecord(const char *binaryPath) {
    //Read index
    bool valid = false;
    unsigned index;
    while(!valid) {
        char input[20];
        printf("\nEnter the index of the record: ");
        if(!fgets(input, sizeof(input), stdin)) {
            printf("Invalid input. Please try again!\n");
            continue;
        }

        input[strcspn(input, "\n")] = 0;

        char *ptr;
        index = strtol(input, &ptr, 10);
        if(*ptr != '\0') {
            printf("Invalid format. Please try again!\n");
            continue;
        }

        valid = true;
    }
    
    FILE *binaryFile = fopen(binaryPath, "rb");
    if(!binaryFile) {
        perror("Error opening the binary file!\n");
        return;
    }

    FileHeader_t header;
    if(fread(&header, sizeof(FileHeader_t), 1, binaryFile) != 1) {
        perror("Error reading the header of the binary file!\n");
        fclose(binaryFile);
        return;
    }

    WeatherRecord_t *records = malloc(sizeof(WeatherRecord_t) * header.recordCount);
    fseek(binaryFile, sizeof(FileHeader_t), SEEK_SET);
    fread(records, sizeof(WeatherRecord_t), header.recordCount, binaryFile);
    fclose(binaryFile);

    FILE *outputBinary = fopen(binaryPath, "wb");
    if(!outputBinary) {
        perror("Error opening the binary file!\n");
        return;
    }
    header.recordCount--;
    fwrite(&header, sizeof(FileHeader_t), 1, outputBinary);

    header.recordCount++;
    for(uint32_t i=0;i<header.recordCount;i++) {
        if(i != index) {
            fwrite(&records[i], sizeof(WeatherRecord_t), 1, outputBinary);
        }
    }

    printf("Record deleted successfully at index: %d!\n", index);
    free(records);
    fclose(outputBinary);
}

//20.140.032
//12.008.633
//11.180.457
