#!/usr/bin/env bash

if [[ $1 == "analyze" ]]
then
  set -e
  make clean
  pvs-studio-analyzer trace -- make
	pvs-studio-analyzer analyze
	plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log
elif [[ $1 == "clean" ]]
then
  rm strace_out report.tasks PVS-Studio.log
fi