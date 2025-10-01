@echo off
set SOURCE=%1
set LIBRARIES=%2
set INCLUDES=.\include\
set STD=c11
set OBJ=.\obj\
set BIN=.\bin\debug\
set PDB=.\pdb\

cl /D_DEBUG /Fe%BIN% /Fo%OBJ% /std:%STD% /I%INCLUDES% /MDd /Zi %SOURCE% /link /LIBPATH:.\lib\ %LIBRARIES% 
