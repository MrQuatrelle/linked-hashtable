#ifndef LHT_HEADER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define INIT_HASH 16

typedef struct lht_entry {
    const char* key;
    void* value;
    struct lht_entry* next;
    struct lht_entry* prev;
    unsigned long i;
} lht_entry_t;

/**
 * @typedef lht_t
 * @brief Struct that represents the linked-hashtable.
 *
 */
typedef struct lht {
    lht_entry_t** raw;
    size_t size;
    size_t capacity;

    lht_entry_t* first;
    lht_entry_t* last;
    lht_entry_t* lht_iterator_current;

    /*
     * Hash functions provided by the user to calculate the indexes of
     * insertion. Why two? Read:
     * https://www.scaler.com/topics/data-structures/double-hashing/
     */
    size_t (*hash_func1)(const void* obj);
    size_t (*hash_func2)(const void* obj);
} lht_t;

typedef enum { BEGIN, KEEP } iter_setting;

/*
 * Alocates memory for an lht and initializes it.
 * Returns a pointer to the generated lht or NULL if there was any error in the
 * process.
 */
lht_t* lht_init(void);

/*
 * Frees the memory given to the lht.
 * Every entry MUST be popped out before to avoid memory leaks.
 */
void lht_destroy(lht_t* self);

/*
 * If there is a value associated with the given key,
 * It'll return a pointer to it.
 * Else, it'll return NULL.
 */
void* lht_get_entry(lht_t* self, const void* key);

/*
 * Insert a new entry into the lht.
 * The key must have the same lifetime as the value (e.g. a pointer to an
 * attribute of the value).
 * If an error occurs, it returns -1.
 * If all went ok, returns 0.
 */
int lht_insert_entry(lht_t* self, const void* key, void* obj);

/*
 * Removes the entry from the given lht.
 * Returns a pointer to the value.
 */
void* lht_remove_entry(lht_t* self, const void* key);

/*
 * Pops the last entry of the lht, according to the linking.
 * Returns NULL if the lht is empty.
 */
void* lht_pop_entry(lht_t* self);

/*
 * Iterates over the given lht, according to the linking.
 * Returns a pointer to the next entry or NULL if it reached the end.
 * If used the BEGIN flag, it'll go to the beggining of the table.
 * If used the KEEP flag, it'll keep going from where it was.
 */
void* lht_iter(lht_t* table, iter_setting setting);

/*
 * Returns the number of entries registered in the lht.
 */
size_t lht_get_size(lht_t* self);

#endif /* !LHT_HEADER */
