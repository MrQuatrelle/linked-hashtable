#include "linked-hash-table.h"

/*
 * alocates memory for an lht and initializes it.
 * returns a pointer to the generated lht or NULL if there was any error in the
 * process.
 */
lht_t* lht_init(void) {
    lht_t* new = (lht_t*)malloc(sizeof(lht_t));
    int i;
    if (!new) {
        fprintf(stderr, "couldn't get memory for the new hash table!\n");
        return NULL;
    }
    /* allocs the hash table */
    new->raw = malloc(sizeof(void*) * INIT_HASH);
    if (!new->raw) {
        free(new);
        fprintf(stderr, "couldn't get memory for the new hash table!\n");
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

/*
 * frees the memory given to the lht.
 * every entry MUST be taken out before.
 */
void lht_destroy(lht_t* self) {
    if (!self)
        return;

    free(self->raw);
    free(self);
}

/*
 * main, string-based, hash function.
 */
size_t calculate_hash1(const char* str) {
    unsigned long hash = 0;
    int c;

    while ((c = *str++))
        hash = hash * 31 + c;

    return hash % INIT_HASH;
}

/*
 * alternative, string-based, hash function.
 * for collision rehashing.
 */
size_t calculate_hash2(const char* str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash % INIT_HASH;
}

/*
 * read:
 * https://www.scaler.com/topics/data-structures/double-hashing/
 */
__always_inline size_t rehash(const char* str, const size_t prev, int round) {
    return (prev + round * calculate_hash2(str)) % INIT_HASH;
}

/*
 * returns the index to the entry registered with the given key.
 * in case there isn't any correspondance, returns -1.
 */
__always_inline ssize_t lht_get_index(lht_t* self, const char* key) {
    size_t init, i;
    int round = 1;
    init = i = calculate_hash1(key);
    /* jumping collisions with multiple-hashing */
    while (self->raw[i]) {
        /* found the key */
        if (!strcmp(self->raw[i]->key, key))
            return i;
        /* didn't find it, rehashing */
        i = rehash(key, init, round);
        round++;
    }

    /* the key doesn't exist */
    return -1;
}

/*
 * if there is a value associated with the given key,
 * it'll return a pointer to it.
 * else, it'll return NULL.
 */
void* lht_get_entry(lht_t* self, const char* key) {
    ssize_t i = lht_get_index(self, key);

    return (i >= 0) ? self->raw[i]->value : NULL;
}

/*
 * insert a new entry into the lht.
 * key must have the same lifetime as the value (e.g. a pointer to an attribute
 * of the value).
 * if an error occurs, it returns -1.
 * if all went ok, returns 0.
 */
int lht_insert_entry(lht_t* self, const char* key, void* value) {
    size_t first_hash, i;
    lht_entry_t* new = malloc(sizeof(lht_entry_t));
    int round = 1;
    first_hash = i = (size_t)calculate_hash1(key);
    if (!new) {
        fprintf(stderr, "couldn't get memory for the new hash table node!\n");
        return -1;
    }
    /* jumping collisions */
    while (self->raw[i]) {
        i = rehash(key, first_hash, round);
        round++;
    }

    /* adding info to the lht entry */
    new->i = i;
    new->key = key;
    new->value = value;

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

/*
 * removes the entry from the given lht.
 * returns a pointer to the value.
 */
void* lht_leak_entry(lht_t* self, const char* key) {
    void* value;
    ssize_t i = lht_get_index(self, key);
    /* the isn't an entry associated to the given key */
    if (i < 0)
        return NULL;

    value = self->raw[i]->value;

    /* if it is not the last */
    if (self->raw[i]->next)
        self->raw[i]->next->prev = self->raw[i]->prev;
    else
        self->last = self->raw[i]->prev;

    /* if it is not the first */
    if (self->raw[i]->prev)
        self->raw[i]->prev->next = self->raw[i]->next;
    else
        self->first = self->raw[i]->next;

    /* frees the allocated space */
    free(self->raw[i]);
    self->raw[i] = NULL;

    /* sanity-check cleanup */
    if (--self->size == 0) {
        self->first = NULL;
        self->last = NULL;
    }

    return value;
}

/*
 * returns the number of entries registered in the lht.
 */
size_t lht_get_size(lht_t* self) { return self->size; }

/*
 * iterates over the given lht, according to the linking.
 * returns a pointer to the next entry or NULL if it reached the end.
 * if used the BEGIN flag, it'll go to the beggining of the table.
 * if used the KEEP flag, it'll keep going from where it was.
 */
void* lht_iter(lht_t* self, iter_setting setting) {
    lht_entry_t* next;
    if (setting == KEEP)
        next = self->lht_iterator_current->next;
    else
        next = self->first;
    self->lht_iterator_current = next;
    return (next) ? next->value : NULL;
}

/*
 * pops the last entry of the lht, according to the linking.
 * returns NULL if the lht is empty.
 */
void* lht_pop_entry(lht_t* self) {
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
