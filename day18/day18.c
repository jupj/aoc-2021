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

/*** Pair ***/

typedef struct pair {
    int number;
    struct pair *left, *right;
} pair;

pair *pair_init() {
    pair *p= malloc(sizeof(pair));
    p->number = 0;
    p->left = NULL;
    p->right = NULL;
    return p;
}

void pair_free(pair *p) {
    if (p == NULL) { return; }

    if (p->left != NULL) {
        pair_free(p->left);
    }

    if (p->right != NULL) {
        pair_free(p->right);
    }

    free(p);
}

bool pair_isleaf(pair *p) {
    return p != NULL && p->left == NULL && p->right == NULL;
}

int pair_magnitude(pair *p) {
    if (pair_isleaf(p)) {
        return p->number;
    }

    return 3*pair_magnitude(p->left) + 2*pair_magnitude(p->right);
}

// find_explosive checks the tree for explosive pairs.
// If an explosive pair is found: explosive, last_regular and next_regular will be updated
bool find_explosive(pair *p, int level, pair **explosive, int **last_regular, int **next_regular) {
    if (*explosive != NULL) {
        // Already found explosive pair

        // Now we're searching for next_regular
        if (*next_regular != NULL) {
            // Already found next_regular
            return true;
        }

        if (pair_isleaf(p)) {
            *next_regular = &p->number;
            return true;
        }

        // Search for next_regular in sub-trees
        find_explosive(p->left, level+1, explosive, last_regular, next_regular);
        if (*next_regular == NULL) {
            find_explosive(p->right, level+1, explosive, last_regular, next_regular);
        }
        return true;
    }

    // Is p an explosive pair?
    if (level >= 4 && !pair_isleaf(p)) {
        if (!pair_isleaf(p->left) || !pair_isleaf(p->right)) {
            fatalf("cannot explode nested pair\n");
        }
        *explosive = p;
        return true;
    }

    // We didn't find an explosive pair yet - look for last_regular and explosive pair in sub-tree

    // Regular number - use as last_regular
    if (pair_isleaf(p)) {
        *last_regular = &p->number;
        return false;
    }

    bool res = find_explosive(p->left, level+1, explosive, last_regular, next_regular);
    if (*next_regular == NULL) {
        return find_explosive(p->right, level+1, explosive, last_regular, next_regular);
    }
    return res;
}

bool split_pair(pair *p) {
    if (!pair_isleaf(p)) {
        // Check for splits in subtrees
        if (split_pair(p->left)) {
            return true;
        }
        return split_pair(p->right);
    }

    if (p->number <= 9) {
        return false;
    }

    // Split this pair
    dprintf("split %d\n", p->number);
    p->left = pair_init();
    p->left->number = p->number/2;
    p->right = pair_init();
    p->right->number = p->number/2 + p->number%2;
    p->number = 0;
    return true;
}

// pair_reduce performs a reduction on p.
// Return true if a pair exploded or split.
bool pair_reduce(pair *p) {
    pair *explosive = NULL;
    int *last_regular = NULL, *next_regular = NULL;

    // First, explode pairs
    if (find_explosive(p, 0, &explosive, &last_regular, &next_regular)) {
        // explode
        if (explosive == NULL) {
            fatalf("cannot explode NULL\n");
        }

        dprintf("explode [%d,%d]\n", explosive->left->number, explosive->right->number);
        // update number to the left
        if (last_regular != NULL) {
            *last_regular += explosive->left->number;
        }
        // update number to the right
        if (next_regular != NULL) {
            *next_regular += explosive->right->number;
        }

        // reduce exploded pair to 0
        pair_free(explosive->left);
        explosive->left = NULL;
        pair_free(explosive->right);
        explosive->right = NULL;
        explosive->number = 0;
        return true;
    }

    return split_pair(p);
}

void pair_print(pair *p) {
    if (pair_isleaf(p)) {
        dprintf("%d", p->number);
        return;
    }

    dprintf("[");
    pair_print(p->left);
    dprintf(",");
    pair_print(p->right);
    dprintf("]");
}

pair *pair_add(pair *a, pair *b) {
    if (a == NULL) { return b; }
    if (b == NULL) { return a; }

    pair *root = malloc(sizeof(pair));
    root->number = 0;
    root->left = a;
    root->right = b;

    pair_print(root);
    dprintf("\n");
    while(pair_reduce(root)) {
        pair_print(root);
        dprintf("\n");
    }
    return root;
}

/*** Parser ***/

typedef struct {
    char *input;
    int pos;
} parser;

char parser_next(parser *p) {
    char c = p->input[p->pos];
    if (c == '\0') {
        fatalf("unexpected EOF\n");
    }
    p->pos++;
    return c;
}

pair *parse(parser *p) {
    char c = parser_next(p);

    pair *new = NULL;

    switch (c) {
    case '#':
        // Comment until end of line
        return NULL;
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        // return regular pair
        new = pair_init();
        new->number = (int) (c - '0');
        return new;
    case '[':
        new = pair_init();
        new->left = parse(p);
        if (parser_next(p) != ',') {
            fatalf("expected ',' got '%c'\n", c);
        }
        new->right = parse(p);
        if (parser_next(p) != ']') {
            fatalf("expected ']' got '%c'\n", c);
        }
        return new;
    default:
        fatalf("expected number or '[' got '%c'\n", c);
    }
}

/*** Main parts ***/

int part_one(FILE *fp) {
    const int bufsize = 1024;
    char buf[bufsize];

    pair *p = NULL;
    while (fgets(buf, bufsize, fp)) {
        //dprintf("Input: %s\n", buf);
        parser prs = { buf, 0 };
        p = pair_add(p, parse(&prs));
        //pair_print(p);
        //dprintf("\n");
    }
    int mag = pair_magnitude(p);
    pair_free(p);
    return mag;
}

unsigned long long part_two(FILE *fp) {
    const int bufsize = 100;
    char numbers[101][bufsize];
    int n = 0;

    while (n < 101 && fgets(numbers[n], bufsize, fp)) {
        n++;
    }
    if (n == 101) {
        fatalf("too many numbers\n");
    }

    int max_mag = 0;
    for (int i=0; i<n; i++) {
        for (int j=0; j<n; j++) {
            if (i == j) {
                continue;
            }
            parser prs = { numbers[i], 0 };
            pair *p1 = parse(&prs);
            prs.input = numbers[j];
            prs.pos = 0;
            pair *p2 = parse(&prs);

            pair *sum = pair_add(p1, p2);
            int mag = pair_magnitude(sum);
            if (mag > max_mag) {
                max_mag = mag;
            }
        }
    }
    return max_mag;
}
