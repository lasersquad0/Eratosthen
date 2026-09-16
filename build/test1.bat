@set TESTSFOLDER=tests

@mkdir %TESTSFOLDER%

copy /Y ..\x64\Debug\Eratosthen.exe %TESTSFOLDER%


@set OPT=txtdiff 0 1m -p "PRE-CALC PRIMES.diffvar.bin"

rem remporarily go to tests folder to run eratosthen.exe there
cd %TESTSFOLDER%

echo Parameters: %OPT%

Eratosthen.exe -s %OPT% >devnulls
Eratosthen.exe -o %OPT% >devnullo
Eratosthen.exe -6 %OPT% >devnull6
Eratosthen.exe -30 %OPT% >devnull30

cd ..



