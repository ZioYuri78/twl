@echo off
call build_DLL_RELEASE.bat
echo.
call build_RELEASE.bat src\api_example.c
echo.
call build_RELEASE.bat src\eventsub_example.c
echo.
call build_RELEASE.bat src\multiple_user_example.c
echo.
