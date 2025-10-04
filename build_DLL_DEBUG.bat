@echo off

if not exist "%~dp0obj\" mkdir "%~dp0obj\"
if not exist "%~dp0lib\" mkdir "%~dp0lib\debug\"
if not exist "%~dp0bin\" mkdir "%~dp0bin\debug\"

setlocal enabledelayedexpansion
set INCLUDES=.\include\
set STD=c11
set OBJ=.\obj\
set DLL=.\lib\debug\twl_api_D.dll
set BIN=.\bin\debug\

set FILES=

for %%f in (src\twl_*.c) do (
	set FILES=!FILES! %%f
)

cl /Fe%DLL% /Fo%OBJ% /std:%STD% /I%INCLUDES% /Zi /LDd /MDd %FILES%

xcopy /Y /Q ".\lib\debug\twl_api_D.dll" %BIN% 
xcopy /Y /Q ".\lib\debug\twl_api_D.pdb" %BIN%
xcopy /Y /Q ".\lib\debug\twl_api_D.exp" %BIN%
xcopy /Y /Q ".\lib\debug\twl_api_D.ilk" %BIN%
