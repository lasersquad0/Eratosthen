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

	uint64_t* m_primes{};   // array of loaded m_loadedPrimes primes
	uint64_t* m_end{};
	size_t m_capacity = 0;  // size of m_primes array
	size_t m_loadedPrimes = 0; // actual number of loaded primes from a file
	std::atomic_uint64_t m_currIndex = 0;
	uint64_t m_offset = 0;
	std::chrono::steady_clock::time_point m_start;
protected:
	uint64_t* nextPrime(uint64_t count);
	void printProgress();
	static uint64_t isPrimeNumber(uint64_t number);
	static uint64_t isPrimeNumber6(uint64_t number);
	static uint64_t isPrimeNumber30(uint64_t number);
	void checkPrimesInThreads(uint8_t numThreads);
public:
	//static const size_t DEFAULT_LIMIT_PRIMES = 150'000'000; // default max number of primes to be processed
	//static const size_t DEFAULT_OFFSET = 0; // used when we need to start checking primes from middle of the file
	//static const uint8_t DEFAULT_THREADS = 4;

	std::mutex m_mutex;
	std::atomic_uint m_threadIDs = 0;
	//uint32_t m_progressUpdated = 0;
	//uint64_t m_limitPrimes = 0;
	std::vector<std::pair<uint64_t, uint64_t>> m_compound;
	//std::osyncstream syncout;

	CheckPrimes(size_t count, size_t offset)//: syncout(std::cout)
	{
		assert(count > 0);
		//count = count == 0 ? DEFAULT_LIMIT_PRIMES : count;
		m_primes = static_cast<uint64_t*>(allocate_aligned(std::hardware_destructive_interference_size, count*sizeof(uint64_t)));
		assert(m_primes);
		m_capacity = count;
		assert(offset < count);
		m_offset = offset;
	}

	~CheckPrimes()
	{
		free_aligned(m_primes);
	}

	void Start(const std::string& fileName, uint8_t numThreads /*= DEFAULT_THREADS*/);
	static void CheckPrimesArray(const std::vector<std::string>& primes);
	
};

