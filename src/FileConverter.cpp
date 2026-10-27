#include <iostream>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <format>
#include "Utils.h"
#include "PrimesFIO.h"
#include "BuferedFileStream.h"
#include "FileConverter.h"


using namespace std;

FileConverter::FileConverter()//: m_maxDiff(0)
{
	m_frmtToStr[PRIMES_FILE_FORMAT::txt]     = "TXT";
	m_frmtToStr[PRIMES_FILE_FORMAT::txtdiff] = "TXTDIFF";
	m_frmtToStr[PRIMES_FILE_FORMAT::bin]     = "BIN";
	m_frmtToStr[PRIMES_FILE_FORMAT::bindiff] = "BINDIFF";
	m_frmtToStr[PRIMES_FILE_FORMAT::bindiffvar] = "BINDIFFVAR";

	m_strToFrmt["TXT"]        = PRIMES_FILE_FORMAT::txt;
	m_strToFrmt["TXTDIFF"]    = PRIMES_FILE_FORMAT::txtdiff;
	m_strToFrmt["BIN"]        = PRIMES_FILE_FORMAT::bin;
	m_strToFrmt["BINDIFF"]    = PRIMES_FILE_FORMAT::bindiff;
	m_strToFrmt["BINDIFFVAR"] = PRIMES_FILE_FORMAT::bindiffvar;

	m_Exts[PRIMES_FILE_FORMAT::txt]        = ".txt";
	m_Exts[PRIMES_FILE_FORMAT::txtdiff]    = ".diff.txt";
	m_Exts[PRIMES_FILE_FORMAT::bin]        = ".bin";
	m_Exts[PRIMES_FILE_FORMAT::bindiff]    = ".diff.bin";
	m_Exts[PRIMES_FILE_FORMAT::bindiffvar] = ".diffvar.bin";
}

// throwa an exception if fromTo is in incorrect format
void FileConverter::Convert(string fromTo, string fileFrom, uint64_t limitPrimes, uint64_t offsetPrimes)
{
	formats from, to;
	ParseFormats(fromTo, from, to);
	Convert(from, to, fileFrom, limitPrimes, offsetPrimes);
}

