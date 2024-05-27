:: I used the raylib installer https://www.raylib.com/ by clicking the "Download Now" button, which on Windows downloads the installer.
:: The installer created C:\raylib that is referenced below.
set RAYLIB_INCLUDE_DIR=C:\raylib\raylib\src
set RAYLIB_LIB_DIR=C:\raylib\raylib\src
set RAYLIB_RES_FILE=C:\raylib\raylib\src\raylib.rc.data
set COMPILER_DIR=C:\raylib\w64devkit\bin
:: I downloaded python 3.11 from the site: https://www.python.org/downloads/
:: The installer places the data in C:\Users\your_user_name\AppData  (you will need to set "LOCAL_DATA" to C:\Users\your_user_name\AppData\Local in window's environment variables)
set PYTHON_DIR=%LOCAL_DATA%\Programs\Python\Python311\include
set PYTHON_RUNTIME=-L%LOCAL_DATA%\Programs\Python\Python311\vcruntime140.dll
set PYTHON_LIB=%LOCAL_DATA%\Programs\Python\Python311\libs\python311.lib
set PATH=%PATH%;%LOCAL_DATA%\Programs\Python\Python311\include;%COMPILER_DIR%

set OPT_LEVEL=-g

set SETTINGS=-std=c99 -Wall -static -lraylib -lopengl32 -lgdi32 -lwinmm
rem -mwindows
set RAYLIB_SETTINGS=%RAYLIB_RES_FILE% -L%RAYLIB_LIB_DIR% -I%RAYLIB_INCLUDE_DIR%
set PYTHON_SETTINGS=-I%PYTHON_DIR% %PYTHON_LIB%

gcc -o gui_app.exe gui_app.c %OPT_LEVEL% %SETTINGS% %RAYLIB_SETTINGS% %PYTHON_SETTINGS%
