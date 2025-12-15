@echo off
setlocal
mingw32-make -C demo_app -f Makefile.windows clean || exit /b %ERRORLEVEL%
mingw32-make -C demo_app -f Makefile.windows || exit /b %ERRORLEVEL%
endlocal
