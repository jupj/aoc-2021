#ifndef UTILS_H
#define UTILS_H

#define fatalf(...) { \
    fprintf(stderr, __VA_ARGS__); \
    exit(EXIT_FAILURE); \
}
#define efatal(msg) { \
    perror(msg); \
    exit(EXIT_FAILURE); \
}

#ifdef DEBUG
#define dprintf(...) { printf("DEBUG: "); printf(__VA_ARGS__); }
#else
#define dprintf(...) /* NO-OP */
#endif

#endif // UTILS_H
