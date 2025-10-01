@echo off
setlocal enabledelayedexpansion

set FILES=

for %%f in (src\twl_*.c) do (
	set FILES=!FILES! %%f
)

echo Files: %FILES%
