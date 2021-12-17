#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

//#define DEBUG
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

    printf("Part two: %lld\n", part_two(fp));

    fclose(fp);
    return 0;
}

/*** Main parts ***/

typedef struct {
    int x, y; // position
    int vx, vy; // velocity
} probe;

void step(probe *p) {
    // Move position
    p->x = p->x + p->vx;
    p->y = p->y + p->vy;

    // Reduce x-velocity due to drag
    if (p->vx > 0) { p->vx--; }
    if (p->vx < 0) { p->vx++; }

    // Decreas y-velocity due to gravity
    p->vy--;
}

int part_one(FILE *fp) {
    int xMin, xMax, yMin, yMax;
    if (fscanf(fp, "target area: x=%d..%d, y=%d..%d", &xMin, &xMax, &yMin, &yMax) != 4) {
        fatalf("invalid input\n");
    }

    dprintf("target area: x=%d..%d, y=%d..%d\n", xMin, xMax, yMin, yMax);

    // Find minimum x-velocity required
    int vx = 0;
    int min_vx = 0, max_vx = xMax+1;
    while (min_vx == 0 || max_vx == 0) {
        vx++;

        probe p = { 0, 0, vx, 0 };
        while (p.vx != 0) {
            step(&p);
        }
        if (min_vx == 0 && p.x >= xMin) {
            min_vx = vx;
            break;
        }
    }

    // Find y-velocity that maximizes y
    int max_y = 0;
    for (int vx=min_vx; vx <= max_vx; vx++) {
        int vy = 0;

        while (vy < -yMin) {
            vy++;

            dprintf("Try vx=%d, vy=%d\n", vx, vy);
            probe p = { 0, 0, vx, vy };
            int local_max_y = 0;
            // Iterate until we overshoot the target area
            while (p.x <= xMax && p.y >= yMin) {
                step(&p);
                if (p.y > local_max_y) {
                    local_max_y = p.y;
                }

                if (xMin <= p.x && p.x <= xMax && yMin <= p.y && p.y <= yMax) {
                    // Hit target area
                    if (max_y < local_max_y) {
                        max_y = local_max_y;
                    }
                    dprintf("Hit target at %d,%d => max_y=%d\n", p.x, p.y, max_y);
                    break;
                }
                dprintf("x=%3d, y=%3d, vx=%3d, vy=%d\n", p.x, p.y, p.vx, p.vy);
                if (p.x > xMax || p.y < yMin) {
                    break;
                }
            }
        }
    }

    return max_y;
}

unsigned long long part_two(FILE *fp) {
    int xMin, xMax, yMin, yMax;
    if (fscanf(fp, "target area: x=%d..%d, y=%d..%d", &xMin, &xMax, &yMin, &yMax) != 4) {
        fatalf("invalid input\n");
    }

    // Find minimum x-velocity required
    int vx = 0;
    int min_vx = 0, max_vx = xMax+1;
    while (min_vx == 0 || max_vx == 0) {
        vx++;

        probe p = { 0, 0, vx, 0 };
        while (p.vx != 0) {
            step(&p);
        }
        if (min_vx == 0 && p.x >= xMin) {
            min_vx = vx;
            break;
        }
    }

    // Count target hits
    int cnt = 0;
    for (int vx=min_vx; vx <= max_vx; vx++) {
        for (int vy=yMin; vy<-yMin; vy++) {
            dprintf("Try vx=%d, vy=%d\n", vx, vy);
            probe p = { 0, 0, vx, vy };
            int local_max_y = 0;
            // Iterate until we overshoot the target area
            while (p.x <= xMax && p.y >= yMin) {
                step(&p);
                if (p.y > local_max_y) {
                    local_max_y = p.y;
                }

                if (xMin <= p.x && p.x <= xMax && yMin <= p.y && p.y <= yMax) {
                    // Hit target area
                    cnt++;
                    break;
                }
                dprintf("x=%3d, y=%3d, vx=%3d, vy=%d\n", p.x, p.y, p.vx, p.vy);
                if (p.x > xMax || p.y < yMin) {
                    break;
                }
            }
        }
    }

    return cnt;
}
