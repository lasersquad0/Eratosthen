#pragma once

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <format>
#include <mutex>

class ProgressPrinter
{
private:
	inline static const std::string DEF_PROGRESS_STR = "Progress ... ";
	uint64_t m_p100 = 0;
	std::ostream& m_cout;
	uint64_t m_delta = 0;
	uint64_t m_current = 0;
	std::string m_progressStr;
	std::streamsize m_defPrecision = 0;
	std::streamsize m_defWidth = 0;
protected:
	std::mutex m_mutex;
public:
	ProgressPrinter(std::ostream& console, std::string progressStr = "") : m_cout(console), m_progressStr(progressStr)
	{
		if (m_progressStr.empty()) m_progressStr = DEF_PROGRESS_STR;
	}

	void StartProgress(uint64_t fullhouse)
	{
		m_p100 = fullhouse;
		m_delta = m_p100 / 100;
		m_current = 0;
		
		m_defPrecision = std::cout.precision();
		m_defWidth = std::cout.width();

		std::cout << std::endl;
		std::cout << std::setw(7) << std::setprecision(4);
	}

	void FinishProgress()
	{
		m_current = m_p100;
		UpdateProgress(m_p100);

		m_cout << std::setprecision(m_defPrecision) << std::setw(m_defWidth);
		//m_cout << "\r                                \r"; // cleaning up progress line of text
		m_cout << std::endl << std::endl;
	}

	void UpdateProgress(uint64_t progress)
	{
		if (progress >= m_current)
		{
			//TODO use mutex guard here?
			m_mutex.lock();
			m_cout << "\r" << m_progressStr << (float)(100 * m_current) / (float)m_p100 << "%      ";
			m_current += m_delta;
			m_mutex.unlock();
		}
	}
};


enum class TProgressResult
{
	OK,
	ABORT,
	PAUSE
};

/**
* This class used for showing ONLY progress on the console
* It shows one line without '\n' at the end and updates info on this line each time when progress() method is called
*/
class ConsoleProgressBar
{
protected:
	uint32_t m_width; // length of progress bar in console symbols
	uint32_t m_100 = 0;
	uint32_t m_delta = 0;
	uint32_t m_current = 0;
	std::mutex m_mutex;
	std::ostream& m_cout;
public:
	// width here should be in console symbols 
	ConsoleProgressBar(std::ostream& console, uint32_t width) : m_cout(console), m_width(width) { if (width < 3) m_width = 3; }

	void StartProgress(uint32_t p100) 
	{ 
		m_100 = p100; 
		m_delta = m_100 / 100;
		m_current = 0;

	}
	void FinishProgress()
	{
		m_current = m_100;
		UpdateProgress(m_100, "DONE");
		std::cout << std::endl << std::endl;
	}

	//TODO implement version with mutex sync
	TProgressResult UpdateProgress(uint32_t prgrs, std::string data = "")
	{
		const uint32_t DATA_WIDTH = 20;

		if (prgrs >= m_current)
		{
			uint32_t realProgress = m_current * m_width / m_100;
			uint32_t figureProgress = m_current * 100 / m_100;

			//string_t str(_T(''\033[32m'));
			std::string str;
			str.reserve(m_width);
			str.append(realProgress, '='); //'\u2588');
			str.append(abs((int32_t)m_width - (int32_t)str.size()), '.'); //'\u2591');
			//str.append(_T("\033[0m"));

			std::lock_guard<std::mutex> guard(m_mutex);

			if(data.empty())
				m_cout << "\r " << str << std::format(" {}% {:<{}}", figureProgress, "", DATA_WIDTH);
			else
				m_cout << "\r " << str << std::format(" {}% ({:.{}}){:<{}}", figureProgress, data, DATA_WIDTH, "", (DATA_WIDTH > data.size() ? DATA_WIDTH - data.size() : 0));
			m_current += m_delta;
		}

		return TProgressResult::OK;
	}
};

