#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
// #include <time.h>
// #include <ncurses.h>
#include <string.h>

#include "Weather.h"
#include "Format.h"
#include "Binary.h"

#define MAX_LINE 2048




void printMenu() {
    printf("Select an option:\n");
    printf("1. Load data from file\n");
    printf("2. Display basic statistics\n");
    printf("3. Filter data by criteria\n");
    printf("4. Show visualizations\n");
    printf("5. Binary file actions\n");
    printf("6. Exit");
    printf("\nAnswer: ");
}


int main(int argc, char* argv[]) {
    bool isDataLoaded = false;
    int answer = -1;

    if(argc < 3) {
        printf("Please include the filepath of the csv file!\n");
        return 1;
    }
    
    FILE *f = fopen(argv[1], "r");
    if(!f) {
        printf("Unable to open file at path: %s\n", argv[1]);
        return 1;
    }

    FILE *binFile = fopen(argv[2], "wb");
    if(!binFile) {
        printf("Unable to open file at path: %s\n", argv[2]);
        return 1;
    }

    Weather_t* data = NULL;
    int size = 0;
    // int records = -1;

    printf("Welcome to the CSV reader\n");
    while(answer != 6) {
        printMenu();
        scanf("%d", &answer);
        switch(answer) {
            case 1:
                if(isDataLoaded) {
                    printf("Data is already loaded\n");
                }
                else {
                    int records = read_csv(f, &data, &size);
                    if(records != -1) {
                        printf("Successfully read %d lines\n", size);
                        isDataLoaded = true;
                    }
                }
                break;
            case 2:
                if(!isDataLoaded) {
                    printf("Data is not loaded yet, please load data first\n\n");
                }
                else {
                    display_statistics(data, size);
                }
                break;
            case 3:
                if(!isDataLoaded) {
                    printf("Data is not loaded yet, please load data first\n\n");
                }
                else {
                    data_filtering(data, size);
                }
                break;
            case 4:
                if(!isDataLoaded) {
                    printf("Data is not loaded yet, please load data first\n\n");
                }
                else {
                    printf("Show visuals\n");
                    show_visuals(data, size);
                }
                break;
            case 5:
                // printf("Converting to binary:\n");
                // convertToBinary(argv[1], argv[2]);
                // readHeader(argv[2]);
                // readRecordByIndex(argv[2], 1);
                // printf("%u\n", verifyHeader(argv[2]));
                getchar();
                bool ok = true, binaryDataLoaded = false;
                while(ok) {
                    ok = binaryMenu(argv[1], argv[2], &binaryDataLoaded);
                }
                break;
            case 6:
                printf("Program ended. Good bye!\n");
                answer = 6;
                break;
            default:
                printf("Wrong entry, please try again!\n");
                break;
        }
    }
    
    
    // if(records > 0) {
    //     for(int i=0;i<size;i++) {
    //         printf("%d. City: %s, Temp: %.2fC, Weather: %s\n", i, data[i].city_name, data[i].temp, data[i].weather_description);
    //     }
    // }

    free(data);
    fclose(f);
}