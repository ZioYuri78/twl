@echo off

setlocal enabledelayedexpansion
set WD=%~dp0

if not exist "%WD%obj\" mkdir "%WD%obj\"
if not exist "%WD%lib\" mkdir "%WD%lib\debug\"
if not exist "%WD%bin\" mkdir "%WD%bin\debug\"

set INCLUDES=%WD%include\
set STD=c11
set OBJ=%WD%obj\
set DLL=%WD%lib\debug\twl_api_D.dll
set BIN=%WD%bin\debug\

set FILES=

for %%f in (%WD%src\twl_*.c) do (
	set FILES=!FILES! %%f
)

cl /Fe%DLL% /Fo%OBJ% /std:%STD% /I%INCLUDES% /Zi /LDd /MDd %FILES%

echo.
echo Copying %WD%lib\debug\twl_api_D.dll to %BIN%
xcopy /Y /Q "%WD%lib\debug\twl_api_D.dll" %BIN% 

echo.
echo Copying %WD%lib\debug\twl_api_D.pdb to %BIN%
xcopy /Y /Q "%WD%lib\debug\twl_api_D.pdb" %BIN%

echo.
echo Copying %WD%lib\debug\twl_api_D.exp to %BIN%
xcopy /Y /Q "%WD%lib\debug\twl_api_D.exp" %BIN%

echo.
echo Copying %WD%lib\debug\twl_api_D.ilk to %BIN%
xcopy /Y /Q "%WD%lib\debug\twl_api_D.ilk" %BIN%
