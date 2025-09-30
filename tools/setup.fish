#!/bin/fish
# Run script from ../

if [ ! -f "build" ]; then
	clang -o build build.c
end

alias kai ./bin/kai
