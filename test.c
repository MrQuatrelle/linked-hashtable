#include "src/linked-hash-table.h"

const size_t fn(const void* key) {
    return 0;
}

int main() {
    lht_t* lht = lht_init(fn, fn, NULL);
    lht_destroy(lht);

    return 0;
}
