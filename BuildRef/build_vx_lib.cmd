@echo off
setlocal
make clean MODE=vxworks || exit /b %ERRORLEVEL%
make MODE=vxworks || exit /b %ERRORLEVEL%
endlocal
