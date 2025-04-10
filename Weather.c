#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

// #include "Format.h"
#include "Weather.h"


int read_csv(FILE *file, Weather_t **data, int *size) {
    char line[2048];
    
    //Skip header
    fgets(line, sizeof(line), file);
    
    //initialize data
    *data = NULL;
    *size = 0;
    int capacity = 10;
    
    *data = malloc(capacity * sizeof(Weather_t));
    if (*data == NULL) {
        printf("Initial memory allocation failed!\n");
        return -1;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        if((*size+1) % 1000 == 0) {
            printf("Reading line %d\\414088\n", *size+1);
        }
        if (*size >= capacity) {
            capacity *= 2;
            *data = realloc(*data, capacity * sizeof(Weather_t));
            
            if(*data == NULL) {
                printf("Memory reallocation failed!\n");
                return -1;
            }
        }

        Weather_t *w = &((*data)[*size]);
        memset(w, 0, sizeof(Weather_t));

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
                case 0:  w->timestamp = *token ? atol(token) : -1; break;
                case 1:  strncpy(w->dt_iso, *token ? token : "", sizeof(w->dt_iso) - 1); break;
                case 2:  w->timezone = *token ? atoi(token) : -1; break;
                case 3:  strncpy(w->city_name, *token ? token : "", sizeof(w->city_name) - 1); break;
                case 4:  w->lat = *token ? atof(token) : -1; break;
                case 5:  w->lon = *token ? atof(token) : -1; break;
                case 6:  w->temp = *token ? atof(token) : -1; break;
                case 7:  w->visibility = *token ? atof(token) : -1; break;
                case 8:  w->dew_point = *token ? atof(token) : -1; break;
                case 9:  w->feels_like = *token ? atof(token) : -1; break;
                case 10: w->temp_min = *token ? atof(token) : -1; break;
                case 11: w->temp_max = *token ? atof(token) : -1; break;
                case 12: w->pressure = *token ? atoi(token) : -1; break;
                case 13: w->sea_level = *token ? atoi(token) : -1; break;
                case 14: w->grnd_level = *token ? atoi(token) : -1; break;
                case 15: w->humidity = *token ? atoi(token) : -1; break;
                case 16: w->wind_speed = *token ? atof(token) : -1; break;
                case 17: w->wind_deg = *token ? atoi(token) : -1; break;
                case 18: w->wind_gust = *token ? atof(token) : -1; break;
                case 19: w->rain_1h = *token ? atof(token) : -1; break;
                case 20: w->rain_3h = *token ? atof(token) : -1; break;
                case 21: w->snow_1h = *token ? atof(token) : -1; break;
                case 22: w->snow_3h = *token ? atof(token) : -1; break;
                case 23: w->clouds_all = *token ? atoi(token) : -1; break;
                case 24: w->weather_id = *token ? atoi(token) : -1; break;
                case 25: strncpy(w->weather_main, *token ? token : "Unknown", sizeof(w->weather_main) - 1); break;
                case 26: strncpy(w->weather_description, *token ? token : "Unknown", sizeof(w->weather_description) - 1); break;
                case 27: strncpy(w->weather_icon, *token ? token : "N/A", sizeof(w->weather_icon) - 1); break;
            }

            //Go to the next column
            token = next_token;
            column++;
        }
        //Increase the number of elements from the structure
        (*size)++;
    }
    return *size;
}



void readDates(int *year, int *month, int *day) {
    int state = 0; //Which date part to read 
    bool validDate = false;

    //Find the desired date
    printf("Please insert the date\n");
    do {
        switch(state) {
            case 0:
                printf("Year: ");
                if(scanf("%d", year) != 1) {
                    printf("\nInvalid input, try again!\n");
                    break;
                }
                if(*year < 1979 || *year > 2024) {
                    printf("Please input a year from the range 1979-2024");
                    break;
                }
                state++; //fall through if necessary
            case 1:
                printf("\nMonth: ");
                if(scanf("%d", month) != 1) {
                    printf("\nInvalid input, try again!\n");
                    break;
                }
                if(*month < 1 || *month > 12 || (*year == 2024 && *month == 12)) {
                    printf("Please input a valid month of the year\n");
                    break;
                }
                state++; //fall through if necessary
            case 2:
                printf("\nDay: ");
                if(scanf("%d", day) != 1) {
                    printf("\nInvalid input, try again!\n");
                    break;
                }
                if(*day < 1  || *day > 31) {
                    printf("Please input a valid day of the month\n");
                    break;
                }
                state = 0;
                validDate = true;
        }                    
    } while(!validDate);
}

