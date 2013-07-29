REM Compile and execute test specified as argument
%echo off
echo =
echo =========== %1 ================
..\atalan\bin\atalan -v0 -p con6502 %1
if not errorlevel 1 goto test
pause
goto exit
:test
echo --- Executing %1 ---
..\atalan\bin\con6502 %~n1.c65
if not errorlevel 1 goto exit
pause
:exit  