#include "src/linked-hash-table.h"

int main() {
    lht_t* lht = lht_init();
    lht_destroy(lht);

    return 0;
}
