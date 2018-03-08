// vim: et:sts=4:ts=4:sw=4
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <linux/mman.h>

#include "syscalls.h"
#include "child_pids_helper.h"
#include "test_helper.h"

#define MAX_MEM_MB    (10 * 1024)
#define MAX_CHILDREN  1000
#define NO_ITERATIONS 10

static int do_master_process(unsigned int mbytes, unsigned int children) {
    pid_t cpids[children];
    fork_children(cpids, children);
    size_t map_len = 1024 * 1024 * mbytes;
    size_t len = map_len / sizeof(pid_t);
    pid_t *buffer = (pid_t *) malloc(map_len);
    if (buffer == NULL) errx(1, "could not allocate memory\n");
    int i;
    int j = NO_ITERATIONS;
    int error_cnt = 0;

    for (i = 0; i < children; i += 1024) buffer[i] = i;

    while (j--) {
        for (i = 0; i < len - children; i += 1024) {
            size_t num_children;
            if (get_child_pids(buffer + i, children, &num_children)) error_cnt++;
        }
    }

    join_children(cpids, children);
    if (error_cnt) {
        errx(1, "There were %d syscall errors during the test\n", error_cnt);
    }

    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 4) errx(1, "Usage: %s <no_concurrent_procs> <mem_mbytes> <no_children>\n", argv[0]);
    unsigned int concurrent_procs = 0;
    unsigned int children = 0;
    unsigned int mbytes = MAX_MEM_MB + 1;
    sscanf(argv[1], "%u", &concurrent_procs);
    sscanf(argv[2], "%u", &mbytes);
    sscanf(argv[3], "%u", &children);
    if (concurrent_procs < 1) errx(1, "Specified invalid number of concurrent processes: %d\n", concurrent_procs);
    if (mbytes < 0 || mbytes > MAX_MEM_MB) errx(1, "Specified invalid memory to use: %d\n", mbytes);
    if (children < 1 || children > MAX_CHILDREN) errx(1, "Specified invalid no of children to use: %d\n", children);

    test_title("GET_CHILD_PIDS syscall - no schedule() w/ lock held");
    fprintf(stdout, "*** Starting %d processes, each allocating %d MB.\n", concurrent_procs, mbytes);
    fprintf(stdout, "*** They will page fault, testing correct lock usage.\n");
    fflush(stdout);
    int i;
    char *test_message;
    pid_t *worker_pids = alloca(concurrent_procs * sizeof(pid_t));

    for (i = 0 ; i < concurrent_procs ; i++) {
        worker_pids[i] = fork();
        if (worker_pids[i] < 0) err(1, "Unable to create concurrent master processes");
        if (worker_pids[i] == 0) do_master_process(mbytes, children); // Doesn't return.
    }

    for (i = 0; i < concurrent_procs; i++) {
        int status = 47;
        pid_t wpid = waitpid(worker_pids[i], &status, 0);
        if (wpid != worker_pids[i]) errx(1, "Bad wait?\n");

        asprintf(&test_message, "Process %d, using %d MB memory, spawning %d children", i, mbytes, children);
        test_true(test_message, (WIFEXITED(status) && (WEXITSTATUS(status) == 0)));
        free(test_message);
    }

    return 0;
}
