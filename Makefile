EXECUTABLES = uniq_sample_test child_pids_sample_test child_pids_swap

.PHONY: build
build:	$(EXECUTABLES)

.PHONY: run
run: build
	chmod +x ./_checker.sh
	./_checker.sh

uniq_sample_test: uniq_sample_test.c syscalls.h
	gcc uniq_sample_test.c -o uniq_sample_test

child_pids_sample_test: child_pids_sample_test.c syscalls.h
	gcc child_pids_sample_test.c -o child_pids_sample_test

child_pids_swap: child_pids_swap.c child_pids_helper.h syscalls.h
	gcc child_pids_swap.c -o child_pids_swap


.PHONY: clean
clean:
	-rm $(EXECUTABLES)
