#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

//#define DEBUG
#include "utils.h"

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

/*** Utils ***/

unsigned long long simulate(FILE *fp, int days) {
    unsigned long long bufA[] = {0,0,0,0,0,0,0,0,0};
    unsigned long long bufB[] = {0,0,0,0,0,0,0,0,0};
    unsigned long long *curr = bufA, *next = bufB;

    // Read initial state
    while (1) {
        int val;
        if (fscanf(fp, "%d", &val) == EOF) {
            break;
        }

        // Add fish to state
        if (val < 0 || val > 8) {
            fatalf("Invalid fish timer %d", val);
        }
        curr[val]++;

        if (fscanf(fp, ",") == EOF) {
            break;
        }
    }

    // Process days
    for (int i=0; i<days; i++) {
        next[0] = curr[1];
        next[1] = curr[2];
        next[2] = curr[3];
        next[3] = curr[4];
        next[4] = curr[5];
        next[5] = curr[6];
        next[6] = curr[0] + curr[7];
        next[7] = curr[8];
        next[8] = curr[0];

        unsigned long long *tmp;
        tmp = curr;
        curr = next;
        next = tmp;

        // Count all the fish
        unsigned long long sum = 0;
        for (int i=0; i<9; i++) {
            dprintf("T=%d: %llu fish\n", i, curr[i]);
            sum += curr[i];
        }
        dprintf("After %2d days: %llu fish\n", i+1, sum);
    }

    // Count all the fish
    unsigned long long sum = 0;
    for (int i=0; i<9; i++) {
        sum += curr[i];
    }
    return sum;
}

/*** Main parts ***/

void part_one(FILE *fp) {
    printf("%llu\n", simulate(fp, 80));
}

void part_two(FILE *fp) {
    printf("%llu\n", simulate(fp, 256));
}