int findFirstDateOcc(Weather_t *data, int size, int year, int month, int day) {
    char dateFormat[11];

    //Create the date format
    snprintf(dateFormat, sizeof(dateFormat), "%04d-%02d-%02d", year, month, day);

    int start = -1;

    //Search by date (with binary search)
    int left = 0, right = size - 1;
    while (left <= right) {
        int middle = (left + right) / 2;
        int stringsCompare = strncmp(data[middle].dt_iso, dateFormat, 10);
        if (stringsCompare == 0) {
            //Keep searching for the first with the desired date
            start = middle;
            right = middle - 1;  
        } else if (stringsCompare < 0) {
            left = middle + 1;
        } else {
            right = middle - 1;
        }
    }

    //Error handle
    if (start == -1) {
        printf("No data found for date:%s\n", dateFormat);
    }

    return start;
}

int findLastDateOcc(Weather_t *data, int size, int year, int month, int day) {
    char dateFormat[11];

    //Create the date format
    snprintf(dateFormat, sizeof(dateFormat), "%04d-%02d-%02d", year, month, day);

    int end = -1;

    //Search by date (with binary search)
    int left = 0, right = size - 1;
    while (left <= right) {
        int middle = (left + right) / 2;
        int stringsCompare = strncmp(data[middle].dt_iso, dateFormat, 10);
        if (stringsCompare == 0) {
            //Keep searching for the first with the desired date
            end = middle;
            left = middle + 1;  
        } else if (stringsCompare < 0) {
            left = middle + 1;
        } else {
            right = middle - 1;
        }
    }

    //Error handle
    if (end == -1) {
        printf("No data found for date:%s\n", dateFormat);
    }

    return end;
}

void findDailyTemp(Weather_t *data, int size, int year, int month, int day) {
    double minTemp = 100.0, maxTemp = -100.0;
    char dateFormat[11];

    int start = findFirstDateOcc(data, size, year, month, day);
    
    // //Create the date format
    snprintf(dateFormat, sizeof(dateFormat), "%04d-%02d-%02d", year, month, day);

    //Find minimum and maximum
    while (start < size && strncmp(data[start].dt_iso, dateFormat, 10) == 0) {
        minTemp = (data[start].temp < minTemp) ? data[start].temp : minTemp;
        maxTemp = (data[start].temp > maxTemp) ? data[start].temp : maxTemp;
        start++;
    }

    if(minTemp == 100.0 || maxTemp == -100.0) {
        printf("Bad read\n");
        return;
    }

    printf("Weather data for %s:\n", dateFormat);
    printf("Minimum Temperature: %.2f°C\n", minTemp);
    printf("Maximum Temperature: %.2f°C\n", maxTemp);

}


void display_statistics(Weather_t *data, int size) {
    int answer = -1;
    while(answer != 6) {
        printf("\nBasic statistics\n");
        printf("1. Daily temperature range (max - min)\n");
        printf("2. Average temperature\n");
        printf("3. Average humidity\n");
        printf("4. Average pressure\n");
        printf("5. Count of each weather type\n");
        printf("6. Exit to main menu\n");
        printf("\nAnswer: ");
        scanf("%d", &answer);
        switch(answer) {
            case 1: //Temp range

                int year, month, day;
                readDates(&year, &month, &day);

                //Find the temperature interval
                findDailyTemp(data, size, year, month, day);
                break;

            case 2:
            case 3:
            case 4:
                int count = 1;
                double sum = 0;
                for(int cnt=0;cnt<size;cnt++) {
                    int currentVal = 0;
                    if(answer == 2) {
                        currentVal = data[cnt].temp;
                    }
                    else if(answer == 3) {
                        currentVal = data[cnt].humidity;
                    }
                    else {
                        currentVal = data[cnt].pressure;
                    }
                    sum = (sum*count + currentVal) / (count + 1);
                    count++;
                }
                if(answer == 2) {
                    printf("Average dataset temperature: %.2lf°C\n", sum);
                }
                else if(answer == 3) {
                    printf("Average dataset humidity: %.2lf%%\n", sum);
                }
                else {
                    printf("Average dataset pressure: %.2lf\n", sum);
                }
                break;
            case 6: //Exit
                break;
            default:
                printf("Wrong choise, please try again!");
                break;
        }
    }
}


