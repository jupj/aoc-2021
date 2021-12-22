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

    printf("Part two: %llu\n", part_two(fp));

    fclose(fp);
    return 0;
}

/*** Main parts ***/

#define BUFSIZE 1024

int part_one(FILE *fp) {
    char input[BUFSIZE];

    struct {
        int pos, points;
    } player1, player2, *prev, *curr, *next;
    prev = &player2;
    curr = &player1;
    next = &player2;

    while (fgets(input, BUFSIZE, fp)) {
        int pos;
        if (sscanf(input, "Player 1 starting position: %d", &pos) == 1) {
            player1.points = 0;
            player1.pos = pos;
        }
        if (sscanf(input, "Player 2 starting position: %d", &pos) == 1) {
            player2.points = 0;
            player2.pos = pos;
        }
    }

    int die_rolls = 0;

    while (player1.points < 1000 && player2.points < 1000) {
        int moves = 0;
        for (int i=0; i<3; i++) {
            moves += (die_rolls % 100) + 1;
            die_rolls++;
        }
        curr->pos += moves % 10;
        if (curr->pos > 10) {
            curr->pos -= 10;
        }
        curr->points += curr->pos;

        prev = curr;
        curr = next;
        next = prev;
        //dprintf("Rolls: %d, Player 1: %d, Player 2: %d\n", die_rolls, player1.points, player2.points);
    }

    int losing = player1.points < player2.points ? player1.points : player2.points;
    return losing*die_rolls;
}

typedef struct {
    int pos, points;
    unsigned long long wins;
} player;

// outcomes lists how many time each 3-die sum occurs in the parallel universes
int outcomes[10];

void init_outcomes() {
    for (int i=0; i<10; i++) {
        outcomes[i] = 0;
    }
    // Loop through all possible die outcomes
    for (int i=1; i<4; i++) {
        for (int j=1; j<4; j++) {
            for (int k=1; k<4; k++) {
                outcomes[i + j + k]++;
            }
        }
    }
}

void play(player *p1, player *p2) {
    // Player 1's turn
    // Loop through all possible die outcomes
    for (int i=0; i<10; i++) {
        if (outcomes[i] == 0) {
            continue;
        }

        // split player 1
        player p1split = {
            .pos = p1->pos,
            .points = p1->points,
            .wins = 0,
        };
        p1split.pos += i;
        while (p1split.pos > 10) {
            p1split.pos -= 10;
        }
        p1split.points += p1split.pos;

        if (p1split.points >= 21) {
            p1split.wins++;
        } else {
            // Let the other player play
            // Split player 2
            player p2split = {
                .pos = p2->pos,
                .points = p2->points,
                .wins = 0,
            };

            play(&p2split, &p1split);
            p2->wins += p2split.wins*outcomes[i];
        }

        p1->wins += p1split.wins*outcomes[i];
    }
}

unsigned long long part_two(FILE *fp) {
    char input[BUFSIZE];

    player p1, p2;

    while (fgets(input, BUFSIZE, fp)) {
        int pos;
        if (sscanf(input, "Player 1 starting position: %d", &pos) == 1) {
            dprintf("Read player 1\n");
            p1.points = 0;
            p1.wins = 0;
            p1.pos = pos;
        }
        if (sscanf(input, "Player 2 starting position: %d", &pos) == 1) {
            dprintf("Read player 2\n");
            p2.points = 0;
            p2.wins = 0;
            p2.pos = pos;
        }
    }

    init_outcomes();
    play(&p1, &p2);

    if (p1.wins > p2.wins) {
        return p1.wins;
    }
    return p2.wins;
}
