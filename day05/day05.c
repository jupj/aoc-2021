#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define fatalf(...) { \
    fprintf(stderr, __VA_ARGS__); \
    exit(EXIT_FAILURE); \
}
#define efatal(msg) { \
    perror(msg); \
    exit(EXIT_FAILURE); \
}

/*#define DEBUG 1*/

#ifdef DEBUG
#define dprintf(...) printf(__VA_ARGS__);
#else
#define dprintf(...) /* NO-OP */
#endif

int part_one(FILE *fp);
int part_two(FILE *fp);

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

    printf("Part two: %d\n", part_two(fp));

    fclose(fp);
    return 0;
}

/*** Utils ***/

void print_grid(int **grid, int x_max, int y_max) {
    dprintf("\n");
    for (int y=0; y<=y_max; y++) {
        for (int x=0; x<=x_max; x++) {
            if (grid[x][y] == 0) {
                dprintf(".");
            } else {
                dprintf("%d", grid[x][y]);
            }
        }
        dprintf("\n");
    }

}

/*** Main parts ***/

int part_one(FILE *fp) {
    int x_max=-1;
    int y_max=-1;

    int x1, y1, x2, y2;
    while (fscanf(fp, "%d,%d -> %d,%d", &x1, &y1, &x2, &y2) == 4) {
        x_max = x1 > x_max ? x1 : x_max;
        x_max = x2 > x_max ? x2 : x_max;
        y_max = y1 > y_max ? y1 : y_max;
        y_max = y2 > y_max ? y2 : y_max;
    }

    // Allocate and initialize the grid
    int **grid = malloc((x_max+1)*sizeof(int*));
    for (int x=0; x<=x_max; x++) {
        grid[x] = malloc((y_max+1)*sizeof(int));
        for (int y=0; y<=y_max; y++) {
            grid[x][y] = 0;
        }
    }

    // Rewind the input
    if (fseek(fp, 0, SEEK_SET)) {
        efatal("fseek failed");
    }

    // Process the input
    while (fscanf(fp, "%d,%d -> %d,%d", &x1, &y1, &x2, &y2) == 4) {
        dprintf("Line (%d,%d) -> (%d,%d)\n", x1, y1, x2, y2);
        if (x1 == x2) {
            dprintf("vertical line\n");
            if (y2 < y1) {
                int tmp = y2;
                y2 = y1;
                y1 = tmp;
            }
            for (int y=y1; y<=y2; y++) {
                grid[x1][y]++;
            }
        } else if (y1 == y2) {
            dprintf("horizontal line\n");
            if (x2 < x1) {
                int tmp = x2;
                x2 = x1;
                x1 = tmp;
            }
            for (int x=x1; x<=x2; x++) {
                grid[x][y1]++;
            }
        }
        print_grid(grid, x_max, y_max);
    }

    // Count points where lines overlap
    int n = 0;
    for (int x=0; x<=x_max; x++) {
        for (int y=0; y<=y_max; y++) {
            if (grid[x][y] > 1) {
                n++;
            }
        }
    }

    for (int i=0; i<=x_max; i++) {
        free(grid[i]);
    }
    free(grid);
    return n;
}

int part_two(FILE *fp) {
    int x_max=-1;
    int y_max=-1;

    int x1, y1, x2, y2;
    while (fscanf(fp, "%d,%d -> %d,%d", &x1, &y1, &x2, &y2) == 4) {
        x_max = x1 > x_max ? x1 : x_max;
        x_max = x2 > x_max ? x2 : x_max;
        y_max = y1 > y_max ? y1 : y_max;
        y_max = y2 > y_max ? y2 : y_max;
    }

    // Allocate and initialize the grid
    int **grid = malloc((x_max+1)*sizeof(int*));
    for (int x=0; x<=x_max; x++) {
        grid[x] = malloc((y_max+1)*sizeof(int));
        for (int y=0; y<=y_max; y++) {
            grid[x][y] = 0;
        }
    }

    // Rewind the input
    if (fseek(fp, 0, SEEK_SET)) {
        efatal("fseek failed");
    }

    // Process the input
    while (fscanf(fp, "%d,%d -> %d,%d", &x1, &y1, &x2, &y2) == 4) {
        dprintf("Line (%d,%d) -> (%d,%d)\n", x1, y1, x2, y2);
        if (x1 == x2) {
            dprintf("vertical line\n");
            if (y2 < y1) {
                int tmp = y2;
                y2 = y1;
                y1 = tmp;
            }
            for (int y=y1; y<=y2; y++) {
                grid[x1][y]++;
            }
        } else if (y1 == y2) {
            dprintf("horizontal line\n");
            if (x2 < x1) {
                int tmp = x2;
                x2 = x1;
                x1 = tmp;
            }
            for (int x=x1; x<=x2; x++) {
                grid[x][y1]++;
            }
        } else {
            dprintf("diagnoal line\n");
            int n = x1 < x2 ? (x2-x1+1) : (x1-x2+1);
            for (int i=0; i<n; i++) {
                int x = x1 < x2 ? (x1 + i) : (x1 - i);
                int y = y1 < y2 ? (y1 + i) : (y1 - i);
                grid[x][y]++;
            }
        }
        print_grid(grid, x_max, y_max);
    }

    // Count points where lines overlap
    int n = 0;
    for (int x=0; x<=x_max; x++) {
        for (int y=0; y<=y_max; y++) {
            if (grid[x][y] > 1) {
                n++;
            }
        }
    }

    for (int i=0; i<=x_max; i++) {
        free(grid[i]);
    }
    free(grid);

    return n;
}
