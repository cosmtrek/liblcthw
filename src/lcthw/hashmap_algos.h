#ifndef _Hashmap_algos_h
#define _Hashmap_algos_h

#include <stdint.h>

uint32_t Hashmap_fnv1a_hash(void *data);
uint32_t Hashmap_adler32_hash(void *data);
uint32_t Hashmap_djb_hash(void *data);

#endif
