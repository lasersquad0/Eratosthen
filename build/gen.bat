@echo off
setlocal EnableDelayedExpansion

set START=1g
set LEN=100M
set FILE=primes - %START%-%LEN%

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

   echo Using Format: !FORMAT!

   Eratosthen.exe -s !FORMAT!  %START% %LEN% >devnull
   Eratosthen.exe -o !FORMAT!  %START% %LEN% >devnull
   Eratosthen.exe -6 !FORMAT!  %START% %LEN% >devnull
   Eratosthen.exe -30 !FORMAT! %START% %LEN% >devnull

   set FILE1="%FILE%.simple.!EXT!"
   set FILE2="%FILE%.opt.!EXT!"  
   set FILE3="%FILE%.6n1.!EXT!"  
   set FILE4="%FILE%.n30x.!EXT!"  

   echo -
   echo -
   
   comp /M !FILE1! !FILE2! 
   if errorlevel 1 echo ******** FILES !FILE1! and !FILE2! ARE NOT EQUAL *******
rem   if errorlevel 0 del !FILE2!

   comp /M !FILE1! !FILE3!
   if errorlevel 1 echo ******** FILES !FILE1! and !FILE3! ARE NOT EQUAL *******
   
   comp /M !FILE1! !FILE4!
   if errorlevel 1 echo ******** FILES !FILE1! and !FILE4! ARE NOT EQUAL *******
   

)

exit /b 1

Eratosthen.exe -s %FORMAT% 0b %LEN%
Eratosthen.exe -s %FORMAT% 1b %LEN%
Eratosthen.exe -s %FORMAT% 2b %LEN%
Eratosthen.exe -s %FORMAT% 3b %LEN%
Eratosthen.exe -s %FORMAT% 4b %LEN%
Eratosthen.exe -s %FORMAT% 5b %LEN%
Eratosthen.exe -s %FORMAT% 6b %LEN%
Eratosthen.exe -s %FORMAT% 7b %LEN%
Eratosthen.exe -s %FORMAT% 8b %LEN%

Eratosthen.exe -o %FORMAT% 0b %LEN%
Eratosthen.exe -o %FORMAT% 1b %LEN%
Eratosthen.exe -o %FORMAT% 2b %LEN%
Eratosthen.exe -o %FORMAT% 3b %LEN%
Eratosthen.exe -o %FORMAT% 4b %LEN%
Eratosthen.exe -o %FORMAT% 5b %LEN%
Eratosthen.exe -o %FORMAT% 6b %LEN%
Eratosthen.exe -o %FORMAT% 7b %LEN%
Eratosthen.exe -o %FORMAT% 8b %LEN%

Eratosthen.exe -6 %FORMAT% 0b %LEN%
Eratosthen.exe -6 %FORMAT% 1b %LEN%
Eratosthen.exe -6 %FORMAT% 2b %LEN%
Eratosthen.exe -6 %FORMAT% 3b %LEN%
Eratosthen.exe -6 %FORMAT% 4b %LEN%
Eratosthen.exe -6 %FORMAT% 5b %LEN%
Eratosthen.exe -6 %FORMAT% 6b %LEN%
Eratosthen.exe -6 %FORMAT% 7b %LEN%
Eratosthen.exe -6 %FORMAT% 8b %LEN%

Eratosthen.exe -30 %FORMAT% 0b %LEN%
Eratosthen.exe -30 %FORMAT% 1b %LEN%
Eratosthen.exe -30 %FORMAT% 2b %LEN%
Eratosthen.exe -30 %FORMAT% 3b %LEN%
Eratosthen.exe -30 %FORMAT% 4b %LEN%
Eratosthen.exe -30 %FORMAT% 5b %LEN%
Eratosthen.exe -30 %FORMAT% 6b %LEN%
Eratosthen.exe -30 %FORMAT% 7b %LEN%
Eratosthen.exe -30 %FORMAT% 8b %LEN%

@set FILE1=primes - 0B-1000B.6n1.%EXT%
@set FILE2=primes - 0B-1000B.opt.%EXT%
comp /M  "%FILE1%" "%FILE2%"
@if errorlevel 1 echo ******** FILES '%FILE1%' and '%FILE2%' ARE NOT EQUAL ********

@set FILE1="primes - 0B-1000B.opt.%EXT%" 
@set FILE2="primes - 0B-1000B.simple.%EXT%" 
comp /M  %FILE1% %FILE2%
@if errorlevel 1 echo ******** FILES '%FILE1%' and '%FILE2%' ARE NOT EQUAL ********


@set FILE1="primes - 1B-1000B.6n1.%EXT%" 
@set FILE2="primes - 1B-1000B.opt.%EXT%" 
comp /M  %FILE1% %FILE2%
@if errorlevel 1 echo ******** FILES '%FILE1%' and '%FILE2%' ARE NOT EQUAL ********

@set FILE1="primes - 1B-1000B.opt.%EXT%" 
@set FILE2="primes - 1B-1000B.simple.%EXT%" 
comp /M  %FILE1% %FILE2%
@if errorlevel 1 echo ******** FILES '%FILE1%' and '%FILE2%' ARE NOT EQUAL ********


@set FILE1="primes - 3B-1000B.6n1.%EXT%" 
@set FILE2="primes - 3B-1000B.opt.%EXT%" 
comp /M  %FILE1% %FILE2%
@if errorlevel 1 echo ******** FILES '%FILE1%' and '%FILE2%' ARE NOT EQUAL ********

@set FILE1="primes - 3B-1000B.opt.%EXT%" 
@set FILE2="primes - 3B-1000B.simple.%EXT%" 
comp /M  %FILE1% %FILE2%
@if errorlevel 1 echo ******** FILES '%FILE1%' and '%FILE2%' ARE NOT EQUAL ********


@set FILE1="primes - 4B-1000B.6n1.%EXT%" 
@set FILE2="primes - 4B-1000B.opt.%EXT%" 
comp /M  %FILE1% %FILE2%
@if errorlevel 1 echo ******** FILES '%FILE1%' and '%FILE2%' ARE NOT EQUAL ********

@set FILE1="primes - 4B-1000B.opt.%EXT%" 
@set FILE2="primes - 4B-1000B.simple.%EXT%" 
comp /M  %FILE1% %FILE2%
@if errorlevel 1 echo ******** FILES '%FILE1%' and '%FILE2%' ARE NOT EQUAL ********


@set FILE1="primes - 6B-1000B.6n1.%EXT%" 
@set FILE2="primes - 6B-1000B.opt.%EXT%" 
comp /M  %FILE1% %FILE2%
@if errorlevel 1 echo ******** FILES '%FILE1%' and '%FILE2%' ARE NOT EQUAL ********

@set FILE1="primes - 6B-1000B.opt.%EXT%" 
@set FILE2="primes - 6B-1000B.simple.%EXT%" 
comp /M  %FILE1% %FILE2%
@if errorlevel 1 echo ******** FILES '%FILE1%' and '%FILE2%' ARE NOT EQUAL ********


@set FILE1="primes - 8B-1000B.6n1.%EXT%" 
@set FILE2="primes - 8B-1000B.opt.%EXT%" 
comp /M  %FILE1% %FILE2%
@if errorlevel 1 echo ******** FILES '%FILE1%' and '%FILE2%' ARE NOT EQUAL ********

@set FILE1="primes - 8B-1000B.opt.%EXT%" 
@set FILE2="primes - 8B-1000B.simple.%EXT%" 
comp /M  %FILE1% %FILE2%
@if errorlevel 1 echo ******** FILES '%FILE1%' and '%FILE2%' ARE NOT EQUAL ********


