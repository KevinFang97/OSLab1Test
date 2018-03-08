#!/bin/bash

xecho() {
  echo "$@" | tee -a /dev/stderr
  sync
}

sudo sysctl -w kernel.watchdog_thresh=1 # force more frequent cpu lock-up checking
./uniq_sample_test
./child_pids_sample_test
sudo sysctl -w vm.swappiness=90
./child_pids_swap 2 350 100
