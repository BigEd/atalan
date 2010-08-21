REM =======================================================
REM Delete all temporary files generated during compilation
REM =======================================================


REM _Output folder contains only the temporary files

if exist Debug rmdir /S /Q Debug
if exist Release rmdir /S /Q Release
if exist ipch rmdir /S /Q ipch
del /q *.ncb *.opt *.plg *.sdf
del /q /AH *.suo
del /q output.asm output.xex

