typedef struct heap Heap;
// CostFn returns the cost of the element
typedef unsigned long long (*CostFn)(void *data);

// HeapNew allocates a new heap
Heap *HeapNew();
// HeapFree frees up all internal memory used for h.
// Doesn't affect the memory of the elements.
void HeapFree(Heap *h);
// HeapInsert adds a new element to the heap, keeping
// the element with minimum cost at the root
void HeapInsert(Heap *h, void *data, CostFn cost);
// HeapExtract pops the element with the minimum cost
// from the heap. Returns NULL if heap is empty
void *HeapExtract(Heap *h, CostFn cost);
