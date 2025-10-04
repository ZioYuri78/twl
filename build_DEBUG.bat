@echo off

if not exist "%~dp0obj\" mkdir "%~dp0obj\"
if not exist "%~dp0bin\" mkdir "%~dp0bin\debug\"

setlocal
set SOURCE=%1
set LIBRARIES=%2
set INCLUDES=.\include\
set STD=c11
set OBJ=.\obj\
set BIN=.\bin\debug\

cl /D_DEBUG /Fe%BIN% /Fo%OBJ% /std:%STD% /I%INCLUDES% /MDd /Zi %SOURCE% /link /LIBPATH:.\lib\ %LIBRARIES% 
