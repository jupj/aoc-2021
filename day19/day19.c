#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#define DEBUG
#include "utils.h"

#define MAX_BEACONS 720
#define MAX_SCANNERS 30

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

/*** Coordinate system ***/

typedef struct {
    int x, y, z;
} point;

point diff(point a, point b) {
    point c = {
        .x = a.x - b.x,
        .y = a.y - b.y,
        .z = a.z - b.z,
    };
    return c;
}

point add(point a, point b) {
    point c = {
        .x = a.x + b.x,
        .y = a.y + b.y,
        .z = a.z + b.z,
    };
    return c;
}

bool equal(point a, point b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

typedef int rotation[3][3];

rotation rotX90 = {
    {1, 0,  0},
    {0, 0, -1},
    {0, 1,  0},
};

rotation rotY90 = {
    { 0, 0, 1},
    { 0, 1, 0},
    {-1, 0, 0},
};

rotation rotZ90 = {
    {0, -1, 0},
    {1,  0, 0},
    {0,  0, 1},
};

point rotate(point p, rotation r) {
    point res = {
        .x = r[0][0]*p.x + r[0][1]*p.y + r[0][2]*p.z,
        .y = r[1][0]*p.x + r[1][1]*p.y + r[1][2]*p.z,
        .z = r[2][0]*p.x + r[2][1]*p.y + r[2][2]*p.z,
    };
    return res;
}

// rot_mul multiplies rotation matrices a*b and stores the result into a
void rot_mul(rotation a, rotation b) {
    for (int i=0; i<3; i++) {
        int a_i_old[3] = {a[i][0], a[i][1], a[i][2]};
        for (int j=0; j<3; j++) {
            a[i][j] = 0;
            for (int k=0; k<3; k++) {
                a[i][j] += a_i_old[k]*b[k][j];
            }
        }
    }
}

#define N_ROT 24
rotation rotations[N_ROT];

void init_non_rotation(rotation r) {
    r[0][0] = 1;
    r[0][1] = 0;
    r[0][2] = 0;
    r[1][0] = 0;
    r[1][1] = 1;
    r[1][2] = 0;
    r[2][0] = 0;
    r[2][1] = 0;
    r[2][2] = 1;
}

void init_rotations() {
    // Initialize rotations for facing directions
    // rotate around y axis, and 90, 270 degrees along z-axis
    rotation rot_facing[6];
    init_non_rotation(rot_facing[0]);

    init_non_rotation(rot_facing[1]);
    rot_mul(rot_facing[1], rotY90);

    init_non_rotation(rot_facing[2]);
    rot_mul(rot_facing[2], rotY90);
    rot_mul(rot_facing[2], rotY90);

    init_non_rotation(rot_facing[3]);
    rot_mul(rot_facing[3], rotY90);
    rot_mul(rot_facing[3], rotY90);
    rot_mul(rot_facing[3], rotY90);
    
    init_non_rotation(rot_facing[4]);
    rot_mul(rot_facing[4], rotZ90);

    init_non_rotation(rot_facing[5]);
    rot_mul(rot_facing[5], rotZ90);
    rot_mul(rot_facing[5], rotZ90);
    rot_mul(rot_facing[5], rotZ90);

    // Initialize rotations for up direction: rotate around x axis
    rotation rot_up[4];
    init_non_rotation(rot_up[0]);

    init_non_rotation(rot_up[1]);
    rot_mul(rot_up[1], rotX90);

    init_non_rotation(rot_up[2]);
    rot_mul(rot_up[2], rotX90);
    rot_mul(rot_up[2], rotX90);

    init_non_rotation(rot_up[3]);
    rot_mul(rot_up[3], rotX90);
    rot_mul(rot_up[3], rotX90);
    rot_mul(rot_up[3], rotX90);

    // Initialize without rotation
    for (int i=0; i<6; i++) {
        for (int j=0; j<4; j++) {
            int ix = 4*i + j;
            init_non_rotation(rotations[ix]);
            rot_mul(rotations[ix], rot_facing[i]);
            rot_mul(rotations[ix], rot_up[j]);
        }
    };
}

/*** Scanners & beacons ***/

typedef struct {
    point beacons[MAX_BEACONS];
    int n;
    point offset;
} scanner;

void add_beacon(scanner *s, int x, int y, int z) {
    s->beacons[s->n].x = x;
    s->beacons[s->n].y = y;
    s->beacons[s->n].z = z;
    s->n = s->n + 1;
}

// overlap returns true if a and b overlap with at least 12 scanners
// rotationIx and offset are updated, and if overlap is found, they describe how to reconstruct the overlap
bool overlap(int *rotationIx, point *offset, scanner *a, scanner *b) {
    for (int b0=0; b0<(a->n-11); b0++) {
        // Try all beacons in a as a reference point,
        // until there are less than 12 left...
        point beacon0 = a->beacons[b0];

        // try all rotations of b
        for (*rotationIx=0; *rotationIx<N_ROT; (*rotationIx)++) {
            for (int i=0; i<b->n; i++) {
                // pick a beacon in b as reference, and equal it to beacon0
                point beacon = rotate(b->beacons[i], rotations[*rotationIx]);
                *offset = diff(beacon0, beacon);
                int matchcnt = 1;

                for (int j=0; j<b->n; j++) {
                    if (j == i) {
                        continue;
                    }
                    if ((b->n - j) < (12 - matchcnt)) {
                        // no chance to match 12 beacons
                        break;
                    }

                    point p = add(rotate(b->beacons[j], rotations[*rotationIx]), *offset);
                    // search for match in a
                    for (int k=0; k<a->n; k++) {
                        if (equal(p, a->beacons[k])) {
                            dprintf("match: %d, %d, %d\n", p.x, p.y, p.z);
                            matchcnt++;
                            break;
                        }
                    }
                    if (matchcnt >= 12) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

// extend adds the beacons from src to dest
void extend(scanner *dest, scanner *src, int rotIx, point offset) {
    for (int i=0; i<src->n; i++) {
        point p = add(rotate(src->beacons[i], rotations[rotIx]), offset);
        bool found = false;
        for (int j=0; j<dest->n; j++) {
            if (equal(p, dest->beacons[j])) {
                found = true;
                break;
            }
        }
        if (!found) {
            dest->beacons[dest->n] = p;
            dest->n++;
        }
    }
}

/*** Main parts ***/

scanner scanners[MAX_SCANNERS];
int scannerCnt = 0;

int part_one(FILE *fp) {
    init_rotations();

    const int bufsize = 1024;
    char buf[bufsize];

    bool found_overlap[MAX_SCANNERS];

    while (fgets(buf, bufsize, fp)) {
        int id;
        if (sscanf(buf, "--- scanner %d ---", &id) == 1) {
            if (id != scannerCnt) {
                fatalf("invalid scanner id\n");
            }
            scannerCnt++;
            scanners[id].n = 0;
            scanners[id].offset.x = 0;
            scanners[id].offset.y = 0;
            scanners[id].offset.z = 0;
            found_overlap[id] = false;
            continue;
        }

        int x, y, z;
        if (sscanf(buf, "%d,%d,%d", &x, &y, &z) == 3) {
            dprintf("add beacon\n");
            add_beacon(&scanners[scannerCnt-1], x, y, z);
            continue;
        }
    }

    // Build map around scanner 0
    scanner s0 = scanners[0];
    found_overlap[0] = true;

    bool scanners_left = true;
    while (scanners_left) {
        scanners_left = false;
        for (int i=1; i<scannerCnt; i++) {
            if (found_overlap[i]) {
                continue;
            }

            int rotIx;
            point offset;
            if (overlap(&rotIx, &offset, &s0, &scanners[i])) {
                dprintf("found match for %d\n", i);
                found_overlap[i] = true;
                extend(&s0, &scanners[i], rotIx, offset);
                scanners[i].offset = offset;
            } else  {
                scanners_left = true;
            }
        }
    }

    return s0.n;
}

unsigned long long part_two(FILE *fp) {
    int max_dist = 0;
    for (int i=0; i<scannerCnt; i++) {
        for (int j=0; j<scannerCnt; j++) {
            int d = abs(scanners[i].offset.x - scanners[j].offset.x) +
                abs(scanners[i].offset.y - scanners[j].offset.y) +
                abs(scanners[i].offset.z - scanners[j].offset.z);
            if (d > max_dist) {
                max_dist = d;
            }
        }
    }
    return max_dist;
}
