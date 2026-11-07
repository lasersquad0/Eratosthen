#pragma once

#include <string>
#include "LogEngine2/LogEngine.h"
#include "GenericBigInt.h"

#define GET_LOGGER auto& logger = LogEngine::GetLogger(ERA::LOGGER_NAME)

// field width for printing various results to console
const uint32_t F_WIDTH = 27;

namespace ERA
{
	inline std::string APP_EXE_NAME = "SOME APP.exe"; // initialized from argv[0] in main()
	inline std::string APP_NAME = "SOME APP";         // initialized from argv[0] in main()
	inline std::string LOGGER_NAME = "SOME NAME";     // initialized from argv[0] in main()
	inline std::string LOGFILE_NAME = "SOME F NAME";  // initialized from argv[0] in main()
	 
	struct MyGroupSeparator : std::numpunct<char>
	{
		char do_thousands_sep() const override { return '\''; }    // thousands separator
		std::string do_grouping() const override { return "\3"; }  // group by 3
	};

	// used in std::cout and in std::format
	inline const std::locale& GetLoc()
	{
		static std::locale LLocale(std::cout.getloc(), new MyGroupSeparator());
		return LLocale;
	}
}

///////////////////////////////////////////////////
// BigInt IsPrimeNumber30 functions
///////////////////////////////////////////////////

constexpr uint64_t CALLBACK_THRESHOLD = 2'000'000;
constexpr int CALLBACK_OK = 0;
constexpr int CALLBACK_ABORT = 1;

template<typename BIGINT>
using TPrimeNumberCallback = int(__stdcall*)(const BIGINT& curr, const BIGINT& max);

/**
* @brief checks youself for primality using 30nx method
* @details it is possible to define a callback function that will be called during primality verifying process
* alloving calling application to show progress and stop this process if needed.
* When callback function returns CALLBACK_ABORT process is stopped.
* @param callback Pointer to callback function
* @param threshold Defines number of iterations that will be done before next callback call
* @return Value 0 means 'no dividers' - number is prime, any other value is a found divider of prime
*/
template<typename BIGINT>
BIGINT IsPrimeNumber30(const BIGINT& number, const uint64_t* primes, const uint64_t primesSize, TPrimeNumberCallback<BIGINT> callbackFun, uint64_t callbackThreshold = CALLBACK_THRESHOLD)
{
    BIGINT stop = number;
    stop.Sqrt();
    stop++;

    BIGINT prime;
    size_t i = 0;
    while (i < primesSize)
    {
        prime = primes[i];
        if (prime >= stop) return 0ull; // number is prime
        if ((number % prime) == 0ull) return prime; // number is NOT prime (compound)
        ++i;
    }

    // if we are here it means that primes array is too small to determine whether number is prime.
    // continue checking with another alg
    // use 30nx alg which starts from last checked prime
    return IsPrimeNumber30<BIGINT>(number, primes[primesSize - 1], stop, callbackFun, callbackThreshold);
}

// array of dividers for numbers less than 30
// 0 in array means prime number (zero dividers), non zero means minimal divider
const uint64_t KNOWN_DIV[30]{ 0,0,0,0,2,0,2,0,2,3,2,0,2,0,2,3,2,0,2,0,2,3,2,0,2,5,2,3,2,0 };

template<typename BIGINT>
BIGINT IsPrimeNumber30(const BIGINT& number, const BIGINT& start, const BIGINT& stop, TPrimeNumberCallback<BIGINT> callbackFun, uint64_t callbackThreshold = CALLBACK_THRESHOLD)
{
    //assert(start <= stop);
    //assert(stop < number);
    assert(callbackThreshold > 0);
    //assert(callbackFun != nullptr);

    if (number < 30) return KNOWN_DIV[number.ToUInt()];
    BIGINT startstart = start, stopstop = stop;
    if (stop > number) stopstop = number;
    if (start > stop) startstart = stop;

    if (startstart < 30)
    {
        if ((number % 2ull) == 0ull)  return 2ull;
        if ((number % 3ull) == 0ull)  return 3ull;
        if ((number % 5ull) == 0ull)  return 5ull;
        if ((number % 7ull) == 0ull)  return 7ull;
        if ((number % 11ull) == 0ull) return 11ull;
        if ((number % 13ull) == 0ull) return 13ull;
        if ((number % 17ull) == 0ull) return 17ull;
        if ((number % 19ull) == 0ull) return 19ull;
        if ((number % 23ull) == 0ull) return 23ull;
        if ((number % 29ull) == 0ull) return 29ull;
    }

    uint64_t cnt = 0;

    BIGINT multiple30;
    multiple30 = (startstart / 30) * 30;
    if (multiple30 < 30) multiple30 = 30;
    if (stopstop >= 30) stopstop -= 30; // when stop=30 we actually check 31,37...53,59, need deduct 30

    for (BIGINT i = multiple30/*30ull*/; i < stopstop; i += 30ull)
    {
        if (cnt > callbackThreshold)
        {
            if (callbackFun)
            {
                auto res = callbackFun(i, stopstop);
                if (res == CALLBACK_ABORT) break; //TODO what value should function return on ABORT?
            }
            cnt = 0;
        }
        ++cnt;

        if (number % (i + 1ull) == 0ull) { i += 1;  return i; }
        if (number % (i + 7ull) == 0ull) { i += 7;  return i; }
        if (number % (i + 11ull) == 0ull) { i += 11; return i; }
        if (number % (i + 13ull) == 0ull) { i += 13; return i; }
        if (number % (i + 17ull) == 0ull) { i += 17; return i; }
        if (number % (i + 19ull) == 0ull) { i += 19; return i; }
        if (number % (i + 23ull) == 0ull) { i += 23; return i; }
        if (number % (i + 29ull) == 0ull) { i += 29; return i; }
    }

    return 0ull;
}

template<typename BIGINT>
BIGINT IsPrimeNumber30(const BIGINT& number, TPrimeNumberCallback<BIGINT> callbackFun)
{
    BIGINT stop = number, zero(0ull);
    stop.Sqrt();
    stop++;
    return IsPrimeNumber30<BIGINT>(number, zero, stop, callbackFun);
}

inline char mytoupper(int c) // to eliminate compile warning "warning C4244: '=': conversion from 'int' to 'char', possible loss of data"
{
	return (char)toupper(c);
}


uint64_t IsPrimeNumber30(uint64_t number, const uint64_t* primes, const uint64_t primesSize);
uint64_t IsPrimeNumber30(uint64_t number, uint64_t start, uint64_t stop);
uint64_t IsPrimeNumber30(uint64_t number);

//Reading either entire file with primes or loading up to len primes, or till we met prime that greater than stopPrime value
uint32_t LoadPrimesFromBINDiffVar(std::string& filename, uint64_t* primes, size_t len, uint64_t stopPrime);
//Reading either entire file with primes or loading up to len primes, or till we met prime that greater than stopPrime value
uint32_t LoadPrimesFromTXTFile(std::string& filename, uint64_t* primes, uint32_t len, uint64_t stopPrime);

size_t GetMemoryUsage();
void PrintMemoryUsage();

std::string RemoveApo(const std::string& str);

struct tm time_t_to_tm(const time_t t);
struct tm GetCurrDateTime();
// retrieves current datetime as string
std::string GetCurrDateTimeAsString(void);

