@set OPT10=-t 10 -o bindiffvar 1T 100G
@set OPT1=-o bindiffvar 2T 10G

@rem Eratosthen.exe %OPT10% >runcritsec1.log
@rem Eratosthen.exe %OPT10% >runcritsec2.log
@rem Eratosthen.exe %OPT10% >runcritsec3.log

Eratosthen.exe %OPT1% >runmutex1.log
Eratosthen.exe %OPT1% >runmutex2.log
Eratosthen.exe %OPT1% >runmutex3.log

@rem comp /M "%IN_FILE%" "%IN_FILE%.diffvar.bin.txt"