#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

//#define DEBUG
#include "utils.h"

#define LINE_MAX 128
#define NO_BASIN -1

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
    char buf[3][LINE_MAX];

    char *prev = NULL;
    char *curr = NULL;
    char *next = NULL;

    int n = 0; // line number

    // Read first line
    curr = fgets(buf[n], LINE_MAX, fp);
    n++;

    int risksum = 0;
    while (curr != NULL) {
        next = fgets(buf[n%3], LINE_MAX, fp);
        n++;

        for (int i=0; curr[i] != '\n'; i++) {
            // Up
            if (prev && prev[i] <= curr[i]) {
                continue;
            }

            // Left
            if (i > 0 && curr[i-1] <= curr[i]) {
                continue;
            }

            // Right
            if (curr[i+1] != '\n' && curr[i+1] <= curr[i]) {
                continue;
            }

            // Down
            if (next && next[i] <= curr[i]) {
                continue;
            }

            risksum += curr[i] - '0' + 1;
            dprintf("Low point %c at (%d, %d)\n", curr[i], i, n-2);
        }

        prev = curr;
        curr = next;
    }
    return risksum;
}

int *new_basin(int *basins, int newCnt) {
    basins = realloc(basins, newCnt*sizeof(int));
    basins[newCnt-1] = 0;
    return basins;
}

int part_two(FILE *fp) {
    int buf[2][LINE_MAX];
    // Initialize as not part of any basin
    for (int i=0; i<LINE_MAX; i++) {
        buf[0][i] = NO_BASIN;
        buf[1][i] = NO_BASIN;
    }

    int *curr = buf[0]; // basin IDs for current line
    int *prev = buf[1]; // basin IDs for previous line


    int *basins = NULL;
    int basinCnt = 0;

    char line[LINE_MAX];
    while (fgets(line, LINE_MAX, fp) != NULL) {
        for (int i=0; line[i] != '\n'; i++) {
            if (line[i] == '9') {
                // Not part of basin
                curr[i] = NO_BASIN;
                continue;
            }

            int up = prev ? prev[i] : NO_BASIN;
            int left = (i > 0) ? curr[i-1] : NO_BASIN;

            if (up != NO_BASIN && left != NO_BASIN && up != left) {
                // Merge basin above with basin to the left
                dprintf("Merge %d => %d\n", up, left);
                basins[left] += basins[up];
                basins[up] = 0;

                // Update left side of curr and entire prev with the merged basin
                for (int j=0; j<LINE_MAX; j++) {
                    if (j<i && curr[j] == up) {
                       curr[j] = left;
                    }
                    if (prev[j] == up) {
                        prev[j] = left;
                    }
                }
                up = left;
            }

            if (up != NO_BASIN) {
                // Part of basin above
                dprintf("part of basin[%d] above\n", up);
                curr[i] = up;
            } else if (left != NO_BASIN) {
                // Part of basin to the left
                dprintf("part of basin[%d] to the left\n", left);
                curr[i] = left;
            } else {
                // New basin
                curr[i] = basinCnt;
                dprintf("part of new basin[%d]\n", basinCnt);
                basinCnt++;
                basins = new_basin(basins, basinCnt);
            }

            basins[curr[i]]++;
        }

        int *tmp = prev;
        prev = curr;
        curr = tmp;
    }

    int largest[] = {0, 0, 0};
    for (int i=0; i<basinCnt; i++) {
        dprintf("basins[%d]: %d\n", i, basins[i]);
        
        if (basins[i] > largest[2]) {
            largest[0] = largest[1];
            largest[1] = largest[2];
            largest[2] = basins[i];
        } else if (basins[i] > largest[1]) {
            largest[0] = largest[1];
            largest[1] = basins[i];
        } else if (basins[i] > largest[0]) {
            largest[0] = basins[i];
        }
        dprintf("largest: %d, %d, %d\n", largest[0], largest[1], largest[2]);
    }
    free(basins);

    return largest[0]*largest[1]*largest[2];
}
