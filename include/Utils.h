#pragma once

#include <string>

// field width for printing various results to console
const uint32_t F_WIDTH = 27;

namespace ERA
{
	inline std::string APP_EXE_NAME = "SOME APP.exe"; // initialized from argv[0] in main()
	inline std::string APP_NAME = "SOME APP";         // initialized from argv[0] in main()

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

inline char mytoupper(int c) // to eliminate compile warning "warning C4244: '=': conversion from 'int' to 'char', possible loss of data"
{
	return (char)toupper(c);
}

size_t GetMemoryUsage();
void PrintMemoryUsage();

void Trim(std::string& str);
void TrimAndUpper(std::string& str);
std::string StripFileExt(const std::string& FileName);
std::string RemoveApo(const std::string& str);

struct tm time_t_to_tm(const time_t t);
struct tm GetCurrDateTime();
// retrieves current datetime as string
std::string GetCurrDateTimeAsString(void);
