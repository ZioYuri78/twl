@echo off

if not exist "%~dp0obj\" mkdir "%~dp0obj\"
if not exist "%~dp0bin\" mkdir "%~dp0bin\release\"

setlocal
set SOURCE=%1
set LIBRARIES=%2
set INCLUDES=.\include\
set STD=c11
set OBJ=.\obj\
set BIN=.\bin\release\

cl /Fe%BIN% /Fo%OBJ% /std:%STD% /I%INCLUDES% /MD /O2 %SOURCE% /link %LIBRARIES%

