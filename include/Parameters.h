#pragma once

#include <string>


#define Pre_Loaded_Primes_Filename "primes - 0-1G.diffvar.bin"
#define DEFAULT_THREADS 1
#define DEFAULT_LIMIT_PRIMES 0
#define DEFAULT_OFFSET_PRIMES 0
//#define APP_NAME "Eratosthen"
//#define APP_EXE_NAME APP_NAME ".exe"
#define PERCENT_FACTOR 100

class Parameters
{
public:
	static uint8_t THREADS;
	static std::string PRIMES_FILENAME;
	static bool SAVEFILE;
	static uint64_t LIMIT_PRIMES;
	static uint64_t OFFSET_PRIMES;
};




