#include <iostream>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <format>
#include <filesystem>
#include "Utils.h"
#include "Ticks.h"
#include "PrimesFIO.h"
#include "BuferedFileStream.h"
#include "Parameters.h"
#include "FileConverter.h"


using namespace std;

FileConverter::FileConverter()//: m_maxDiff(0)
{
	using enum PrimesFIO::PRIMES_FILE_FORMAT;
	m_Exts[txt]        = ".txt";
	m_Exts[txtdiff]    = ".diff.txt";
	m_Exts[bin]        = ".bin";
	m_Exts[bindiff]    = ".diff.bin";
	m_Exts[bindiffvar] = ".diffvar.bin";
}

// throws an exception if fromTo is in incorrect format
void FileConverter::Convert(string fromTo, string fileNameFrom, uint64_t limitPrimes, uint64_t offsetPrimes)
{
	formats from, to;
	ParseFormats(fromTo, from, to);

	using namespace PrimesFIO;

	Ticks::Start("converting");

	cout << format("{:<{}}: {}", "Requested operation", F_WIDTH, "Convert file with primes to another format") << endl;
	cout << format("{:<{}}:'{}'", "File to convert", F_WIDTH, fileNameFrom) << endl;
	cout << format("{:<{}}: {}", "Format FROM", F_WIDTH, PFFToStr(from)) << endl;
	cout << format("{:<{}}: {}", "Format TO", F_WIDTH, PFFToStr(to)) << endl;

	fstream fin;
	//fin.exceptions(/*ifstream::failbit | */ ifstream::badbit);
	//fin.sync_with_stdio(false);
	fin.open(fileNameFrom, ios::in | ios::binary);
	if (fin.fail())
		throw invalid_argument("Error: cannot open file '" + fileNameFrom);

	uint64_t fileSize = std::filesystem::file_size(fileNameFrom);
	
	//MappedFileStream bfs(fileNameFrom); // to use in SOME file format conversion calls
	BuferedFileStream bfs(fileNameFrom);
	
	string fileTo = Parameters::OUTPUT_FOLDER + "\\" + ExtractFileName(fileNameFrom) + m_Exts[to];

	fstream fout;
	//fout.exceptions(/*ifstream::failbit |*/ ifstream::badbit);
	//fout.sync_with_stdio(false);
	fout.open(fileTo, ios::out | ios::binary);
	if (fout.fail())
		throw invalid_argument("Error: cannot open file '" + fileTo);

	const size_t ARRAY_LEN = 100'000'000;
	const char* MAX_DIFF_STR = "Max diff btw 2 primes";

	uint64_t* primes = new uint64_t[ARRAY_LEN];
	
	size_t totalSaved = 0;
	bool noLimit = limitPrimes == 0;
	uint64_t totalBytesRead = 0, array_len, maxDiff = 0;
	LoadPrimesResult lpResult;

	// NOTE - not all conversion pairs are supported in the big "switch" below
	// Unsupported pairs: txtdiff-bindiffvar,
	// bindiff-bindiffvar, 
	// bindiffvar-txtdiff, bindiffvar-bindiff
	
	if (from == PFF::txt && to == PFF::txtdiff) //***** 1 *****  txt-txtdiff
	{
		uint64_t lastPrime = 0;
		//m_maxDiff = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		lpResult = BypassTXT(fin, offsetPrimes);
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromTXT(fin, primes, array_len);
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsTXTDiff(primes, lpResult.PrimesLoaded, lastPrime, maxDiff, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if (!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file
		
		FinishProgress();

		cout << format("{:<{}}: {:L}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
	}
	else if (from == PFF::txt && to == PFF::bin)    //***** 2 *****  txt-bin
	{
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		lpResult = BypassTXT(fin, offsetPrimes);
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromTXT(fin, primes, array_len);
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsBIN(primes, lpResult.PrimesLoaded, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if (!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file

		FinishProgress();
	}
	else if (from == PRIMES_FILE_FORMAT::txt && to == PRIMES_FILE_FORMAT::bindiff) //***** 3 *****  txt-bindiff
	{
		uint64_t lastPrime = 0;
		//m_maxDiff = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		lpResult = BypassTXT(fin, offsetPrimes);
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromTXT(fin, primes, array_len);
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsBINDiff(primes, lpResult.PrimesLoaded, lastPrime, maxDiff, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if (!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break;  // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file

		FinishProgress();

		cout << format("{:<{}}: {:L}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
	}
	else if (from == PRIMES_FILE_FORMAT::bin && to == PRIMES_FILE_FORMAT::bindiff) //***** 4 *****  bin-bindiff
	{
		uint64_t lastPrime = 0;
		//size_t loaded;
		//m_maxDiff = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		lpResult = BypassBIN(fin, offsetPrimes);
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromBIN(fin, primes, array_len);
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsBINDiff(primes, lpResult.PrimesLoaded, lastPrime, maxDiff, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if (!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file

		FinishProgress();

		cout << format("{:<{}}: {:L}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
	}
	else if (from == PRIMES_FILE_FORMAT::bin && to == PRIMES_FILE_FORMAT::txt) //***** 5 *****  bin-txt
	{
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;
		//size_t loaded;

		lpResult = BypassBIN(fin, offsetPrimes);
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromBIN(fin, primes, array_len);
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsTXT(primes, lpResult.PrimesLoaded, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if (!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file

		FinishProgress();
	}
	else if (from == PRIMES_FILE_FORMAT::bin && to == PRIMES_FILE_FORMAT::txtdiff) //***** 6 *****  bin-txtdiff
	{
		uint64_t lastPrime = 0;
		//size_t loaded;
		//m_maxDiff = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		lpResult = BypassBIN(fin, offsetPrimes);
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromBIN(fin, primes, array_len);
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsTXTDiff(primes, lpResult.PrimesLoaded, lastPrime, maxDiff, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if (!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file

		FinishProgress();

		cout << format("{:<{}}: {:L}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
	}
	else if (from == PRIMES_FILE_FORMAT::txtdiff && to == PRIMES_FILE_FORMAT::txt) //***** 7 *****  txtdiff-txt
	{
		uint64_t lastPrime = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		lpResult = BypassTXTDiff(fin, offsetPrimes, lastPrime);
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromTXTDiff(fin, primes, array_len, lastPrime);
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsTXT(primes, lpResult.PrimesLoaded, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if (!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file

		FinishProgress();
	}
	else if (from == PRIMES_FILE_FORMAT::txtdiff && to == PRIMES_FILE_FORMAT::bin) //***** 8 *****  txtdiff-bin
	{
		uint64_t lastPrime = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		lpResult = BypassTXTDiff(fin, offsetPrimes, lastPrime);
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromTXTDiff(fin, primes, array_len, lastPrime);
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsBIN(primes, lpResult.PrimesLoaded, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if (!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file

		FinishProgress();
	}
	else if (from == PRIMES_FILE_FORMAT::txtdiff && to == PRIMES_FILE_FORMAT::bindiff) //***** 9 *****  txtdiff-bindiff
	{
		uint64_t lastPrime1 = 0;
		uint64_t lastPrime2 = 0;
		//m_maxDiff = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		lpResult = BypassTXTDiff(fin, offsetPrimes, lastPrime1);
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromTXTDiff(fin, primes, array_len, lastPrime1);
			assert(lpResult.BytesRead > 0);
			assert(lpResult.BytesRead < ARRAY_LEN*sizeof(uint64_t));
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsBINDiff(primes, lpResult.PrimesLoaded, lastPrime2, maxDiff, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if(!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file

		FinishProgress();

		cout << format("{:<{}}: {:L}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
	}
	else if (from == PRIMES_FILE_FORMAT::txtdiff && to == PRIMES_FILE_FORMAT::bindiffvar) //***** 10 *****  txtdiff-bindiffvar
	{
		uint64_t lastPrime1 = 0;
		uint64_t lastPrime2 = 0;
		
		lpResult = BypassTXTDiff(fin, offsetPrimes, lastPrime1);
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromTXTDiff(fin, primes, array_len, lastPrime1);
			assert(lpResult.BytesRead > 0);
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsBINDiffVar(primes, lpResult.PrimesLoaded, lastPrime2, maxDiff, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if (!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file

		FinishProgress();

		cout << format("{:<{}}: {:L}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
		}
	else if (from == PRIMES_FILE_FORMAT::bindiff && to == PRIMES_FILE_FORMAT::bin) //***** 11 *****  bindiff-bin
	{
		uint64_t lastPrime = 0;
		//size_t loaded;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		lpResult = BypassBINDiff(fin, offsetPrimes, lastPrime);
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromBINDiff(fin, primes, array_len, lastPrime);
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsBIN(primes, lpResult.PrimesLoaded, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if (!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file

		FinishProgress();
	}
	else if (from == PRIMES_FILE_FORMAT::bindiff && to == PRIMES_FILE_FORMAT::txt) //***** 12 *****  bindiff-txt
	{
		uint64_t lastPrime = 0;
		//size_t loaded;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		lpResult = BypassBINDiff(fin, offsetPrimes, lastPrime);
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromBINDiff(fin, primes, array_len, lastPrime);
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsTXT(primes, lpResult.PrimesLoaded, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if (!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file

		FinishProgress();
	}
	else if (from == PRIMES_FILE_FORMAT::bindiff && to == PRIMES_FILE_FORMAT::txtdiff) //***** 13 *****  bindiff-txtdiff
	{
		uint64_t lastPrime1 = 0;
		uint64_t lastPrime2 = 0;
		//size_t loaded;
		//m_maxDiff = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		lpResult = BypassBINDiff(fin, offsetPrimes, lastPrime1);
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromBINDiff(fin, primes, array_len, lastPrime1);
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsTXTDiff(primes, lpResult.PrimesLoaded, lastPrime2, maxDiff, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if (!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file

		FinishProgress();

		cout << format("{:<{}}: {:L}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
	}
	else if (from == PRIMES_FILE_FORMAT::bindiff && to == PRIMES_FILE_FORMAT::bindiffvar) //***** 14 *****  bindiff-bindiffvar
	{
		uint64_t lastPrime1 = 0;
		uint64_t lastPrime2 = 0;
		
		lpResult = BypassBINDiff(fin, offsetPrimes, lastPrime1);
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromBINDiff(fin, primes, array_len, lastPrime1);
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsBINDiffVar(primes, lpResult.PrimesLoaded, lastPrime2, maxDiff, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if (!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file

		FinishProgress();

		cout << format("{:<{}}: {:L}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
	}
	else if (from == PRIMES_FILE_FORMAT::bin && to == PRIMES_FILE_FORMAT::bindiffvar) //***** 15 *****  bin-bindiffvar
	{
		uint64_t lastPrime = 0;
		//size_t loaded;
		//m_maxDiff = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		lpResult = BypassBIN(fin, offsetPrimes);
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromBIN(fin, primes, array_len);
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsBINDiffVar(primes, lpResult.PrimesLoaded, lastPrime, maxDiff, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if (!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file

		FinishProgress();

		cout << format("{:<{}}: {:L}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
	}
	else if (from == PRIMES_FILE_FORMAT::txt && to == PRIMES_FILE_FORMAT::bindiffvar) //***** 16 *****  txt-bindiffvar
	{
		uint64_t lastPrime = 0;
		//uint64_t array_len;
		//m_maxDiff = 0;
		//bool noLimit = limitPrimes == 0;

		lpResult = BypassTXT(fin, offsetPrimes);
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromTXT(fin, primes, array_len);
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsBINDiffVar(primes, lpResult.PrimesLoaded, lastPrime, maxDiff, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if (!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file

		FinishProgress();

		cout << format("{:<{}}: {:L}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
	}
	else if (from == PRIMES_FILE_FORMAT::bindiffvar && to == PRIMES_FILE_FORMAT::bin) //***** 17 *****  bindiffvar-bin
	{
		uint64_t lastPrime = 0;
		//size_t loaded;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;
		
		lpResult = BypassBINDiffVar(bfs, offsetPrimes, lastPrime);
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromBINDiffVar(bfs, primes, array_len, lastPrime);
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsBIN(primes, lpResult.PrimesLoaded, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if (!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file
		
		FinishProgress();
	}
	else if (from == PRIMES_FILE_FORMAT::bindiffvar && to == PRIMES_FILE_FORMAT::txt) //***** 18 *****  bindiffvar-txt
	{
		uint64_t lastPrime = 0;
		//size_t loaded;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;
		
		lpResult = BypassBINDiffVar(bfs, offsetPrimes, lastPrime); // for 'diff' formats we still need to calculate lastPrime during bypassing
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromBINDiffVar(bfs, primes, array_len, lastPrime);
			assert(lpResult.BytesRead < ARRAY_LEN);
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsTXT(primes, lpResult.PrimesLoaded, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if (!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file

		FinishProgress();
	}
	else if (from == PRIMES_FILE_FORMAT::bindiffvar && to == PRIMES_FILE_FORMAT::txtdiff) //***** 19 *****  bindiffvar-txtdiff
	{
		uint64_t lastPrime1 = 0;
		uint64_t lastPrime2 = 0;

		lpResult = BypassBINDiffVar(bfs, offsetPrimes, lastPrime1); // for 'diff' formats we still need to calculate lastPrime during bypassing
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromBINDiffVar(bfs, primes, array_len, lastPrime1);
			assert(lpResult.BytesRead < ARRAY_LEN);
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsTXTDiff(primes, lpResult.PrimesLoaded, lastPrime2, maxDiff, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if (!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file

		FinishProgress();

		cout << format("{:<{}}: {:L}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
	}
	else if (from == PRIMES_FILE_FORMAT::bindiffvar && to == PRIMES_FILE_FORMAT::bindiff) //***** 20 *****  bindiffvar-bindiff
	{
		uint64_t lastPrime1 = 0;
		uint64_t lastPrime2 = 0;

		lpResult = BypassBINDiffVar(bfs, offsetPrimes, lastPrime1); // for 'diff' formats we still need to calculate lastPrime during bypassing
		totalBytesRead = lpResult.BytesRead;

		do
		{
			PrintProgress(totalBytesRead, fileSize);
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			lpResult = LoadFromBINDiffVar(bfs, primes, array_len, lastPrime1);
			assert(lpResult.BytesRead < ARRAY_LEN);
			totalBytesRead += lpResult.BytesRead;
			PrintProgress(totalBytesRead, fileSize);
			SaveAsBINDiff(primes, lpResult.PrimesLoaded, lastPrime2, maxDiff, fout);
			assert(noLimit || limitPrimes >= lpResult.PrimesLoaded);
			if (!noLimit) limitPrimes -= lpResult.PrimesLoaded;
			totalSaved += lpResult.PrimesLoaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (lpResult.PrimesLoaded == array_len); // end of primes numbers in a file

		FinishProgress();

		cout << format("{:<{}}: {:L}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
	}
	else if (from == PRIMES_FILE_FORMAT::bin && to == PRIMES_FILE_FORMAT::bin) //*** SPECIAL CASE FOR CONVERTING FROM Java BIN FILE TO C++ BIN FILE ***
	{
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;
		size_t loaded;

		BypassBIN(offsetPrimes, fin);

		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = LoadFromBINJava(primes, array_len, fin);
			PrintProgress();
			SaveAsBIN(primes, loaded, fout);
			assert(noLimit || limitPrimes >= loaded);
			if (!noLimit) limitPrimes -= loaded;
			totalSaved += loaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (loaded == array_len); // end of primes numbers in a file

		FinishProgress();
	}
	else if (from == PRIMES_FILE_FORMAT::bindiffvar && to == PRIMES_FILE_FORMAT::bindiffvar) //**** SPECIAL CASE WHEN CONVERTING FROM OLD bindiffvar to NEW bindiffvar
	{
		uint64_t lastPrime1 = 0;
		uint64_t lastPrime2 = 0;
		size_t loaded;
		//uint64_t array_len;
		//m_maxDiff = 0;
		//bool noLimit = limitPrimes == 0;

		BypassBINDiffVarOLD(offsetPrimes, lastPrime1, fin);

		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = LoadFromBINDiffVarOLD(primes, array_len, lastPrime1, fin);
			PrintProgress();
			SaveAsBINDiffVar(primes, loaded, lastPrime2, maxDiff, fout);
			assert(noLimit || limitPrimes >= loaded);
			if (!noLimit) limitPrimes -= loaded;
			totalSaved += loaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (loaded == array_len); // end of primes numbers in a file

		FinishProgress();
	}
	else
	{
		throw invalid_argument(format("Error: incorrect conversion direction specified '{}'.", fromTo));
	}

	delete[] primes;

	fin.close();
	fin.open(fileNameFrom, ios::in | ios::binary); // need reopen fin for reading filesize properly. othersize it returns -1.
	fin.seekg(0, fin.end);
	fout.seekg(0, fin.end);
    
	uint64_t len = std::max(fileNameFrom.size(), fileTo.size()) + 2;

	cout << format(ERA::GetLoc(), "{:<{}}: {:L}", "Primes saved", F_WIDTH, totalSaved) << endl;
	cout << format(ERA::GetLoc(), "{:<{}}:'{:<{}}' : {:L} bytes", "FROM file name", F_WIDTH, fileNameFrom, len, (uint64_t)fin.tellg()) << endl;
	cout << format(ERA::GetLoc(), "{:<{}}:'{:<{}}' : {:L} bytes", "TO file name", F_WIDTH, fileTo, len, (uint64_t)fout.tellg()) << endl;
	cout << format("{:<{}}: {}", "Conversion time", F_WIDTH, MillisecToStr<string>(Ticks::Finish("converting"))) << endl;
}

void FileConverter::PrintProgress(uint64_t totalBytesRead, uint64_t fileSize)
{
	if (fileSize == 0)
		cout << '.';
	else
		cout << "Progress... " << totalBytesRead * 100 / fileSize << "%   \r";
}

void FileConverter::FinishProgress()
{
	cout << '\r'; //endl;
}

// Possible formats: frmt1-frmt2 and frmt1->frmt2
//TODO shall we have an option to omit from in 'from-to' arg and get 'from' from filename 
void FileConverter::ParseFormats(const string& fromTo, formats& from, formats& to)
{
	constexpr const char* ErrorText = "Incorrect format of 'from-to' argument for option '-c' ({}).";

	string FROMTO;
	FROMTO.resize(fromTo.size());

	// make string uppercase
	transform(fromTo.begin(), fromTo.end(), FROMTO.begin(), ::mytoupper);

	size_t index1 = FROMTO.find('-');

	if (index1 == string::npos)
		throw invalid_argument(format(ErrorText, fromTo));

	size_t index2 = index1;
	if ((FROMTO.length() > index1 + 1) && (FROMTO.at(index1 + 1) == '>'))
		index2 = index1 + 1;

	string fromstr = FROMTO.substr(0, index1);
	string tostr = FROMTO.substr(index2 + 1);

	from = PrimesFIO::StrToPFF(fromstr);
	to   = PrimesFIO::StrToPFF(tostr);

	if(from == PrimesFIO::PFF::none || to == PrimesFIO::PFF::none)	
		throw invalid_argument(format(ErrorText, fromTo));
}

#define awaitinvalid_argument(code) try{code;throw logic_error("awaitexception exception.");}catch(invalid_argument e){}

void FileConverter::TestParseFormats()
{
	formats from, to;
	ParseFormats("txt-bin", from, to);
	ParseFormats("txT->bIn", from, to);
	ParseFormats("TXT->BIN", from, to);
	try
	{
		ParseFormats("t->b", from, to);
	}
	catch (invalid_argument e)
	{
	}

	awaitinvalid_argument(ParseFormats("txt-", from, to));
	awaitinvalid_argument(ParseFormats("txt--", from, to));
	awaitinvalid_argument(ParseFormats("bin->", from, to));
	awaitinvalid_argument(ParseFormats("-txt", from, to));
	awaitinvalid_argument(ParseFormats("txt bin", from, to));
	awaitinvalid_argument(ParseFormats("t-", from, to));
	awaitinvalid_argument(ParseFormats("txt->b", from, to));
	awaitinvalid_argument(ParseFormats("txt->", from, to));
	awaitinvalid_argument(ParseFormats("bn-txt", from, to));
	awaitinvalid_argument(ParseFormats("-", from, to));
	awaitinvalid_argument(ParseFormats("", from, to));

}


