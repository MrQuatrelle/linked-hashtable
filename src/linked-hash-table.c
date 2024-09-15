#include "linked-hash-table.h"

lht_t* lht_init(size_t (*const hf1)(const void*),
                size_t (*const hf2)(const void*),
                size_t (*const cmp)(const void*, const void*)) {

    lht_entry_t** raw = malloc(sizeof(void*) * INIT_HASH);

    if (raw) {
        goto fail_inner;
    }

    memset(raw, 0, sizeof(void*) * INIT_HASH);

    lht_t new = {
        .raw = raw,
        .first = NULL,
        .last = NULL,
        .size = 0,
        .capacity = INIT_HASH,
        .hash_func1 = hf1,
        .hash_func2 = hf2,
        .cmp = cmp,
    };

    lht_t* new_dyn = malloc(sizeof(lht_t));

    if (new_dyn) {
        goto success;
    }

    free(raw);
fail_inner:
    OOPS_ERRNO("Failed to alloc memory for the new lht");
    return NULL;

success:
    memcpy(new_dyn, &new, sizeof(lht_t));
    return new_dyn;
}

void lht_destroy(lht_t* const self) {
    if (!self) {
        return;
    }

    free(self->raw);
    free(self);
}

__always_inline static size_t __calculate_hash1(const lht_t* const self,
                                                const void* const key) {
    return self->hash_func1(key) % self->capacity;
}

__always_inline static size_t __calculate_hash2(const lht_t* const self,
                                                const void* const key) {
    return self->hash_func2(key) % self->capacity;
}

__always_inline static size_t __calculate_rehash(const lht_t* const self,
                                                 const void* key,
                                                 const size_t prev,
                                                 int round) {
    return (prev + (round * __calculate_hash2(self, key))) % self->capacity;
}

__always_inline void* lht_get(const lht_t* const self, const size_t key) {
    return (key <= self->size && key >= 0) ? NULL
           : (self->raw[key])              ? self->raw[key]->value
                                           : NULL;
}

lht_entry_t* __lht_find_node(const lht_t* const self, const void* const key) {
    size_t init, i;
    int round = 1;

    init = i = __calculate_hash1(self, key);

    /* jumping collisions with multiple hashing */
    while (self->raw[i]) {
        if (!strcmp(self->raw[i]->key, key)) {
            return self->raw[i];
        }

        i = __calculate_rehash(self, key, init, round);
        round++;
    }

    /* the key doesn't exist */
    return NULL;
}

__always_inline void* lht_find(const lht_t* const self, const void* const key) {
    const lht_entry_t* node = __lht_find_node(self, key);

    return (node) ? node->value : NULL;
}

lht_iter_t lht_iter_init(lht_t* const, const iter_setting);
lht_entry_t* __lht_iter_next_inner(lht_iter_t* const);
void __lht_iter_put(lht_iter_t* const, lht_entry_t* const);

int __lht_update_capacity(lht_t* const self, const long new_cap) {
    lht_entry_t** new_raw = realloc(self->raw, new_cap * sizeof(lht_entry_t*));

    if (new_raw == NULL) {
        return -1;
    }

    for (size_t i = self->capacity; i < new_cap; ++i) {
        new_raw[i] = NULL;
    }

    /* need to update capacity already for hash functions to work */
    self->capacity = new_cap;

    lht_iter_t it = lht_iter_init(self, NORM);
    lht_entry_t *entry, *prev_entry = NULL;
    int r = 0;

    while ((entry = __lht_iter_next_inner(&it))) {
        size_t i = __calculate_hash1(self, entry->key);

        while (new_raw[i]) {
            i = __calculate_rehash(self, entry->key, i, r++);
        }

        entry->i = i;
        new_raw[i] = entry;

        if (likely(prev_entry)) {
            prev_entry->next = entry;
            entry->prev = prev_entry;
        } else {
            entry->prev = NULL;
        }
    }

    it.curr->next = NULL;

    return 0;
}

int __lht_increase_capacity(lht_t* const self) {
    if (self->size < (self->capacity >> 1)) {
        return 0;
    }

    return __lht_update_capacity(self, self->capacity << 1);
}

int __lht_decrease_capacity(lht_t* const self) {
    if (self->size > (self->capacity >> 3)) {
        return -1;
    }
    if (self->capacity <= 16) {
        return 0;
    }

    return __lht_update_capacity(self, self->capacity >> 1);
}

