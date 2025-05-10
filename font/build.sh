#!/bin/bash

echo "#ifndef __FONT_H__" > font.h
echo "#define __FONT_H__" >> font.h
echo >> font.h
echo "#include <span.h>" >> font.h
echo >> font.h

rm font_*.c

gcc png_to_font.c -I include -o png_to_font -lm
find . -maxdepth 1 -name "*.png" -type f -exec ./png_to_font {} \; >> font.h

echo >> font.h
echo "#endif" >> font.h
