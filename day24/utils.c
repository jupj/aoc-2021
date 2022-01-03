#include "utils.h"
#include <stdlib.h>

/*** Queue data structure implementation ***/

struct queue_element {
    void *data;
    Queue *next;
};

Queue *Enqueue(Queue *q, void *data) {
    Queue *elem = malloc(sizeof(Queue));
    elem->data = data;
    elem->next = NULL;

    if (q == NULL) {
        // Init new queue
        return elem;
    }

    // Append to queue
    Queue *ptr = q;
    while (ptr->next != NULL) {
        ptr = ptr->next;
    }
    ptr->next = elem;

    return q;
}

Queue *Dequeue(void *data, Queue *q) {
    if (q == NULL) {
        return NULL;
    }

    data = q->data;
    Queue *next = q->next;
    free(q);
    return next;
}
