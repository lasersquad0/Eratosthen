#pragma once

#include <string>


#define Pre_Loaded_Primes_Filename "primes - 0-1G.diffvar.bin"
#define DEFAULT_THREADS 1
#define APP_NAME "Eratosthen"
#define APP_EXE_NAME APP_NAME ".exe"
#define PERCENT_FACTOR 100

class Parameters
{
public:
	static int THREADS;
	static std::string PRIMES_FILENAME;
	static bool SAVEFILE;
};




