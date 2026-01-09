@echo off
setlocal
cd legacy_lib
python tools\gen_api.py tools\struct tools\xml generated || exit /b %ERRORLEVEL%
make clean MODE=vxworks || exit /b %ERRORLEVEL%
make MODE=vxworks || exit /b %ERRORLEVEL%
cd ..
endlocal
