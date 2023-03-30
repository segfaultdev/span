#!/usr/bin/sh

gcc -Iinclude $(find . -name "*.c") -o test -Ofast -s
