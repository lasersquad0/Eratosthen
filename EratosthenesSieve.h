#pragma once

#include <string>
#include <functional>
#include "PrimesFIO.h"
#include "SegmentedArray.h"
#include "ProgressPrinter.h"


class invalid_cmd_option: public std::invalid_argument
{
public:
	invalid_cmd_option(std::string str): invalid_argument(str)
	{
	}
};

class EratosthenesSieve;
typedef std::function<void(uint64_t NPrime, uint64_t currPrime, EratosthenesSieve* es)> StrikeoutComposedPred;
void FindPrimesThread(EratosthenesSieve* es, StrikeoutComposedPred predStrikeoutComposed); // forward declaration

enum class CALC_MODE { none, simple, optimized, n6n1, n30x };

#define STR_TO_CALCMODE(arg) ((arg)=="S"?CALC_MODE::simple:(arg)=="O"?CALC_MODE::optimized:(arg)=="6"?CALC_MODE::n6n1:(arg)=="30"?CALC_MODE::n30x:CALC_MODE::none)

#define CALCMODE_TO_STR(md) ((md)==CALC_MODE::simple?"simple":(md)==CALC_MODE::optimized?"opt":(md)==CALC_MODE::n6n1?"6n1":(md)==CALC_MODE::n30x?"n30x":"unknown")

class EratosthenesSieve: public ConsoleProgressBar //: public ProgressPrinter
{
private:
	const std::string FACTOR_SYMBOLS = "BKMGTP";
	static const uint64_t FACTOR_B  = 1ULL; // B means "bytes", so the factor = 1.
	static const uint64_t FACTOR_K  = 1'000ULL; // 'kilobytes'
	static const uint64_t FACTOR_M  = 1'000'000ULL; // 'megabytes'
	static const uint64_t FACTOR_G  = 1'000'000'000ULL; // 'gigabytes'
	static const uint64_t FACTOR_T  = 1'000'000'000'000ULL; // 'terabytes'
	static const uint64_t FACTOR_P  = 1'000'000'000'000'000ULL; // petabytes
	static const uint64_t MAX_START_K = 18400'000'000'000'000ULL; // 18400P
	static const uint64_t MAX_START_M = 18400'000'000'000ULL; // 18400P
	static const uint64_t MAX_START_G = 18400'000'000ULL; // 18400P
	static const uint64_t MAX_START_T = 18400'000ULL; // 18400P
	static const uint64_t MAX_START_P = 18400ULL; // 18400P
	static const uint64_t MAX_LEN   = 1'000'000'000'000ULL; // 1 terabyte (1T)

	char m_StartFactorSym = 'G'; // symbol suffix of Factor for m_realStart value. used for making output file name.
	char m_LengthFactorSym = 'G'; // symbol suffix of Factor for m_realLength value.used for making output file name.

	uint64_t m_StartFactor = FACTOR_G; // default value is G. will be overwriten by actual factor value from requested range.
	uint64_t m_LengthFactor = FACTOR_G;
	PRIMES_FILE_FORMAT m_OutputFileType; // 1-just txt file no diff.  2-txt file with diff. 3 - bin file no diff. 4-bin file with diff. 5-bin file with variable diff
	
	CALC_MODE m_mode;
	uint64_t m_realStart;  
	uint64_t m_realLength; // range length where we are looking for prime numbers
	uint64_t m_realEnd; 
	std::atomic_uint m_nextPrimeIndex;

	SegmentedArray* m_bitArray;
	
	std::atomic_uint m_threadIDs = 0;
	uint32_t m_readPrimes;
	uint64_t* m_prldPrimes;
	std::mutex m_mutex; // to sync printing to std::cout from different threads

	typedef std::pair<uint64_t, uint64_t> RangePair;
	typedef std::mutex SyncType;

	typedef std::pair<SyncType*, RangePair> RangeItem;
	std::vector<RangeItem> m_ranges;

private:
	friend void FindPrimesThread(EratosthenesSieve* es, StrikeoutComposedPred predStrikeoutComposed);
	bool predBIN6n1(std::ostream& stream, uint64_t index);

	void calculateSimple();
	void calculateOptimum();
	void calculate6n1();
	void calculate30nx();

	typedef std::function<uint64_t(const uint64_t index)> PrimeFromIndexPred;
	typedef std::function<bool(std::ostream& stream, uint64_t index)> SavePrimeToStreamPred;
	typedef std::function<uint64_t(std::ostream& stream)> StartPrimePred;

	uint64_t saveAsTXT(std::string& outputFilename, StartPrimePred startPred, SavePrimeToStreamPred pred);
	uint64_t saveAsBIN(std::string& outputFilename, StartPrimePred startPred, SavePrimeToStreamPred pred);
	uint64_t saveAsBINDiffVarSimpleMode(uint64_t start, SegmentedArray* sarr, std::string& outputFilename);

	//uint64_t saveAsTXTOptimumMode(SegmentedArray* sarr, std::string& outputFilename, StartPrimePred startPred, PrimeDiffFromIndexPred pred);
	//uint64_t saveAsTXTDiffOptimumMode(uint64_t start, SegmentedArray* sarr, std::string& outputFilename, PrimeFromIndexPred pred);
	//uint64_t saveAsBINOptimumMode(uint64_t start, SegmentedArray* sarr, std::string& outputFilename, PrimeFromIndexPred pred);
	//uint64_t saveAsBINDiffOptimumMode(uint64_t start, SegmentedArray* sarr, std::string& outputFilename, PrimeFromIndexPred pred);
	uint64_t saveAsBINDiffVarOptimumMode(uint64_t start, SegmentedArray* sarr, std::string& outputFilename, PrimeFromIndexPred pred);
	//uint32_t loadPrimesFromTXTFile(string filename, uint64_t* primes, uint64_t stopPrime);
	uint32_t loadPrimesFromTXTFile(std::string& filename, uint64_t* primes, uint32_t len, uint64_t stopPrime);
	uint32_t loadPrimesFromBINDiffVar(std::string& filename, uint64_t* primes, size_t len, uint64_t stopPrime);
	void loadPrimes();

	std::string getOutputFilename();
	uint64_t parseOption(std::string s, char& symFactor, uint64_t& factor);
	void checkStartParam(std::string start);
	void defineRanges(uint64_t start, uint64_t length);
	RangeItem* getRange(uint64_t v);
	uint64_t nextPrime();
	void FindPrimesInThreads(uint32_t numThreads, StrikeoutComposedPred predIndexFromPrime);

public:
	static const uint64_t DEFAULT_START = 20ULL * FACTOR_T; // default start value to calculating primes when is not specified in command line
	static const uint64_t DEFAULT_LENGTH = 10'000'000'000ULL; // length of range to calculate primes if not specified in command line

	EratosthenesSieve();

	void SetParams(std::string mode, std::string oft, std::string& start, std::string& len);
	void PrintUsage();
	void Calculate();

};