void data_filtering(Weather_t *data, int size) {
    int answer = -1;
    while(answer != 5) {
        printf("\nData filtering\n");
        printf("1. Find by date range\n");
        printf("2. Find by weather type\n");
        printf("3. Find higest temperature record\n");
        printf("4. Find strongest wind record\n");
        printf("5. Exit\n");
        scanf("%d", &answer);

        switch(answer) {
            case 1:
                int year1, month1, day1, year2, month2, day2;
                readDates(&year1, &month1, &day1);
                readDates(&year2, &month2, &day2);

                int pos1 = findFirstDateOcc(data, size, year1, month1, day1);
                // printf("%d\n", pos1);
                int pos2 = findLastDateOcc(data, size, year2, month2, day2);
                // printf("%d\n", pos2);

                for(int cnt=pos1;cnt<=pos2;cnt++) {
                    printf("%d. Date: %s, City: %s, Temp: %.2fC, Weather: %s\n", cnt, data[cnt].dt_iso, data[cnt].city_name, data[cnt].temp, data[cnt].weather_description);
                }
                break;
            case 2:
                char searchTerm[50];

                printf("Please enter the type of weather you want to find:\n");
                char c;
                while(((c = getchar()) != '\n' && c != EOF)) {
                    continue;
                }

                if(fgets(searchTerm, sizeof(searchTerm), stdin) == NULL) {
                    printf("Error reading input\n");
                    break;
                }

                searchTerm[strcspn(searchTerm, "\n")] = 0; //Remove end of line

                printf("Searching for weather descriptions containing: \"%s\"\n", searchTerm);

                int found = 0;
                for (int i = 0; i < size; i++) {
                    if (strcasestr(data[i].weather_description, searchTerm)) { // Case-insensitive search
                        printf("%s - Temp: %.2fC, Humidity: %d%%, Weather: %s\n", 
                            data[i].dt_iso, data[i].temp, data[i].humidity, data[i].weather_description);
                        found = 1;
                    }
                }

                if (!found) {
                    printf("No entries found for weather description: \"%s\"\n", searchTerm);
                }
                break;
            case 3:
                double highestTemp = -100;
                int entry = 0;
                for(int i=0;i<size;i++) {
                    if(data[i].temp > highestTemp) {
                        highestTemp = data[i].temp;
                        entry = i;
                    }
                }
                printf("The record with highest temperature is: \n");
                printf("%d. Date: %s, City: %s, Temp: %.2fC, Weather: %s\n", entry, data[entry].dt_iso, data[entry].city_name, data[entry].temp, data[entry].weather_description);
                break;
            case 4:
                double strongestWind = -100;
                int entry2 = 0;
                for(int i=0;i<size;i++) {
                    if(data[i].wind_speed > strongestWind) {
                        strongestWind = data[i].wind_speed;
                        entry2 = i;
                    }
                }
                printf("The record with strongest wind is: \n");
                printf("%d. Date: %s, City: %s, Temp: %.2fC, Wind speed: %lf\n", entry2, data[entry2].dt_iso, data[entry2].city_name, data[entry2].temp, data[entry2].wind_speed);
                break;
            case 5:
                break;
        }
    }
}


