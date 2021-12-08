#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

//#define DEBUG
#include "utils.h"

#define LINE_MAX 1024

int part_one(FILE *fp);
int part_two(FILE *fp);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fatalf("Please provide input file\n");
    }

    char *infile = argv[1];

    FILE *fp;
    fp = fopen(infile, "r");
    if (fp == NULL) {
        fatalf("Cannot open %s: %s\n", infile, strerror(errno));
    }

    printf("Part one: %d\n", part_one(fp));

    if (fseek(fp, 0, SEEK_SET)) {
        efatal("fseek failed");
    }

    printf("Part two: %d\n", part_two(fp));

    fclose(fp);
    return 0;
}

/*** Main parts ***/

int part_one(FILE *fp) {
    char line[LINE_MAX];

    int cnt = 0;
    while (fgets(line, LINE_MAX, fp) != NULL) {
        // Split by '|' to get output values
        strtok(line, "|");
        char *output = strtok(NULL, "|");
        dprintf("Output: %s", output);

        char *token = strtok(output, " \n");
        while (token != NULL) {
            dprintf("Token: %s, len: %ld\n", token, strlen(token));
            switch (strlen(token)) {
            case 2: case 3: case 4: case 7:
                    cnt++;
            }
            token = strtok(NULL, " \n");
        }

    }

    return cnt;
}

// pattern2bits converts a segment pattern string into a bit pattern, one bit per segment.
int pattern2bits(char *pattern) {
    int val = 0;
    while (*pattern) {
        // a = 0b_0000001
        // b = 0b_0000010
        // . = ...
        // g = 0b_1000000
        val |= 1 << (*pattern - 'a');
        pattern++;
    }
    return val;
}

int part_two(FILE *fp) {
    char line[LINE_MAX];

    int sum = 0;
    while (fgets(line, LINE_MAX, fp) != NULL) {
        // Split "<segment patterns> | <output values>"
        char *patterns = strtok(line, "|");
        char *output = strtok(NULL, "|");

        // Read segment patterns as bit patterns
        int cf = 0; // Digit 1 (length 2)
        int acf = 0; // Digit 7 (length 3)
        int bcdf = 0; // Digit 4 (length 4)
        int adg = 0xff; // AND of digits 2, 3, 5 (lengths 5)
        int abfg = 0xff; // AND of digits 0, 6, 9 (lengths 6)
        int abcdefg = 0; // Digit 8 (length 7)

        char *pattern = strtok(patterns, " ");
        while (pattern != NULL) {
            switch (strlen(pattern)) {
            case 2:
                cf = pattern2bits(pattern);
                break;
            case 3:
                acf = pattern2bits(pattern);
                break;
            case 4:
                bcdf = pattern2bits(pattern);
                break;
            case 5:
                adg &= pattern2bits(pattern);
                break;
            case 6:
                abfg &= pattern2bits(pattern);
                break;
            case 7:
                abcdefg = pattern2bits(pattern);
                break;
            default:
                fatalf("invalid pattern length %ld\n", strlen(pattern));
            }
            pattern = strtok(NULL, " ");
        }

        // Deduce the segments:
        int a = acf & adg;
        int f = cf & abfg;
        int c = cf & ~f;
        int d = adg & bcdf;
        int b = bcdf ^ (cf | d);
        int g = adg ^ (a | d);
        int e = abcdefg ^ (a|b|c|d|f|g);

        int digits[] = {
            a|b|c|e|f|g,
            cf,
            a|c|d|e|g,
            a|c|d|f|g,
            bcdf,
            a|b|d|f|g,
            a|b|d|e|f|g,
            acf,
            abcdefg,
            a|b|c|d|f|g,
        };

        // Process output values
        dprintf("Output: %s", output);
        int value = 0;
        char *token = strtok(output, " \n");
        while (token != NULL) {
            // Convert pattern to digit
            int bits = pattern2bits(token);
            for (int i=0; i<10; i++) {
                if (digits[i] == bits) {
                    // digit = i; Update value:
                    value = 10*value + i;
                    break;
                }
            }
            token = strtok(NULL, " \n");
        }
        sum += value;
        dprintf(" = %d\n", value);
    }

    return sum;
}
