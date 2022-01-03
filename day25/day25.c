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

/*** Main parts ***/

#define MAP_MAX 256

typedef struct {
    char grids[2][MAP_MAX][MAP_MAX];
    int curr, next;
    int width, height;
} map;

int step(map *m) {
    int cnt = 0;

    // Move eastwards
    for (int y=0; y<m->height; y++) {
        for (int x=0; x<m->width; x++) {
            int x1 = (x+1) % m->width;
            if (m->grids[m->curr][y][x] == '>' && m->grids[m->curr][y][x1] == '.') {
                m->grids[m->next][y][x] = '.';
                m->grids[m->next][y][x1] = '>';
                x++;
                cnt++;
            } else {
                m->grids[m->next][y][x] = m->grids[m->curr][y][x];
            }
        }
    }
    int tmp = m->curr;
    m->curr = m->next;
    m->next = tmp;

    // Move downwards
    for (int x=0; x<m->width; x++) {
        for (int y=0; y<m->height; y++) {
            int y1 = (y+1) % m->height;
            if (m->grids[m->curr][y][x] == 'v' && m->grids[m->curr][y1][x] == '.') {
                m->grids[m->next][y][x] = '.';
                m->grids[m->next][y1][x] = 'v';
                y++;
                cnt++;
            } else {
                m->grids[m->next][y][x] = m->grids[m->curr][y][x];
            }
        }
    }
    tmp = m->curr;
    m->curr = m->next;
    m->next = tmp;

    return cnt;
}

long long part_one(FILE *fp) {
    map m;
    m.curr = 0;
    m.next = 1;
    m.width = 0;
    m.height = 0;
    // Read map:
    while (fgets(m.grids[m.curr][m.height], MAP_MAX, fp) != NULL) {
        char *c = m.grids[m.curr][m.height];
        while (*c) {
            if (*c == '\n') {
                *c = '\0';
            } else if (strchr(">v.", *c) == NULL) {
                fatalf("invalid char %c (%d)\n", *c, *c);
            }
            c++;
        }
        if (m.width == 0) {
            m.width = strlen(m.grids[m.curr][m.height]);
        }
        if (m.width != strlen(m.grids[m.curr][m.height])) {
            fatalf("invalid width\n");
        }
        m.height++;
    }

    int n = 1;
    while (step(&m) > 0) {
        n++;
    }

    return n;
}

long long part_two(FILE *fp) {
    return 0;
}
