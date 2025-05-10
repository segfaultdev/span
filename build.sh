#!/bin/bash

cd font
	bash build.sh
cd ..

cd icon
	bash build.sh
cd ..

gcc demo.c span/span.c font/font_*.c icon/icon_*.c -I include -o demo -lSDL2 -Os -s
