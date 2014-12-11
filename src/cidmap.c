#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include "cidmap.h"


static cidmap_t *g_cidmap_head;


void
coroutine_cidmap_init()
{
  assert(g_cidmap_head == NULL);

  //g_cidmap_head = (cidmap_t*)calloc(1, sizeof(cidmap_t));
  g_cidmap_head = (cidmap_t*)malloc(sizeof(cidmap_t));
  g_cidmap_head->next = NULL;
  g_cidmap_head->used = 0;
  g_cidmap_head->map = (ull*)mmap(NULL, 1024 * 4, PROT_READ | PROT_WRITE,
      MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
}


coroutine_t
coroutine_get_free_cid()
{
  unsigned long i, j, map_offset;
  coroutine_t ret;
  cidmap_t *cidmap;

  ret = 0;
  cidmap = g_cidmap_head;
  while (cidmap != NULL) {
    if (cidmap->used == bits_per_map) {
      cidmap = cidmap->next;
      ret += bits_per_map;
      continue;
    }

    for (i = 0; i < bits_per_map / ull_bits; ++i) {
      if (cidmap->map[i] == ~(0ull)) {
        continue;
      }

      for (j = 0; j < ull_bits; ++j) {
        if (!(cidmap->map[i] & ((1ull) << j))) {
          map_offset = i * ull_bits + j;
          BITSET_SETBIT(cidmap->map, map_offset, 1);
          cidmap->used++;
          return ret + map_offset;
        }
      }

    }
  }

  /* Create a new map for cid. */
  //cidmap->next = (cidmap_t*)calloc(1, sizeof(cidmap_t));
  cidmap->next = (cidmap_t*)malloc(sizeof(cidmap_t));
  cidmap = cidmap->next;
  cidmap->map = (ull*)mmap(NULL, sizeof(cidmap_t), PROT_READ | PROT_WRITE,
      MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  cidmap->used = 1;
  BITSET_SETBIT(cidmap->map, 0, 1);

  return ret;
}


void
coroutine_erase_cid(coroutine_t cid)
{
  cidmap_t *map;

  map = g_cidmap_head;
  while (cid > bits_per_map) {
    cid -= bits_per_map;
    map = map->next;
  }

  map->used--;
  BITSET_SETBIT(map->map, cid, 0);
}

