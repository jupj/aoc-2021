#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

//#define DEBUG
#include "utils.h"
#include "heap.h"

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

#define HEIGHT 7
#define WIDTH 13
#define Y_HALL 1
#define Y_ROOM_UP 2

typedef unsigned long long tenergy;

tenergy energy(char typ) {
    switch (typ) {
    case 'A':
        return 1;
    case 'B':
        return 10;
    case 'C':
        return 100;
    case 'D':
        return 1000;
    }
    fatalf("energy: invalid typ %d\n", typ);
}

int target_room(char typ) {
    switch (typ) {
    case 'A':
        return 3;
    case 'B':
        return 5;
    case 'C':
        return 7;
    case 'D':
        return 9;
    }
    fatalf("target_room: invalid typ %c\n", typ);
}

typedef struct {
    int x, y;
} coord;

typedef struct {
    coord pos;
    char typ;
} amphipod;

typedef struct {
    char map[WIDTH][HEIGHT];
    tenergy energy;
} state;

unsigned long long state_cost(void *stateptr) {
    state *s = (state*) stateptr;
    return s->energy;
}

void clone(state *dest, state *src) {
    for (int x=0; x<WIDTH; x++) {
        for (int y=0; y<HEIGHT; y++) {
            dest->map[x][y] = src->map[x][y];
        }
    }
    dest->energy = src->energy;
}

typedef struct state_list {
    state *s;
    struct state_list *next;
} state_list;

state_list *state_list_insert(state_list *sl, state *s) {
    state_list *first = malloc(sizeof(state_list));
    first->s = s;
    first->next = sl;
    return first;
}

void free_state_list(state_list *sl) {
    while (sl != NULL) {
        state_list *next = sl->next;
        free(sl);
        sl = next;
    }
}

bool is_amphipod(char c) {
    switch (c) {
    case 'A': case 'B': case 'C': case 'D':
        return true;
    }
    return false;
}

bool is_at_room(int x) {
    return x == 3 || x == 5 || x == 7 || x == 9;
}

bool is_valid_pos(int x, int y) {
    if (y == Y_HALL) {
        return !is_at_room(x);
    }
    if (y >= Y_ROOM_UP) {
        return is_at_room(x);
    }
    return false;
}

void dump_map(state *s) {
    for (int y=0; y<HEIGHT; y++) {
        for (int x=0; x<WIDTH; x++) {
            if (s->map[x][y] < 0x20 || s->map[x][y] == '#') {
                // non-visible character
                dprintf(" ");
            } else {
                dprintf("%c", s->map[x][y]);
            }
        }
        dprintf("\n");
    }
}

void dump_maps(state *s1, state *s2, int x1, int y1, int x2, int y2) {
    dprintf("Moving %c (%d,%d) -> (%d, %d) = %llu e\n", s1->map[x1][y1], x1, y1, x2, y2, s2->energy - s1->energy);

    for (int y=0; y<HEIGHT; y++) {
        for (int x=0; x<WIDTH; x++) {
            if (s1->map[x][y] < 0x20 || s1->map[x][y] == '#') {
                // non-visible character
                dprintf(" ");
            } else if (x == x1 && y == y1) {
                dprintf("*");
            } else {
                dprintf("%c", s1->map[x][y]);
            }
        }

        dprintf("  >  ");

        for (int x=0; x<WIDTH; x++) {
            if (s2->map[x][y] < 0x20 || s2->map[x][y] == '#') {
                // non-visible character
                dprintf(" ");
            } else if (x == x2 && y == y2) {
                dprintf("*");
            } else {
                dprintf("%c", s2->map[x][y]);
            }
        }
        dprintf("\n");
    }
    if (!is_valid_pos(x1, y1)) {
        fatalf("%c at invalid pos (%d, %d)\n", s1->map[x1][y1], x1, y1);
    }
    if (!is_valid_pos(x2, y2)) {
        fatalf("%c at invalid pos (%d, %d)\n", s2->map[x2][y2], x2, y2);
    }
}

bool is_complete(state *s) {
    for (int x=0; x<WIDTH; x++) {
        // hall should be empty:
        if (is_amphipod(s->map[x][Y_HALL])) {
            return false;
        }
    }

    for (int x=0; x<WIDTH; x++) {
        // Rooms should have matching amphipods
        if (is_at_room(x)) {
            for (int y=Y_ROOM_UP; s->map[x][y] != '#'; y++) {
                if (!is_amphipod(s->map[x][y])) {
                    // Empty space in room
                    return false;
                }
                if (target_room(s->map[x][y]) != x) {
                    // Wrong room
                    return false;
                }
            }
        }
    }
    return true;
}

bool has_free_surrounding(state *s, int x, int y) {
    return s->map[x-1][y] == '.' ||
           s->map[x+1][y] == '.' ||
           s->map[x][y-1] == '.' ||
           s->map[x][y+1] == '.';
}

int sign(int val) {
    if (val < 0) {
        return -1;
    }
    if (val > 0) {
        return 1;
    }
    return 0;
}

