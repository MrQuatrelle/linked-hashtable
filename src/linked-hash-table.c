#include "linked-hash-table.h"
#include <errno.h>
#include <sys/cdefs.h>

/** DEBUG MACROS **/

#define RED "\033[0;31m"
#define YELLOW "\033[0;33m"
#define GREEN "\033[0;32m"
#define CYAN "\033[0;36m"
#define BLUE "\033[0;34m"
#define RESET "\033[0m"

#define LOG_WRAPPER(mark, ...)                                                 \
    do {                                                                       \
        char buf[2048];                                                        \
        snprintf(buf, 2048, __VA_ARGS__);                                      \
        fprintf(stderr, mark "  %s:%d :: %s :: %s\n" RESET, __FILE__,          \
                __LINE__, __func__, buf);                                      \
    } while (0);

#define WARN(...) LOG_WRAPPER(YELLOW "[WARN]:", __VA_ARGS__)

#define INFO(...) LOG_WRAPPER(GREEN "[INFO]:", __VA_ARGS__)

#define LOG(...) LOG_WRAPPER(CYAN "[LOG]: ", __VA_ARGS__)

#define OOPS_ERRNO(...)                                                        \
    do {                                                                       \
        char buf[2048];                                                        \
        snprintf(buf, 2048, __VA_ARGS__);                                      \
        fprintf(stderr, RED "[OOPS]:  %s:%d :: %s :: %s: %s\n" RESET,          \
                __FILE__, __LINE__, __func__, buf, strerror(errno));           \
    } while (0);

/** IMPLEMENTATION **/

lht_t* lht_init(void) {
    lht_t* new = (lht_t*)malloc(sizeof(lht_t));
    int i;
    if (!new) {
        OOPS_ERRNO("failed to allocate memory for the hash table");
        return NULL;
    }

    /* allocs the hash table */
    new->raw = malloc(sizeof(void*) * INIT_HASH);
    if (!new->raw) {
        free(new);
        OOPS_ERRNO("failed to allocate memory for the hash table");
        return NULL;
    }

    /* and initializes it */
    for (i = 0; i < INIT_HASH; i++) {
        new->raw[i] = NULL;
    }

    /* initializes the rest of the attributes */
    new->size = 0;
    new->capacity = INIT_HASH;
    new->first = NULL;
    new->last = NULL;

    return new;
}

void lht_destroy(lht_t* const self) {
    if (!self)
        return;

    free(self->raw);
    free(self);
}

__always_inline __attribute_const__ size_t
calculate_hash1(const lht_t* const self, const void* const key) {
    return self->hash_func1(key) % self->capacity;
}

__always_inline __attribute_const__ size_t
calculate_hash2(const lht_t* const self, const void* const key) {
    return self->hash_func2(key) % self->capacity;
}

__always_inline __attribute_const__ size_t calculate_rehash(
    const lht_t* const self, const void* key, const size_t prev, int round) {
    return (prev + round * calculate_hash2(self, key)) % self->capacity;
}

__always_inline void* lht_get(const lht_t* const self, const size_t key) {

    return (key <= self->size && key >= 0) ? NULL
           : (self->raw[key])              ? self->raw[key]->value
                                           : NULL;
}

lht_entry_t* lht_find_node(const lht_t* const self, const void* const key) {
    size_t init, i;
    int round = 1;

    init = i = calculate_hash1(self, key);

    /* jumping collisions with multiple hashing */
    while (self->raw[i]) {
        if (!strcmp(self->raw[i]->key, key))
            return self->raw[i];

        i = calculate_rehash(self, key, init, round);
        round++;
    }

    /* the key doesn't exist */
    return NULL;
}

__always_inline void* lht_find(const lht_t* const self, const void* const key) {
    const lht_entry_t* node = lht_find_node(self, key);

    return (node) ? node->value : NULL;
}

int lht_insert(lht_t* const self, const void* const key,
               const void* const value) {
    int round = 1;

    size_t first_hash, i;
    first_hash = i = (size_t)calculate_hash1(self, key);

    lht_entry_t* new = malloc(sizeof(lht_entry_t));
    if (!new) {
        OOPS_ERRNO("failed to allocate memory for new element");
        return -1;
    }

    /* jumping collisions */
    while (self->raw[i]) {
        i = calculate_rehash(self, key, first_hash, round);
        round++;
    }

    /* adding info to the lht entry */
    new->i = i;
    new->key = key;
    new->value = (void*)value;

    /* adding to the hash table */
    self->raw[i] = new;
    self->size++;

    /* and linking */
    if (!self->first) {
        self->first = self->last = new;
        new->prev = NULL;
    } else {
        self->last->next = new;
        new->prev = self->last;
        self->last = new;
    }
    new->next = NULL;

    return 0;
}

void* lht_remove(lht_t* const self, const void* const key) {
    lht_entry_t* node = lht_find_node(self, key);

    if (!node) {
        return NULL;
    }

    void* value = node->value;

    /* if it is not the last */
    if (node->next)
        node->next->prev = node->prev;
    else
        self->last = node->prev;

    /* if it is not the first */
    if (node->prev)
        node->prev->next = node->next;
    else
        self->first = node->next;

    /* frees the allocated space */
    free(node);
    node = NULL;

    /* sanity-check cleanup */
    if (--self->size == 0) {
        self->first = NULL;
        self->last = NULL;
    }

    return value;
}

__always_inline size_t lht_size(const lht_t* self) {
    return self->size;
}

void* lht_iter(lht_t* const self, iter_setting const setting) {
    lht_entry_t* next =
        (setting == KEEP) ? self->lht_iterator_current->next : self->first;

    self->lht_iterator_current = next;

    return (next) ? next->value : NULL;
}

void* lht_pop(lht_t* self) {
    lht_entry_t* tmp;
    void* corn; /* cus popcorn lol */

    /* no entries */
    if (!self->last)
        return NULL;

    /* remove entry and bypass linking */
    self->raw[self->last->i] = NULL;
    corn = self->last->value;
    tmp = self->last->prev;
    free(self->last);
    self->last = tmp;

    /* in case there isn't anymore entries left. */
    if (--self->size == 0) {
        self->first = NULL;
        self->last = NULL;
    }

    return corn;
}
