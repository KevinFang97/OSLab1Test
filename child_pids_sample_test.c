// vim: et:sts=4:ts=4:sw=4
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <sys/wait.h>
#include "syscalls.h"
#include "test_helper.h"

void print_list(pid_t *list, size_t limit) {
    int i;
    fprintf(stderr, "List content: [");
    for (i = 0 ; i < limit ; i++) {
        fprintf(stderr, "%d%s", list[i], i == (limit - 1) ? "]\n" : ", ");
    }
}

int main (int argc, char **argv) {

    test_title("GET_CHILD_PIDS syscall");

    size_t limit = 3;
    size_t nr_children;
    pid_t pid_list[limit]; // to store our results

    fatal_not_test("syscall is implemented", (int) get_child_pids(pid_list, limit, &nr_children), ENOSYS);

    int child_pid = fork();
    if (child_pid < 0)
        err(child_pid, "Fork failed\n");
    else if (child_pid > 0) {
        // CASE : Arbitrary address for num_children
        not_test("Invalid num_children address (47424742)", (int) get_child_pids(pid_list, limit, (size_t *)47424742), 0);

        // CASE : NULL pid_list, non initialized
        not_test("Invalid pid_list (NULL)", (int) get_child_pids(NULL, limit, &nr_children), 0);

        // CASE : Normal execution, num_children < limit
        test("Testing for nr_children = 1, limit = 3", (int) get_child_pids(pid_list, limit, &nr_children), 0);
        print_list(pid_list, (nr_children <= limit) ? nr_children : limit);

        wait(NULL);
    }
    return 0;
}
