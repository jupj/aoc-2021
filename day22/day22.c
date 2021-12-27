#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#define DEBUG
#include "utils.h"

int part_one(FILE *fp);
unsigned long long part_two(FILE *fp);
void unit_test();

int main(int argc, char *argv[]) {
    //unit_test();
    //return 1;
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

#define BUFSIZE 1024
#define OFFSET 50
#define N_CUBES 101
#define MIN -50
#define MAX 50

int part_one(FILE *fp) {
    bool cubes[N_CUBES][N_CUBES][N_CUBES];
    for (int x=MIN; x<=MAX; x++) {
        for (int y=MIN; y<=MAX; y++) {
            for (int z=MIN; z<=MAX; z++) {
                cubes[x+OFFSET][y+OFFSET][z+OFFSET] = false;
            }
        }
    }

    char input[BUFSIZE];

    while (fgets(input, BUFSIZE, fp)) {
        int x1, x2, y1, y2, z1, z2;
        char state[10];
        if (sscanf(input, "%s x=%d..%d,y=%d..%d,z=%d..%d", state, &x1, &x2, &y1, &y2, &z1, &z2) == 7) {
            if (x1 > MAX || y1 > MAX || z1 > MAX) {
                continue;
            }
            if (x2 < MIN || y2 < MIN || z2 < MIN) {
                continue;
            }
            x1 = x1 < MIN ? MIN : x1;
            y1 = y1 < MIN ? MIN : y1;
            z1 = z1 < MIN ? MIN : z1;
            x2 = x2 > MAX ? MAX : x2;
            y2 = y2 > MAX ? MAX : y2;
            z2 = z2 > MAX ? MAX : z2;
            bool s = false;
            if (strcmp(state, "on") == 0){
                s = true;
            } else if (strcmp(state, "off") != 0) {
                fatalf("unexpected state %s\n", state);
            }
            for (int x=x1; x<=x2; x++) {
                for (int y=y1; y<=y2; y++) {
                    for (int z=z1; z<=z2; z++) {
                        cubes[x+OFFSET][y+OFFSET][z+OFFSET] = s;
                    }
                }
            }
            continue;
        }
        fatalf("unknown syntax: %s\n", input);
    }

    int n = 0;
    int cnt = 0;
    for (int x=MIN; x<=MAX; x++) {
        for (int y=MIN; y<=MAX; y++) {
            for (int z=MIN; z<=MAX; z++) {
                cnt++;
                if (cubes[x+OFFSET][y+OFFSET][z+OFFSET]) {
                    n++;
                }
            }
        }
    }

    return n;
}

typedef struct {
    int x1, x2, y1, y2, z1, z2;
} cuboid;

bool cuboid_empty(cuboid c) {
    return c.x1>c.x2 || c.y1>c.y2 || c.z1>c.z2;
}

unsigned long long cuboid_vol(cuboid c) {
    if (cuboid_empty(c)) {
        return 0;
    }
    unsigned long long dx = c.x2 - c.x1 + 1;
    unsigned long long dy = c.y2 - c.y1 + 1;
    unsigned long long dz = c.z2 - c.z1 + 1;
    return dx*dy*dz;
}

bool cuboid_eq(cuboid a, cuboid b) {
    return a.x1 == b.x1 && a.x2 == b.x2 && a.y1 == b.y1 && a.y2 == b.y2 && a.z1 == b.z1 && a.z2 == b.z2;
}

typedef struct {
    cuboid *elems;
    int len, cap;
} set;

set *set_new(cuboid c) {
    set *s = malloc(sizeof(set));

    s->elems = malloc(sizeof(cuboid));
    s->cap = 1;

    s->elems[0] = c;
    s->len = 1;

    return s;
}

void set_free(set *s) {
    if (s != NULL) {
        free(s->elems);
        free(s);
    }
}

int set_len(set *s) {
    if (s == NULL) {
        return 0;
    }
    return s->len;
}

void set_resize(set *s, int cap) {
    bool resize = false;
    while (s->cap < cap) {
        if (s->cap <= 0) {
            s->cap = 1;
        } else {
            s->cap = 2*s->cap;
        }
        resize = true;
    }
    if (resize) {
        s->elems = realloc(s->elems, s->cap*sizeof(cuboid));
    }
}

// set_replace overwrites the contents of dst with src
void set_replace(set *dst, set *src) {
    //dprintf("copy src(%d)\n", set_len(src));
    if (src == NULL && dst == NULL) {
        return;
    }
    
    if (dst == NULL) {
        dst = malloc(sizeof(set));
        dst->elems = NULL;
        dst->len = 0;
        dst->cap = 0;
    }

    set_resize(dst, set_len(src));
    dst->len = set_len(src);
    if (dst->len > 0) {
        memcpy(dst->elems, src->elems, set_len(src)*sizeof(cuboid));
    }
}

cuboid set_elem(set *s, int index) {
    if (s == NULL || s->len < index) {
        fatalf("set_elem: out of range\n");
    }
    return s->elems[index];
}

// set_add adds c to s.
// Caller is responsible to make sure that s and {c} are disjoint.
set *set_add(set *s, cuboid c) {
    if (s == NULL) {
        return set_new(c);
    }

    set_resize(s, s->len+1);

    s->elems[s->len] = c;
    s->len++;
    return s;
}

// elem_difference removes the [overlap of a and b] from a.
// Returns the remaining set of parts of a
set *elem_difference(cuboid a, cuboid b) {
    set *res = NULL;

    if (a.x1 > b.x2 || b.x1 > a.x2 ||
        a.y1 > b.y2 || b.y1 > a.y2 ||
        a.z1 > b.z2 || b.z1 > a.z2) {
        // No overlap. Return a
        //dprintf("elem_difference = 1\n");
        return set_new(a);
    }

    if (a.x1 < b.x1) {
        // Split a at b.x1
        cuboid c = {
            .x1 = a.x1,
            .x2 = b.x1-1,
            .y1 = a.y1,
            .y2 = a.y2,
            .z1 = a.z1,
            .z2 = a.z2,
        };
        a.x1 = b.x1;
        // res and c are disjoint
        res = set_add(res, c);
    }
    if (a.x2 > b.x2) {
        // Split a at b.x2
        cuboid c = {
            .x1 = b.x2+1,
            .x2 = a.x2,
            .y1 = a.y1,
            .y2 = a.y2,
            .z1 = a.z1,
            .z2 = a.z2,
        };
        a.x2 = b.x2;
        // res and c are disjoint
        res = set_add(res, c);
    }

    if (a.y1 < b.y1) {
        // Split a at b.y1
        cuboid c = {
            .x1 = a.x1,
            .x2 = a.x2,
            .y1 = a.y1,
            .y2 = b.y1-1,
            .z1 = a.z1,
            .z2 = a.z2,
        };
        a.y1 = b.y1;
        // res and c are disjoint
        res = set_add(res, c);
    }
    if (a.y2 > b.y2) {
        // Split a at b.y2
        cuboid c = {
            .x1 = a.x1,
            .x2 = a.x2,
            .y1 = b.y2+1,
            .y2 = a.y2,
            .z1 = a.z1,
            .z2 = a.z2,
        };
        a.y2 = b.y2;
        // res and c are disjoint
        res = set_add(res, c);
    }

    if (a.z1 < b.z1) {
        // Split a at b.z1
        cuboid c = {
            .x1 = a.x1,
            .x2 = a.x2,
            .y1 = a.y1,
            .y2 = a.y2,
            .z1 = a.z1,
            .z2 = b.z1-1,
        };
        a.z1 = b.z1;
        // res and c are disjoint
        res = set_add(res, c);
    }
    if (a.z2 > b.z2) {
        // Split a at b.z2
        cuboid c = {
            .x1 = a.x1,
            .x2 = a.x2,
            .y1 = a.y1,
            .y2 = a.y2,
            .z1 = b.z2+1,
            .z2 = a.z2,
        };
        // res and c are disjoint
        res = set_add(res, c);
    }

    //dprintf("elem_difference = %d\n", set_len(res));
    return res;
}

// set_difference returns a new set containing the remaining parts of a after
// removing [overlap of a and b] from a.
// Returns the updated a.
set *set_difference(set *a, set *b) {
    if (set_len(a) == 0 || set_len(b) == 0) {
        return a;
    }
    int len_a = set_len(a);
    int len_b = set_len(b);
    for (int i=0; i<set_len(b); i++) {
        // Remove b[i] from a
        set *res = NULL;
        for (int j=0; j<set_len(a); j++) {
            set *diff = elem_difference(set_elem(a, j), set_elem(b, i));
            // Since a's elems are disjoint and diff consists of parts of a,
            // diff is disjoint from res
            for (int k=0; k<set_len(diff); k++) {
                res = set_add(res, set_elem(diff, k));
            }
            set_free(diff);
        }
        // Update a by res:
        set_replace(a, res);
        set_free(res);
    }
    dprintf("Difference of a(%d), b(%d) = res(%d)\n", len_a, len_b, set_len(a));
    return a;
}

// set_union updates a with the union of a and b
set *set_union(set *a, set *b) {
    int len_a = set_len(a);
    int len_b = set_len(b);

    b = set_difference(b, a);
    // b is now disjoint from a
    for (int i=0; i<set_len(b); i++) {
        a = set_add(a, set_elem(b, i));
    }
    dprintf("Union of a(%d), b(%d) = a(%d)\n", len_a, len_b, set_len(a));
    return a;
}

unsigned long long part_two(FILE *fp) {
    char input[BUFSIZE];
    set *on = NULL;
    cuboid c;
    set *tmp = set_new(c);
    while (fgets(input, BUFSIZE, fp)) {
        char state[10];
        if (sscanf(input, "%s x=%d..%d,y=%d..%d,z=%d..%d", state, &c.x1, &c.x2, &c.y1, &c.y2, &c.z1, &c.z2) == 7) {
            tmp->elems[0] = c;
            tmp->len = 1;
            if (strcmp(state, "on") == 0){
                on = set_union(on, tmp);
            } else if (strcmp(state, "off") == 0) {
                on = set_difference(on, tmp);
            } else {
                fatalf("unexpected state %s\n", state);
            }
            continue;
        }
        fatalf("unknown syntax: %s\n", input);
    }
    set_free(tmp);

    unsigned long long n = 0;
    for (int i=0; i<set_len(on); i++) {
        cuboid c = set_elem(on, i);
        n += cuboid_vol(c);
    }

    set_free(on);

    return n;
}

void dump_set(set *s) {
    for (int i=0; i<set_len(s); i++) {
        cuboid c = set_elem(s, i);
        dprintf("s[%d] = {%d, %d, %d, %d, %d, %d}\n", i, c.x1, c.x2, c.y1, c.y2, c.z1, c.z2);
    }
}

void unit_test() {
    dprintf("*** UNIT TESTS ***\n");

    {
        cuboid a = {0, 9, 0, 9, 0, 9};
        dprintf("cuboid_vol...");
        if (cuboid_vol(a) != 1000) {
            fatalf("fail: got %llu, expected 1000\n", cuboid_vol(a));
        }
        cuboid empty = {0, -1, 0, 0, 0, 0};
        if (cuboid_vol(empty) != 0) {
            fatalf("fail: got %llu, expected 0\n", cuboid_vol(empty));
        }
        dprintf("ok\n");
    }

    {
        dprintf("set_new...");
        cuboid a = {0, 9, 0, 9, 0, 9};
        set *s = set_new(a);
        if (set_len(s) != 1) {
            dprintf("fail: got set length %d, expected 1\n", set_len(s));
            dump_set(s);
            exit(1);
        }
        if (!cuboid_eq(set_elem(s, 0), a)) {
            dprintf("fail: unexpected contents\n");
            dump_set(s);
            exit(1);
        }
        set_free(s);
        dprintf("ok\n");
    }

    {
        dprintf("elem_difference...");

        cuboid a = {0, 9, 0, 9, 0, 9};
        cuboid b = {1, 10, 0, 9, 0, 9};
        cuboid expected = {0, 0, 0, 9, 0, 9};

        set *s = elem_difference(a, b);
        if (set_len(s) != 1) {
            dprintf("fail: got set length %d, expected 1\n", set_len(s));
            dump_set(s);
            exit(1);
        }
        if (!cuboid_eq(set_elem(s, 0), expected)) {
            dprintf("fail: unexpected first element\n");
            dump_set(s);
            exit(1);
        }
        set_free(s);

        cuboid b2 = {0, 9, 1, 8, 0, 9};
        cuboid expected_0 = {0, 9, 0, 0, 0, 9};
        cuboid expected_1 = {0, 9, 9, 9, 0, 9};

        s = elem_difference(a, b2);
        if (set_len(s) != 2) {
            dprintf("fail: got set length %d, expected 2\n", set_len(s));
            dump_set(s);
            exit(1);
        }
        if (!cuboid_eq(set_elem(s, 0), expected_0)) {
            dprintf("fail: unexpected first element\n");
            dump_set(s);
            exit(1);
        }
        if (!cuboid_eq(set_elem(s, 1), expected_1)) {
            dprintf("fail: unexpected second element\n");
            dump_set(s);
            exit(1);
        }
        set_free(s);

        dprintf("ok\n");
    }

    {
        dprintf("set_difference...");

        cuboid a = {0, 9, 0, 9, 0, 9};
        cuboid b = {1, 10, 0, 9, 0, 9};
        cuboid expected = {0, 0, 0, 9, 0, 9};

        set *s = set_new(a);
        set *tmp = set_new(b);
        
        s = set_difference(s, tmp);
        if (set_len(s) != 1) {
            dprintf("fail: got set length %d, expected 1\n", set_len(s));
            dump_set(s);
            exit(1);
        }
        if (!cuboid_eq(set_elem(s, 0), expected)) {
            dprintf("fail: unexpected first element\n");
            dump_set(s);
            exit(1);
        }

        set_free(tmp);
        set_free(s);
        dprintf("ok\n");
    }

    {
        dprintf("set_union...");

        cuboid a = {0, 9, 0, 9, 0, 9};
        cuboid b = {1, 10, 0, 9, 0, 9};

        set *s = set_new(a);
        set *tmp = set_new(b);
        s = set_union(s, tmp);
        set_free(tmp);
        if (set_len(s) != 2) {
            dprintf("fail: got set length %d, expected 2\n", set_len(s));
            for (int i=0; i<set_len(s); i++) {
                cuboid c = set_elem(s, i);
                dprintf("s[%d] = {%d, %d, %d, %d, %d, %d}\n", i, c.x1, c.x2, c.y1, c.y2, c.z1, c.z2);
            }
            exit(1);
        }
        if (!cuboid_eq(set_elem(s, 0), a)) {
            fatalf("fail: unexpected first element\n");
        }
        cuboid got = set_elem(s, 1);
        cuboid expected = {10, 10, 0, 9, 0, 9};
        if (!cuboid_eq(got, expected)) {
            fatalf("fail: unexpected second element {%d, %d, %d, %d, %d, %d}\n", got.x1, got.x2, got.y1, got.y2, got.z1, got.z2);
        }
        set_free(s);
        dprintf("ok\n");
    }
}
