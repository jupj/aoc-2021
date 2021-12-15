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
#define dprintf(...) { fprintf(stderr, __VA_ARGS__); }
#else
#define dprintf(...) /* NO-OP */
#endif

/*** Queue data structure interface ***/
typedef struct queue_element Queue;
// Enqueue appends an item to the end of the queue
Queue *Enqueue(Queue *q, void *data);
// Dequeue pops the first item of the queue
Queue *Dequeue(void *data, Queue *q);

#endif // UTILS_H
