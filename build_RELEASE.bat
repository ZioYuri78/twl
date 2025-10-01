@echo off
set SOURCE=%1
set LIBRARIES=%2
set INCLUDES=.\include\
set STD=c11
set OBJ=.\obj\
set BIN=.\bin\release\
set PDB=.\pdb\

cl /Fe%BIN% /Fo%OBJ% /std:%STD% /I%INCLUDES% /MD /O2 %SOURCE% /link %LIBRARIES%

