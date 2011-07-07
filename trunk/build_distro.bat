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
cd ..

echo \svn\ > exclude.list

xcopy src\atalan\platform atalan\platform /E /I /Q /EXCLUDE:exclude.list
xcopy src\atalan\cpu atalan\cpu /E /I /Q /EXCLUDE:exclude.list
xcopy src\atalan\module atalan\module /E /I /Q /EXCLUDE:exclude.list

REM Bin

cd src\atalan
call build.bat release
cd ..\..

cd src\con6502
call build.bat release
cd ..\..

copy src\atalan\Release\atalan.exe atalan\bin
copy bin\con6502.exe atalan\bin
REM copy src\atalan\mads.exe atalan\bin

copy tools\Atalan.ini atalan\tools

REM Doc generating

if exist www rmdir /S /Q www
mkdir www
cd www
mkdir template
mkdir raster
mkdir examples
mkdir download
mkdir projects
cd ..
xcopy www_src\template www\template /E /I /Q /EXCLUDE:exclude.list
xcopy www_src\raster www\raster /E /I /Q /EXCLUDE:exclude.list
xcopy projects www\projects /E /I /Q /EXCLUDE:exclude.list

docgen.rb

copy examples\*.* atalan\examples /Y

del exclude.list

echo \download\ > exclude.list

xcopy www atalan\doc /E /I /Q /EXCLUDE:exclude.list

del exclude.list

copy doc\index.txt atalan\about.txt
copy license.txt atalan\

Rem Build examples

cd atalan\examples

..\bin\atalan hello_world

..\bin\atalan loop

..\bin\atalan hello_font

..\bin\atalan count_2sec

..\bin\atalan rainbow

..\bin\atalan stars

..\bin\atalan esieve

..\bin\atalan tetris

..\bin\atalan interrupts

..\bin\atalan bit_sieve

..\bin\atalan unit_test

..\bin\atalan PMG

cd ..
cd ..


REM if exist www\download\atalan.zip del www\download\atalan.zip
REM if exist www\download\atalan_src.zip del www\download\atalan_src.zip
 
zip -q -r www\download\atalan.zip atalan\

cd src
call cleanup.bat
cd ..

zip -q -r www\download\atalan_src.zip src\

copy atalan\examples\*.* www\examples /Y

pause
