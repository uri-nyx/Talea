#!/bin/bash

LCC="$HOME/Akai/lcc-sirius/lcc/bin/lcc"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

for file in test/*.sas; do
  base_name=$(basename "$file" .sas)
  
  ./sas "$file" "test/$base_name.sas.out" > /dev/null

  "$LCC" -h -o "test/$base_name.as.out" "test/$base_name.s" 2>/dev/null
  
  # Compare the binary outputs
  if diff "test/$base_name.sas.out" "test/$base_name.as.out" >/dev/null; then
    echo -e "${GREEN}Success:${NC} $base_name"
  else
    echo -e "${RED}Failure:${NC} $base_name"
  fi
done

rm test/*.out