#!/bin/bash

# This script generates a file with n variables and prints them
# in a new lox file. The variables are named a1, a2, ..., an
# and are assigned the values 1, 2, ..., n respectively.
# the goal is to test long instruction sets

# Output file
output_file="examples/variables.lox"

# Clear the file if it exists
> "$output_file"

n=1000

# Loop to generate n variables
for i in $(seq 1 $n); do
  echo "var a$i = $i;" >> "$output_file"
done
for i in $(seq 1 $n); do
  echo "var s$i = \"string $i\";" >> "$output_file"
done
for i in $(seq 2 $n); do
  echo "print a$i + a$((i-1));" >> "$output_file"
done
for i in $(seq 2 $n); do
  echo "print s$i + \" \" + s$((i-1));" >> "$output_file"
done

echo "Variables written to $output_file"