void show_visuals(Weather_t *data, int size) {
    int answer = -1;
    while(answer != 4) {
        printf("1. Daily temperature bars\n");
        printf("2. Weather conditions in range\n");
        printf("3. Temperature heat map in range\n");
        printf("4. Exit to previous\n");
        scanf("%d", &answer);

        //Cases
        switch(answer) {
            case 1: //Daily temp
                int year, month, day;
                readDates(&year, &month, &day);

                int startPos = findFirstDateOcc(data, size, year, month, day);
                int endPos = findLastDateOcc(data, size, year, month, day);

                printf("%.10s\n", data[startPos].dt_iso);
                
                //Find highest and lowest temperatures in the day
                double lowestTemp=100.0, highestTemp=-100.0;
                for(int i=startPos;i<=endPos;i++) {
                    if(data[i].temp > highestTemp) {
                        highestTemp = data[i].temp;
                    }
                    if(data[i].temp < lowestTemp) {
                        lowestTemp = data[i].temp;
                    }
                }
                printf("Min temp: %.2lf, Max temp: %.2lf ", lowestTemp, highestTemp);

                double procentValue = (highestTemp - lowestTemp) / 100;
                int minHashtags = 5, intervalHashtags = 15;
                for(int i=startPos;i<=endPos;i++) {
                    printf("%.8s ", data[i].dt_iso+11);
                    double intervalPosition = (double)((data[i].temp - lowestTemp) / procentValue);
                    // printf("%.3lf\n", intervalPosition);
                    int numOfHastags = (double)(intervalPosition * intervalHashtags / 100) + minHashtags + 1;

                    printf("[");
                    for(int j=0;j<numOfHastags;j++) {
                        printf("#");
                    }
                    printf("]  %.2lf°C\n", data[i].temp);
                }
                putchar('\n');
                break;
            case 2: //Weather conditions in range
                int year1, month1, day1, year2, month2, day2;
                readDates(&year1, &month1, &day1);
                readDates(&year2, &month2, &day2);

                int startPostition = findFirstDateOcc(data, size, year1, month1, day1);
                int endPosition = findLastDateOcc(data, size, year2, month2, day2);

                //Clear, Clouds, Mist, Snow, Rain
                int app[5] = {0};
                for(int i=startPostition;i<endPosition;i++) {
                    if(strcmp(data[i].weather_main, "Clear") == 0) {
                        app[0]++;
                    }
                    else if(strcmp(data[i].weather_main, "Clouds") == 0) {
                        app[1]++;
                    }
                    else if(strcmp(data[i].weather_main, "Mist") == 0) {
                        app[2]++;
                    }
                    else if(strcmp(data[i].weather_main, "Snow") == 0) {
                        app[3]++;
                    }
                    else if(strcmp(data[i].weather_main, "Rain") == 0) {
                        app[4]++;
                    }
                }
                int minHashtags2 = 2, intervalHashtags2 = 10;
                int minVal=1000000000, maxVal = -1;
                for(int i=0;i<5;i++) {
                    if(app[i] > maxVal) {
                        maxVal = app[i];
                    }
                    if(app[i] < minVal && app[i] != 0) {
                        minVal = app[i];
                    }
                }
                double procentValue2 = (maxVal - minVal);
                for(int i=0;i<5;i++) {
                    if(app[i] != 0) {
                        double intervalPosition2 = app[i] != minVal ? (double)((app[i] - minVal) / procentValue2* 100) : 0;
                        int numOfHastags2 = (double)(intervalPosition2 * intervalHashtags2 / 100) + minHashtags2 + 1;
                        switch(i) {
                            case 0:
                                printf("Clear   ");
                                break;
                            case 1:
                                printf("Clounds ");
                                break;
                            case 2:
                                printf("Mist    ");
                                break;
                            case 3:
                                printf("Snow    ");
                                break;
                            case 4:
                                printf("Rain    ");
                                break;
                        }
                        printf("[");
                        for(int i=0;i<numOfHastags2;i++) {
                            printf("#");
                        }
                        printf("] %d\n", app[i]);
                    }
                }
                break;
            case 3:
                int year3, month3, day3;
                readDates(&year3, &month3, &day3);
                int intervalStart = findFirstDateOcc(data, size, year3, month3, day3);
                
                for(int i=0;i<16;i++) {
                    float avg = 0, count = 0;
                    for(int j=0;j<24;j++) {
                        avg = (count * avg + data[intervalStart+i*24+j].temp)/(count+1);
                        count++;
                    }
                    // printf("Temp: %f\n", avg);
                    if(avg < -10) {
                        printf("_");
                    }
                    else if(avg < 0) {
                        printf(".");
                    }
                    else if(avg < 10) {
                        printf("+");
                    }
                    else if(avg < 20){
                        printf("*");
                    }
                    else {
                        printf("^");
                    }
                }
                putchar('\n');
                break;
            case 4:
                break;
        }
    }
}
