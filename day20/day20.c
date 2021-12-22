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

#define BUFSIZE 1024

typedef struct {
    char outside;
    char *alg;
    char output[BUFSIZE];
    int linewidth;
    char linebuffers[3][BUFSIZE];
    char *prev, *curr, *next;
} reader;

void reader_init(reader *r, char *alg, char outside) {
    r->outside = outside;
    r->alg = alg;
    r->linewidth = 0;
    // Initialize buffers as empty strings
    r->prev = r->linebuffers[0];
    r->prev[0] = '\0';
    r->curr = r->linebuffers[1];
    r->curr[0] = '\0';
    r->next = r->linebuffers[2];
    r->next[0] = '\0';
}

void reader_init_empty_line(reader *r, char *line) {
    for (int i=0; i<r->linewidth; i++) {
        line[i] = r->outside;
    }
    line[r->linewidth] = '\0';
}

void reader_readline(reader *r, char *input) {
    char *tmp = r->prev;
    r->prev = r->curr;
    r->curr = r->next;
    r->next = tmp;

    if (input == NULL || strlen(input) == 0) {
        // No input: create an empty dotted line
        char emptyline[BUFSIZE];
        reader_init_empty_line(r, emptyline);
        sprintf(r->next, "%s", emptyline);
    } else {
        // Process input:

        // Pad input
        sprintf(r->next, "%c%c%s%c%c", r->outside, r->outside, input, r->outside, r->outside);

        // Remove trailing '\n' from input
        char *end = strchr(input, '\n');
        if (end != NULL) {
            // Replace with terminating null
            *end = '\0';
        }
    }

    // Initialize prev and curr
    if (r->linewidth == 0) {
        r->linewidth = strlen(r->next);

        // If prev and curr have not been used: pad with '.'
        if (strlen(r->prev) != 0 && strlen(r->prev) != r->linewidth) {
            fatalf("line len corrupted\n");
        }
        reader_init_empty_line(r, r->prev);
        reader_init_empty_line(r, r->curr);
    }
}

void reader_process(reader *r, char* input) {
    reader_readline(r, input);

    // Run algorithm over current line
    for (int i=0; i<r->linewidth; i++) {
        int index = 0;
        for (int j=i-1; j<i+2; j++) {
            index = (index << 1);
            if (0 <= j && j < strlen(r->prev)) {
                if (r->prev[j] == '#') {
                    index += 1;
                }
            } else if (r->outside == '#') {
                index += 1;
            }
        }
        for (int j=i-1; j<i+2; j++) {
            index = (index << 1);
            if (0 <= j && j < strlen(r->curr)) {
                if (r->curr[j] == '#') {
                    index += 1;
                }
            } else if (r->outside == '#') {
                index += 1;
            }
        }
        for (int j=i-1; j<i+2; j++) {
            index = (index << 1);
            if (0 <= j && j < strlen(r->next)) {
                if (r->next[j] == '#') {
                    index += 1;
                }
            } else if (r->outside == '#') {
                index += 1;
            }
        }
        r->output[i] = r->alg[index];
    }
    r->output[r->linewidth] = '\0';
}

int count_pixels(char *line) {
    int n = 0;
    for (char *c=line; *c != '\0'; c++) {
        if (*c == '#') {
            n++;
        }
    }
    return n;
}

int enhance(FILE *fp, int iterations) {
    char alg[BUFSIZE];

    // Read image enhancement algorithm:
    if (!fgets(alg, BUFSIZE, fp)) {
        fatalf("no image enhancement algorithm\n");
    }

    reader readers[iterations];
    char outside = '.';
    for (int i=0; i<iterations; i++) {
        reader_init(&readers[i], alg, outside);
        if (outside == '.') {
            outside = alg[0];
        } else {
            outside = alg[0x1FF];
        }
    }

    char input[BUFSIZE];

    // Read empty line
    if (!fgets(input, BUFSIZE, fp)) {
        fatalf("could not read empty line\n");
    }

    int pixels = 0;
    while (fgets(input, BUFSIZE, fp)) {
        //dprintf("input    : %s\n", input);
        reader_process(&readers[0], input);
        for (int i=1; i<iterations; i++) {
            //dprintf("stage %2d: %s\n", i, readers[i-1].output);
            reader_process(&readers[i], readers[i-1].output);
        }
        pixels += count_pixels(readers[iterations-1].output);
        dprintf("out: %s\n", readers[iterations-1].output);
    }
    for (int i=0; i<iterations; i++) {
        // Add 2 empty lines for every iteration
        for (int j=0; j<2; j++) {
            reader_process(&readers[i], NULL);
            for (int k=i+1; k<iterations; k++) {
                reader_process(&readers[k], readers[k-1].output);
            }
            pixels += count_pixels(readers[iterations-1].output);
            dprintf("out: %s\n", readers[iterations-1].output);
        }
    }
    dprintf("line length: %d\n", readers[iterations-1].linewidth);
    return pixels;
}

int part_one(FILE *fp) {
    //return 0;
    return enhance(fp, 2);
}

unsigned long long part_two(FILE *fp) {
    return enhance(fp, 50);
}
