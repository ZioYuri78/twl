@echo off
setlocal enabledelayedexpansion

set INCLUDES=.\include\
set STD=c11
set OBJ=.\obj\
set DLL=.\lib\release\twl_api.dll
set BIN=.\bin\release\

set FILES=

for %%f in (src\twl_*.c) do (
	set FILES=!FILES! %%f
)

cl /Fe%DLL% /Fo%OBJ% /std:%STD% /I%INCLUDES% /LD /MD %FILES%

xcopy /Y /Q ".\lib\release\twl_api.dll" %BIN%
