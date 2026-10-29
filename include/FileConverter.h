#pragma once

#include <string>
#include <map>
#include "PrimesFIO.h"

class FileConverter
{
	//friend void CheckPrimeThread(FileConverter* fc);
public:
	typedef PrimesFIO::PRIMES_FILE_FORMAT formats;
	FileConverter();
	//void Convert(formats from, formats to, std::string fileNameFrom, uint64_t limitPrimes, uint64_t offsetPrimes);
	void Convert(std::string fromTo, std::string fileNameFrom, uint64_t limitPrimes, uint64_t offsetPrimes);
	void ParseFormats(const std::string& fromTo, formats& from, formats& to);
	void TestParseFormats();

private:
	//std::map<formats, std::string> m_frmtToStr;
	//std::map<std::string, formats> m_strToFrmt;
	std::map<formats, std::string> m_Exts;
	//uint64_t m_maxDiff;
	//std::mutex m_mutex;
	//std::atomic_uint m_threadIDs = 0;

	void PrintProgress(uint64_t totalBytesRead = 0, uint64_t fileSize = 0);
	void FinishProgress();
	uint64_t readJavaLong(std::fstream& f);
	//void StartThreads(uint8_t numThreads);

};

