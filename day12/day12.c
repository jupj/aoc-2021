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

    printf("Part two: %llu\n", part_two(fp));

    fclose(fp);
    return 0;
}

/*** Main parts ***/

typedef struct list_item list;
struct list_item {
    int id;
    struct list_item *next;
};

list *list_insert(list *l, int id) {
    list *new_item = malloc(sizeof(list));
    new_item->id = id;
    new_item->next = l;
    return new_item;
}

void list_free(list *l) {
    list *item = l;
    while (item) {
        list *tmp = item->next;
        free(item);
        item = tmp;
    }
}

struct cave {
    int id;
    char *name;
    bool only_once;
    list *neighbors;
};

struct cavesystem {
    struct cave *caves;
    int n;
};

// find_cave returns the id of cave with name
// Returns -1 if cave not found
int find_cave(struct cavesystem *c, char *name) {
    for (int i=0; i < c->n; i++) {
        if (strcmp(c->caves[i].name, name) == 0) {
            // exists
            return i;
        }
    }
    return -1;
}

int add_cave(struct cavesystem *c, char *name) {
    int id = find_cave(c, name);

    if (id != -1) {
        dprintf("Cave %s already exists with id %d\n", name, id);
        return id;
    }

    struct cave newcave = {
        .id = c->n,
        .name = name,
        .only_once = strcmp("a", name) < 0,
        .neighbors = NULL
    };
    dprintf("Add new cave[%d] = { %s", newcave.id, newcave.name);
    if (newcave.only_once) {
        dprintf(", only_once");
    }
    dprintf(" }\n");

    c->n++;
    c->caves = realloc(c->caves, c->n*sizeof(struct cave));
    c->caves[c->n - 1] = newcave;
    return newcave.id;
}

void add_neighbor(struct cavesystem *c, int id_a, int id_b) {
    c->caves[id_a].neighbors = list_insert(c->caves[id_a].neighbors, id_b);
    c->caves[id_b].neighbors = list_insert(c->caves[id_b].neighbors, id_a);
}

int find_paths(struct cavesystem *c, int start_id, bool *visited) {
    dprintf("find_paths(%s)\n", c->caves[start_id].name);
    struct cave *node = &c->caves[start_id];
    if (strcmp(node->name, "end") == 0) {
        // End of path
        return 1;
    }

    // Copy visited
    bool *visited_copy = malloc(c->n*sizeof(bool));
    if (visited == NULL) {
        for (int i=0; i < c->n; i++) { visited_copy[i] = false; };
    } else {
        memcpy(visited_copy, visited, c->n*sizeof(bool));
    }
    visited_copy[node->id] = true;

    list *neighbors = node->neighbors;
    int pathcnt = 0;
    while (neighbors != NULL) {
        dprintf("  processing neighbor %d", neighbors->id);
        struct cave *neighbor = &c->caves[neighbors->id];
        dprintf(" = %s\n", neighbor->name);
        if (!neighbor->only_once || !visited_copy[neighbor->id]) {
            pathcnt += find_paths(c, neighbor->id, visited_copy);
        }
        neighbors = neighbors->next;
    }

    free(visited_copy);
    return pathcnt;
}

int find_paths2(struct cavesystem *c, char *prefix, int start_id, bool *visited, bool small_visited_twice) {
    //dprintf("find_paths2(%s)\n", c->caves[start_id].name);
    struct cave *node = &c->caves[start_id];
    if (strcmp(node->name, "end") == 0) {
        // End of path
        dprintf("  %send\n", prefix);
        return 1;
    }

    char pathstr[1024];
    sprintf(pathstr, "%s%s,", prefix, node->name);

    // Copy visited
    bool *visited_copy = malloc(c->n*sizeof(bool));
    if (visited == NULL) {
        for (int i=0; i < c->n; i++) { visited_copy[i] = false; };
    } else {
        memcpy(visited_copy, visited, c->n*sizeof(bool));
    }
    visited_copy[node->id] = true;

    list *neighbors = node->neighbors;
    int pathcnt = 0;
    while (neighbors != NULL) {
        bool local_visited_twice = small_visited_twice;
        struct cave *neighbor = &c->caves[neighbors->id];
        neighbors = neighbors->next;

        if (strcmp(neighbor->name, "start") == 0) {
            // start can't be visited twice
            continue;
        }
        if (neighbor->only_once && visited_copy[neighbor->id]) {
            if (local_visited_twice) {
                continue;
            }
            local_visited_twice = true;
            //dprintf("visiting %s twice\n", neighbor->name);
        }

        //dprintf("  processing neighbor %d = %s\n", neighbor->id, neighbor->name);
        pathcnt += find_paths2(c, pathstr, neighbor->id, visited_copy, local_visited_twice);
    }

    free(visited_copy);
    return pathcnt;
}

int part_one(FILE *fp) {
    struct cavesystem caves = { .caves = NULL, .n = 0 };

    // Read data
    char lines[100][100];
    int linecnt = 0;
    int start_id = -1;
    while (fscanf(fp, "%s", lines[linecnt]) != EOF) {
        char *a = strtok(lines[linecnt], "-");
        char *b = strtok(NULL, "-");
        dprintf("DEBUG: %s -- %s\n", a,b);
        int id_a = add_cave(&caves, a);
        int id_b = add_cave(&caves, b);
        add_neighbor(&caves, id_a, id_b);

        if (strcmp(a, "start") == 0) {
            start_id = id_a;
        } else if (strcmp(b, "start") == 0) {
            start_id = id_b;
        }
        linecnt++;
    }

    // Find paths
    if (start_id == -1) {
        fatalf("no start");
    }

    int result = find_paths(&caves, start_id, NULL);
    for (int i=0; i<caves.n; i++) {
        list_free(caves.caves[i].neighbors);
    }
    free(caves.caves);
    return result;
}

unsigned long long part_two(FILE *fp) {
    struct cavesystem caves = { .caves = NULL, .n = 0 };

    // Read data
    char lines[100][100];
    int linecnt = 0;
    int start_id = -1;
    while (fscanf(fp, "%s", lines[linecnt]) != EOF) {
        char *a = strtok(lines[linecnt], "-");
        char *b = strtok(NULL, "-");
        dprintf("DEBUG: %s -- %s\n", a,b);
        int id_a = add_cave(&caves, a);
        int id_b = add_cave(&caves, b);
        add_neighbor(&caves, id_a, id_b);

        if (strcmp(a, "start") == 0) {
            start_id = id_a;
        } else if (strcmp(b, "start") == 0) {
            start_id = id_b;
        }
        linecnt++;
    }

    // Find paths
    if (start_id == -1) {
        fatalf("no start");
    }

    int result = find_paths2(&caves, "", start_id, NULL, false);

    for (int i=0; i<caves.n; i++) {
        list_free(caves.caves[i].neighbors);
    }
    free(caves.caves);
    return result;
}
