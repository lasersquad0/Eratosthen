
#include <thread>
#include <iostream>
#include <vector>
#include <string>

#include "Utils.h"
#include "Ticks.h"
#include "CheckPrimes.h"
#include "PrimesFIO.h"


using namespace std;

std::string RemoveApo(const std::string& str);

#define NO_MORE_PRIMES (uint64_t)-1

void CheckPrimesThread(CheckPrimes* cp)
{
    auto end = cp->m_end; //make it local for speed

    constexpr int64_t PIECE = 1<<15; //32768 to be multible by cache line (64bytes) to avoid conflicts
    try
    {
        uint64_t* nxt;
        while ((nxt = cp->nextPrime(PIECE)) != nullptr) //NO_MORE_PRIMES)
        {
            assert(end > nxt);
            auto cnt = PIECE;
            if (cnt > end - nxt) cnt = end - nxt;
            while (true /*nxt < end*/)
            {
                //uint64_t div = cp->isPrimeNumber(nxt);
                //uint64_t div = cp->isPrimeNumber6(nxt);
                uint64_t div = cp->isPrimeNumber30(*nxt);
                if (div > 0)
                {
                    const lock_guard<mutex> lock(cp->m_mutex);
                    cp->m_compound.push_back(make_pair(*nxt, div));
                }

                ++nxt;
                if (--cnt == 0) break;
            }
        }
    }
    catch (std::exception& ex)
    {
        cout << "Exception caught in CheckPrimeThread." << endl;
        cout << ex.what() << endl;
    }
    catch (...)
    {
        cout << "UNKNOWN EXCEPTION caught in CheckPrimeThread. Application aborted." << endl;
    }
}

void PrintProgressThread(CheckPrimes* cp)
{
    auto loadedPrimes = cp->m_loadedPrimes; // make it local for better sharing between CPU cores
    try
    {
        while (cp->m_currIndex < loadedPrimes)
        {
            cp->printProgress();
            std::this_thread::sleep_for(1s);
        }
    }
    catch (std::exception& ex)
    {
        cout << "Exception caught in PrintProgressThread." << endl;
        cout << ex.what() << endl;
    }
    catch (...)
    {
        cout << "UNKNOWN EXCEPTION caught in PrintProgressThread." << endl;
    }
}

// returns "range" primes to calculate.
// begin of range - m_primes + idx, size of range - count
uint64_t* CheckPrimes::nextPrime(uint64_t count)
{
    uint64_t idx = m_currIndex.fetch_add(count);//, std::memory_order_relaxed);
    if (idx < m_loadedPrimes)
        return m_primes + idx;

    return nullptr; //NO_MORE_PRIMES;
}

// because m_currIndex points to index that will be requested next, % and speed may show incorrectly
// primes which are being calculated are counted as already calculated.
// this is visible when threads ask for big chunks of numbers.
void CheckPrimes::printProgress()
{
    uint64_t index = m_currIndex.load();
   
    auto ms = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - m_start).count();
    cout << format(ERA::GetLoc(), "Progress: {:.2f}% | Speed: {:L} primes/sec | Compound: {:L}      ",
        (float)(100 * index) / (float)m_loadedPrimes, (index - m_offset) * 1000 / ms, m_compound.size()) << "\r";
}

void CheckPrimes::checkPrimesInThreads(uint8_t numThreads)
{
    vector<thread*> thr;

    for (size_t i = 0; i < numThreads; i++)
    {
        thread* t = new thread(CheckPrimesThread, this);
        thr.push_back(t);
    }

    thread* tprgs = new thread(PrintProgressThread, this);

    for (size_t i = 0; i < numThreads; i++)
    {
        thr.at(i)->join();
    }

    tprgs->join();

    delete tprgs;

    for (size_t i = 0; i < numThreads; i++)
    {
        delete thr.at(i);
    }
}

