#ifndef LAZYCOPY_H
#define LAZYCOPY_H

// This defines the size of a chunk of data we can request or copy. Must be a multiple of page size.
#define CHUNKSIZE 0x10000

// This function will be called at startup so you can set up a signal handler
void chunk_startup();

// This function should return a new chunk of memory for use
void* chunk_alloc();

// This function should return a copy of a chunk created with eager (normal) copying
void* chunk_copy_eager(void* chunk);

// This function should return a copy of a chunk created with lazy copying
void* chunk_copy_lazy(void* chunk);


// Struct to store begin and end adress for original and copy
typedef struct {
  void* start;
  void* end;
  struct lazy_t* next;
} lazy_t;

// Linked list to store data for all chunks
typedef struct {
  lazy_t* head;
} List;

// This function lets you add the address for a protectd chunk to our list
void chunk_add(void* chunk, List* list);

#endif
