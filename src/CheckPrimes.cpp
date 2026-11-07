
#include <thread>
#include <iostream>
#include <vector>
#include <string>
#include <format>

#include "Utils.h"
#include "Ticks.h"
#include "CheckPrimes.h"
#include "PrimesFIO.h"
#include "GenericBigInt.h"
#include "Parameters.h"

using namespace std;

//#define NO_MORE_PRIMES (uint64_t)-1

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
                //uint64_t div = cp->isPrimeNumber(*nxt);
                uint64_t div = IsPrimeNumber30(*nxt);
                //uint64_t div = cp->isPrimeNumber6(*nxt);
                //uint64_t div = cp->isPrimeNumber30(*nxt);
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
    auto loadedPrimes = cp->m_countPrimesToVerify; // make it local for better sharing between CPU cores
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
// begin of range - m_primesToVerify + idx, size of range - count
uint64_t* CheckPrimes::nextPrime(uint64_t count)
{
    uint64_t idx = m_currIndex.fetch_add(count);//, std::memory_order_relaxed);
    if (idx < m_countPrimesToVerify)
        return m_primesToVerify + idx;

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
        (float)(100 * index) / (float)m_countPrimesToVerify, (index - m_offset) * 1000 / ms, m_compound.size()) << "\r";
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

    auto frmt = PrimesFIO::GetFileType(fileName);
    if(frmt == PrimesFIO::PFF::none)
        throw invalid_argument(std::format("Error: cannot recognize file format '{}'.", fileName));
    
    cout << format("{:<{}}:'{}'", "File with numbers to check", F_WIDTH, fileName) << endl;

    string tp = PFFToStr(frmt);
    cout << format(ERA::GetLoc(), "{:<{}}: {}", "File format", F_WIDTH, tp) << endl;
    cout << format(ERA::GetLoc(), "{:<{}}: {}", "Primes asked to be checked", F_WIDTH, m_limitPrimes == 0? "<unlimited>" : format(ERA::GetLoc(), "{:L}", m_limitPrimes)) << endl;
    cout << format(ERA::GetLoc(), "{:<{}}: {:L}", "Primes start offset", F_WIDTH, m_offset) << endl;
    
    fstream f;
    f.open(fileName, ios::in | ios::binary);
    if (f.fail())
        throw invalid_argument("Error: cannot open file '" + fileName + "'\n");
 
    BuferedFileStream bfs(f);
    uint64_t lastPrime = 0;

    cout << "Loading numbers from file ..." << "\r";

    Ticks::Start("preload");

    switch (frmt)
    {
    case PrimesFIO::PFF::txt:        m_countPrimesToVerify = PrimesFIO::LoadFromTXT(m_primesToVerify, m_capacity, f); break;
    case PrimesFIO::PFF::bin:        m_countPrimesToVerify = PrimesFIO::LoadFromBIN(m_primesToVerify, m_capacity, f); break;
    case PrimesFIO::PFF::txtdiff:    m_countPrimesToVerify = PrimesFIO::LoadFromTXTDiff(m_primesToVerify, m_capacity, lastPrime, f); break;
    case PrimesFIO::PFF::bindiff:    m_countPrimesToVerify = PrimesFIO::LoadFromBINDiff(m_primesToVerify, m_capacity, lastPrime, f); break;
    case PrimesFIO::PFF::bindiffvar: m_countPrimesToVerify = PrimesFIO::LoadFromBINDiffVar(m_primesToVerify, m_capacity, lastPrime, bfs); break;
    case PrimesFIO::PFF::none: throw invalid_argument(std::format("Error: cannot recognize file format '{}'.", fileName)); break;
    }

    cout << format(ERA::GetLoc(), "{:<{}}: {:L}", "Primes actually loaded", F_WIDTH, m_countPrimesToVerify);
    
    if (m_limitPrimes == 0 && m_countPrimesToVerify == m_capacity)
        cout << " (not all numbers loaded from a file, use -l option to load more numbers)" << endl;
    else
        cout << endl;

    cout << format("{:<{}}: {}", "File loading time", F_WIDTH, MillisecToStr<string>(Ticks::Finish("preload"))) << endl;

    Ticks::Start("verifyprimes");

    m_start = chrono::high_resolution_clock::now();
    m_end = m_primesToVerify + m_countPrimesToVerify;
    m_currIndex = m_offset;
    checkPrimesInThreads(numThreads); // waits till all threads finish
    
    auto elapsed = Ticks::Finish("verifyprimes");
    cout << format("{:<{}}: {}", "Primes Checking Time", F_WIDTH, MillisecToStr<string>(elapsed)) << endl;
    cout << format(ERA::GetLoc(), "{:<{}}: {:L} primes/sec", "Average Speed", F_WIDTH, m_countPrimesToVerify * 1000 / elapsed) << endl;

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

    cout << format("{:<{}}: {}", "Time Spent", F_WIDTH, MillisecToStr<string>(elapsed)) << endl;
}

