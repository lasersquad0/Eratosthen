#pragma once

#include <cassert>
#include <atomic>
#include <string>
#include <mutex>
#include <locale>

#ifdef _MSC_VER

#include <malloc.h>
#define allocate_aligned(align, size) _aligned_malloc(size, align)
#define free_aligned(ptr) _aligned_free(ptr)

#else // Linux / macOS / GCC / Clang

#define allocate_aligned(align, size) std::aligned_alloc(align, size)
#define free_aligned(ptr) std::free(ptr)

#endif

class CheckPrimes;
void CheckPrimesThread(CheckPrimes* cp);


class CheckPrimes
{
private:
	friend void CheckPrimesThread(CheckPrimes* cp);
	friend void PrintProgressThread(CheckPrimes* cp);

	uint64_t* m_primesToVerify{};   // array of loaded m_countPrimesToVerify primes
	uint64_t* m_end{};
	size_t m_capacity = 0;  // size of m_primesToVerify array
	size_t m_countPrimesToVerify = 0; // actual number of loaded primes from a file
	uint64_t* m_prldPrimes = nullptr;
	uint64_t m_countPrldPrimes = 0;
	std::atomic_uint64_t m_currIndex = 0;
	uint64_t m_offset = 0;
	uint64_t m_limitPrimes = 0;
	std::chrono::steady_clock::time_point m_start;
protected:
	uint64_t* nextPrime(uint64_t count);
	void printProgress();
	void preloadPrimes(uint64_t forNumber);
	static uint64_t isPrimeNumber(uint64_t number);
	static uint64_t isPrimeNumber6(uint64_t number);
	//static uint64_t isPrimeNumber30(uint64_t number);
	uint64_t findMax(const std::vector<std::string>& primes);
	void checkPrimesInThreads(uint8_t numThreads);
public:
	// size of array to be allocated for primes loaded from a file
	// if file contains more primes, only MAX_PRIMES_TO_STORE will be checked still.
	//TODO think how to avoid this constraint
	static const size_t MAX_PRIMES_TO_STORE = 200'000'000; // default max number of primes that can be processed
	
	std::mutex m_mutex;
	//std::atomic_uint m_threadIDs = 0;
	std::vector<std::pair<uint64_t, uint64_t>> m_compound;
	//std::osyncstream syncout;

	CheckPrimes(size_t limitPrimes, size_t offset)//: syncout(std::cout)
	{
		assert(limitPrimes > 0);
		assert(offset < limitPrimes);
		m_capacity = limitPrimes == 0 ? MAX_PRIMES_TO_STORE : std::min(limitPrimes, MAX_PRIMES_TO_STORE);
		m_primesToVerify = static_cast<uint64_t*>(allocate_aligned(std::hardware_destructive_interference_size, m_capacity*sizeof(uint64_t)));
		assert(m_primesToVerify);
		m_offset = offset;
		m_limitPrimes = limitPrimes;
	}

	CheckPrimes()
	{
		// field default values are assigned in place of their definition
	}

	~CheckPrimes()
	{
		free_aligned(m_primesToVerify);
	}

	void Start(const std::string& fileName, uint8_t numThreads /*= DEFAULT_THREADS*/);
	void CheckPrimesArray(const std::vector<std::string>& primes, bool forceBigInt = false);
	
};