// find_moves_to_room adds moves for a specific amphipod to its own room
state_list *find_moves_to_room(state *s, int x, int y) {
    char c = s->map[x][y];

    if (!is_amphipod(c)) {
        fatalf("expected amphipod at (%d,%d)\n", x, y);
    }

    coord start = {x, y};

    // started in hall => we MUST find the correct room
    if (y != Y_HALL) {
        fatalf("invalid y in hall: %d\n", y);
    }

    // Move in the hall to the target room
    int x_target = target_room(c);
    int dx = sign(x_target-x);
    tenergy e = s->energy;
    while (x!=x_target) {
        // Move horizontally
        x += dx;
        e += energy(c);
        if (s->map[x][Y_HALL] != '.') {
            // Obstacle in the hall
            dprintf("DEBUG: move_to_room: obstacle in hall\n");
            return NULL;
        }
    }

    // Check that target Y_ROOM_UP is not occupied
    if (s->map[x_target][Y_ROOM_UP] != '.') {
        // Room is occupied
        dprintf("DEBUG: move_to_room: room is occupied\n");
        return NULL;
    }

    // Move into Y_ROOM_UP
    y = Y_ROOM_UP;
    e += energy(c);

    // Move down in room
    while (s->map[x_target][y+1] == '.') {
        y++;
        e += energy(c);
    }

    // Check that target room is not occupied by another type of amphipod
    if (s->map[x_target][y+1] != '#' && s->map[x_target][y+1] != c) {
        // Room is occupied by another type of amphipod
        dprintf("DEBUG: move_to_room: room is occupied by another type of amphipod\n");
        return NULL;
    }

    // Done moving this piece
    // Run next iteration
    state *newstate = malloc(sizeof(state));
    clone(newstate, s);
    newstate->map[x][y] = c;
    newstate->map[start.x][start.y] = '.';
    newstate->energy = e;
    //dump_maps(s, &newstate, start.x, start.y, x, y);
    return state_list_insert(NULL, newstate);
}

// find_moves_to_hall adds to heap moves for a specific amphipod from a room into the hall.
state_list *find_moves_to_hall(state *s, int x, int y) {
    char c = s->map[x][y];

    if (!is_amphipod(c)) {
        fatalf("expected amphipod at (%d,%d)\n", x, y);
    }

    // Moving out of a room
    if (y == Y_HALL) {
        fatalf("move_to_hall: unexpected y == Y_HALL\n");
    }

    if (x == target_room(c)) {
        // Already in target room
        bool stay = true;
        for (int i=y+1; s->map[x][i] != '#'; i++) {
            if (s->map[x][i] != c) {
                // Room has differing amphipods beneath. Move out
                stay = false;
                break;
            }
        }
        if (stay) {
            // Room has only target amphipods. Do not move
            return NULL;
        }
    }

    // Starting state
    coord start = {x, y};
    tenergy ye = 0;

    // Move up from room
    while (y != Y_HALL) {
        y--;
        ye += energy(c);
        if (s->map[x][y] != '.') {
            // blocked way out of the room
            return NULL;
        }
    }

    state_list *result = NULL;

    // Try moving to the left
    tenergy xe = 0;
    while (s->map[x-1][y] == '.') {
        x--;
        xe += energy(c);
        if (!is_at_room(x)) {
            // Iterate from this position
            state *newstate = malloc(sizeof(state));
            clone(newstate, s);
            newstate->map[x][y] = c;
            newstate->map[start.x][start.y] = '.';
            newstate->energy += ye + xe;
            //dump_maps(s, &newstate, start.x, start.y, x, y);
            result = state_list_insert(result, newstate);
        }
    }

    // Try moving to the right
    xe = 0;
    x = start.x;
    while (s->map[x+1][y] == '.') {
        x++;
        xe += energy(c);
        if (!is_at_room(x)) {
            // Iterate from this position
            state *newstate = malloc(sizeof(state));
            clone(newstate, s);
            newstate->map[x][y] = c;
            newstate->map[start.x][start.y] = '.';
            newstate->energy += ye + xe;
            //dump_maps(s, &newstate, start.x, start.y, x, y);
            result = state_list_insert(result, newstate);
        }
    }

    return result;
}

bool eq(state *a, state *b) {
    for (int y=0; y<HEIGHT; y++) {
        for (int x=0; x<WIDTH; x++) {
            if (a->map[x][y] != b->map[x][y]) {
                return false;
            }
        }
    }
    return true;
}

/*** Hash table ***/

// Hash table for states, key = hash(state)
// To keep track of lowest energy level per state

#define BUCKET_SIZE 10

typedef struct {
    state_list **buckets;
    size_t n_elems, n_buckets;
} hashtable;

typedef unsigned long long thash64;

thash64 hash(state *s) {
    const thash64 FNV_offset_basis = 0xcbf29ce484222325;
    const thash64 FNV_prime = 0x100000001b3;

    int hash = FNV_offset_basis;
    for (int y=0; y<HEIGHT; y++) {
        for (int x=0; x<WIDTH; x++) {
            hash ^= s->map[x][y];
            hash *= FNV_prime;
        }
    }
    return hash;
}

