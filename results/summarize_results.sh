#!/bin/bash

echo 'Filename,Nodes,Width,Height,Functions,Max Function,Total,CPU Setup,GPU Total,GPU Upload,GPU Upload Create,Preprocessing,Instruction Count,Instruction Gen,Optimize,Regalloc,Jump Fix,Postprocess'
find . -iname *.out -exec ./summarize_result.sh {} \; | sort -V
