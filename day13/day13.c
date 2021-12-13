#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#define DEBUG
#include "utils.h"

int part_one(FILE *fp);
unsigned long long part_two(FILE *fp);

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

    printf("Part two:\n");
    part_two(fp);

    fclose(fp);
    return 0;
}

/*** Main parts ***/

int part_one(FILE *fp) {
    const int cap = 2000;
    int max_x = cap, max_y = cap;;

    bool points[max_x][max_y];
    for (int x=0; x<max_x; x++) {
        for (int y=0; y<max_y; y++) {
            points[x][y] = false;
        }
    }

    const int bufsize = 1024;
    char buf[bufsize];
    while (fgets(buf, bufsize, fp)) {
        int x, y;
        if (sscanf(buf, "%d,%d", &x, &y) == 2) {
            if (x >= max_x) {
                fatalf("x=%d is too large\n", x);
            }
            if (y >= max_y) {
                fatalf("y=%d is too large\n", y);
            }
            points[x][y] = true;
            dprintf("%d,%d\n", x, y);
            continue;
        }

        int val;
        if (sscanf(buf, "fold along x=%d", &val) == 1) {
            dprintf("fold x=%d\n", val);

            // Fold along x
            for (int x=val; x<max_x; x++) {
                for (int y=0; y<max_y; y++) {
                    int x2 = val - (x-val);
                    points[x2][y] = points[x2][y] || points[x][y];
                }
            }
            max_x = val;
            break;
        }

        if (sscanf(buf, "fold along y=%d", &val) == 1) {
            // Fold along y
            for (int x=0; x<max_x; x++) {
                for (int y=val; y<max_y; y++) {
                    int y2 = val - (y-val);
                    points[x][y2] = points[x][y2] || points[x][y];
                }
            }
            max_y = val;
            break;
        }
    }

    // Count points
    int n=0;
    for (int x=0; x<max_x; x++) {
        for (int y=0; y<max_y; y++) {
            if (points[x][y]) {
                n++;
            }
        }
    }

    return n;
}

unsigned long long part_two(FILE *fp) {
    const int cap = 2000;
    int max_x = cap, max_y = cap;;

    bool points[max_x][max_y];
    for (int x=0; x<max_x; x++) {
        for (int y=0; y<max_y; y++) {
            points[x][y] = false;
        }
    }

    const int bufsize = 1024;
    char buf[bufsize];
    while (fgets(buf, bufsize, fp)) {
        int x, y;
        if (sscanf(buf, "%d,%d", &x, &y) == 2) {
            if (x >= max_x) {
                fatalf("x=%d is too large\n", x);
            }
            if (y >= max_y) {
                fatalf("y=%d is too large\n", y);
            }
            points[x][y] = true;
            dprintf("%d,%d\n", x, y);
            continue;
        }

        int val;
        if (sscanf(buf, "fold along x=%d", &val) == 1) {
            dprintf("fold x=%d\n", val);

            // Fold along x
            for (int x=val; x<max_x; x++) {
                for (int y=0; y<max_y; y++) {
                    int x2 = val - (x-val);
                    points[x2][y] = points[x2][y] || points[x][y];
                }
            }
            max_x = val;
            continue;
        }

        if (sscanf(buf, "fold along y=%d", &val) == 1) {
            // Fold along y
            for (int x=0; x<max_x; x++) {
                for (int y=val; y<max_y; y++) {
                    int y2 = val - (y-val);
                    points[x][y2] = points[x][y2] || points[x][y];
                }
            }
            max_y = val;
            continue;
        }
    }

    // Print the points
    for (int y=0; y<max_y; y++) {
        for (int x=0; x<max_x; x++) {
            if (points[x][y]) {
                printf("#");
            } else {
                printf(".");
            }
        }
        printf("\n");
    }
    return 0;
}
