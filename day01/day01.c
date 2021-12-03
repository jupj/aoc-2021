#include <stdio.h>
#include <string.h>
#include <errno.h>
#define INFILE "input01.txt"

int count_increased(FILE *fp);
int count_window_increased(FILE *fp);

int main() {
    FILE *fp;
    fp = fopen(INFILE, "r");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open %s: %s\n", INFILE, strerror(errno));
        return 1;
    }

    printf("Day 1, part one: %d\n", count_increased(fp));

    if (fseek(fp, 0, SEEK_SET)) {
        perror("fseek failed");
        return 1;
    }

    printf("Day 1, part two: %d\n", count_window_increased(fp));

    fclose(fp);
    return 0;
}

int count_increased(FILE *fp) {
    int prev, val;
    int n = 0;

    if (fscanf(fp, "%d", &prev) == EOF) {
        return EOF;
    }

    while (1) {
        if (fscanf(fp, "%d", &val) == EOF) {
            return n;
        }

        if (val > prev) {
            n++;
        }
        prev = val;
    }
}

int count_window_increased(FILE *fp) {
    int prev;
    int vals[3];
    int n = 0;

    // Read initial sum
    for (int i=0; i<3; i++) {
        if (fscanf(fp, "%d", &vals[i]) == EOF) {
            return EOF;
        }
    }

    prev = vals[0]+vals[1]+vals[2];
    int ix = 0;

    while (fscanf(fp, "%d", &vals[ix]) != EOF) {
        int sum = vals[0]+vals[1]+vals[2];
        if (sum > prev) {
            n++;
        }
        prev = sum;
        ix = (ix + 1) % 3;
    }
    return n;
}
