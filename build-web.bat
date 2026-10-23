@echo off
set BUILD_DIR=dist\web

call emcmake cmake -G "MinGW Makefiles" -S . -B %BUILD_DIR%
call emmake cmake --build %BUILD_DIR%