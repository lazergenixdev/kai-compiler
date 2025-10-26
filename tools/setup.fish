#!/bin/fish

# Assume ran from `tools/` if `build.c` is not found 
if [ ! -f "build.c" ]; then
	cd ../
end

if [ ! -f "build" ]; then
	clang -o build build.c
end

alias kai ./bin/kai
alias build ./build
alias newbuild ./newbuild # TODO: remove when new build is complete
