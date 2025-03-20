#include "src/linked-hash-table.h"

size_t fn(const void* key) {
    (void)key;
    return 0;
}

int main(void) {
    lht_t* lht = lht_init(fn, fn, NULL);
    lht_t* lht2 = lht_init(fn, fn, NULL);
    int i = 2;
    lht_insert(lht, "bla", "bla2");
    lht_insert(lht2, "bla", &i);

    return 0;
}
