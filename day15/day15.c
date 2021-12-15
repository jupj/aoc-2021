#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#define DEBUG
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

const int x_max=500, y_max=500;
const int N = x_max*y_max;
const int bufsize = 1024;

int pos(int x, int y) { return y*y_max + x; }
void coord(int pos, int *x, int *y) {
    *x = pos % y_max;
    *y = pos / y_max;
}

int up(int pos) { return pos-y_max; }
int down(int pos) { return pos+y_max; }
int left(int pos) { return pos-1; }
int right(int pos) { return pos+1; }

typedef struct {
    int *elems;
    int cap, len;
} heap;

heap heap_init() {
    heap h = {
        .elems = NULL,
        .cap = 0,
        .len = 0,
    };
    return h;
}

void heap_free(heap *h) {
    free(h->elems);
}

bool heap_empty(heap *h) { return h->len == 0; }

void heap_insert(heap *h, int pos, int *cost) {
    if (h->elems == NULL) {
        // Initialize empty heap
        h->elems = malloc(sizeof(int));
        h->len = h->cap = 1;
        h->elems[0] = pos;
        return;
    }

    if (h->len >= h->cap) {
        // Grow heap
        h->cap = 2*h->cap + 1;
        h->elems = realloc(h->elems, h->cap*sizeof(int));
    }

    // Add to bottom level, first free space
    h->elems[h->len] = pos;
    int k = h->len;
    h->len++;

    // Satisfy the heap property: parent must cost less than child
    while (k > 0) {
        int parent = (k-1)/2;
        if (cost[h->elems[k]] < cost[h->elems[parent]]) {
            // Swap with parent
            int tmp = h->elems[k];
            h->elems[k] = h->elems[parent];
            h->elems[parent] = tmp;
        }
        k = parent;
    }
}

int heap_extract(heap *h, int *cost) {
    if (h->len == 0) {
        return -1;
    }

    h->len--;
    int extracted = h->elems[0];
    h->elems[0] = h->elems[h->len];

    // Satisfy the heap property: parent must cost less than child
    int k = 0;
    while (k < h->len/2) {
        // Find the minimum child node
        int left = 2*k + 1;
        int right = 2*k + 2;
        int min_child = left;
        if (right < h->len && cost[h->elems[right]] < cost[h->elems[left]]) {
            min_child = right;
        }
        
        if (cost[h->elems[min_child]] >= cost[h->elems[k]]) {
            // Heap property satisfied!
            break;
        }

        // swap parent and child
        int tmp = h->elems[k];
        h->elems[k] = h->elems[min_child];
        h->elems[min_child] = tmp;
        k = min_child;
    }

    return extracted;
}


int part_one(FILE *fp) {
    int risk[N], cost[N];
    int cols = 0, rows = 0;

    // Initialize risks and costs
    for (int i=0; i<N; i++) {
        risk[i] = 0;
        cost[i] = 0;
    }

    // Read the risk map
    char buf[bufsize];
    while(fgets(buf, bufsize, fp)) {
        char row[bufsize];
        if (sscanf(buf, "%s", row) == 1) {
            if (rows > y_max) {
                fatalf("too many rows");
            }
            for (int x=0; row[x]; x++) {
                risk[pos(x,rows)] = row[x] - '0';

                if (x >= cols) {
                    cols = x+1;
                    if (cols > x_max) {
                        fatalf("too many cols");
                    }
                }
            }
            rows++;
        }
    }

    // Djikstra's search algorithm
    // Use heap as priority queue
    heap h = heap_init();
    int start = 0;
    int goal = pos(cols-1, rows-1);
    heap_insert(&h, start, cost);

    while (!heap_empty(&h)) {
        int node = heap_extract(&h, cost);
        if (node == goal) {
            break;
        }
        int x, y;
        coord(node, &x, &y);

        // Add neighbors
        if (x > 0) {
            int next = left(node);
            int newcost = cost[node] + risk[next];
            if (!cost[next] || cost[next] > newcost) {
                cost[next] = newcost;
                heap_insert(&h, next, cost);
            }
        }
        if (x < cols-1) {
            int next = right(node);
            int newcost = cost[node] + risk[next];
            if (!cost[next] || cost[next] > newcost) {
                cost[next] = newcost;
                heap_insert(&h, next, cost);
            }
        }
        if (y > 0) {
            int next = up(node);
            int newcost = cost[node] + risk[next];
            if (!cost[next] || cost[next] > newcost) {
                cost[next] = newcost;
                heap_insert(&h, next, cost);
            }
        }
        if (y < rows-1) {
            int next = down(node);
            int newcost = cost[node] + risk[next];
            if (!cost[next] || cost[next] > newcost) {
                cost[next] = newcost;
                heap_insert(&h, next, cost);
            }
        }
    }
    heap_free(&h);

    return cost[goal];
}

