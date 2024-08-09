#include <stdint.h>
#include <inttypes.h>

#define size 128

typedef struct node
{
  uint64_t key;
  uint64_t value;
  struct node *next;
} node;

typedef struct
{
  node *table[size];
} hashtable;

uint64_t hash(uint64_t key);
hashtable *ht_init(void);
void ht_insert(hashtable *ht, uint64_t key, uint64_t value);
uint64_t ht_get(hashtable *ht, uint64_t key);
void ht_free(hashtable *ht);
