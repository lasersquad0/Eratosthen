#pragma once

// this is to remove defines min, max in windows headers because they conflict with std::min std::max 
#define NOMINMAX
#include <windows.h>


class CriticalSection
{
public:
	CriticalSection()
	{
		InitializeCriticalSection(&cs);
	}

	~CriticalSection()
	{
		DeleteCriticalSection(&cs);
	}

	void lock()
	{
		EnterCriticalSection(&cs);
	}

	void unlock()
	{
		LeaveCriticalSection(&cs);
	}

private:
	CriticalSection(const CriticalSection&) = delete;
	void operator=(const CriticalSection&) = delete;

	CRITICAL_SECTION cs;
};