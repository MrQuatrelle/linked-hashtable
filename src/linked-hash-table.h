#ifndef LHT_HEADER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define INIT_HASH 16

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

typedef struct lht_entry {
    const void* key;
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
    size_t (*hash_func1)(const void* key);
    size_t (*hash_func2)(const void* key);
} lht_t;

typedef enum {
    NORM,
    REV,
} iter_setting;

typedef struct lht_iter {
    lht_t* lht;
    lht_entry_t* curr;
    const iter_setting setting;
} lht_iter_t;

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
void* lht_get(const lht_t* self, const size_t key);

/*
 * Insert a new entry into the lht.
 * The key must have the same lifetime as the value (e.g. a pointer to an
 * attribute of the value).
 * If an error occurs, it returns -1.
 * If all went ok, returns 0.
 */
int lht_insert(lht_t* self, const void* key, const void* obj);

/*
 * Removes the entry from the given lht.
 * Returns a pointer to the value.
 */
void* lht_remove(lht_t* const self, const void* key);

/*
 * Returns the number of entries registered in the lht.
 */
size_t lht_size(const lht_t* self);

/*
 * Pops the last entry of the lht, according to the linking.
 * Returns NULL if the lht is empty.
 */
void* lht_pop(lht_t* self);

/*
 * Creates an iterator over the given lht, which iterates according to the
 * linking in normal order or reversed order (check the iter_setting enum)
 * table.
 * Use the lht_iter_next() and lht_iter_prev() functions to push it along.
 */
lht_iter_t* lht_iter_init(lht_t* table, iter_setting setting);

/**
 * Destroys the dyamic memory used to keep its state.
 */
void lht_iter_destroy(lht_iter_t* self);

/**
 * Pushes the iterator to the next element, if it exists, and returns it.
 * If it doesn't exist, then this function will return NULL and won't push the
 * iterator.
 */
void* lht_iter_next(lht_iter_t* self);

/**
 * Pushes the iterator to the previous element, if it exists, and returns it.
 * If it doesn't exist, then this function will return NULL and won't push the
 * iterator.
 */
void* lht_iter_prev(lht_iter_t* self);

#endif /* !LHT_HEADER */