hashtable *h_init() {
    hashtable *ht = malloc(sizeof(hashtable));
    ht->n_elems = 0;
    ht->n_buckets = 1;
    ht->buckets = calloc(ht->n_buckets, sizeof(state_list*));
    return ht;
}

void h_free(hashtable *ht) {
    for (int i=0; i<ht->n_buckets; i++) {
        state_list *sl = ht->buckets[i];
        while (sl != NULL) {
            state_list *next = sl->next;
            free(sl->s);
            free(sl);
            sl = next;
        }
    }
    free(ht->buckets);
    free(ht);
}

// h_insert inserts a new element into ht. Caller is responsible to avoid duplicates.
void h_insert(hashtable *ht, state *s) {
    // Check load factor. If n_elems/n_buckets > 0.5: resize/rehash
    if ((2*ht->n_elems) > ht->n_buckets) {
        // Double the number of buckets
        size_t new_len = ht->n_buckets << 1;
        state_list **new_buckets = calloc(new_len, sizeof(state_list*));
        // Re-hash elements:
        for (int i=0; i<ht->n_buckets; i++) {
            state_list *b = ht->buckets[i];
            while (b) {
                int ix = hash(b->s) % new_len;
                new_buckets[ix] = state_list_insert(new_buckets[ix], b->s);
                b = b->next;
            }
            free_state_list(ht->buckets[i]);
        }
        free(ht->buckets);
        ht->buckets = new_buckets;
        ht->n_buckets = new_len;
    }

    int ix = hash(s) % ht->n_buckets;
    ht->buckets[ix] = state_list_insert(ht->buckets[ix], s);
    ht->n_elems++;
}

// h_get searches for an existing element (state) with identical map
// Returns the already-stored-state, or NULL
state *h_get(hashtable *ht, state *s) {
    int ix = hash(s) % ht->n_buckets;
    state_list *b = ht->buckets[ix];
    while (b != NULL) {
        if (eq(b->s, s)) {
            return b->s;
        }
        b = b->next;
    }
    return NULL;
}

tenergy optimize(state *initial) {
    // Djikstra's search algorithm
    // Use heap as priority queue
    Heap *h = HeapNew();
    HeapInsert(h, initial, state_cost);
    hashtable *ht = h_init();

    tenergy min_energy = 0;
    state *s;
    while ((s = (state*) HeapExtract(h, state_cost)) != NULL) {
        if (is_complete(s)) {
            dprintf("COMPLETE with energy %llu!\n", s->energy);
            min_energy = s->energy;
            break;
        }

        // Add next moves
        for (int x=0; x<WIDTH; x++) {
            for (int y=0; y<HEIGHT; y++) {
                if (is_amphipod(s->map[x][y]) && has_free_surrounding(s, x, y)) {
                    // Try to move if feasible
                    state_list *list = NULL;
                    if (y == Y_HALL) {
                        list = find_moves_to_room(s, x, y);
                    } else {
                        list = find_moves_to_hall(s, x, y);
                    }

                    while (list != NULL) {
                        state_list *next = list->next;
                        state *curr = list->s;
                        free(list);
                        list = next;

                        // Check for existing cost
                        state *existing = h_get(ht, curr);
                        if (existing == NULL) {
                            // Unvisited:
                            HeapInsert(h, curr, state_cost);
                            h_insert(ht, curr);
                        } else if (existing->energy > curr->energy ) {
                            // Visited with worse energy level:
                            existing->energy = curr->energy;
                            free(curr);
                            HeapInsert(h, existing, state_cost);
                        } else {
                            // We visited this state with same or better energy
                            // level. Skip:
                            free(curr);
                        }
                    }
                }
            }
        }
    }
    HeapFree(h);
    h_free(ht);
    return min_energy;
}

/*** Main parts ***/

// fail: 15459 is too high
int part_one(FILE *fp) {
    const int buf_size = 20;
    char buf[buf_size];

    // Read initial game state
    state initial;
    initial.energy = 0;
    for (int y=0; y<HEIGHT; y++) {
        if (fgets(buf, buf_size, fp) == NULL) {
            buf[0] = '\0';
        }

        for (int x=0; x<WIDTH; x++) {
            if (x>0 && buf[x-1] == '\0') {
                // Pad with NUL bytes
                buf[x] = '\0';
            }
            initial.map[x][y] = buf[x];
        }
    }
    return optimize(&initial);
}

unsigned long long part_two(FILE *fp) {
    const int buf_size = 20;
    char buf[buf_size];

    // Read initial game state
    state initial;
    initial.energy = 0;
    for (int y=0; y<HEIGHT; y++) {
        if (y == Y_ROOM_UP+1) {
            sprintf(buf, "  #D#C#B#A#");
        } else if (y == Y_ROOM_UP+2) {
            sprintf(buf, "  #D#B#A#C#");
        } else if (fgets(buf, buf_size, fp) == NULL) {
            buf[0] = '\0';
        }

        for (int x=0; x<WIDTH; x++) {
            if (x>0 && buf[x-1] == '\0') {
                // Pad with NUL bytes
                buf[x] = '\0';
            }
            initial.map[x][y] = buf[x];
        }
    }
    dump_map(&initial);

    return optimize(&initial);
}
