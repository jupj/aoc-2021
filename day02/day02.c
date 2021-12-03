#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

typedef struct {
    int horizontal;
    int depth;
} position;

bool apply_op_one(position *pos, char *dir, int val);
bool apply_op_two(position *pos, int *aim, char *dir, int val);

int part_one(FILE *fp, position *pos);
int part_two(FILE *fp, position *pos);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Please provide input file\n");
        return 1;
    }

    char *infile = argv[1];

    FILE *fp;
    fp = fopen(infile, "r");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open %s: %s\n", infile, strerror(errno));
        return 1;
    }

    position pos;
    if (part_one(fp, &pos) != 0) {
        return 1;
    }
    printf("Part one: %d\n", pos.horizontal*pos.depth);

    if (fseek(fp, 0, SEEK_SET)) {
        perror("fseek failed");
        return 1;
    }

    if (part_two(fp, &pos) != 0) {
        return 1;
    }
    printf("Part two: %d\n", pos.horizontal*pos.depth);

    fclose(fp);
    return 0;
}

int part_one(FILE *fp, position *pos) {
    pos->horizontal = 0;
    pos->depth = 0;

    char direction[100];
    int val;

    while (fscanf(fp, "%s %d", direction, &val) != EOF) {
        if (!(apply_op_one(pos, direction, val))) {
            fprintf(stderr, "unsupported direction %s\n", direction);
            return 1;
        }
    }

    return 0;
}

// apply_op_one carries out the operation (dir, val) on pos.
// Returns false if dir is not a valid op, true otherwise
bool apply_op_one(position *pos, char *dir, int val) {
    if (strcmp(dir, "forward") == 0) {
        pos->horizontal += val;
        return true;
    }

    if (strcmp(dir, "up") == 0) {
        pos->depth -= val;
        return true;
    }

    if (strcmp(dir, "down") == 0) {
        pos->depth += val;
        return true;
    }

    return false;
}

int part_two(FILE *fp, position *pos) {
    pos->horizontal = 0;
    pos->depth = 0;
    int aim = 0;

    char direction[100];
    int val;

    while (fscanf(fp, "%s %d", direction, &val) != EOF) {
        if (!(apply_op_two(pos, &aim, direction, val))) {
            fprintf(stderr, "unsupported direction %s\n", direction);
            return 1;
        }
    }

    return 0;
}

// apply_op_two carries out the operation (dir, val) on (pos, aim).
// Returns false if dir is not a valid op, true otherwise
bool apply_op_two(position *pos, int *aim, char *dir, int val) {
    if (strcmp(dir, "forward") == 0) {
        pos->horizontal += val;
        pos->depth += (*aim)*val;
        return true;
    }

    if (strcmp(dir, "up") == 0) {
        *aim -= val;
        return true;
    }

    if (strcmp(dir, "down") == 0) {
        *aim += val;
        return true;
    }

    return false;
}
