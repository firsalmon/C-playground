#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

typedef struct buffer_head {
  struct buffer_head *prev;
  struct buffer_head *next;
  size_t size;
} buffer_head;

const static size_t buffer_head_size = sizeof(buffer_head);
const static size_t page_size = 4096;
static size_t pages_quantity = 0;
static uintptr_t start_adress = 0;
static size_t page_size_remaining = 0;
static buffer_head *current_head;

void *my_malloc(size_t size) {

  void *res;

  if ((size + buffer_head_size) > page_size_remaining) {
    void *p = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) {
      return NULL;
    }
    page_size_remaining = 4096;
    start_adress = (uintptr_t)(p);
    pages_quantity += 1;
  };

  buffer_head *point = (buffer_head *)((void *)start_adress);

  if (!current_head) {
    point->prev = NULL;
    point->next = NULL;
    point->size = size;
  } else {
    point->prev = current_head;
    point->next = NULL;
    point->size = size;
  }

  current_head = point;
  res = (void *)(point + 1);

  start_adress = (uintptr_t)res + size;
  page_size_remaining = page_size_remaining - size - buffer_head_size;

  return res;
}
