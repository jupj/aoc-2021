#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#define DEBUG
#include "utils.h"

long long part_one(FILE *fp);
long long part_two(FILE *fp);

void unit_test();

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

    printf("Part one: %lld\n", part_one(fp));

    if (fseek(fp, 0, SEEK_SET)) {
        efatal("fseek failed");
    }

    printf("Part two: %lld\n", part_two(fp));

    fclose(fp);
    return 0;
}

/*** Analysis ***/

/* 
The MONAD program is split into 14 stages. (see input.txt)
The stages can be parametrized using integers p1, p2, p3.
Each stage uses z from the previous stage and reads an input w.
Then z is updated as follows:

    if (((z % 26) + p2) == w) {
        z = z/p1;
    } else {
        z = 26*(z/p1) + w + p3;
    }

There are 7 stages with p1=1, and 7 stages with p1=26.
For z to be zeroed, the stages with p1=26 must always
use the branch where z' = z/p1.
*/

#define N_STAGES 14

typedef struct {
    int p1, p2, p3;
} stageparams;

typedef struct {
    stageparams params[N_STAGES];
} program;

typedef void (*callback)(long serial);

#define INVALID_SERIAL -1
typedef long long tserial;

tserial prog_maximize(program *p, int stageIx, tserial z, tserial serial) {
    //dprintf("stage %d: z=%lld, serial=%lld\n", stageIx, z, serial);
    if (stageIx >= N_STAGES) {
        if (z == 0) {
            return serial;
        }
        dprintf("Invalid serial: %lld\n", serial);
        return INVALID_SERIAL;
    }

    int p1 = p->params[stageIx].p1;
    int p2 = p->params[stageIx].p2;
    int p3 = p->params[stageIx].p3;

    tserial res1 = INVALID_SERIAL, res2 = INVALID_SERIAL;
    for (int w=9; w>0; w--) {

        if (((z % 26) + p2) == w) {
            res1 = prog_maximize(p, stageIx+1, z/p1, serial*10 + w);
        } else {
            // When p1 == 26, we must utilize the divide by 26
            // We can do this branch (z grows) only when p1 == 1
            if (p1 == 1) {
                res2 = prog_maximize(p, stageIx+1, 26*(z/p1) + w + p3, serial*10 + w);
            }
        }

        if (res1 != INVALID_SERIAL && res1 > res2) {
            return res1;
        }
        if (res2 != INVALID_SERIAL) {
            return res2;
        }
    }
    return INVALID_SERIAL;
}


tserial prog_minimize(program *p, int stageIx, tserial z, tserial serial) {
    //dprintf("stage %d: z=%lld, serial=%lld\n", stageIx, z, serial);
    if (stageIx >= N_STAGES) {
        if (z == 0) {
            return serial;
        }
        dprintf("Invalid serial: %lld\n", serial);
        return INVALID_SERIAL;
    }

    int p1 = p->params[stageIx].p1;
    int p2 = p->params[stageIx].p2;
    int p3 = p->params[stageIx].p3;

    tserial res1 = INVALID_SERIAL, res2 = INVALID_SERIAL;
    for (int w=1; w<10; w++) {

        if (((z % 26) + p2) == w) {
            res1 = prog_minimize(p, stageIx+1, z/p1, serial*10 + w);
        } else {
            // When p1 == 26, we must utilize the divide by 26
            // We can do this branch (z grows) only when p1 == 1
            if (p1 == 1) {
                res2 = prog_minimize(p, stageIx+1, 26*(z/p1) + w + p3, serial*10 + w);
            }
        }

        if (res1 != INVALID_SERIAL && res1 > res2) {
            return res1;
        }
        if (res2 != INVALID_SERIAL) {
            return res2;
        }
    }
    return INVALID_SERIAL;
}

/*** Main parts ***/

long long part_one(FILE *fp) {
    const int buf_size = 256;
    char buf[buf_size];

    program prog;
    int n = 0;

    // Read params:
    while (fgets(buf, buf_size, fp) != NULL) {
        if (sscanf(buf, "%d,%d,%d", &prog.params[n].p1, &prog.params[n].p2, &prog.params[n].p3) == 3) {
            n++;
        }
    }
    if (n!=N_STAGES) {
        fatalf("unexpected number of stages: %d\n", n);
    }

    return prog_maximize(&prog, 0, 0, 0);
}

long long part_two(FILE *fp) {
    const int buf_size = 256;
    char buf[buf_size];

    program prog;
    int n = 0;

    // Read params:
    while (fgets(buf, buf_size, fp) != NULL) {
        if (sscanf(buf, "%d,%d,%d", &prog.params[n].p1, &prog.params[n].p2, &prog.params[n].p3) == 3) {
            n++;
        }
    }
    if (n!=N_STAGES) {
        fatalf("unexpected number of stages: %d\n", n);
    }

    return prog_minimize(&prog, 0, 0, 0);
}
