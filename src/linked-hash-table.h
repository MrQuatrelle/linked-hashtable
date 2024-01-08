#ifndef LHT_HEADER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/* just a decently large prime number */
#define INIT_HASH 216091

typedef struct lht_entry {
    const char* key;
    void* value;
    struct lht_entry* next;
    struct lht_entry* prev;
    unsigned long i;
} lht_entry_t;

typedef struct lht {
    lht_entry_t** raw;
    size_t size;
    size_t capacity;
    lht_entry_t* first;
    lht_entry_t* last;
    lht_entry_t* lht_iterator_current;
} lht_t;

typedef enum {
    BEGIN,
    KEEP
} iter_setting;

lht_t* lht_init(void);
void lht_destroy(lht_t* self);
int lht_insert_entry(lht_t* self, const char* key, void* value);
void* lht_leak_entry(lht_t* self, const char* key);
void* lht_get_entry(lht_t* self, const char* key);
void* lht_pop_entry(lht_t* self);
void* lht_iter(lht_t* table, iter_setting setting);
size_t lht_get_size(lht_t* self);

#endif /* !LHT_HEADER */