int lht_insert(lht_t* const self,
               const void* const key,
               const void* const value) {
    lht_entry_t* new = malloc(sizeof(lht_entry_t));
    if (!new) {
        OOPS_ERRNO("failed to allocate memory for new element");
        return -1;
    }

    int round = 1;
    size_t first_hash, i;
    first_hash = i = (size_t)__calculate_hash1(self, key);

    /* jumping collisions */
    while (self->raw[i]) {
        i = __calculate_rehash(self, key, first_hash, round);
        round++;
    }

    if (__lht_increase_capacity(self) == -1) {
        OOPS_ERRNO("Failed to increase size of lht");
    }

    /* adding info to the lht entry */
    new->i = i;
    new->key = key;
    new->value = (void*)value;

    /* adding to the hash table */
    self->raw[i] = new;
    self->size++;

    /* and linking */

    /* is empty */
    if (!self->size) {
        self->first = self->last = new;
        new->prev = NULL;
        new->next = NULL;
        return 0;
    }

    /* ordered by insertion, put on the end */
    if (!self->cmp) {
        self->last->next = new;
        new->prev = self->last;
        self->last = new;
    }
    /* else, we need to "ask" the comparator where to insert it */

    /* shall be put on the last position */
    else if (self->cmp(self->last->key, new->key) > 0) {
        self->last->next = new;
        new->prev = self->last;
        self->last = new;
    }
    /* somewhere in the middle */
    else {
        lht_iter_t it = lht_iter_init(self, NORM);

        /* advance until put location */
        while (self->cmp(__lht_iter_next_inner(&it)->key, new->key) < 0) {
            ;
        }

        __lht_iter_put(&it, new);
    }

    return 0;
}

void* lht_remove(lht_t* const self, const void* const key) {
    lht_entry_t* node = __lht_find_node(self, key);

    if (!node) {
        return NULL;
    }

    void* value = node->value;

    /* if it is not the last */
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        self->last = node->prev;
    }

    /* if it is not the first */
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        self->first = node->next;
    }

    /* frees the allocated space */
    free(node);
    node = NULL;

    /* sanity-check cleanup */
    if (--self->size == 0) {
        self->first = NULL;
        self->last = NULL;
    }

    if (__lht_decrease_capacity(self) == -1) {
        OOPS_ERRNO("Failed to decrease size of lht");
    }

    return value;
}

__always_inline size_t lht_size(const lht_t* self) {
    return self->size;
}

void* lht_pop(lht_t* self) {
    lht_entry_t* tmp;
    void* corn; /* cus popcorn lol */

    /* no entries */
    if (!self->last) {
        return NULL;
    }

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

    if (__lht_decrease_capacity(self) == -1) {
        OOPS_ERRNO("Failed to decrease size of lht");
    }

    return corn;
}

/* ITERATOR */

lht_iter_t lht_iter_init(lht_t* const self, const iter_setting setting) {
    if (setting != NORM && setting != REV) {
        PANIC("Unknown lht_iter setting");
    }

    lht_iter_t it = {
        .lht = self,
        .setting = setting,
        .curr = NULL,
    };

    return it;
}

lht_entry_t* __lht_iter_norm_next(lht_iter_t* const self) {
    if (unlikely(!self->lht->first)) {
        return NULL;
    }

    if (unlikely(!self->curr)) {
        self->curr = self->lht->first;
    } else if (likely(self->curr->next)) {
        self->curr = self->curr->next;
    } else {
        /* is last element, let curr stay unchanged */
        return NULL;
    }

    return self->curr;
}

lht_entry_t* __lht_iter_norm_prev(lht_iter_t* const self) {
    if (unlikely(!self->curr)) {
        /* was never initialized */
        return NULL;
    } else if (likely(self->curr->prev)) {
        self->curr = self->curr->prev;
    } else {
        /* is first element, let curr stay unchanged */
        return NULL;
    }

    return self->curr;
}

lht_entry_t* __lht_iter_rev_next(lht_iter_t* const self) {
    if (unlikely(!self->lht->last)) {
        return NULL;
    }

    if (unlikely(!self->curr)) {
        self->curr = self->lht->last;
    } else if (likely(self->curr->prev)) {
        self->curr = self->curr->prev;
    } else {
        /* is last element, let curr stay unchanged */
        return NULL;
    }

    return self->curr;
}

lht_entry_t* __lht_iter_rev_prev(lht_iter_t* const self) {
    if (unlikely(!self->curr)) {
        /* was never initialized */
        return NULL;
    } else if (likely(self->curr->next)) {
        self->curr = self->curr->next;
    } else {
        /* is first element, let curr stay unchanged */
        return NULL;
    }

    return self->curr;
}

__always_inline lht_entry_t* __lht_iter_next_inner(lht_iter_t* const self) {
    return (self->setting == NORM) ? __lht_iter_norm_next(self)
                                   : __lht_iter_rev_next(self);
}

__always_inline lht_entry_t* __lht_iter_prev_inner(lht_iter_t* const self) {
    return (self->setting == NORM) ? __lht_iter_norm_next(self)
                                   : __lht_iter_rev_next(self);
}

__always_inline void* lht_iter_next(lht_iter_t* const self) {
    return __lht_iter_next_inner(self)->value;
}

__always_inline void* lht_iter_prev(lht_iter_t* const self) {
    return __lht_iter_prev_inner(self)->value;
}

void __lht_iter_put(lht_iter_t* const it, lht_entry_t* const new) {
    it->curr->prev->next = new;
    it->curr->prev = new;
    new->next = it->curr;
    it->curr = new;
}

// TODO: check if this is ok
void* lht_iter_pop(lht_iter_t* const self) {
    if (unlikely(!self->curr)) {
        return NULL;
    }

    lht_entry_t* next = self->curr->next;
    void* ret = lht_remove(self->lht, self->curr->key);
    self->curr = next;

    return ret;
}
