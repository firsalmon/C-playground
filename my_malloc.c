#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

typedef size_t P_SIZE;
typedef size_t BHEAD_SIZE;
typedef size_t AHEAD_SIZE;
typedef size_t SARENA_SIZE;

typedef struct arena_head {
  struct arena_head *prev;
  struct arena_head *next;
  size_t size;
  uintptr_t adress;
} arena_head;

typedef struct buffer_head {
  struct buffer_head *prev;
  struct buffer_head *next;
  arena_head *current_arena;
  size_t size;
} buffer_head;

const P_SIZE SIZE_PAGE = 4096;
const BHEAD_SIZE SIZE_BUFFER_HEAD = sizeof(buffer_head);
const AHEAD_SIZE SIZE_ARENA_HEAD = sizeof(arena_head);
const SARENA_SIZE SIZE_SMALL_ARENA = SIZE_PAGE * 16;

//
//

static arena_head *small_arena;
static buffer_head *current_head;

void *my_malloc(size_t size) {
  size_t aligned_size = (16 - (size % 16)) + size;
  //
  size_t current_size = size + SIZE_BUFFER_HEAD;
  size_t current_aligned_size = (16 - (current_size % 16)) + current_size;

  if (aligned_size >= SIZE_PAGE) {
    uint8_t page_quantity = ceil((float_t)aligned_size / (float_t)SIZE_PAGE);
    size_t page_size = SIZE_PAGE * page_quantity;

    void *res = mmap(NULL, page_size, PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (res == MAP_FAILED) {
      return NULL;
    }
    arena_head *r = (arena_head *)res;
    r->prev = NULL;
    r->next = NULL;
    r->size = aligned_size;
    r->adress = (uintptr_t)res;
    return r;
  } else {
    if (!small_arena) {
      void *p = mmap(NULL, SIZE_SMALL_ARENA, PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
      if (p == MAP_FAILED) {
        return NULL;
      }
      arena_head *pv = (arena_head *)p;
      pv->prev = NULL;
      pv->next = NULL;
      pv->size = SIZE_SMALL_ARENA;
      pv->adress = (uintptr_t)p;
      small_arena = pv;
    }
    if (current_aligned_size > small_arena->size) {
      void *p = mmap(NULL, SIZE_SMALL_ARENA, PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
      arena_head *r;
      r->prev = small_arena;
      r->next = NULL;
      r->size = SIZE_SMALL_ARENA;
      r->adress = (uintptr_t)p;
      //
      small_arena->next = r;
      small_arena = r;
    }
    //
    buffer_head *res = (buffer_head *)(small_arena->adress +
                                       (SIZE_SMALL_ARENA - small_arena->size) +
                                       SIZE_ARENA_HEAD);

    return 0;
  }
}

void my_calloc(size_t size) {}

void my_realloc() {}

void my_free(void *p) {}

void my_memcpy() {}

void my_memmfree() {}
