#pragma once

#include <string>


#define Pre_Loaded_Primes_Filename "primes - 0-1G.diffvar.bin"
#define DEFAULT_G_THREADS 1     // default number of threads for generating primes (option -g)
#define DEFAULT_V_THREADS 4     // default number of threads for primes verify (option -v)
#define DEFAULT_LIMIT_PRIMES 0  // 0 means 'unlimited'
#define DEFAULT_OFFSET_PRIMES 0

class Parameters
{
public:
	static uint8_t THREADS;
	static std::string PRIMES_FILENAME;
	static bool SAVEFILE;
	static uint64_t LIMIT_PRIMES;
	static uint64_t OFFSET_PRIMES;
	static std::string OUTPUT_FOLDER;
};




