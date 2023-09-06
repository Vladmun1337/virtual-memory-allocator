#pragma once
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#define MAXSIZE 255

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);				        \
		}							\
	} while (0)

/* TODO : add your implementation for doubly-linked list */
typedef struct {
	
	void* head;
} list_t;

typedef struct block_t block_t;
struct block_t {
   uint64_t start_address; 
   size_t size; 
   void* miniblock_list;
   struct block_t *prev, *next;
};

typedef struct miniblock_t miniblock_t;
struct miniblock_t {
   uint64_t start_address;
   size_t size;
   uint8_t perm;
   void* rw_buffer;
   struct miniblock_t *prev, *next;
};

typedef struct arena_t arena_t;
struct arena_t {
   uint64_t arena_size;
   uint64_t used_mem;
	size_t size_block, size_minib;
   list_t *alloc_list;
};

arena_t* alloc_arena(const uint64_t size);
void dealloc_arena(arena_t* arena);

void alloc_block(arena_t* arena, const uint64_t address, const uint64_t size);
void free_block(arena_t* arena, const uint64_t address);

void read(arena_t* arena, uint64_t address, uint64_t size);
void write(arena_t* arena, const uint64_t address,  const uint64_t size, int8_t *data);
void pmap(const arena_t* arena);
void mprotect(arena_t* arena, uint64_t address, int8_t *permission);
void full_read(char carac);
