#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

//
//

typedef size_t P_SIZE;
typedef size_t BHEAD_SIZE;
typedef size_t AHEAD_SIZE;
typedef size_t SARENA_SIZE;

typedef struct arena_head {
  struct arena_head *prev;
  struct arena_head *next;
  size_t size;
  uintptr_t adress;
  bool status;
} arena_head;

typedef struct buffer_head {
  struct buffer_head *prev;
  struct buffer_head *next;
  arena_head *current_arena;
  size_t size;
  bool status;
} buffer_head;

//
//

const P_SIZE SIZE_PAGE = 4096;
const BHEAD_SIZE SIZE_BUFFER_HEAD = sizeof(buffer_head);
const AHEAD_SIZE SIZE_ARENA_HEAD = sizeof(arena_head);
const SARENA_SIZE SIZE_SMALL_ARENA = SIZE_PAGE * 16;

//
//

static arena_head *small_arena;
static buffer_head *current_head;

//
size_t align16(size_t x) { return ((x + 15) & ~15); }
//

void *my_malloc(size_t size) {

  size_t aligned_size = align16(size);
  size_t aligned_buffer_head = align16(SIZE_BUFFER_HEAD);
  size_t aligned_arena_head = align16(SIZE_ARENA_HEAD);
  size_t aligned_current_size = aligned_size + aligned_buffer_head;

  if (aligned_current_size >= SIZE_PAGE) {
    //
    // если размер буфера > 4кб (размер страницы)
    // тогда создаем отдельную арену под буфер > 4кб
    //
    // ps а когда будет использоваться free() то просто освободим страницы под
    // буфер (поэтому и не вижу особо смысла делать ссылки на другие большие
    // буферы)
    //
    size_t page_quantity =
        ceil((float_t)aligned_current_size / (float_t)SIZE_PAGE);
    size_t page_size = SIZE_PAGE * page_quantity + aligned_buffer_head;

    void *res = mmap(NULL, page_size, PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (res == MAP_FAILED) {
      return NULL;
    }
    buffer_head *r = (buffer_head *)res;
    r->prev = NULL;
    r->next = NULL;
    r->size = aligned_size;
    r->status = true;
    uintptr_t mid_r = (uintptr_t)(r + 1);
    mid_r += (aligned_buffer_head - SIZE_BUFFER_HEAD);
    return (void *)(mid_r);
    //
    //
  } else {
    //
    // иначе
    // создаем буфер в small_arena
    //
    if (!small_arena) {
      //
      // если нет small_arena,
      // тогда создаем small_arena
      //
      void *p = mmap(NULL, SIZE_SMALL_ARENA, PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
      if (p == MAP_FAILED) {
        return NULL;
      }
      arena_head *pv = (arena_head *)p;
      pv->prev = NULL;
      pv->next = NULL;
      pv->size = SIZE_SMALL_ARENA;
      uintptr_t mid_r = (uintptr_t)(pv + 1);
      mid_r += (aligned_arena_head - SIZE_ARENA_HEAD);
      pv->adress = (uintptr_t)(pv + 1) + mid_r;
      
      small_arena = pv;
    }
    if (aligned_current_size > small_arena->size) {
      //
      // если буфер больше свободного места в small_arena,
      // тогда создаем новую small_arena
      //
      void *p = mmap(NULL, SIZE_SMALL_ARENA + aligned_arena_head, PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
      arena_head *r = (arena_head *)p;
      r->prev = small_arena;
      r->next = NULL;
      r->size = SIZE_SMALL_ARENA;
      r->adress = (uintptr_t)p;
      //
      small_arena->next = r;
      small_arena = r;
    }
    //
    // добавляем буфер в small_arena
    // возвращаем буфер
    //
    buffer_head *res = (buffer_head *)(small_arena->adress);
    res->current_arena = small_arena;
    res->size = current_aligned_size;
    res->prev = NULL;
    res->next = NULL;
    res->status = true;

    if (!current_head) {
      current_head = res;
    } else {
      current_head->next = res;
      res->prev = current_head;
      current_head = res;
    }

    small_arena->adress = small_arena->adress + current_aligned_size;
    small_arena->size = small_arena->size - current_aligned_size;

    return (void *)(res + 1);
  }
}

void my_calloc(size_t size) {}

void my_realloc() {}

void my_free(void *p) {}

void my_memcpy() {}

void my_memmfree() {}
