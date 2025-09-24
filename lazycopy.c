#define _GNU_SOURCE
#include "lazycopy.h"

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

// Initialize global list
List chunks = { .head = NULL };
// Declare seg_handler function
void seg_handler(int signal, siginfo_t* info, void* ctx);


/**
 * This function will be called at startup so you can set up a signal handler.
 */
void chunk_startup() {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = seg_handler;
  sa.sa_flags = SA_SIGINFO;
  // Check sigaction for errors
  if (sigaction(SIGSEGV, &sa, NULL) != 0) {
    perror("sigaction failed");
    exit(2);
  }
}

// Signal handler for segmentation faults
void seg_handler(int signal, siginfo_t* info, void* ctx){
  // Get adress of writing caused segfault
  intptr_t addr = (intptr_t)info->si_addr;
  lazy_t* curr = chunks.head;
  // Check if adress is within any of the protected chunks
  while(curr != NULL){
    if(addr >= (intptr_t)curr->start && addr < (intptr_t)curr->end){
      // Create local array and copy contents of chunk to it
      uint8_t temp[CHUNKSIZE];
      memcpy(temp, curr->start, CHUNKSIZE);
      // Override protected memory to make it writable
      mmap(curr->start, CHUNKSIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS |MAP_SHARED | MAP_FIXED, -1, 0);
      // Restore Chunk to new memory
      memcpy(curr->start, temp, CHUNKSIZE);
      return;
    }
    curr = curr->next;
  }
  // If we get here, the segfault was not in a protected chunk. Exit with error.
  fprintf(stderr, "Segmentation fault at address %p\n", info->si_addr);
  exit(1);
}

/**
 * This function should return a new chunk of memory for use.
 *
 * \returns a pointer to the beginning of a 64KB chunk of memory that can be read, written, and
 * copied
 */
void* chunk_alloc() {
  // Call mmap to request a new chunk of memory. See comments below for description of arguments.
  void* result = mmap(NULL, CHUNKSIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
  // Arguments:
  //   NULL: this is the address we'd like to map at. By passing null, we're asking the OS to
  //   decide. CHUNKSIZE: This is the size of the new mapping in bytes. PROT_READ | PROT_WRITE: This
  //   makes the new reading readable and writable MAP_ANONYMOUS | MAP_SHARED: This mapes a new
  //   mapping to cleared memory instead of a file,
  //                               which is another use for mmap. MAP_SHARED makes it possible for
  //                               us to create shared mappings to the same memory.
  //   -1: We're not connecting this memory to a file, so we pass -1 here.
  //   0: This doesn't matter. It would be the offset into a file, but we aren't using one.

  // Check for an error
  if (result == MAP_FAILED) {
    perror("mmap failed in chunk_alloc");
    exit(2);
  }

  // Everything is okay. Return the pointer.
  return result;
}

/**
 * Create a copy of a chunk by copying values eagerly.
 *
 * \param chunk This parameter points to the beginning of a chunk returned from chunk_alloc()
 * \returns a pointer to the beginning of a new chunk that holds a copy of the values from
 *   the original chunk.
 */
void* chunk_copy_eager(void* chunk) {
  // First, we'll allocate a new chunk to copy to
  void* new_chunk = chunk_alloc();

  // Now copy the data
  memcpy(new_chunk, chunk, CHUNKSIZE);

  // Return the new chunk
  return new_chunk;
}

/**
 * Create a copy of a chunk by copying values lazily.
 *
 * \param chunk This parameter points to the beginning of a chunk returned from chunk_alloc()
 * \returns a pointer to the beginning of a new chunk that holds a copy of the values from
 *   the original chunk.
 */
void* chunk_copy_lazy(void* chunk) {
  // Cite online man page
  void* copy = mremap(chunk, 0, CHUNKSIZE, MREMAP_MAYMOVE | MREMAP_FIXED, NULL);
  if (mprotect(copy, CHUNKSIZE, PROT_READ) != 0) exit(1);
  if (mprotect(chunk, CHUNKSIZE, PROT_READ) != 0) exit(1);
  chunk_add(chunk, &chunks);
  chunk_add(copy, &chunks);
  return copy;
}

void chunk_add(void* chunk, List* list) {
  // create a new lazy_t struct to hold the start and end of the chunk
  lazy_t* lazy_chunk = (lazy_t*)malloc(sizeof(lazy_t));
  lazy_chunk->start = chunk;
  lazy_chunk->end = (uint8_t*)chunk + CHUNKSIZE;
  // add the new lazy_t struct to the head of the linked list if empty
  if (list->head == NULL) {
    list->head = lazy_chunk;
    lazy_chunk->next = NULL;
  } else {
    // otherwise, add it to the end of the linked list
    for (lazy_t* curr = list->head; curr != NULL; curr = curr->next) {
      if (curr->next == NULL) {
        curr->next = lazy_chunk;
        lazy_chunk->next = NULL;
        break;
      }
    }
  }
}