void FileConverter::Convert(formats from, formats to, string fileNameFrom, uint64_t limitPrimes, uint64_t offsetPrimes)
{
	cout << format("{:<{}}: {}", "Requested operation", F_WIDTH, "Convert file with primes to another format") << endl;
	cout << format("{:<{}}:'{}'", "File to convert", F_WIDTH, fileNameFrom) << endl;
	cout << format("{:<{}}: {}", "Format FROM", F_WIDTH, m_frmtToStr[from]) << endl;
	cout << format("{:<{}}: {}", "Format TO", F_WIDTH, m_frmtToStr[to]) << endl;

	fstream fin;
	//fin.exceptions(/*ifstream::failbit | */ ifstream::badbit);
	//fin.sync_with_stdio(false);
	fin.open(fileNameFrom, ios::in | ios::binary);
	if (fin.fail())
		throw invalid_argument("Error: cannot open file '" + fileNameFrom);

	//MappedFileStream bfs(fileNameFrom); // to use in SOME file format conversion calls
	BuferedFileStream bfs(fileNameFrom);
	
	string fileTo = fileNameFrom + m_Exts[to];

	fstream fout;
	//fout.exceptions(/*ifstream::failbit |*/ ifstream::badbit);
	//fout.sync_with_stdio(false);
	fout.open(fileTo, ios::out | ios::binary);
	if (fout.fail())
		throw invalid_argument("Error: cannot open file '" + fileTo);

	const size_t ARRAY_LEN = 100'000'000;
	const char* MAX_DIFF_STR = "Max diff btw 2 primes";

	uint64_t* primes = new uint64_t[ARRAY_LEN];

	size_t loaded, totalSaved = 0;
	bool noLimit = limitPrimes == 0;
	uint64_t array_len, maxDiff = 0;
	PrimesFIO pf;

	// NOTE - not all conversion pairs are supported in the big "switch" below
	// Unsupported pairs: txtdiff-bindiffvar, bindiff-bindiffvar, bindiffvar-txtdiff, bindiffvar-bindiff
	
	if (from == PRIMES_FILE_FORMAT::txt && to == PRIMES_FILE_FORMAT::txtdiff) //*** 1 ***  txt-txtdiff
	{
		uint64_t lastPrime = 0;
		//m_maxDiff = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		pf.BypassTXT(offsetPrimes, fin);

		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = pf.LoadFromTXT(primes, array_len, fin);
			PrintProgress();
			pf.SaveAsTXTDiff(primes, loaded, lastPrime, maxDiff, fout);
			assert(noLimit || limitPrimes >= loaded);
			if (!noLimit) limitPrimes -= loaded;
			totalSaved += loaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (loaded == array_len); // end of primes numbers in a file
		
		FinishProgress();

		cout << format("{:<{}}: {}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
	}
	else if (from == PRIMES_FILE_FORMAT::txt && to == PRIMES_FILE_FORMAT::bin) //*** 2 ***  txt-bin
	{
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		pf.BypassTXT(offsetPrimes, fin);

		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = pf.LoadFromTXT(primes, array_len, fin);
			PrintProgress();
			pf.SaveAsBIN(primes, loaded, fout);
			assert(noLimit || limitPrimes >= loaded);
			if (!noLimit) limitPrimes -= loaded;
			totalSaved += loaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (loaded == array_len); // end of primes numbers in a file

		FinishProgress();
	}
	else if (from == PRIMES_FILE_FORMAT::txt && to == PRIMES_FILE_FORMAT::bindiff) //*** 3 ***  txt-bindiff
	{
		uint64_t lastPrime = 0;
		//m_maxDiff = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		pf.BypassTXT(offsetPrimes, fin);

		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = pf.LoadFromTXT(primes, array_len, fin);
			PrintProgress();
			pf.SaveAsBINDiff(primes, loaded, lastPrime, maxDiff, fout);
			assert(noLimit || limitPrimes >= loaded);
			if (!noLimit) limitPrimes -= loaded;
			totalSaved += loaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (loaded == array_len); // end of primes numbers in a file

		FinishProgress();

		cout << format("{:<{}}: {}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
	}
	else if (from == PRIMES_FILE_FORMAT::bin && to == PRIMES_FILE_FORMAT::bin) //*** SPECIAL CASE FOR CONVERTING FROM Java BIN FILE TO C++ BIN FILE ***
	{
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;
		
		pf.BypassBIN(offsetPrimes, fin);

		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = pf.LoadFromBINJava(primes, array_len, fin);
			PrintProgress();
			pf.SaveAsBIN(primes, loaded, fout);
			assert(noLimit || limitPrimes >= loaded);
			if (!noLimit) limitPrimes -= loaded;
			totalSaved += loaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (loaded == array_len); // end of primes numbers in a file

		FinishProgress();
	}
	else if (from == PRIMES_FILE_FORMAT::bin && to == PRIMES_FILE_FORMAT::bindiff) //*** 4 ***  bin-bindiff
	{
		uint64_t lastPrime = 0;
		//m_maxDiff = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		pf.BypassBIN(offsetPrimes, fin);

		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = pf.LoadFromBIN(primes, array_len, fin);
			PrintProgress();
			pf.SaveAsBINDiff(primes, loaded, lastPrime, maxDiff, fout);
			assert(noLimit || limitPrimes >= loaded);
			if (!noLimit) limitPrimes -= loaded;
			totalSaved += loaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (loaded == array_len); // end of primes numbers in a file

		FinishProgress();

		cout << format("{:<{}}: {}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
	}
	else if (from == PRIMES_FILE_FORMAT::bin && to == PRIMES_FILE_FORMAT::txt) //*** 5 ***  bin-txt
	{
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		pf.BypassBIN(offsetPrimes, fin);
		
		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = pf.LoadFromBIN(primes, array_len, fin);
			PrintProgress();
			pf.SaveAsTXT(primes, loaded, fout);
			assert(noLimit || limitPrimes >= loaded);
			if (!noLimit) limitPrimes -= loaded;
			totalSaved += loaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (loaded == array_len); // end of primes numbers in a file

		FinishProgress();
	}
	else if (from == PRIMES_FILE_FORMAT::bin && to == PRIMES_FILE_FORMAT::txtdiff) //*** 6 ***  bin-txtdiff
	{
		uint64_t lastPrime = 0;
		//m_maxDiff = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		pf.BypassBIN(offsetPrimes, fin);
		
		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = pf.LoadFromBIN(primes, array_len, fin);
			PrintProgress();
			pf.SaveAsTXTDiff(primes, loaded, lastPrime, maxDiff, fout);
			assert(noLimit || limitPrimes >= loaded);
			if (!noLimit) limitPrimes -= loaded;
			totalSaved += loaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (loaded == array_len); // end of primes numbers in a file

		FinishProgress();

		cout << format("{:<{}}: {}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
	}
	else if (from == PRIMES_FILE_FORMAT::txtdiff && to == PRIMES_FILE_FORMAT::txt) //*** 7 ***  txtdiff-txt
	{
		uint64_t lastPrime = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		pf.BypassTXTDiff(offsetPrimes, lastPrime, fin);

		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = pf.LoadFromTXTDiff(primes, array_len, lastPrime, fin);
			PrintProgress();
			pf.SaveAsTXT(primes, loaded, fout);
			assert(noLimit || limitPrimes >= loaded);
			if (!noLimit) limitPrimes -= loaded;
			totalSaved += loaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (loaded == array_len); // end of primes numbers in a file

		FinishProgress();
	}
	else if (from == PRIMES_FILE_FORMAT::txtdiff && to == PRIMES_FILE_FORMAT::bin) //*** 8 ***  txtdiff-bin
	{
		uint64_t lastPrime = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		pf.BypassTXTDiff(offsetPrimes, lastPrime, fin);

		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = pf.LoadFromTXTDiff(primes, array_len, lastPrime, fin);
			PrintProgress();
			pf.SaveAsBIN(primes, loaded, fout);
			assert(noLimit || limitPrimes >= loaded);
			if (!noLimit) limitPrimes -= loaded;
			totalSaved += loaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (loaded == array_len); // end of primes numbers in a file

		FinishProgress();
	}
	else if (from == PRIMES_FILE_FORMAT::txtdiff && to == PRIMES_FILE_FORMAT::bindiff) //*** 9 ***  txtdiff-bindiff
	{
		uint64_t lastPrime1 = 0;
		uint64_t lastPrime2 = 0;
		//m_maxDiff = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		pf.BypassTXTDiff(offsetPrimes, lastPrime1, fin);

		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = pf.LoadFromTXTDiff(primes, array_len, lastPrime1, fin);
			PrintProgress();
			pf.SaveAsBINDiff(primes, loaded, lastPrime2, maxDiff, fout);
			assert(noLimit || limitPrimes >= loaded);
			if(!noLimit) limitPrimes -= loaded;
			totalSaved += loaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (loaded == array_len); // end of primes numbers in a file

		FinishProgress();

		cout << format("{:<{}}: {}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
	}
	else if (from == PRIMES_FILE_FORMAT::bindiff && to == PRIMES_FILE_FORMAT::bin) //*** 10 ***  bindiff-bin
	{
		uint64_t lastPrime = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		pf.BypassBINDiff(offsetPrimes, lastPrime, fin);

		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = pf.LoadFromBINDiff(primes, array_len, lastPrime, fin);
			PrintProgress();
			pf.SaveAsBIN(primes, loaded, fout);
			assert(noLimit || limitPrimes >= loaded);
			if (!noLimit) limitPrimes -= loaded;
			totalSaved += loaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (loaded == array_len); // end of primes numbers in a file

		FinishProgress();
	}
	else if (from == PRIMES_FILE_FORMAT::bindiff && to == PRIMES_FILE_FORMAT::txt) //*** 11 ***  bindiff-txt
	{
		uint64_t lastPrime = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		pf.BypassBINDiff(offsetPrimes, lastPrime, fin);
		
		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = pf.LoadFromBINDiff(primes, array_len, lastPrime, fin);
			PrintProgress();
			pf.SaveAsTXT(primes, loaded, fout);
			assert(noLimit || limitPrimes >= loaded);
			if (!noLimit) limitPrimes -= loaded;
			totalSaved += loaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (loaded == array_len); // end of primes numbers in a file

		FinishProgress();
	}
	else if (from == PRIMES_FILE_FORMAT::bindiff && to == PRIMES_FILE_FORMAT::txtdiff) //*** 12 ***  bindiff-txtdiff
	{
		uint64_t lastPrime1 = 0;
		uint64_t lastPrime2 = 0;
		//m_maxDiff = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		pf.BypassBINDiff(offsetPrimes, lastPrime1, fin);
		
		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = pf.LoadFromBINDiff(primes, array_len, lastPrime1, fin);
			PrintProgress();
			pf.SaveAsTXTDiff(primes, loaded, lastPrime2, maxDiff, fout);
			assert(noLimit || limitPrimes >= loaded);
			if (!noLimit) limitPrimes -= loaded;
			totalSaved += loaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (loaded == array_len); // end of primes numbers in a file

		FinishProgress();

		cout << format("{:<{}}: {}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
	}
	else if (from == PRIMES_FILE_FORMAT::bin && to == PRIMES_FILE_FORMAT::bindiffvar) //*** 13 ***  bin-bindiffvar
	{
		uint64_t lastPrime = 0;
		//m_maxDiff = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;

		pf.BypassBIN(offsetPrimes, fin);
		
		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = pf.LoadFromBIN(primes, array_len, fin);
			PrintProgress();
			pf.SaveAsBINDiffVar(primes, loaded, lastPrime, maxDiff, fout);
			assert(noLimit || limitPrimes >= loaded);
			if (!noLimit) limitPrimes -= loaded;
			totalSaved += loaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (loaded == array_len); // end of primes numbers in a file

		FinishProgress();

		cout << format("{:<{}}: {}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
	}
	else if (from == PRIMES_FILE_FORMAT::txt && to == PRIMES_FILE_FORMAT::bindiffvar) //*** 14 ***  txt-bindiffvar
	{
		uint64_t lastPrime = 0;
		//uint64_t array_len;
		//m_maxDiff = 0;
		//bool noLimit = limitPrimes == 0;

		pf.BypassTXT(offsetPrimes, fin);

		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = pf.LoadFromTXT(primes, array_len, fin);
			PrintProgress();
			pf.SaveAsBINDiffVar(primes, loaded, lastPrime, maxDiff, fout);
			assert(noLimit || limitPrimes >= loaded);
			if (!noLimit) limitPrimes -= loaded;
			totalSaved += loaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (loaded == array_len); // end of primes numbers in a file

		FinishProgress();

		cout << format("{:<{}}: {}", MAX_DIFF_STR, F_WIDTH, maxDiff) << endl;
	}
	else if (from == PRIMES_FILE_FORMAT::bindiffvar && to == PRIMES_FILE_FORMAT::bin) //*** 15 ***  bindiffvar-bin
	{
		uint64_t lastPrime = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;
		
		pf.BypassBINDiffVar(offsetPrimes, lastPrime, bfs);

		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = pf.LoadFromBINDiffVar(primes, array_len, lastPrime, bfs);
			PrintProgress();
			pf.SaveAsBIN(primes, loaded, fout);
			assert(noLimit || limitPrimes >= loaded);
			if (!noLimit) limitPrimes -= loaded;
			totalSaved += loaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (loaded == array_len); // end of primes numbers in a file
		
		FinishProgress();
	}
	else if (from == PRIMES_FILE_FORMAT::bindiffvar && to == PRIMES_FILE_FORMAT::txt) //*** 16 ***  bindiffvar-txt
	{
		uint64_t lastPrime = 0;
		//uint64_t array_len;
		//bool noLimit = limitPrimes == 0;
		
		pf.BypassBINDiffVar(offsetPrimes, lastPrime, bfs); // for 'diff' formats we still need to calculate lastPrime during bypassing
		
		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = pf.LoadFromBINDiffVar(primes, array_len, lastPrime, bfs);
			PrintProgress();
			pf.SaveAsTXT(primes, loaded, fout);
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
		//uint64_t array_len;
		//m_maxDiff = 0;
		//bool noLimit = limitPrimes == 0;

		pf.BypassBINDiffVarOLD(offsetPrimes, lastPrime1, fin);

		do
		{
			PrintProgress();
			array_len = noLimit ? ARRAY_LEN : min(ARRAY_LEN, limitPrimes);
			loaded = pf.LoadFromBINDiffVarOLD(primes, array_len, lastPrime1, fin);
			PrintProgress();
			pf.SaveAsBINDiffVar(primes, loaded, lastPrime2, maxDiff, fout);
			assert(noLimit || limitPrimes >= loaded);
			if (!noLimit) limitPrimes -= loaded;
			totalSaved += loaded;
			if (!noLimit && limitPrimes == 0) break; // we have limit and we met limitPrimes limit
		} while (loaded == array_len); // end of primes numbers in a file

		FinishProgress();
	}
	else
	{
		throw invalid_argument("Error: incorrect conversion direction specified.");
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

}

void FileConverter::PrintProgress()
{
	cout << '.';
}

void FileConverter::FinishProgress()
{
	cout << '\r'; //endl;
}

// Possible formats: frmt1-frmt2 and frmt1->frmt2
//TODO shall we have an option to omit from in 'from-to' arg and get 'from' from filename 
void FileConverter::ParseFormats(string fromTo, formats& from, formats& to)
{
	const char* ErrorText = "Incorrect format of 'from-to' argument for option '-c'.";

	// make string uppercase
	transform(fromTo.begin(), fromTo.end(), fromTo.begin(), ::mytoupper);

	size_t index1 = fromTo.find('-');

	if (index1 == string::npos)
		throw invalid_argument(ErrorText);

	size_t index2 = index1;
	if ((fromTo.length() > index1 + 1) && (fromTo.at(index1 + 1) == '>'))
		index2 = index1 + 1;

	string fromstr = fromTo.substr(0, index1);
	string tostr = fromTo.substr(index2 + 1);

	try
	{
		from = m_strToFrmt.at(fromstr);
		to = m_strToFrmt.at(tostr);
	}
	catch (out_of_range&)
	{
		throw invalid_argument(ErrorText);
	}

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


