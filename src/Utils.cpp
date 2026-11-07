
#include <iostream>
#include <cassert>
#include <chrono>
#include <algorithm>

// this is to remove defines min, max in windows headers because they conflict with std::min std::max 
#define NOMINMAX
#include <windows.h> // for GetProcessMemoryInfo()

#include <psapi.h>
#include "PrimesFIO.h" // for var_len_encode and var_len_decode functions
#include "Utils.h" 


/////////////////////////////////////////////////////////
// uint64_t IsPrimeNumber30 functions
/////////////////////////////////////////////////////////

// return 0 means 'no dividers' - number is prime
// any other value is a divider of prime
uint64_t IsPrimeNumber30(uint64_t number, const uint64_t* primes, const uint64_t primesSize)
{
    uint64_t stop = (uint64_t)sqrt(number) + 1;
    
    uint64_t prime;
    size_t i = 0;
    while (i < primesSize)
    {
        prime = primes[i];
        if (prime >= stop) return 0; // number is prime
        if ((number % prime) == 0) return prime; // number is NOT prime (compound)
        ++i;
    }

    // if we are here it means that primes array is too small to determine whether number is prime.
    // continue checking with another alg
    // use 30nx alg which starts from last checked prime
    return IsPrimeNumber30(number, primes[primesSize - 1], stop);
}

uint64_t IsPrimeNumber30(uint64_t number)
{
    return IsPrimeNumber30(number, 0ull, (uint64_t)sqrt(number) + 1);
}

uint64_t IsPrimeNumber30(uint64_t number, uint64_t start, uint64_t stop)
{   
    // if number < 30 we don't care about start and stop 
    if (number < 30) return KNOWN_DIV[number];
    if (stop > number) stop = number;
    if (start > stop) start = stop;
    //assert(start <= stop);
    //assert(stop < number);

    if (start < 30)
    {
        if ((number & 1) == 0)  return 2; // right, here should be & instead of %
        if ((number % 3) == 0)  return 3;
        if ((number % 5) == 0)  return 5;
        if ((number % 7) == 0)  return 7;
        if ((number % 11) == 0) return 11;
        if ((number % 13) == 0) return 13;
        if ((number % 17) == 0) return 17;
        if ((number % 19) == 0) return 19;
        if ((number % 23) == 0) return 23;
        if ((number % 29) == 0) return 29;
    }

    uint64_t multiple30 = (start / 30) * 30;
    if (multiple30 < 30) multiple30 = 30;
    if (stop >= 30) stop -= 30; // when stop=30 we actually check 31,37...53,59, need deduct 30

    for (uint64_t i = multiple30; i <= stop; i += 30)
    {
        if (number % (i + 1) == 0) return i + 1;
        if (number % (i + 7) == 0) return i + 7;
        if (number % (i + 11) == 0) return i + 11;
        if (number % (i + 13) == 0) return i + 13;
        if (number % (i + 17) == 0) return i + 17;
        if (number % (i + 19) == 0) return i + 19;
        if (number % (i + 23) == 0) return i + 23;
        if (number % (i + 29) == 0) return i + 29;
    }

    return 0;
}

//Reading either entire file with primes or loading up to len primes, or till we met prime that greater than stopPrime value
uint32_t LoadPrimesFromBINDiffVar(std::string& filename, uint64_t* primes, size_t len, uint64_t stopPrime)
{
    //TODO why don't use similar functions from PrimesFIO?


    std::fstream f;
    f.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    f.open(filename, std::ios::in | std::ios::binary);
    f.exceptions(std::ifstream::badbit);

    uint64_t diff;
    uint64_t lastPrime = 0;
    uint8_t buf[9];
    size_t maxSize;

    uint32_t cnt = 0;
    while (cnt < len)
    {
        maxSize = 0;

        while (true)
        {
            f.read((char*)(buf + maxSize), 1);
            if (f.eof()) break;
            if ((buf[maxSize++] & 0x80) == 0) break;
        }

        if (f.eof()) break;

        [[maybe_unused]] size_t res = var_len_decode(buf, maxSize, &diff);

        assert(res == maxSize);

        if (lastPrime > 2) diff *= 2; // multiply by 2 все: кроме первого числа(lastPrime==0), и когда lastPrime==2

        primes[cnt] = diff + lastPrime;
        lastPrime = primes[cnt++];

        if (lastPrime > stopPrime) break;
    }

    return cnt;
}

//Reading either entire file with primes or loading up to len primes, or till we met prime that greater than stopPrime value
uint32_t LoadPrimesFromTXTFile(std::string& filename, uint64_t* primes, uint32_t len, uint64_t stopPrime)
{
    std::fstream f;
    f.exceptions(/*ifstream::failbit |*/ std::ifstream::badbit);
    f.open(filename, std::ios::in);

    std::string line;

    uint32_t cnt = 0;
    while (cnt < len)
    {
        getline(f, line, ',');
        if (f.eof()) break;

        primes[cnt] = atoll(line.c_str());

        if (primes[cnt] > stopPrime) break;
        cnt++;
    }

    f.close();

    return cnt;
}


#pragma comment(lib, "psapi.lib")
size_t GetMemoryUsage()
{
    PROCESS_MEMORY_COUNTERS_EX2 pmc = {};
    if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc)))
    {
        return pmc.PrivateUsage;
    }
    return 0;
}


void PrintMemoryUsage()
{
    PROCESS_MEMORY_COUNTERS_EX2 pmc = {};
    if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc)))
    {
        std::cout << std::endl << "Memory Usage" << std::endl;
        std::cout << std::format("{:<{}}: ", "PrivateUsage", F_WIDTH) << pmc.PrivateUsage << std::endl;
        std::cout << std::format("{:<{}}: ", "PrivateWorkingSetSize", F_WIDTH) << pmc.PrivateWorkingSetSize << std::endl;
        std::cout << std::format("{:<{}}: ", "WorkingSetSize", F_WIDTH) << pmc.WorkingSetSize << std::endl;
        std::cout << std::format("{:<{}}: ", "PeakWorkingSetSize", F_WIDTH) << pmc.PeakWorkingSetSize << std::endl;
        std::cout << std::format("{:<{}}: ", "SharedCommitUsage", F_WIDTH) << pmc.SharedCommitUsage << std::endl;
    }
    else
        std::cout << std::endl << "Error getting Memory Usage info" << std::endl;

}

void PrintTime(const std::string& msg)
{
    std::cout << GetCurrDateTimeAsString();
    std::cout << ' ' << msg << std::endl;
}

struct tm time_t_to_tm(const time_t t)
{
    struct tm ttm;
    localtime_s(&ttm, &t);
    return ttm;
}

struct tm GetCurrDateTime()
{
    const std::time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    return time_t_to_tm(tt);
}

#define DATETIME_BUF 100

// retrieves current datetime as string
std::string GetCurrDateTimeAsString(void)
{
    struct tm t = GetCurrDateTime();
    char ss[DATETIME_BUF];
    std::strftime(ss, DATETIME_BUF, "%c", &t);

    return ss;
}

std::string RemoveApo(const std::string& str)
{
    std::string res;
    res.reserve(str.size());
    for (size_t i = 0; i < str.length(); i++)
    {
        if (str[i] != '\'') res += str[i];
    }

    return res;
}
