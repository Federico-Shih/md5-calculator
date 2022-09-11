#!/usr/bin/env bash

if [[ $1 == "analyze" ]]
then
  set -e
  make clean
  pvs-studio-analyzer trace -- make
	pvs-studio-analyzer analyze
	plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log
elif [[ $1 == "clean-analyze" ]]
then
  rm strace_out report.tasks PVS-Studio.log
elif [[ $1 == "clean" ]]
then
  make clean
elif [[ $1 == "run" && -n $2 ]]
then
  ./app "${@:2}" | ./view
elif [[ $1 == "compile" ]]
then
  make all
elif [[ $1 == "compile-run" ]]
then
  make all
  ./app "${@:2}" | ./view
fi