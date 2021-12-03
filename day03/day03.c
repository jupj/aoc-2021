#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#define MAXBITS 100

int part_one(FILE *fp);
int part_two(FILE *fp);

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

    printf("Part one: %d\n", part_one(fp));

    if (fseek(fp, 0, SEEK_SET)) {
        perror("fseek failed");
        return 1;
    }

    printf("Part two: %d\n", part_two(fp));

    fclose(fp);
    return 0;
}

int part_one(FILE *fp) {
    char binary[MAXBITS];
    int zeros[MAXBITS], ones[MAXBITS];

    for (int i=0; i<MAXBITS; i++) {
        zeros[i] = ones[i] = 0;
    }

    while (fscanf(fp, "%s", binary) != EOF) {
        for (int i=0; i<MAXBITS; i++) {
            if (binary[i] == '\0') {
                // no more digits
                break;
            }

            switch (binary[i]) {
            case '0':
                zeros[i]++;
                break;
            case '1':
                ones[i]++;
                break;
            default:
                fprintf(stderr, "invalid input data: %s\n", binary);
                return -1;
            }
        }
    }

    int gamma = 0, epsilon = 0;

    for (int i=0; i<MAXBITS; i++) {
        if (zeros[i] == 0 && ones[i] == 0) {
            // no more digits
            break;
        }

        if (zeros[i] > ones[i]) {
            gamma = gamma << 1;
            epsilon = (epsilon << 1) + 1;
        } else {
            gamma = (gamma << 1) + 1;
            epsilon = epsilon << 1;
        }
    }

    return gamma*epsilon;
}

typedef struct treenode {
    int n;
    char number[MAXBITS];
    struct treenode *zero, *one;
} tnode;

tnode *tree_new() {
    tnode *root = malloc(sizeof(tnode));
    root->n = 0;
    root->number[0] = '\0';
    root->zero = NULL;
    root->one = NULL;
    return root;
}

tnode *tree_add(tnode *node, bool is_one) {
    node->n++;

    if (is_one) {
        if (node->one == NULL) {
            node->one = tree_new();
        }
        return node->one;
    }

    if (node->zero == NULL) {
        node->zero = tree_new();
    }
    return node->zero;

}

void tree_free(tnode *root) {
    if (root->zero != NULL) {
        tree_free(root->zero);
    }
    if (root->one != NULL) {
        tree_free(root->one);
    }
    free(root);
}

int parse_binary(char *binary);

int part_two(FILE *fp) {
    char binary[MAXBITS];
    tnode *root = tree_new();

    while (fscanf(fp, "%s", binary) != EOF) {
        tnode *nod = root;
        for (int i=0; i<MAXBITS; i++) {
            strcpy(nod->number, binary);
            if (binary[i] == '\0') {
                // no more digits
                break;
            }

            switch (binary[i]) {
            default:
                fprintf(stderr, "invalid input data: %s\n", binary);
                return -1;
            case '0':
            case '1':
                nod = tree_add(nod, binary[i]=='1');
            }
        }
    }

    // Find oxygen generator rating
    tnode *nod = root;
    while (nod != NULL && nod->n > 1) {
        int zeros = 0, ones = 0;
        if (nod->zero != NULL) {
            zeros = nod->zero->n;
        }
        if (nod->one != NULL) {
            ones = nod->one->n;
        }

        if (zeros > ones) {
            nod = nod->zero;
        } else {
            nod = nod->one;
        }
    }

    int oxygen = parse_binary(nod->number);

    // Find CO2 scrubber rating
    nod = root;
    while (nod != NULL && nod->n > 1) {
        int zeros = 0, ones = 0;
        if (nod->zero != NULL) {
            zeros = nod->zero->n;
        }
        if (nod->one != NULL) {
            ones = nod->one->n;
        }

        if (zeros <= ones) {
            nod = nod->zero;
        } else {
            nod = nod->one;
        }
    }

    int co2 = parse_binary(nod->number);

    tree_free(root);

    return oxygen*co2;
}

int parse_binary(char *binary) {
    int n = 0;
    while (*binary) {
        n <<= 1;
        switch (*binary) {
        default:
            fprintf(stderr, "invalid bit %c\n", *binary);
            exit(1);
        case '1':
            n++;
        case '0':
            ;
        }
        binary++;
    }
    return n;
}
