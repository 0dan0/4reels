@echo off
setlocal enabledelayedexpansion

set "latestFile="
set "latestDateTime="

rem Loop through all .rbn files
for %%F in (*.rbn) do (
     rem Get file timestamp as string (e.g., "03/23/2025  01:18 PM")
    set "file=%%F"
    for /f "tokens=1-3" %%a in ("%%~tF") do (
        set "date=%%a"
        set "time=%%b"
        set "ampm=%%c"
    )

    rem Extract date parts
    set "mm=!date:~0,2!"
    set "dd=!date:~3,2!"
    set "yyyy=!date:~6,4!"

    rem Extract time parts
    set "hh=!time:~0,2!"
    set "mi=!time:~3,2!"

    rem Remove leading zero
    set /a hour=1!hh!-100
	
	rem Convert to 24-hour
    if /i "!ampm!"=="PM" if !hour! LSS 12 set /a hour+=12
    if /i "!ampm!"=="AM" if !hour! EQU 12 set /a hour=0

    rem Pad hour
    if !hour! LSS 10 set "hour=0!hour!"

    rem Construct sortable timestamp
    set "timestamp=!yyyy!!mm!!dd!!hour!!mi!"
	
	rem Compare with latest timestamp
	if "!latestDateTime!"=="" (
		set "latestDateTime=!timestamp!"
		set "latestFile=%%F"
	) else if "!timestamp!" GTR "!latestDateTime!" (
		set "latestDateTime=!timestamp!"
		set "latestFile=%%F"
		echo "!latestFile! has !timestamp! > !latestDateTime!"
	)
)


set "bclFile=%latestFile:~0,-4%.bcl"
    
ntkcalcVS.exe -cw "!latestFile!"
bfc4ntkVS.exe -c "!latestFile!" "!bclFile!"
ntkcalcVS.exe -cw "!bclFile!"

copy /Y "!bclFile!" "FWDV280.BIN"

endlocal

