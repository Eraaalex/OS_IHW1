#!/bin/bash

gcc main.c -o main

for i in {1..5}; do
  ./main "input${i}.txt" "output${i}.txt"
  echo "output${i}.txt:"
  cat "output${i}.txt"
  echo ""
done
