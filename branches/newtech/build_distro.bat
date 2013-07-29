REM Create release folder of Atalan

if exist atalan rmdir /S /Q atalan

mkdir atalan
cd atalan
mkdir bin
mkdir platform
mkdir cpu
mkdir module
mkdir doc
mkdir tools
mkdir examples
REM mkdir examples\atari
REM mkdir examples\nes

cd ..

REM Bin

cd src\atalan
call build.bat release
cd ..\..

cd src\con6502
call build.bat release
cd ..\..

cd src\conZ80
call build.bat release
cd ..\..

copy src\atalan\Release\atalan.exe bin
copy src\con6502\Release\con6502.exe bin
copy src\conZ80\Release\conZ80.exe bin

REM Copy modules to distribution

echo \svn\ > exclude.list
xcopy src\atalan\platform atalan\platform /E /I /Q /EXCLUDE:exclude.list
xcopy src\atalan\cpu atalan\cpu /E /I /Q /EXCLUDE:exclude.list
xcopy src\atalan\module atalan\module /E /I /Q /EXCLUDE:exclude.list
xcopy examples atalan\examples /E /I /Q /EXCLUDE:exclude.list

REM Copy bin to distribution

copy bin\con6502.exe atalan\bin
copy bin\conZ80.exe atalan\bin
copy bin\atalan.exe atalan\bin

REM Tools

xcopy tools atalan\tools /E /I /Q /EXCLUDE:exclude.list

REM Doc generating

if exist www rmdir /S /Q www
mkdir www
cd www
mkdir template
mkdir raster
mkdir examples
mkdir examples\atari
mkdir examples\nes
mkdir download
mkdir projects
cd ..
xcopy www_src\template www\template /E /I /Q /EXCLUDE:exclude.list
xcopy www_src\raster www\raster /E /I /Q /EXCLUDE:exclude.list
xcopy projects www\projects /E /I /Q /EXCLUDE:exclude.list

docgen.rb

del exclude.list

echo \download\ > exclude.list

xcopy www atalan\doc /E /I /Q /EXCLUDE:exclude.list

del exclude.list

copy doc\index.txt atalan\about.txt
copy license.txt atalan\

Rem Build examples

cd atalan\examples\atari

REM ..\..\bin\atalan hello_world

REM ..\..\bin\atalan loop

..\..\bin\atalan hello_font

..\..\bin\atalan count_2sec

..\..\bin\atalan rainbow

..\..\bin\atalan stars

..\..\bin\atalan esieve

REM ..\..\bin\atalan tetris

..\..\bin\atalan interrupts

REM ..\..\bin\atalan bit_sieve

REM ..\..\bin\atalan unit_test

..\..\bin\atalan PMG

cd ..\..\..

REM if exist www\download\atalan.zip del www\download\atalan.zip
REM if exist www\download\atalan_src.zip del www\download\atalan_src.zip
 
zip -q -r www\download\atalan.zip atalan\

cd src\atalan
call cleanup.bat
cd ..\..

cd src\con6502
call cleanup.bat
cd ..\..

zip -q -r www\download\atalan_src.zip src\

copy atalan\examples\*.* www\examples /Y

pause
