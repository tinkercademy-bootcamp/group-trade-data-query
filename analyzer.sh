#!/bin/bash

# Check if the file exists
FILE="trades-medium.csv"

if [ ! -f "$FILE" ]; then
  echo "File '$FILE' not found!"
  exit 1
fi

# Print the first 20 lines
head -n 20 "$FILE"
