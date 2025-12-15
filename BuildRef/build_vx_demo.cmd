@echo off
setlocal
make -C demo_app clean MODE=vxworks || exit /b %ERRORLEVEL%
make -C demo_app MODE=vxworks || exit /b %ERRORLEVEL%
endlocal
