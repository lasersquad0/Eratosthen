

#include <iostream>
#include <cassert>
#include <chrono>
//#include <sstream>
#include <algorithm>
#include <windows.h>
#include <psapi.h>
#include "Utils.h"


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
        std::cout << "PrivateUsage         : " << pmc.PrivateUsage << std::endl;
        std::cout << "PrivateWorkingSetSize: " << pmc.PrivateWorkingSetSize << std::endl;
        std::cout << "WorkingSetSize       : " << pmc.WorkingSetSize << std::endl;
        std::cout << "PeakWorkingSetSize   : " << pmc.PeakWorkingSetSize << std::endl;
        std::cout << "SharedCommitUsage    : " << pmc.SharedCommitUsage << std::endl;
    }
    else
        std::cout << std::endl << "Error getting Memory Usage info" << std::endl;

}

void Trim(std::string& str) // str passed by value here intentionally
{
    // remove any leading and traling spaces tabs.
    size_t strBegin = str.find_first_not_of(" \t");
    if (strBegin == std::string::npos) return;

    size_t strEnd = str.find_last_not_of(" \t");
    assert(strEnd != std::string::npos);

    str.erase(strEnd + 1 /*, str.size() - strEnd*/);
    str.erase(0, strBegin);
}

static char mytoupper(int c) // to eliminate compile warning "warning C4244: '=': conversion from 'int' to 'char', possible loss of data"
{
    return (char)toupper(c);
}

void TrimAndUpper(std::string& str)
{
    Trim(str);

    // to uppercase
    transform(str.begin(), str.end(), str.begin(), ::mytoupper);
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


