#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#define DEBUG
#include "utils.h"
#define c2i(c)  (int)((c) - 'A')
#define i2c(n)  (char)((n) + 'A')

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
unsigned long long run_process(FILE *fp, int steps) {
    // Initialize rules table
    const int N = 'Z' - 'A';
    char rules[N][N];
    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            rules[i][j] = 0;
        }
    }

    // Read input
    const int bufsize = 1024;
    char buf[bufsize];
    char initial[bufsize];
    while (fgets(buf, bufsize, fp)) {
        char a, b, c;
        if (sscanf(buf, "%c%c -> %c", &a, &b, &c) == 3) {
            // Read rule
            rules[c2i(a)][c2i(b)] = c;
            continue;
        }

        if (sscanf(buf, "%s", initial) == 1) {
            // Read the starting point
            continue;
        }
    }

    // Initialize two pair count buffers
    unsigned long long pairs[2][N][N];
    int current = 0, next = 1;
    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            pairs[current][i][j] = 0;
        }
    }

    // Count initial pairs
    for (int i=1; initial[i]; i++) {
        pairs[current][c2i(initial[i-1])][c2i(initial[i])]++;
    }

    // Process the steps
    for (int step=0; step<steps; step++) {
        // Reset next pair counts
        for (int i=0; i<N; i++) {
            for (int j=0; j<N; j++) {
                pairs[next][i][j] = 0;
            }
        }

        // Expand current pairs into next
        for (int a=0; a<N; a++) {
            for (int b=0; b<N; b++) {
                int c = rules[a][b];
                if (c == 0) {
                    continue;
                }
                
                pairs[next][a][c2i(c)] += pairs[current][a][b];
                pairs[next][c2i(c)][b] += pairs[current][a][b];
            }
        }

        // Switch pair buffers
        int temp = next;
        next = current;
        current = temp;
    }

    // Initialize counts
    unsigned long long cnt[N];
    for (int i=0; i<N; i++) {
        cnt[i] = 0;
    }

    // Count the first element once
    cnt[c2i(initial[0])]++;
    // Then, count the latter part of each pair
    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            cnt[j] += pairs[current][i][j];
        }
    }

    // Find min and max
    unsigned long long min = 0, max = 0;
    for (int i=0; i<N; i++) {
        if (cnt[i] <= 0) {
            continue;
        }

        dprintf("%c: %llu\n", i2c(i), cnt[i]);
        if (min == 0 || cnt[i] < min) {
            min = cnt[i];
        }
        if (cnt[i] > max) {
            max = cnt[i];
        }
    }

    dprintf("max %lld, min %lld\n", max, min);
    return max - min;
}

int part_one(FILE *fp) {
    return run_process(fp, 10);
}

unsigned long long part_two(FILE *fp) {
    return run_process(fp, 40);
}
