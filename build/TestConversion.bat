@goto start

----------------------------------------------------------------------------------------
This file tests file conversions 'to' and 'back'.
For example conversion from txt to bindiff and then conversion from bindiff to txt.
After two conversions it comnpares original file with resulting file. They must be equal.
All possible conversion pairs are tested here.
-----------------------------------------------------------------------------------------

:start
@echo off
setlocal EnableDelayedExpansion

set FILEFORMAT=txtdiff
set FILEEXT=diff.txt
set FILE=primes - 1G-100M.%FILEEXT%

set FOLDER=tmp
set OPT=-o %FOLDER%

copy /Y ..\x64\Release\Eratosthen.exe .

set FORMAT[0]=txt
set FORMAT[1]=txtdiff
set FORMAT[2]=bin
set FORMAT[3]=bindiff
set FORMAT[4]=bindiffvar

set EXT[0]=txt
set EXT[1]=diff.txt
set EXT[2]=bin
set EXT[3]=diff.bin
set EXT[4]=diffvar.bin

for /L %%i in (0,1,4) do (
   set FORMAT=!FORMAT[%%i]!
   set EXT=!EXT[%%i]!
  
   if not "!FORMAT!"== "%FILEFORMAT%" (
  
      Eratosthen.exe %OPT% -c %FILEFORMAT%-!FORMAT! "%FILE%"
      Eratosthen.exe %OPT% -c !FORMAT!-%FILEFORMAT% "%FOLDER%\%FILE%.!EXT!"
   
      comp /M "%FILE%" "%FOLDER%\%FILE%.!EXT!.%FILEEXT%"
      if errorlevel 1 echo ******** FILES ARE NOT EQUAL *******
   
   )
)
