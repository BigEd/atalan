REM Build Atalan tools and prepare correct bin

cd src\atalan
call build.bat release
cd ..\..

cd src\con6502
call build.bat release
cd ..\..

cd src\conZ80
call build.bat release
cd ..\..

