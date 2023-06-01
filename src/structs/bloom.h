/*file : bloom.h*/
#pragma once
#include <stdbool.h>

#define K 16 // the number of hash functions the filter uses

typedef struct bloom_filter* Bloom;

/* first hash function : djb2*/
unsigned long djb2(unsigned char *str);
/* second hash function : sdbm*/
unsigned long sdbm(unsigned char *str);
/* Return the result of the Kth hash function. This function uses djb2 and sdbm. */
unsigned long hash_i(unsigned char *str, unsigned int i);
/* creates a bloom filter of given size, returns a pointer to the structure */
Bloom bloom_create(unsigned int bloom_size);
/* checks if a given object-string is in bloom filter */
bool bloom_check(Bloom bloom, unsigned char * string);
/* inserts given object-string into bloom filter */
void bloom_insert(Bloom bloom, unsigned char * string);
/* deletes bloom filter data structure */
void bloom_destroy(Bloom bloom);