void CheckPrimes::Start(const string& fileName, uint8_t numThreads)
{
    if(numThreads < 1)
        throw invalid_argument("Error: number of threads cannot be zero.\n");
    if (numThreads > 20)
        throw invalid_argument("Error: number of threads cannot be more than 24.\n");

    cout << format("{:<{}}: {}", "Requested operation", F_WIDTH, "Check numbers from file") << endl;
    cout << format("{:<{}}: {}", "Number of threads", F_WIDTH, numThreads) << endl;

    PRIMES_FILE_FORMAT frmt = PrimesFIO::GetFileType(fileName);
    if(frmt == PRIMES_FILE_FORMAT::none)
        throw invalid_argument(std::format("Error: cannot recognize file format '{}'.", fileName));
    
    cout << format("{:<{}}:'{}'", "File with numbers to check", F_WIDTH, fileName) << endl;

    string tp = FORMAT_TO_STR(frmt);
    cout << format(ERA::GetLoc(), "{:<{}}: {}", "File format", F_WIDTH, tp) << endl;
    cout << format(ERA::GetLoc(), "{:<{}}: {:L}", "Primes will be checked", F_WIDTH, m_capacity) << endl;
    cout << format(ERA::GetLoc(), "{:<{}}: {:L}", "Primes offset", F_WIDTH, m_offset) << endl;
    
    fstream f;
    f.open(fileName, ios::in | ios::binary);
    if (f.fail())
        throw invalid_argument("Error: cannot open file '" + fileName + "'\n");
 
    BuferedFileStream bfs(f);
    PrimesFIO pf;
    uint64_t lastPrime = 0;

    cout << "Loading numbers from file ..." << "\r";

    Ticks::Start("preload");

    switch (frmt)
    {
    case PRIMES_FILE_FORMAT::txt:        m_loadedPrimes = pf.LoadFromTXT(m_primes, m_capacity, f); break;
    case PRIMES_FILE_FORMAT::bin:        m_loadedPrimes = pf.LoadFromBIN(m_primes, m_capacity, f); break;
    case PRIMES_FILE_FORMAT::txtdiff:    m_loadedPrimes = pf.LoadFromTXTDiff(m_primes, m_capacity, lastPrime, f); break;
    case PRIMES_FILE_FORMAT::bindiff:    m_loadedPrimes = pf.LoadFromBINDiff(m_primes, m_capacity, lastPrime, f); break;
    case PRIMES_FILE_FORMAT::bindiffvar: m_loadedPrimes = pf.LoadFromBINDiffVar(m_primes, m_capacity, lastPrime, bfs); break;
    case PRIMES_FILE_FORMAT::none: throw invalid_argument(std::format("Error: cannot recognize file format '{}'.", fileName)); break;
    }

    cout << format(ERA::GetLoc(), "{:<{}}: {:L}", "Numbers loaded", F_WIDTH, m_loadedPrimes);
    if (m_loadedPrimes == m_capacity)
        cout << " (not all numbers loaded from a file, use -l option to load more numbers)" << endl;
    else
        cout << endl;

    cout << format("{:<{}}: {}", "File loading time", F_WIDTH, MillisecToStr<string>(Ticks::Finish("preload"))) << endl;

    Ticks::Start("checkprimes");

    m_start = chrono::high_resolution_clock::now();
    m_end = m_primes + m_loadedPrimes;
    m_currIndex = m_offset;
    checkPrimesInThreads(numThreads); // waits till all threads finish
    
    cout << format("{:<{}}: {}", "Primes Checking Time", F_WIDTH, MillisecToStr<string>(Ticks::Finish("checkprimes"))) << endl;
    cout << format(ERA::GetLoc(), "{:<{}}: {:L} primes/sec", "Average Speed", F_WIDTH, m_loadedPrimes * 1000 / Ticks::GetTick("checkprimes")) << endl;

    if (m_compound.size() == 0)
    {
        cout << "All numbers are PRIME. No compound numbers found" << endl;
    }
    else
    {
        cout << format(ERA::GetLoc(), "{:<{}}: {:L}", "COMPOUND numbers found", F_WIDTH, m_compound.size()) << endl;
        for (auto item : m_compound)
            cout << format(ERA::GetLoc(), "  {:L} | min divider {:L}", item.first, item.second) << endl;
    }
}

