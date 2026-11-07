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

rem set FILEFORMAT=txtdiff
rem set FILEEXT=diff.txt
set FILETMPL=primes - 1G-100M
rem %FILEEXT%

set FOLDER=tmp
set OPT=-o %FOLDER%

copy /Y ..\prj\Eratosthen\x64\Release\Eratosthen.exe .

set FILE[0]=%FILETMPL%.txt
set FILE[1]=%FILETMPL%.diff.txt
set FILE[2]=%FILETMPL%.bin
set FILE[3]=%FILETMPL%.diff.bin
set FILE[4]=%FILETMPL%.diffvar.bin

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

@rem number of fails running Eratosthen.exe
set /a FAILS=0
@rem number of failed compares of resulting files
set /a FAILSC=0

for /L %%f in (0,1,4) do (
   set FROMFMT=!FORMAT[%%f]!
   set FILE=!FILE[%%f]!
   set FROMEXT=!EXT[%%f]!
  
for /L %%i in (0,1,4) do (
   set TOFMT=!FORMAT[%%i]!
   set TOEXT=!EXT[%%i]!
  
   if not "!FROMFMT!"== "!TOFMT!" (
  
      Eratosthen.exe %OPT% -c !FROMFMT!-!TOFMT! "!FILE!"
      if errorlevel 1 (
     	 echo ******** Eratosthen.exe exited with ERROR *******
       	 set /a FAILS+=1
      )

      Eratosthen.exe %OPT% -c !TOFMT!-!FROMFMT! "%FOLDER%\!FILE!.!TOEXT!"
      if errorlevel 1 (
     	 echo ******** Eratosthen.exe exited with ERROR *******
       	 set /a FAILS+=1
      )

      comp /M "!FILE!" "%FOLDER%\!FILE!.!TOEXT!.!FROMEXT!"
      if errorlevel 1 (
         set /a FAILSC+=1
	 echo ******** FILES '!FILE!' and '%FOLDER%\!FILE!.!TOEXT!.!FROMEXT!' ARE NOT EQUAL *******
      )   
   )
)
)


echo -------- RESULTS --------
echo Failed Executions: !FAILS!
echo Failed Compares: !FAILSC!
echo -------------------------

