@echo off
call build_DLL_DEBUG.bat
echo.
call build_DEBUG.bat src\api_example.c
echo.
call build_DEBUG.bat src\eventsub_example.c
echo.
call build_DEBUG.bat src\multiple_user_example.c
echo.

