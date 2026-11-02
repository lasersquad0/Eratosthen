@rem @set OPT10=-t 10 -g optimum bindiffvar 1T 100G
@rem @set OPT1=-g optimum bindiffvar 2T 10G

@set START=1p
@set LEN=20G
@set FORMAT=bindiffvar
@set EXT=diffvar.bin

Eratosthen.exe -t 10 -g simple  %FORMAT% %START% %LEN% >simple.log
Eratosthen.exe -t 10 -g optimum %FORMAT% %START% %LEN% >optimum.log
Eratosthen.exe -t 10 -g 6n1     %FORMAT% %START% %LEN% >6n1.log
Eratosthen.exe -t 10 -g 30nx    %FORMAT% %START% %LEN% >30nx.log

@rem Eratosthen.exe -g simple  %FORMAT% %START% %LEN% >simple.log
@rem Eratosthen.exe -g optimum %FORMAT% %START% %LEN% >optimum.log
@rem Eratosthen.exe -g 6n1     %FORMAT% %START% %LEN% >6n1.log
@rem Eratosthen.exe -g 30nx    %FORMAT% %START% %LEN% >30nx.log

@set NAME=primes - 

comp /M "%NAME%%START%-%LEN%.simple.%EXT%" "%NAME%%START%-%LEN%.optimum.%EXT%"
comp /M "%NAME%%START%-%LEN%.simple.%EXT%" "%NAME%%START%-%LEN%.6n1.%EXT%"
comp /M "%NAME%%START%-%LEN%.simple.%EXT%" "%NAME%%START%-%LEN%.30nx.%EXT%"