void CheckPrimes::CheckPrimesArray(const std::vector<std::string>& primes)
{
    cout << format("{:<{}}: {}", "Requested operation", F_WIDTH, "Check numbers from command line") << endl;
    cout << format("{:<{}}: {}", "Number of threads", F_WIDTH, 1) << endl;

    uint64_t compCnt = 0;
    uint64_t errCnt = 0;
    for (auto& sprime : primes)
    {
        try
        {
            string ssprime = RemoveApo(sprime);
            uint64_t prime = stoull(ssprime);
            auto div = isPrimeNumber30(prime);
            if (div > 0)
            {
                cout << std::format(ERA::GetLoc(), "{:L} - compound, min divider {:L}", prime, div) << endl;
                ++compCnt;
            }
        }
        catch (...)
        {
            cout << std::format("'{}' is not a number or out of range, bypassing it", sprime) << endl;
            ++errCnt;
        }
    }

    cout << format(ERA::GetLoc(), "{:<{}}: {:L}", "Count of checked numbers", F_WIDTH, primes.size()) << endl;
    cout << format(ERA::GetLoc(), "{:<{}}: {:L}", "Count of compound numbers", F_WIDTH, compCnt) << endl;
    if(errCnt > 0)
        cout << format(ERA::GetLoc(), "{:<{}}: {:L}", "Count of error numbers", F_WIDTH, errCnt) << endl;
}

// return 0 means 'no dividers' - number is prime
// any other value is a divider of prime
uint64_t CheckPrimes::isPrimeNumber(uint64_t number)
{
    if ((number == 2) || (number == 3))
        return 0;

    uint64_t middle = (uint64_t)sqrt(number) + 1;

    if ((number & 0x01) == 0) return 2; // divided by 2
    if ((number % 3) == 0) return 3; // divided by 3

    for (uint64_t i = 5; i < middle; i += 2)
    {
        if (number % i == 0) return i;
    }

    return 0;
}

// return 0 means 'no dividers' - number is prime
// any other value is a divider of prime
uint64_t CheckPrimes::isPrimeNumber6(uint64_t number)
{
    if ((number == 2) || (number == 3))
        return 0;

    uint64_t middle = (uint64_t)sqrt(number) + 2;

    if ((number & 0x01) == 0) return 2; // divided by 2
    if ((number % 3) == 0) return 3; // divided by 3

    for (uint64_t i = 6; i < middle; i += 6)
    {
        if (number % (i - 1) == 0) return i - 1;
        if (number % (i + 1) == 0) return i + 1;
    }

    return 0;
}

// return 0 means 'no dividers' - number is prime
// any other value is a divider of prime
uint64_t CheckPrimes::isPrimeNumber30(uint64_t number)
{
    if ((number == 2) || (number == 3) || (number == 5)) 
        return 0;

    uint64_t middle = (uint64_t)sqrt(number) + 1;

    if ((number & 0x01) == 0) return 2;
    if ((number % 3) == 0) return 3;
    if ((number % 5) == 0) return 5; 
    /*if ((number % 7) == 0) return 7; 
    if ((number % 11) == 0) return 11; 
    if ((number % 13) == 0) return 13; 
    if ((number % 17) == 0) return 17;
    if ((number % 19) == 0) return 19;
    if ((number % 23) == 0) return 23;
    if ((number % 29) == 0) return 29;
    */
    for (uint64_t i = 30; i < middle; i += 30)
    {
        if (number % (i + 1) == 0) return i + 1;
        if (number % (i + 7) == 0) return i + 7;
        if (number % (i + 11) == 0) return i + 11;
        if (number % (i + 13) == 0) return i + 13;
        if (number % (i + 17) == 0) return i + 17;
        if (number % (i + 19) == 0) return i + 19;
        if (number % (i + 23) == 0) return i + 23;
        if (number % (i + 29) == 0) return i + 29;
    }

    return 0;
}

std::string RemoveApo(const std::string& str)
{
    std::string res;
    res.reserve(str.size());
    for (size_t i = 0; i < str.length(); i++)
    {
        if (str[i] != '\'') res += str[i];
    }

    return res;
}
