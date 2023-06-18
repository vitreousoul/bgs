#!/usr/bin/env sh

SOURCE_FILE="test.c"
SETTINGS="-std=c99 -Wall -Wextra -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations"
LIBS=`/usr/local/Cellar/python@3.11/3.11.2_1/bin/python3.11-config --cflags --ldflags --embed`

gcc -o testing $SOURCE_FILE $SETTINGS $LIBS
