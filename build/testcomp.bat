goto :start
----------------------------------
Test all primes generating algorithms (simple, optimised, 6n1, n30x) and all output formats with a specified range (start-length).
This test runs Eratosthen.exe for each pair algorithm-output_format and compares output files. 
All algorithms must generate equal files for each output format.
----------------------------------

:start
@echo off
setlocal EnableDelayedExpansion

rem set START=0
@set STARTS=0b 1b 2b 3b 4b 5b 6b 7b 8b 9b 1001b 1K 100K 100001b 100002b
set LEN=1m
set TESTSFOLDER=tests

rem Also test custom name for files with precalculated prime numbers
set OPT_P=-p "PRE-CALC PRIMES.diffvar.bin"

mkdir %TESTSFOLDER%

copy /Y ..\x64\Debug\Eratosthen.exe %TESTSFOLDER%

rem remporarily go to tests folder to run eratosthen.exe there
cd %TESTSFOLDER%


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
   echo Generating primes using params: %%k %LEN% %OPT_P%

   Eratosthen.exe -s !FORMAT!  %%k %LEN% %OPT_P% >>devnull
   if errorlevel 1 echo ******** Eratosthen.exe exited with ERROR *******
   Eratosthen.exe -o !FORMAT!  %%k %LEN% %OPT_P% >>devnull
   if errorlevel 1 echo ******** Eratosthen.exe exited with ERROR *******
   Eratosthen.exe -6 !FORMAT!  %%k %LEN% %OPT_P% >>devnull
   if errorlevel 1 echo ******** Eratosthen.exe exited with ERROR *******
   Eratosthen.exe -30 !FORMAT! %%k %LEN% %OPT_P% >>devnull
   if errorlevel 1 echo ******** Eratosthen.exe exited with ERROR *******

   set FILE1="!FILE!.simple.!EXT!"
   set FILE2="!FILE!.opt.!EXT!"  
   set FILE3="!FILE!.6n1.!EXT!"  
   set FILE4="!FILE!.n30x.!EXT!"  

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

rem return back to the main folder (where we started)
cd ..



