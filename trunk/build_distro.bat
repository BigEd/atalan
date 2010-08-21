if exist atalan rmdir /S /Q atalan

mkdir atalan
mkdir atalan\bin
mkdir atalan\doc
mkdir atalan\tools
mkdir atalan\examples

REM Bin

cd src
call build.bat release
cd ..

copy src\Release\atalan.exe atalan\bin
copy src\system.atl atalan\bin
copy src\p_6502.atl atalan\bin
copy src\atari.asm atalan\bin
copy src\atari.atl atalan\bin
copy src\mads.exe atalan\bin

copy tools\Atalan.ini atalan\tools

docgen.rb

copy examples\*.* atalan\examples /Y

echo \download\ > exclude.list
xcopy www atalan\doc /E /I /Q /EXCLUDE:exclude.list
del exclude.list

copy doc\contents.txt atalan\
copy license.txt atalan\

Rem Build examples

cd atalan\bin
atalan ..\examples\hello_world

atalan ..\examples\loop

atalan ..\examples\unit_test

atalan ..\examples\hello_font

atalan ..\examples\count_2sec

atalan ..\examples\rainbow

atalan ..\examples\stars

atalan ..\examples\esieve

atalan ..\examples\tetris

atalan ..\examples\interrupts

cd ..
cd ..


if exist www\download\atalan.zip del www\download\atalan.zip
if exist www\download\atalan_src.zip del www\download\atalan_src.zip
 
zip -q -r www\download\atalan.zip atalan\

cd src
call cleanup.bat
cd ..

zip -q -r www\download\atalan_src.zip src\

copy atalan\examples\*.* www\examples /Y

pause
