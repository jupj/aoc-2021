#include <stdlib.h>
#include <stdbool.h>
#include "heap.h"

// heap_elem represents an element in the heap
typedef struct {
    void *data;
} heap_elem;

struct heap {
    heap_elem *elems;
    int cap, len;
};

Heap *HeapNew() {
    struct heap *h = malloc(sizeof(struct heap));
    h->elems = NULL;
    h->cap = 0;
    h->len = 0;
    return (Heap*) h;
}

void HeapFree(Heap *h) {
    struct heap *heap = (struct heap*) h;
    free(heap->elems);
    free(heap);
}

bool heap_empty(Heap *h) { return ((struct heap *) h)->len == 0; }

void HeapInsert(Heap *h, void *data, CostFn cost) {
    struct heap *heap = (struct heap*) h;
    if (heap->elems == NULL) {
        // Initialize empty heap
        heap->elems = malloc(sizeof(heap_elem));
        heap->len = heap->cap = 1;
        heap->elems[0].data = data;
        return;
    }

    if (heap->len >= heap->cap) {
        // Grow heap
        heap->cap = 2*heap->cap + 1;
        heap->elems = realloc(heap->elems, heap->cap*sizeof(heap_elem));
    }

    // Add to bottom level, first free space
    heap->elems[heap->len].data = data;
    int k = heap->len;
    heap->len++;

    // Satisfy the heap property: parent must cost less than child
    while (k > 0) {
        int parent = (k-1)/2;
        if (cost(heap->elems[k].data) < cost(heap->elems[parent].data)) {
            // Swap with parent
            heap_elem tmp = heap->elems[k];
            heap->elems[k] = heap->elems[parent];
            heap->elems[parent] = tmp;
        }
        k = parent;
    }
}

void *HeapExtract(Heap *h, CostFn cost) {
    struct heap *heap = (struct heap*) h;
    if (heap->len == 0) {
        return NULL;
    }

    heap->len--;
    void *extracted = heap->elems[0].data;
    heap->elems[0] = heap->elems[heap->len];

    // Satisfy the heap property: parent must cost less than child
    int k = 0;
    while (k < heap->len/2) {
        // Find the minimum child node
        int left = 2*k + 1;
        int right = 2*k + 2;
        int min_child = left;
        if (right < heap->len && cost(heap->elems[right].data) < cost(heap->elems[left].data)) {
            min_child = right;
        }
        
        if (cost(heap->elems[min_child].data) >= cost(heap->elems[k].data)) {
            // Heap property satisfied!
            break;
        }

        // swap parent and child
        heap_elem tmp = heap->elems[k];
        heap->elems[k] = heap->elems[min_child];
        heap->elems[min_child] = tmp;
        k = min_child;
    }

    return extracted;
}
