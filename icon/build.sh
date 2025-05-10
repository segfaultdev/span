#!/bin/bash

echo "#ifndef __ICON_H__" > icon.h
echo "#define __ICON_H__" >> icon.h
echo >> icon.h
echo "#include <span.h>" >> icon.h
echo >> icon.h

rm icon_*.c

gcc png_to_icon.c -I include -o png_to_icon -lm
find . -maxdepth 1 -name "*.png" -type f -exec ./png_to_icon {} \; >> icon.h

echo >> icon.h
echo "#endif" >> icon.h
