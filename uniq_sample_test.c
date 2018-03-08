// vim: et:sts=4:ts=4:sw=4
#include <stdio.h>
#include "syscalls.h"
#include "test_helper.h"

#define MAX_PRINT_UUIDS    100
#define MIN(a,b)           ((a) < (b) ? (a) : (b))
static int
comparator(const void *a, const void *b) {
    return *(int *) a - *(int *) b;
}

static inline
long check_n_successive_calls(unsigned int n) {
    int i;
    long err;
    int *results = alloca(n * sizeof(int));
    int *sorted_results = alloca(n * sizeof(int));

    if (n < 2) return 0;

    for (i = 0 ; i < n ; i++) {
        err = get_unique_id(results + i);
        if (err) {
            fprintf(stderr, "Error on call #%d to get_unique_id(): %ld (%s).\n", i + 1, err, strerror((int) err));
            return err;
        }
        sorted_results[i] = results[i];
    }

    qsort(sorted_results, n, sizeof(int), &comparator);
    for (i = 1 ; i < n ; i++) {
        if (sorted_results[i - 1] == sorted_results[i]) {
            fprintf(stderr, "At least 2 (not necessarily successive) calls returned the same value.\n");
            fprintf(stderr, "Returned values: [");
            for (i = 0 ; i < MIN(n, MAX_PRINT_UUIDS) ; i++)
                fprintf(stderr, "%d%s", results[i], i == (n - 1) ? "]" : ", ");
            if (n > MAX_PRINT_UUIDS) fprintf(stderr, "..., %d]", results[n - 1]);
            fprintf(stderr, "\n");
            return -1;
        }
    }

    return 0;
}

int main (int argc, char **argv) {
    int uuid;

    test_title("GET_UNIQUE_ID syscall");
    fatal_not_test("syscall is implemented", (int) get_unique_id(&uuid), ENOSYS);
    test("2 successive calls return unique values", (int) check_n_successive_calls(2), 0);
    test("100,000 successive calls return unique values", (int) check_n_successive_calls(100000), 0);
    not_test("Invalid uuid address (47424742)", (int) get_unique_id((int *)47424742), 0);

    return 0;
}
