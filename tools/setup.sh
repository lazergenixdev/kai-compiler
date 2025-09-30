#!/bin/bash
# Run script from ../

if [ ! -f "build" ]; then
	clang -o build build.c
fi

alias kai=./bin/kai
