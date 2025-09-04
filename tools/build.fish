#!/bin/bash
# Run script from ../

alias kai ./bin/kai

if [ ! -f "build" ]; then
	clang -o build build.c
end

./build
