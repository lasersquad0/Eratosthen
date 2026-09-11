#pragma once

#include <string>

size_t GetMemoryUsage();
void PrintMemoryUsage();

void Trim(std::string& str);
void TrimAndUpper(std::string& str);

struct tm time_t_to_tm(const time_t t);
struct tm GetCurrDateTime();
// retrieves current datetime as string
std::string GetCurrDateTimeAsString(void);
