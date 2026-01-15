@echo off
setlocal
python legacy_lib\tools\gen_api.py legacy_lib\tools\struct legacy_lib\tools\xml legacy_lib\generated || exit /b %ERRORLEVEL%
cd legacy_lib
mingw32-make clean MODE=native || exit /b %ERRORLEVEL%
mingw32-make MODE=native || exit /b %ERRORLEVEL%
cd ..
endlocal