/*static int callbackImpl(const BigInt& curr, const BigInt& max)
{
    cout << "Progress... " << (float)toUInt64(curr * 100'0000 / max)/10'000 << "%      \r";
    return CALLBACK_OK;
}*/

uint64_t CheckPrimes::findMax(const std::vector<std::string>& primes)
{
    uint64_t maxPrime = 0;

    for (auto& sprime : primes)
    {
        string ssprime = RemoveApo(sprime);

        uint64_t prime = 0;
        auto [ptr, ec] = std::from_chars(ssprime.data(), ssprime.data() + ssprime.size(), prime);

        if (ec == std::errc{}) // success and fit into uint64_t
        {
            if (maxPrime < prime) maxPrime = prime;
        }
        else if (ec == std::errc::invalid_argument)
        {
            //do nothing, just bypass this item
        }
        // number is large use TBigInt
        else if (ec == std::errc::result_out_of_range)
        {
            return ULLONG_MAX;
        }
    }

    return maxPrime;
}


static int callbackImpl(const TBigInt& curr, const TBigInt& max)
{
    cout << "Progress... " << (float)(curr * 100'0000 / max).ToUInt() / 10'000 << "%      \r";
    return CALLBACK_OK;
}

void CheckPrimes::CheckPrimesArray(const std::vector<std::string>& primes, bool forceBigInt)
{
    cout << format("{:<{}}: {}", "Requested operation", F_WIDTH, "Check numbers from command line") << endl;

    auto maxPrime = findMax(primes);
    preloadPrimes(maxPrime);

    cout << format("{:<{}}: {}", "Numbers to check", F_WIDTH, primes.size()) << endl;

    Ticks::Start("checkprimes");

    uint64_t compCnt = 0;
    uint64_t errCnt = 0;
    for (auto& sprime : primes)
    {
        string ssprime = RemoveApo(sprime);

        if (forceBigInt) // force TBigInt
        {
            TBigInt bprime(ssprime);
            TBigInt bdiv = IsPrimeNumber30(bprime, m_prldPrimes, m_countPrldPrimes, callbackImpl);
            if (bdiv > 0)
            {
                cout << std::format(ERA::GetLoc(), "{:<{}}: compound, min divider {:L}", sprime, F_WIDTH, bdiv) << endl;
                ++compCnt;
            }
        }
        else // uint64_t
        {
            uint64_t prime = 0;
            char* end = ssprime.data() + ssprime.size();
            auto [ptr, ec] = std::from_chars(ssprime.data(), end, prime);

            
            if (ec == std::errc{}) // success and fit into uint64_t
            {
                if (ptr == end)
                {
                    auto div = IsPrimeNumber30(prime, m_prldPrimes, m_countPrldPrimes);
                    if (div > 0)
                    {
                        cout << std::format(ERA::GetLoc(), "{:<{}L}: compound, min divider {:L}", prime, F_WIDTH, div) << endl;
                        ++compCnt;
                    }
                }
                else
                {
                    cout << std::format("'{}' is not a number, bypassing it", sprime) << endl;
                    ++errCnt;
                }
            }
            else if (ec == std::errc::invalid_argument)
            {
                cout << std::format("'{}' is not a number, bypassing it", sprime) << endl;
                ++errCnt;
            }
            // number is large use TBigInt
            else if (ec == std::errc::result_out_of_range)
            {
                TBigInt bprime(ssprime);
                TBigInt bdiv = IsPrimeNumber30(bprime, m_prldPrimes, m_countPrldPrimes, callbackImpl);
                if (bdiv > 0)
                {
                    cout << std::format(ERA::GetLoc(), "{:<{}}: compound, min divider {:L}", sprime, F_WIDTH, bdiv) << endl;
                    ++compCnt;
                }
            }
        }
    }

    //TODO add calculated value instead of 50
    cout << string(50, ' ') << "\r";

    cout << format(ERA::GetLoc(), "{:<{}}: {:L}", "Prime Numbers", F_WIDTH, primes.size() - compCnt - errCnt) << endl;
    cout << format(ERA::GetLoc(), "{:<{}}: {:L}", "Compound Numbers", F_WIDTH, compCnt) << endl;
    if(errCnt > 0)
        cout << format(ERA::GetLoc(), "{:<{}}: {:L}", "Bypased Numbers", F_WIDTH, errCnt) << endl;

    auto elapsed = Ticks::Finish("checkprimes"); // for small numbers elapsed can be =0 (less than 1 millisecond)
    elapsed = elapsed == 0 ? 1 : elapsed;

    if(primes.size() * 1000 / elapsed == 0)
        cout << format(ERA::GetLoc(), "{:<{}}: {}", "Avg time for one number", F_WIDTH, MillisecToStr<string>(elapsed/primes.size())) << endl;
    else
        cout << format(ERA::GetLoc(), "{:<{}}: {:L} primes/sec", "Average Speed", F_WIDTH, primes.size() * 1000 / elapsed) << endl;
    cout << format("{:<{}}: {}", "Time spent", F_WIDTH, MillisecToStr<string>(elapsed)) << endl;
}

void CheckPrimes::preloadPrimes(uint64_t forNumber)
{
    Ticks::Start("preload");

    cout << format("{:<{}}:'{}'", "Pre-loaded primes file", F_WIDTH, Parameters::PRIMES_FILENAME) << endl;

    uint64_t maxPrimeToLoad = (uint64_t)(sqrt(forNumber)); // max prime number required to verify forNumber
    uint64_t numPrimesLessThan = (uint64_t)((double)maxPrimeToLoad / log(maxPrimeToLoad)); // approx number of primes from 2 to maxPrimeToLoad
    //cout << format("{:<{}}: ", "Need for verification", F_WIDTH) << numPrimesLessThan << " primes";
    //cout << " (approximate number of primes in a range from 2 to " << maxPrimeToLoad << "=sqrt(" << forNumber << ") )" << endl;
    
    // read at least 1000 primes from file because approximate formula for numPrimesLessThan does not work well for low numbers
    uint32_t prldPrimesSize = std::max(1000u, (uint32_t)(numPrimesLessThan * 1.2)); // give extra 20% for primes array size since formular above is not presize

    m_prldPrimes = new uint64_t[prldPrimesSize];

    // loading predefined primes that we will use for new primes calculations
    auto ft = PrimesFIO::GetFileType(Parameters::PRIMES_FILENAME);
    switch (ft)
    {
    case PrimesFIO::PFF::bindiffvar:
        m_countPrldPrimes = LoadPrimesFromBINDiffVar(Parameters::PRIMES_FILENAME, m_prldPrimes, prldPrimesSize, maxPrimeToLoad);
        break;
    case PrimesFIO::PFF::txt:
        m_countPrldPrimes = LoadPrimesFromTXTFile(Parameters::PRIMES_FILENAME, m_prldPrimes, prldPrimesSize, maxPrimeToLoad);
        break;
    default:
        throw invalid_argument(format("File format {} is not recognized or not supported for pre-loaded primes.", PFFToStr(ft)));
        //m_readPrimes = loadPrimesFromBINDiffVar(Parameters::PRIMES_FILENAME, m_prldPrimes, prldPrimesSize, maxPrimeToLoad);
    }

    cout << format("{:<{}}: ", "Primes actually loaded", F_WIDTH) << m_countPrldPrimes << endl;

    if (m_prldPrimes[m_countPrldPrimes - 1] < maxPrimeToLoad) // not enough primes in file, show warning 
        cout << "*WARNING*: There might be not enough primes in a file '" << Parameters::PRIMES_FILENAME << "' for calculation new primes!" << endl;

    cout << format("{:<{}}: {}", "Primes load time", F_WIDTH, MillisecToStr<string>(Ticks::Finish("preload"))) << endl;
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

/*
// return 0 means 'no dividers' - number is prime
// any other value is a divider of prime
uint64_t CheckPrimes::isPrimeNumber30(uint64_t number)
{
    // 0 in array means prime number (zero dividers)
    // non zero means the smallest divider
    const uint64_t known[30]{ 1,1,0,0,2,0,2,0,2,3,2,0,2,0,2,3,2,0,2,0,2,3,2,0,2,5,2,3,2,0 };
    
    if (number < 30) return known[number];
    //if ((number == 2) || (number == 3) || (number == 5)) 
    //    return 0;

    uint64_t middle = (uint64_t)sqrt(number) + 1;

    if ((number & 0x01) == 0) return 2;
    if ((number % 3) == 0) return 3;
    if ((number % 5) == 0) return 5; 
    if ((number % 7) == 0) return 7; 
    if ((number % 11) == 0) return 11; 
    if ((number % 13) == 0) return 13; 
    if ((number % 17) == 0) return 17;
    if ((number % 19) == 0) return 19;
    if ((number % 23) == 0) return 23;
    if ((number % 29) == 0) return 29;
    
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
*/
