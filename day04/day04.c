#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#define BUFSIZE 1024
#define ROWSIZE 16

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

/*** Bingo subsystem ***/

typedef int board[25];

typedef struct {
    char *upcoming_numbers;
    char *saveptr;
    bool is_drawn[100];
    board **boards;
    int board_cnt;
} game;

// board_read reads a board from fp.
// Returns true if a board could be read.
bool board_read(board *b, FILE *fp) {
    char row[ROWSIZE];

    // Read empty line
    if (fgets(row, ROWSIZE, fp) == NULL) {
        return false;
    } else if (row[0] != '\n') {
        fprintf(stderr, "Line was not empty\n");
        exit(EXIT_FAILURE);
    }

    // Read board from fp
    for (int i=0; i<5; i++) {
        if (fgets(row, ROWSIZE, fp) == NULL) {
            perror("Cannot read bingo numbers");
            exit(EXIT_FAILURE);
        }
        char *token = strtok(row, " \n");
        for (int j=0; j<5 && token != NULL; j++) {
            (*b)[i*5 + j] = atoi(token);
            token = strtok(NULL, " \n");
        }
    }
    return true;
}

// board_score returns the score if the board has won.
// Returns 0 if the board didn't win.
int board_score(board b, game *g) {
    return 0;
}

void board_print(board b) {
    for (int i=0; i<5; i++) {
        for (int j=0; j<5; j++) {
            if (j !=0 ) { printf(" "); }
            printf("%2d", b[i*5+j]);
        }
        printf("\n");
    }
}

// game_new reads a new game from fp.
game *game_new(FILE *fp) {
    game *g = malloc(sizeof(game));

    // Initialize g
    g->upcoming_numbers = malloc(BUFSIZE*sizeof(char));
    g->saveptr = NULL;
    for (int i=0; i<100; i++) {
        g->is_drawn[i] = false;
    }
    g->boards = NULL;
    g->board_cnt = 0;

    // Read string of drawn numbers
    if (fgets(g->upcoming_numbers, BUFSIZE, fp) == NULL) {
        perror("Cannot read bingo numbers");
        exit(EXIT_FAILURE);
    }

    // Read boards
    while (true) {
        // Append a board
        g->boards = realloc(g->boards, (g->board_cnt+1)*sizeof(board*));
        g->boards[g->board_cnt] = malloc(sizeof(board));
        if (!board_read(g->boards[g->board_cnt], fp)) {
            free(g->boards[g->board_cnt]);
            g->boards = realloc(g->boards, (g->board_cnt)*sizeof(board*));
            break;
        }
        g->board_cnt++;
    }

    return g;
}


// game_draw draws the next number for the game.
// Returns -1 if no more numbers.
int game_draw(game *g) {
    char *token;
    if (g->saveptr == NULL) {
        token = strtok_r(g->upcoming_numbers, ",\n", &g->saveptr);
    } else {
        token = strtok_r(NULL, ",\n", &g->saveptr);
    }

    if (token == NULL){
        return -1;
    }
    return atoi(token);
}

// game_score returns the sum of the unmarked numbers on b
int game_score(game *g, board *b) {
    int sum = 0;
    for (int i=0; i<25; i++) {
        int n = (*b)[i];
        if (!g->is_drawn[n]) {
            sum += n;
        }
    }
    return sum;
}

// game_play draws a number, checks all boards and returns the score of the
// winning board, if a board won.
// All boards that won are removed from the game.
// Returns 0 if no board won.
// Returns -1 if no more numbers.
int game_play(game *g) {
    int num = game_draw(g);
    if (num < 0) {
        return -1;
    }
    g->is_drawn[num] = true;

    int score = 0;
    for (int i=0; i<g->board_cnt; i++) {
        board *b = g->boards[i];
        if (b == NULL) {
            continue;
        }

        bool won = false;
        // check rows
        for (int row=0; row<5; row++) {
            bool won_row = true;
            for (int col=0; col<5; col++) {
                if (!g->is_drawn[(*b)[row*5+col]]) {
                    won_row = false;
                    break;
                }
            }
            if (won_row) {
                won = true;
            }
        }

        // check cols
        for (int col=0; col<5; col++) {
            bool won_col = true;
            for (int row=0; row<5; row++) {
                if (!g->is_drawn[(*b)[row*5+col]]) {
                    won_col = false;
                    break;
                }
            }
            if (won_col) {
                won = true;
            }
        }

        if (won) {
            score = num*game_score(g, b);
            free(g->boards[i]);
            g->boards[i] = NULL;
        }
    }
    return score;
}

void game_free(game *g) {
    free(g->upcoming_numbers);
    for (int i=0; i<g->board_cnt; i++) {
        if (g->boards[i] != NULL) {
            free(g->boards[i]);
        }
    }
    free(g->boards);
}

/*** Main parts ***/

int part_one(FILE *fp) {
    game *g = game_new(fp);

    int score = 0;
    while (score == 0) {
        score = game_play(g);
    }
    game_free(g);

    return score;
}

int part_two(FILE *fp) {
    // 4176 is too low
    game *g = game_new(fp);

    int prev_score = 0;
    while (1) {
        int score = game_play(g);

        if (score == -1) {
            game_free(g);
            return prev_score;
        }
        if (score != 0) {
            prev_score = score;
        }
    }
}
