#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

//#define DEBUG
#include "utils.h"

#define LINE_MAX 128

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

    printf("Part two: %llu\n", part_two(fp));

    fclose(fp);
    return 0;
}

/*** Main parts ***/

struct result {
    enum { COMPLETE, INCOMPLETE, CORRUPTED } code;
    unsigned long long score;
};

struct result check(char *input) {
    char stack[LINE_MAX];
    int sp = -1;

    for (int pos=0; input[pos] != '\0'; pos++) {
        char c = input[pos];

        // opening brackets: push corresponding closing bracket on stack
        if (c == '(') {
            sp++;
            stack[sp] = ')';
            continue;
        }
        if (c == '[') {
            sp++;
            stack[sp] = ']';
            continue;
        }
        if (c == '{') {
            sp++;
            stack[sp] = '}';
            continue;
        }
        if (c == '<') {
            sp++;
            stack[sp] = '>';
            continue;
        }

        if (sp < 0) {
            // Stack is empty, we shouldn't have any non-opening input
            fatalf("check loop: unexpected '%d' at pos %d\n", c, pos);
        }

        if (c == stack[sp]) {
            // Found matching closing bracket on stack. Pop it.
            sp--;
            continue;
        }

        // Stack has contents, but input doesn't match top element on stack.
        // => input is corrupted
        struct result res = { CORRUPTED, 0 };
        dprintf("`%s` - Expected `%c`, but found `%c` instead\n", input, stack[sp], c);
        // Return score
        switch (c) {
        case ')':
            res.score = 3;
            return res;
        case ']':
            res.score = 57;
            return res;
        case '}':
            res.score = 1197;
            return res;
        case '>':
            res.score = 25137;
            return res;
        }
        fatalf("check: corrupted line has unexpected '%d' at pos %d\n", c, pos);
    }

    // Line ended unexpectedly
    struct result res = { INCOMPLETE, 0 };
    stack[sp+1] = '\0';
    dprintf("`%s` - Complete by adding `%s`.\n", input, stack);

    for (int i=sp; i>=0; i--) {
        res.score = res.score*5;
        switch (stack[i]) {
        case ')':
            res.score += 1;
            break;
        case ']':
            res.score += 2;
            break;
        case '}':
            res.score += 3;
            break;
        case '>':
            res.score += 4;
            break;
        }
    }
    return res;
}

int part_one(FILE *fp) {
    int score = 0;

    char line[LINE_MAX];
    while (fgets(line, LINE_MAX, fp) != NULL) {
        // Remove '\n'
        line[strcspn(line, "\n")] = 0;

        struct result res = check(line);
        if (res.code == CORRUPTED) {
            score += res.score;
        }
        //dprintf("%d, %d : %s\n", res.code, res.score, line);
    }
    return score;
}

unsigned long long part_two(FILE *fp) {
    unsigned long long *scores = NULL;
    int len = 0;

    char line[LINE_MAX];
    while (fgets(line, LINE_MAX, fp) != NULL) {
        // Remove '\n'
        line[strcspn(line, "\n")] = 0;

        struct result res = check(line);
        if (res.code == INCOMPLETE) {
            len++;
            scores = realloc(scores, len*sizeof(unsigned long long *));
            scores[len-1] = res.score;

            for (int i=len-1; i>0; i--) {
                if (scores[i-1] > scores[i]) {
                    // swap
                    unsigned long long tmp = scores[i];
                    scores[i] = scores[i-1];
                    scores[i-1] = tmp;
                }
            }
        }
        //dprintf("%d, %d : %s\n", res.code, res.score, line);
    }

    for (int i=0; i<len; i++) {
        dprintf("Line %3d: %llu\n", i, scores[i]);
    }
    unsigned long long middle = scores[len/2];
    free(scores);
    return middle;
}
