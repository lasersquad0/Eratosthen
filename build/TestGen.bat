goto :start

----------------------------------
Tests all primes generating algorithms (simple, optimum, 6n1, 30nx) together with all output formats.
with a specified range (start-length).
This test runs Eratosthen.exe for each pair algorithm-output_format and compares output files. 
All algorithms must generate equal files for each output format.
----------------------------------

:start
@echo off
setlocal EnableDelayedExpansion

set STARTS=0b 1b 2b 3b 4b 5b 6b 7b 8b 9b 1001b 1K 100K 100001b 100002b
set LEN=1m
set TESTSFOLDER=tests

rem Also test custom name for files with precalculated prime numbers
set OPT=-o %TESTSFOLDER% -p "PRE-CALC PRIMES.diffvar.bin"

mkdir %TESTSFOLDER%

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

for %%k in (%STARTS%) do (

set FILE=primes - %%k-%LEN%

for /L %%i in (0,1,4) do (
   set FORMAT=!FORMAT[%%i]!
   set EXT=!EXT[%%i]!
   
   echo --------------------
   
   echo Testing Format: !FORMAT!
   echo Generating primes using params: %%k %LEN% %OPT%

   Eratosthen.exe -g simple !FORMAT! %%k %LEN% %OPT% >>devnull
   if errorlevel 1 echo ******** Eratosthen.exe exited with ERROR *******
   Eratosthen.exe -g optimum !FORMAT!  %%k %LEN% %OPT% >>devnull
   if errorlevel 1 echo ******** Eratosthen.exe exited with ERROR *******
   Eratosthen.exe -g 6n1 !FORMAT!  %%k %LEN% %OPT% >>devnull
   if errorlevel 1 echo ******** Eratosthen.exe exited with ERROR *******
   Eratosthen.exe -g 30nx !FORMAT! %%k %LEN% %OPT% >>devnull
   if errorlevel 1 echo ******** Eratosthen.exe exited with ERROR *******

   set FILE1="%TESTSFOLDER%\!FILE!.simple.!EXT!"
   set FILE2="%TESTSFOLDER%\!FILE!.optimum.!EXT!"  
   set FILE3="%TESTSFOLDER%\!FILE!.6n1.!EXT!"  
   set FILE4="%TESTSFOLDER%\!FILE!.30nx.!EXT!"  

   echo Done
   echo -  
   echo Checking equality of output files

   comp /M !FILE1! !FILE2! 
   if errorlevel 1 echo ******** FILES !FILE1! and !FILE2! ARE NOT EQUAL *******
   if errorlevel 0 del !FILE2!

   comp /M !FILE1! !FILE3!
   if errorlevel 1 echo ******** FILES !FILE1! and !FILE3! ARE NOT EQUAL *******
   if errorlevel 0 del !FILE3!

   comp /M !FILE1! !FILE4!
   if errorlevel 1 echo ******** FILES !FILE1! and !FILE4! ARE NOT EQUAL *******
   if errorlevel 0 del !FILE4!   

)
)




