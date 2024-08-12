#!/usr/bin/env sh

DEBUG=0

GUI_APP_NAME="gui_app"

PYTHON_LIBS_VALUE=`python3-config --cflags --ldflags --embed`

TARGET_NAME="$1";

if [ -z "$1" ]; then
    TARGET_NAME="gui_app";
    PYTHON_LIBS="$PYTHON_LIBS_VALUE"
elif [ "$1" == "gui_app" ] || [ "$1" == "chess_bot" ]; then
    PYTHON_LIBS="$PYTHON_LIBS_VALUE"
else
    PYTHON_LIBS=""
fi


if [ $DEBUG -eq 0 ]; then
    echo "Optimized build";
    TARGET="-O2 -DDEBUG=0 -o $TARGET_NAME.exe"
elif [ $DEBUG -eq 1 ]; then
    echo "Debug build";
    TARGET="-g3 -O0 -DDEBUG=1 -o $TARGET_NAME.out"
fi


FRAMEWORKS="-framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL"
SETTINGS="-std=c99 -Wall -Wextra -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations"
SETTINGS="$SETTINGS -Wno-unused-function"
SETTINGS="$SETTINGS -Wno-unused-parameter"
SETTINGS="$SETTINGS -Wno-unused-variable"
SOURCE_FILE="$TARGET_NAME.c"
RAY_LIB="clibs/libraylib.a"

gcc $FRAMEWORKS $RAY_LIB $PYTHON_LIBS $SETTINGS $SOURCE_FILE $TARGET
