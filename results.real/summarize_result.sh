#!/bin/bash

echo -n "$1,"
cat $1 | cut -d':' -f2 | tr -dc '0-9\n' | tr '\n' ',' | rev | cut -c2- | rev
