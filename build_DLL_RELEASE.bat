@echo off

setlocal enabledelayedexpansion
set WD=%~dp0

if not exist "%WD%obj\" mkdir "%WD%obj\"
if not exist "%WD%lib\" mkdir "%WD%lib\release\"
if not exist "%WD%bin\" mkdir "%WD%bin\release\"

set INCLUDES=%WD%include\
set STD=c11
set OBJ=%WD%obj\
set DLL=%WD%lib\release\twl_api.dll
set BIN=%WD%bin\release\

set FILES=

for %%f in (%WD%src\twl_*.c) do (
	set FILES=!FILES! %%f
)

cl /Fe%DLL% /Fo%OBJ% /std:%STD% /I%INCLUDES% /LD /MD %FILES%

echo.
echo Copying %WD%lib\release\twl_api.dll to %BIN%
xcopy /Y /Q "%WD%lib\release\twl_api.dll" %BIN%
