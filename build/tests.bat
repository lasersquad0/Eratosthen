goto :start
----------------------------------
Test all primes generating algorithms (simple, optimised, 6n1, n30x) and all output formats with a specified range (start-length).
This test runs Eratosthen.exe for each pair algorithm-output_format and compares output files. 
All algorithms must generate equal files for each output format.
----------------------------------

:start
@echo off
setlocal EnableDelayedExpansion

set START=100m
set LEN=2m
set FILE=primes - %START%-%LEN%
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

for /L %%i in (0,1,4) do (
   set FORMAT=!FORMAT[%%i]!
   set EXT=!EXT[%%i]!
   
   echo --------------------
   
   echo Using Format: !FORMAT!

   Eratosthen.exe -s !FORMAT!  %START% %LEN% %OPT_P% >>devnull
   if errorlevel 1 echo ******** Eratosthen.exe exited with ERROR *******
   Eratosthen.exe -o !FORMAT!  %START% %LEN% %OPT_P% >>devnull
   if errorlevel 1 echo ******** Eratosthen.exe exited with ERROR *******
   Eratosthen.exe -6 !FORMAT!  %START% %LEN% %OPT_P% >>devnull
   if errorlevel 1 echo ******** Eratosthen.exe exited with ERROR *******
   Eratosthen.exe -30 !FORMAT! %START% %LEN% %OPT_P% >>devnull
   if errorlevel 1 echo ******** Eratosthen.exe exited with ERROR *******

   set FILE1="%FILE%.simple.!EXT!"
   set FILE2="%FILE%.opt.!EXT!"  
   set FILE3="%FILE%.6n1.!EXT!"  
   set FILE4="%FILE%.n30x.!EXT!"  

   echo Checking that output files are EQUAL
   echo -

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

rem return back to the main folder (where we started)
cd ..