unsigned long long part_two(FILE *fp) {
    int risk[N], cost[N];
    int cols = 0, rows = 0;

    // Initialize risks and costs
    for (int i=0; i<N; i++) {
        risk[i] = 0;
        cost[i] = 0;
    }

    // Read the risk map
    char buf[bufsize];
    while(fgets(buf, bufsize, fp)) {
        char row[bufsize];
        if (sscanf(buf, "%s", row) == 1) {
            if (rows > y_max) {
                fatalf("too many rows");
            }
            for (int x=0; row[x]; x++) {
                risk[pos(x,rows)] = row[x] - '0';

                if (x >= cols) {
                    cols = x+1;
                    if (cols > x_max) {
                        fatalf("too many cols");
                    }
                }
            }
            rows++;
        }
    }

    // Duplicate width 5 times, wrapping from 9 => 1
    for (int i=1; i<5; i++) {
        for (int x=0; x<cols; x++) {
            for (int y=0; y<rows; y++) {
                int cur = pos(i*cols+x,y);
                int left = pos((i-1)*cols+x,y);
                risk[cur] = risk[left] + 1;
                if (risk[cur] > 9) {
                    risk[cur] = 1;
                }
            }
        }
    }
    cols = 5*cols;
    // Duplicate height 5 times, wrapping from 9 => 1
    for (int i=1; i<5; i++) {
        for (int x=0; x<cols; x++) {
            for (int y=0; y<rows; y++) {
                int cur = pos(x,i*rows+y);
                int up = pos(x,(i-1)*rows+y);
                risk[cur] = risk[up] + 1;
                if (risk[cur] > 9) {
                    risk[cur] = 1;
                }
            }
        }
    }
    rows = 5*rows;

    // Djikstra's search algorithm
    // Use heap as priority queue
    heap h = heap_init();
    int start = 0;
    int goal = pos(cols-1, rows-1);
    heap_insert(&h, start, cost);

    while (!heap_empty(&h)) {
        int node = heap_extract(&h, cost);
        if (node == goal) {
            break;
        }
        int x, y;
        coord(node, &x, &y);

        // Add neighbors
        if (x > 0) {
            int next = left(node);
            int newcost = cost[node] + risk[next];
            if (!cost[next] || cost[next] > newcost) {
                cost[next] = newcost;
                heap_insert(&h, next, cost);
            }
        }
        if (x < cols-1) {
            int next = right(node);
            int newcost = cost[node] + risk[next];
            if (!cost[next] || cost[next] > newcost) {
                cost[next] = newcost;
                heap_insert(&h, next, cost);
            }
        }
        if (y > 0) {
            int next = up(node);
            int newcost = cost[node] + risk[next];
            if (!cost[next] || cost[next] > newcost) {
                cost[next] = newcost;
                heap_insert(&h, next, cost);
            }
        }
        if (y < rows-1) {
            int next = down(node);
            int newcost = cost[node] + risk[next];
            if (!cost[next] || cost[next] > newcost) {
                cost[next] = newcost;
                heap_insert(&h, next, cost);
            }
        }
    }
    heap_free(&h);

    return cost[goal];
}
