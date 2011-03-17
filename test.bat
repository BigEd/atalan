%echo off
REM Perform all tests in tests folder
cd tests
FOR %%T IN (*.atl) DO call test %%T
cd ..
