# BGS
Chess engine

BGS stands for Botez Gambit Simulator, or Best Game Search, or maybe Big Giant Search

## Windows install
- Install raylib from https://www.raylib.com/ by clicking the "Download Now" button, which on Windows downloads the installer. The installer creats C:\raylib, which is referenced in `gui_build.bat`
- Download python 3.11 from the site: https://www.python.org/downloads/
- The installer places the data in C:\Users\your_user_name\AppData  (you will need to set "LOCAL_DATA" to C:\Users\your_user_name\AppData\Local in window's environment variables)
- Add LOCAL_DATA as an environment variable and set it to C:\Users\your_username\AppData\Local (this is so that windows can find Python)
## Building on Windows
- I've only tested building the GUI by using raylib's dev-console, which can be found at C:\raylib\w64devkit\w64devkit.exe
- The console is a unix-like console with gcc available. If you are able to get gcc working without w64devkit.exe, then that could be an option.
- Just run `gui_build.bat` and then run `gui_app.exe`

## Mac install
- raylib.a and raylib.h are stored in this repo, so for mac builds we just link off of those. 
- Python 3.11 was installed using `brew`, which is why the path to `python3.11-config` is in `/usr/local/Cellar` (we can also find other ways to reference python at some point, but for now we at least have homebrew.)
## Building on Mac
- just run `./gui_build.sh` and then run the app `./gui_app.exe`

## Running Tests
Execute `test.py` to run tests
``` sh
python3 test.py
```
or run with the verbose option to see more output
``` sh
python3 test.py -v
python3 test.py --verbose
```
