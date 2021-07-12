#!/bin/bash

find test -iname '*.in' -exec ./run_test.sh {} \;
