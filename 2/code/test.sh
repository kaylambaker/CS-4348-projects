#!/bin/bash

RED="\033[0;31m"
GREEN="\033[0;32m"
RESET="\033[0m"
output=test/output
testdir=test/

make all
res=$?
if [[ $res -ne 0 ]]
then
  exit $res
fi

for file in test/*
do
  if [[ $file != $output ]]
  then
  ./computer < $file > $output && echo -e "\n${GREEN}$file printer.out${RESET}" && cat printer.out && echo -e "${RED}end $file printer.out${RESET}"
  fi
done
