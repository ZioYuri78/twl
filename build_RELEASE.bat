@echo off

setlocal
set WD=%~dp0

if not exist "%WD%obj\" mkdir "%WD%obj\"
if not exist "%WD%bin\" mkdir "%WD%bin\release\"

set SOURCE=%1
set LIBRARIES=%2
set INCLUDES=%WD%include\
set STD=c11
set OBJ=%WD%obj\
set BIN=%WD%bin\release\

cl /Fe%BIN% /Fo%OBJ% /std:%STD% /I%INCLUDES% /MD /O2 %SOURCE% /link %LIBRARIES%

