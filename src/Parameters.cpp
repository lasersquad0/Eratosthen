#include "Parameters.h"

uint8_t Parameters::THREADS = DEFAULT_G_THREADS; // means nothing here, default value is re-assigned in main()
std::string Parameters::PRIMES_FILENAME = Pre_Loaded_Primes_Filename;
bool Parameters::SAVEFILE = true;
uint64_t Parameters::LIMIT_PRIMES = DEFAULT_LIMIT_PRIMES;
uint64_t Parameters::OFFSET_PRIMES = DEFAULT_OFFSET_PRIMES;
std::string Parameters::OUTPUT_FOLDER;
