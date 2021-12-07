#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

//#define DEBUG
#include "utils.h"

#define MAXCRABS 1024
#define sumnumbers(n) ((n)*((n)+1)/2)

void part_one(FILE *fp);
void part_two(FILE *fp);

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

    printf("Part one: ");
    part_one(fp);

    if (fseek(fp, 0, SEEK_SET)) {
        efatal("fseek failed");
    }

    printf("Part two: ");
    part_two(fp);

    fclose(fp);
    return 0;
}

/*** Main parts ***/

void part_one(FILE *fp) {
    int len = 0;
    int crabs[MAXCRABS];

    int crab;
    int max = 0;
    while (fscanf(fp, "%d", &crab) != EOF) {
        if (len >= MAXCRABS) {
            fatalf("too many crabs");
        }
        crabs[len] = crab;

        if (crab > max) {
            max = crab;
        }
        len++;
        fscanf(fp, ",");
    }

    int min_fuel = max*len;
    for (int pos=0; pos <=max; pos++) {
        int fuel = 0;
        for (int i=0; i<len; i++) {
            if (crabs[i] < pos) {
                fuel += pos - crabs[i];
            } else {
                fuel += crabs[i] - pos;
            }
        }
        dprintf("Position %3d: %d fuel\n", pos, fuel);
        if (fuel < min_fuel) {
            min_fuel = fuel;
        }
    }

    printf("%d\n", min_fuel);
}

void part_two(FILE *fp) {
    int len = 0;
    int crabs[MAXCRABS];

    int crab;
    int max = 0;
    while (fscanf(fp, "%d", &crab) != EOF) {
        if (len >= MAXCRABS) {
            fatalf("too many crabs");
        }
        crabs[len] = crab;

        if (crab > max) {
            max = crab;
        }
        len++;
        fscanf(fp, ",");
    }

    int min_fuel = max*sumnumbers(len);
    for (int pos=0; pos <=max; pos++) {
        int fuel = 0;
        for (int i=0; i<len; i++) {
            int n;
            if (crabs[i] < pos) {
                n = pos - crabs[i];
            } else {
                n = crabs[i] - pos;
            }
            fuel += sumnumbers(n);
        }
        dprintf("Position %3d: %d fuel\n", pos, fuel);
        if (fuel < min_fuel) {
            min_fuel = fuel;
        }
    }

    printf("%d\n", min_fuel);
}
