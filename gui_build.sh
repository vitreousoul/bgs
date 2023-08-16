#!/usr/bin/env sh

FRAMEWORKS="-framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL"
SETTINGS="-std=c99 -Wall -Wextra -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations"
SOURCE_FILE="gui_app.c"
PYTHON_LIBS=`python3-config --cflags --ldflags --embed`
RAY_LIB="clibs/libraylib.a"

gcc $FRAMEWORKS $RAY_LIB $PYTHON_LIBS $SETTINGS $SOURCE_FILE -o gui_app.exe
