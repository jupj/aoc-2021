#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

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

    printf("Part two: %llu\n", part_two(fp));

    fclose(fp);
    return 0;
}

/*** Main parts ***/

const int max_x = 10;
const int max_y = 10;

void print_octopi(int octopi[][max_x]) {
    for (int y=0; y<max_y; y++) {
        for (int x=0; x<max_x; x++) {
            dprintf("%d", octopi[x][y]);
        }
        dprintf("\n");
    }
    dprintf("\n");
}

typedef struct queue_element queue;
struct queue_element {
    int x, y;
    queue *next;
};

queue *enqueue(queue *q, int x, int y) {
    queue *elem = malloc(sizeof(queue));
    elem->x = x;
    elem->y = y;
    elem->next = NULL;

    if (q == NULL) {
        // Init new queue
        return elem;
    }

    // Append to queue
    queue *ptr = q;
    while (ptr->next != NULL) {
        ptr = ptr->next;
    }
    ptr->next = elem;

    return q;
}

queue *dequeue(int *x, int *y, queue *q) {
    if (q == NULL) {
        return NULL;
    }

    *x = q->x;
    *y = q->y;
    queue *next = q->next;
    free(q);
    return next;
}

int part_one(FILE *fp) {
    int octopi[max_y][max_x];

    const int line_len = max_x+2; // 2 extra for "\n\0"
    char line[line_len];
    int y = 0;
    while (fgets(line, line_len, fp) != NULL) {
        for (int x=0; x<max_x; x++) {
            octopi[x][y] = line[x] - '0';
        }
        y++;
    }

    dprintf("\nBefore any step:\n");
    print_octopi(octopi);
    int flashcnt = 0;

    for (int step=1; step<=100; step++) {
        // Increment all octopi, queue octopi with level over 9
        queue *q = NULL;
        for (int x=0; x<max_x; x++) {
            for (int y=0; y<max_y; y++) {
                octopi[x][y]++;
                if (octopi[x][y] > 9) {
                    q = enqueue(q, x, y);
                }
            }
        }

        // Flash all queued octopi, including adjacent flashing octopi
        int x, y;
        while (q != NULL) {
            q = dequeue(&x, &y, q);
            if (octopi[x][y] == 0) {
                // flashed already
                continue;
            }

            octopi[x][y] = 0;
            flashcnt++;

            // Increment adjacent octopi that hasn't flashed
            for (int i=-1; i<2; i++) {
                for (int j=-1; j<2; j++) {
                    if ((x+i) >= 0 &&
                        (x+i) < max_x &&
                        (y+j) >= 0 &&
                        (y+j) < max_y &&
                        octopi[x+i][y+j] > 0) {
                        octopi[x+i][y+j]++;
                        if (octopi[x+i][y+j] > 9) {
                            q = enqueue(q, x+i, y+j);
                        }
                    }
                }
            }
        }

        dprintf("After step %d: %d flashes\n", step, flashcnt);
        //print_octopi(octopi);
    }
    return flashcnt;
}

unsigned long long part_two(FILE *fp) {
    int octopi[max_y][max_x];

    const int line_len = max_x+2; // 2 extra for "\n\0"
    char line[line_len];
    int y = 0;
    while (fgets(line, line_len, fp) != NULL) {
        for (int x=0; x<max_x; x++) {
            octopi[x][y] = line[x] - '0';
        }
        y++;
    }

    dprintf("\nBefore any step:\n");
    print_octopi(octopi);

    int steps = 0;
    int flashcnt = 0;
    while (flashcnt < (max_x*max_y)) {
        // Increment all octopi, queue octopi with level over 9
        queue *q = NULL;
        for (int x=0; x<max_x; x++) {
            for (int y=0; y<max_y; y++) {
                octopi[x][y]++;
                if (octopi[x][y] > 9) {
                    q = enqueue(q, x, y);
                }
            }
        }

        // Flash all queued octopi, including adjacent flashing octopi
        int x, y;
        flashcnt = 0;
        while (q != NULL) {
            q = dequeue(&x, &y, q);
            if (octopi[x][y] == 0) {
                // flashed already
                continue;
            }

            octopi[x][y] = 0;
            flashcnt++;

            // Increment adjacent octopi that hasn't flashed
            for (int i=-1; i<2; i++) {
                for (int j=-1; j<2; j++) {
                    if ((x+i) >= 0 &&
                        (x+i) < max_x &&
                        (y+j) >= 0 &&
                        (y+j) < max_y &&
                        octopi[x+i][y+j] > 0) {
                        octopi[x+i][y+j]++;
                        if (octopi[x+i][y+j] > 9) {
                            q = enqueue(q, x+i, y+j);
                        }
                    }
                }
            }
        }

        steps++;
    }
    return steps;
}
