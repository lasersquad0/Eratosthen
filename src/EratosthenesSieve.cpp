
#include <iostream>
#include <format>
#include <algorithm>
#include "EratosthenesSieve.h"
#include "string_utils.h"
#include "Utils.h"
#include "Ticks.h"
#include "Parameters.h"
#include "PrimesFIO.h" // for var_len_encode and var_len_decode functions

using namespace std;

// constant COMMA is used for saving primes in TXT format
const char* COMMA = ",";
const uint32_t PROGRESS_BAR_WIDTH = 70;

// initialize to DEFAULT values
EratosthenesSieve::EratosthenesSieve(): ConsoleProgressBar(cout, PROGRESS_BAR_WIDTH) 
{
    m_OutputFileType = PrimesFIO::PFF::txt;
    m_mode = CALC_MODE::optimized;
    m_realStart = DEFAULT_START;
    m_realLength = DEFAULT_LENGTH;
    m_realEnd = m_realStart + m_realLength;
    m_bitArray = nullptr;
    m_readPrimes = 0;
    m_prldPrimes = nullptr;
};

// makes output file name in format: primes - <start>-<length>.<format_ext>
string EratosthenesSieve::getOutputFilename()
{
    string fname = std::format("primes - {}{}-{}{}.{}", m_realStart / m_StartFactor, m_StartFactorSym, m_realLength / m_LengthFactor, m_LengthFactorSym, CALCMODE_TO_STR(m_mode));
    
    switch (m_OutputFileType)
    {
    case PrimesFIO::PFF::txt:       fname += ".txt";        break;
    case PrimesFIO::PFF::txtdiff:   fname += ".diff.txt";   break;
    case PrimesFIO::PFF::bin:       fname += ".bin";        break;
    case PrimesFIO::PFF::bindiff:   fname += ".diff.bin";   break;
    case PrimesFIO::PFF::bindiffvar:fname += ".diffvar.bin";break;
    }

    return fname;
}

void EratosthenesSieve::Calculate()
{

    cout << format(ERA::GetLoc(), "{:<{}}: {:L}", "Start", F_WIDTH, m_realStart) << endl;
    cout << format(ERA::GetLoc(), "{:<{}}: {:L}", "End",   F_WIDTH, m_realEnd) << endl;
    cout << format(ERA::GetLoc(), "{:<{}}: {:L}", "Length",F_WIDTH, m_realLength) << endl;
    cout << format(ERA::GetLoc(), "{:<{}}: {}", "Calculation algorithm", F_WIDTH, CALCMODE_TO_STR(m_mode)) << endl;
    cout << format(ERA::GetLoc(), "{:<{}}: {:L}", "Calculation threads", F_WIDTH, Parameters::THREADS) << endl;
    cout << format("{:<{}}: {}", "Output file format", F_WIDTH, PrimesFIO::PFFToStr(m_OutputFileType)) << endl;
    cout << format("{:<{}}:'{}'", "Output file name", F_WIDTH, getOutputFilename()) << endl;

    switch (m_mode)
    {
    case CALC_MODE::simple   : calculateSimple(); break;
    case CALC_MODE::optimized: calculateOptimum();break;
    case CALC_MODE::n6n1     : calculate6n1();    break;
    case CALC_MODE::n30x     : calculate30nx();   break;
    default: throw std::invalid_argument(std::format("Incorrect calc mode: {}", (uint32_t)m_mode).c_str());
    }
}

#define NO_MORE_PRIMES (-1LL)

void FindPrimesThread(EratosthenesSieve* es, StrikeoutComposedPred predStrikeoutComposed)
{
    //es->m_mutex.lock();
    //uint32_t id = es->m_threadIDs.fetch_add(1);
    //cout << "\rFindPrimesThread#" << id << " BEGIN" << endl;
    //es->m_mutex.unlock();

    uint64_t NPrime, currPrime, rem;
    const auto start = es->m_realStart; // AI recommended to have local var here

    while ((currPrime = es->nextPrime()) != NO_MORE_PRIMES)
    {
        NPrime = currPrime * currPrime; 
        if (NPrime < start)
        {
            //NPrime = es->m_realStart;
            rem = start % currPrime;
            //if (rem > 0)
            //    NPrime = es->m_realStart - rem + currPrime; // calc first number inside specified range that divisible to currPrime
            NPrime = start + ((currPrime - rem) % currPrime);

            if ( (currPrime != 2) && !(NPrime & 0x01ULL) ) NPrime += currPrime; // we do not store even numbers in bit array therefore we take next number for NPrime which is odd  
        }

        assert( (currPrime == 2) || (NPrime & 0x01ULL) == 1 );

        predStrikeoutComposed(NPrime, currPrime, es);

        /*j = (NPrime - 1) / 2;
        auto range = es->getRange(j);
        if (range != nullptr)
            range->first->lock();

        while (NPrime < es->m_realEnd)
        {
            es->m_bitArray->setTrue(j);
            NPrime += 2*currPrime; // NPrime + currPrime makes NPrime even. we have to bypass cases when NPrime is even. That is why we do NPrime + 2*currPrime
            j = (NPrime - 1) / 2;

            if (j > range->second.second)
            {
                range->first->unlock();
                range = es->getRange(j);
                if(range != nullptr) // range can be null one time when j becomes bigger than m_realEnd in this loop.  
                    range->first->lock(); // we jumped into next range. lock it and continue filtering primes in this range
            }
        }*/
    }

    //es->m_mutex.lock();
    //cout << endl << "\rFindPrimesThread#" << id << " END"; // << endl;
    //es->m_mutex.unlock();
}

uint64_t EratosthenesSieve::nextPrime()
{
   // static atomic_uint index(1);

    UpdateProgress(m_nextPrimeIndex);

    if (m_nextPrimeIndex < m_readPrimes)
        return m_prldPrimes[m_nextPrimeIndex.fetch_add(1)];

    //cout << "\r                                \r"; // cleaning up progress line of text

    return NO_MORE_PRIMES;
}

void EratosthenesSieve::FindPrimesInThreads(uint32_t numThreads, StrikeoutComposedPred predStrikeoutComposed)
{
    //m_mutex.lock(); // do not run threads untill all threads are created.
    
    vector<thread*> thr;
    for (size_t i = 0; i < numThreads; i++)
    {
        thread* t = new thread(FindPrimesThread, this, predStrikeoutComposed);
        thr.push_back(t);
    }

    //m_mutex.unlock();
    
    // wait until all threads finished
    for (size_t i = 0; i < numThreads; i++)
    {
        thr.at(i)->join();
    }

    for (size_t i = 0; i < numThreads; i++)
    {
        delete thr.at(i);
    }
}

void EratosthenesSieve::loadPrimes()
{
    Ticks::Start("preload");
    //auto start1 = chrono::high_resolution_clock::now();

    cout << format("{:<{}}:'{}'", "Pre-calc primes file", F_WIDTH, Parameters::PRIMES_FILENAME) << endl;

    uint64_t maxPrimeToLoad = (uint64_t)(sqrt(m_realStart + m_realLength)); // max prime num in a specified range 
    uint64_t numPrimesLessThan = (uint64_t)((double)maxPrimeToLoad / log(maxPrimeToLoad)); // approx number of prime numbers in a range 
    cout << format("{:<{}}: ", "Need for calculation", F_WIDTH) << numPrimesLessThan << " primes";
    cout << " (approximate number of primes in a range from 2 to " << maxPrimeToLoad << "=sqrt(" << m_realEnd << ") )" << endl;
    //cout << "There are *approximately* " << numPrimesLessThan << " primes less than " << maxPrimeToLoad << " = sqrt(" << m_realStart + m_realLength << ") we need for calculations." << endl;

    // read at least 1000 primes from file because approximate formula for numPrimesLessThan does not work well for low numbers
    uint32_t prldPrimesSize = std::max(1000u, (uint32_t)(numPrimesLessThan * 1.2)); // give extra 20% for primes array size since formular above is not presize

    //cout << "Array size for primes to be read from a file: " << prldPrimesSize << endl;

    m_prldPrimes = new uint64_t[prldPrimesSize];

    // loading predefined primes that we will use for new primes calculations
    auto ft = PrimesFIO::GetFileType(Parameters::PRIMES_FILENAME);
    switch (ft)
    {
    case PrimesFIO::PFF::bindiffvar:
        m_readPrimes = loadPrimesFromBINDiffVar(Parameters::PRIMES_FILENAME, m_prldPrimes, prldPrimesSize, maxPrimeToLoad);
        break;
    case PrimesFIO::PFF::txt:
        m_readPrimes = loadPrimesFromTXTFile(Parameters::PRIMES_FILENAME, m_prldPrimes, prldPrimesSize, maxPrimeToLoad);
        break;
    default:
        m_readPrimes = loadPrimesFromBINDiffVar(Parameters::PRIMES_FILENAME, m_prldPrimes, prldPrimesSize, maxPrimeToLoad);
    }

    cout << format("{:<{}}: ", "Primes actually loaded", F_WIDTH) << m_readPrimes << endl;
    
    if (m_prldPrimes[m_readPrimes - 1] < maxPrimeToLoad) // not enough primes in file, show warning 
        cout << "*WARNING*: There might be not enough primes in a file '" << Parameters::PRIMES_FILENAME << "' for calculation new primes!" << endl;

    //auto stop = chrono::high_resolution_clock::now();
    //cout << format("{:<{}}: {}", "Primes load time", F_WIDTH, MillisecToStr<string>(chrono::duration_cast<chrono::milliseconds>(stop - start1).count())) << endl;
    cout << format("{:<{}}: {}", "Primes load time", F_WIDTH, MillisecToStr<string>(Ticks::Finish("preload"))) << endl;
}


//Reading either entire file with primes or loading up to len primes, or till we met prime that greater than stopPrime value
uint32_t EratosthenesSieve::loadPrimesFromBINDiffVar(string& filename, uint64_t* primes, size_t len, uint64_t stopPrime)
{
    fstream f;
    f.exceptions(ifstream::failbit | ifstream::badbit);
    f.open(filename, ios::in | ios::binary);
    f.exceptions(ifstream::badbit);

    uint64_t diff;
    uint64_t lastPrime = 0;
    uint8_t buf[9];
    size_t maxSize;

    uint32_t cnt = 0;
    while (cnt < len)
    {
        maxSize = 0;

        while (true)
        {
            f.read((char*)(buf + maxSize), 1);
            if (f.eof()) break;
            if ((buf[maxSize++] & 0x80) == 0) break;
        }

        if (f.eof()) break;

        [[maybe_unused]] size_t res = var_len_decode(buf, maxSize, &diff);
        
        assert(res > 0);

        if (lastPrime > 2) diff *= 2; // multiply by 2 все: кроме первого числа(lastPrime==0), и когда lastPrime==2

        primes[cnt] = diff + lastPrime;
        lastPrime = primes[cnt++];

        if (lastPrime > stopPrime) break;
    }

    return cnt;
}

//Reading either entire file with primes or loading up to len primes, or till we met prime that greater than stopPrime value
uint32_t EratosthenesSieve::loadPrimesFromTXTFile(string& filename, uint64_t* primes, uint32_t len, uint64_t stopPrime)
{
    fstream f;
    f.exceptions(/*ifstream::failbit |*/ ifstream::badbit);
    f.open(filename, ios::in);

    string line;

    uint32_t cnt = 0;
    while (cnt < len)
    {
        getline(f, line, ',');
        if (f.eof()) break;

        primes[cnt] = atoll(line.c_str());

        if (primes[cnt] > stopPrime) break;
        cnt++;
    }

    f.close();

    return cnt;
}


// check validity of received cmd params and convert them into convenient formats
// also defines ranges for multi-threaded calculation
void EratosthenesSieve::SetParams(string alg, string oft, string& start, string& len)
{
    TrimAndUpper(alg);

    m_mode = STR_TO_CALCMODE(alg);
    if (m_mode == CALC_MODE::none)
        throw invalid_argument("Unsupported calculation mode specified.");

    checkStartParam(start); // generates an exception is anything wrong with start param value
    m_realStart = parseOption(start, m_StartFactorSym, m_StartFactor);

    // no need to call checkStartParam() for len, see below
    m_realLength = parseOption(len, m_LengthFactorSym, m_LengthFactor);

    m_realEnd = m_realStart + m_realLength;

    if (m_realLength > MAX_LEN)
        throw invalid_argument("Length argument is out of bounds (1...1000G)");

    TrimAndUpper(oft);

    m_OutputFileType = PrimesFIO::StrToPFF(oft);
    if (m_OutputFileType == PrimesFIO::PFF::none)
        throw invalid_argument("Unsupported output file type specified: " + oft);

    if (!std::filesystem::exists(Parameters::PRIMES_FILENAME))
        throw std::filesystem::filesystem_error("File with pre-calc primes not found", Parameters::PRIMES_FILENAME, 
                                                std::make_error_code(std::errc::no_such_file_or_directory));

}

// start param passed by value intentionally
// used only for checking validity of START parameter (valid format, value has to fit into uint64_t type)
void EratosthenesSieve::checkStartParam(string start)
{
    assert(start.size() > 0);

    // + remove any leading and traling spaces, just in case.
    TrimAndUpper(start);

    char sFactor = start.at(start.length() - 1);

    const string ErrStr = std::format("Start parameter value should be a number with factor symbol at the end (factor symbol is a single letter from list: {}). Current incorrect Start value: {}\n", FACTOR_SYMBOLS, start);

    if (FACTOR_SYMBOLS.find(sFactor) == string::npos)
        // if last sym is a digit then we have no factor (equivalent to having factor B)
        if (std::isdigit(sFactor))
            sFactor = 'B';
        else
            throw invalid_argument(ErrStr);
    else
        start = start.substr(0, start.length() - 1); // remove letter G at the end. Or T, or P, or M or K or B.

    uint64_t dStart;
    try
    {
        dStart = stoull(start);
    }
    catch (...)
    {
        throw invalid_argument("Start parameter value should be a number: '" + start + "'.\n");
    }

    // used below in switch()
    invalid_argument e("Start parameter value is out of bounds (0..18'400P)");

    switch (sFactor)
    {
    case 'B': break;
    case 'K': if (dStart > MAX_START_K) throw e; break;
    case 'M': if (dStart > MAX_START_M) throw e; break;
    case 'G': if (dStart > MAX_START_G) throw e; break;
    case 'T': if (dStart > MAX_START_T) throw e; break;
    case 'P': if (dStart > MAX_START_P) throw e; break;
    default:
        throw invalid_argument(ErrStr);
    }
}

// parses one value either m_realStart or m_realLength in format with Factor multiplier: B, K, M, G, T, P
// examples: 100G, 5T, 20000G, 400M, 0B
// returns back symFactor and factor values, and real calculated value as uint64 as a result of function call 
uint64_t EratosthenesSieve::parseOption(string opt, char& symFactor, uint64_t& factor)
{
    assert(opt.size() > 0);

    // remove any leading and traling spaces, just in case.
    TrimAndUpper(opt);

    char sFactor = opt.at(opt.length() - 1); // we need symFACTOR later for output filename

    if (FACTOR_SYMBOLS.find(sFactor) == string::npos)
        // if last sym is a digit then we have no factor (equivalent to having factor B)
        if (std::isdigit(sFactor))
            sFactor = 'B';
        else
            throw invalid_argument("Incorrect command line parameter '" + opt + "'.\n");
    else
        opt = opt.substr(0, opt.length() - 1); // remove letter G at the end. Or T, or P, or M, or K or B.
    
    uint64_t dStart;
    try
    {
        dStart = stoull(opt);
    }
    catch (...)
    {
        throw invalid_argument("Parameter value should be a number: '" + opt + "'.\n");
    }

    uint64_t ffactor = 0;
    switch (sFactor)
    {
    case 'B': ffactor = FACTOR_B; break;
    case 'K': ffactor = FACTOR_K; break;
    case 'M': ffactor = FACTOR_M; break;
    case 'G': ffactor = FACTOR_G; break;
    case 'T': ffactor = FACTOR_T; break;
    case 'P': ffactor = FACTOR_P; break;
    default:
        throw invalid_argument("Param value should be a number with factor (one letter from list: BKMGTP). Factor is incorrect here: '" + opt + "'.\n");
    }

    symFactor = sFactor;
    factor = ffactor;

    return dStart * ffactor;
}

void EratosthenesSieve::defineRanges(uint64_t start, uint64_t length)
{
    if (Parameters::THREADS < 2) return; // no threads needed => no ranges needed

    uint64_t k = 1;
    uint64_t rangesCount = Parameters::THREADS * k;
    uint64_t rangeLen = length / rangesCount;

    //m_ranges.push_back(RangeItem(new SyncType(), RangePair(start, start + rangeLen - 1))); //TODO this needs a call delete somewhere to free memory

    uint64_t currRange = 0; //rangeLen;

    for (size_t i = 0; i < rangesCount - 1; i++)
    {
        m_ranges.push_back(RangeItem(new SyncType(), RangePair(start + currRange, start + currRange + rangeLen - 1)));
        currRange += rangeLen;
    }

    m_ranges.push_back(RangeItem(new SyncType(), RangePair(start + currRange, start + length - 1)));
}

EratosthenesSieve::RangeItem* EratosthenesSieve::getRange(uint64_t v)
{
    assert(v >= m_bitArray->ibegin());
    
    if (v >= m_bitArray->iend()) 
        return nullptr;

    for (size_t i = 0; i < m_ranges.size(); i++)
    {
        RangePair& p = m_ranges[i].second;
        if ((v >= p.first) && (v <= p.second))
            return &m_ranges[i];
    }
    return nullptr;
}


// stores bits only for 30N+X numbers which covers all prime numbers. where X is one pf the following remainders 1, 7, 11, 13, 17, 19, 23, 29
// if current number is 30N+X format then set bit in bit field to mark this number as compound
// if current number is NOT 30N+X format then do nothing and go to the next number 
// calculate30nx works slightly faster than Optimum method AND requires 1.5 times less memory than Optimum
void EratosthenesSieve::calculate30nx()
{
    static int BIT_TO_REM[] = { 1, 7, 11, 13, 17, 19, 23, 29 };

    static int REM_TO_BIT[] = {
        -1, 0,
            -1, -1, -1, -1, -1, 1,
            -1, -1, -1, 2,
            -1, 3,
            -1, -1, -1, 4,
            -1, 5,
            -1, -1, -1, 6,
            -1, -1, -1, -1, -1, 7,
        };

    // 8 reminders we need to store, that is why we multiply by 8 here
    const uint64_t START = (m_realStart / 30ull) << 3; // value is in bits
    const uint64_t END   = ((m_realEnd + 29)/30ull) << 3; // value is in bits

    defineRanges(START, END);
    m_nextPrimeIndex = 1; // bypass 2

    loadPrimes(); // loading primes from file, result - in m_prldPrimes array. 

    //uint8_t* bitArray = (uint8_t*)calloc(LENGTH, sizeof(uint8_t));
    m_bitArray = new SegmentedArray(START, END);

    //auto start1 = chrono::high_resolution_clock::now();
    Ticks::Start("n30xCalc");

    StartProgress(m_readPrimes);

    if (Parameters::THREADS > 1)
    {
        StrikeoutComposedPred predStrikeoutComposed = [this](uint64_t NPrime, uint64_t currPrime, EratosthenesSieve*)
            {
                assert(currPrime > 1);
                assert(NPrime > 1);

                std::lldiv_t divres;
                uint64_t index = 0;
 
                // we need to lock a range
                // for that we need to have index in compressed array
                // if bit =-1 that means composed number that is not stored in array => no index exists in array for that number
                // we check numbers one-by-one till find first number that has index in array
                while (NPrime < m_realEnd)
                {
                    divres = std::div(NPrime, 30ll);
                    int bit = REM_TO_BIT[divres.rem];

                    if (bit >= 0)
                    {
                        index = (divres.quot << 3) + bit;
                        break;
                    }
                    NPrime += 2 * currPrime;
                }

                if (NPrime >= m_realEnd) return;

                auto range = getRange(index);
                if (range != nullptr)
                    range->first->lock();
                               
                m_bitArray->setTrue(index); // set 'composed' to index calculated in loop above

                while (NPrime < m_realEnd)
                {
                    divres = std::div(NPrime, 30ll);
                    //assert(divres.quot >= START);
                    int bit = REM_TO_BIT[divres.rem];

                    if (bit >= 0)
                    {
                        index = (divres.quot << 3) + bit;

                        if (index > range->second.second)
                        {
                            range->first->unlock();
                            range = getRange(index);
                            if (range != nullptr) // range can be null one time when index becomes bigger than iend() in this loop.  
                                range->first->lock(); // we jumped into next range. lock it and continue filtering primes in this range
                        }
                        m_bitArray->setTrue(index); 
                    }
                    NPrime += 2 * currPrime;  // NPrime + currPrime makes NPrime even. we have to bypass cases when NPrime is even. That is why we do NPrime + 2*currPrime
                }

                range->first->unlock(); // unlock last range
            };

        FindPrimesInThreads(Parameters::THREADS, predStrikeoutComposed); // waits until all threads are finished
    }
    else
    {
        std::lldiv_t divres;
        uint64_t NPrime, currPrime;

        for (uint32_t i = 1; i < m_readPrimes; ++i)
        {
            UpdateProgress(i);

            currPrime = m_prldPrimes[i];
            
            NPrime = currPrime * currPrime; // start to strike out composed nums from p^2 because variants like p*3, p*5, p*7 have already visited in previous iterations 
            
            if (NPrime < m_realStart)
            {
                //NPrime = m_realStart;
                //rem = m_realStart % currPrime;
                //if (rem > 0)
                //    NPrime = m_realStart - rem + currPrime; // calc first number inside specified range that divisible to currPrime
                NPrime = m_realStart + ((currPrime - m_realStart % currPrime) % currPrime);

                if ((NPrime & 0x01ULL) == 0) NPrime += currPrime; // we do not store even numbers in bit array therefore we take next number for NPrime which is odd  
            }
            
            assert((NPrime & 0x01ULL) != 0); // NPrime must be odd here

            while (NPrime < m_realEnd)
            {
                divres = std::div(NPrime, 30ll);

                //assert(divres.quot >= START);

                int bit = REM_TO_BIT[divres.rem];
                
                if (bit >= 0)
                {
                    uint64_t index = (divres.quot << 3) + bit;
                    m_bitArray->setTrue(index);
                    //bitArray[divres.quot - START] |= (1ll << bit);
                }

                NPrime += 2*currPrime;  // NPrime + currPrime makes NPrime even. we have to bypass cases when NPrime is even. That is why we do NPrime + 2*currPrime
            }
        }
    }

    if (m_realStart < 7) m_bitArray->setTrue(0);

    FinishProgress();

    cout << format("{:<{}}: {}", "Calculation time", F_WIDTH, MillisecToStr<string>(Ticks::Finish("n30xCalc"))) << endl;

    ///////////////////////////////////////////////////////////////////////////

    uint64_t lastPrime = 0; // kind of global var between predStartTXT and predTXTDiff

    StartPrimePred predStartTXT = [this, &lastPrime](ostream& stream)
        {
            assert(stream.tellp() == 0); // empty stream should come here
            uint64_t cnt = 0;

            if (m_realStart < 3)
            {
                stream << "2,"; // 2, 3 and 5 are not included into calc because they are out of n30x formular, adding them manually here
                cnt++;
            }
            if (m_realStart < 4)
            {
                //lastPrime = 3;
                stream << "3,";
                cnt++;
            }
            if (m_realStart < 6)
            {
                lastPrime = 5;
                stream << "5,";
                cnt++;
            }

            return cnt;
        };

    StartPrimePred predStartTXTDiff = [this, &lastPrime](ostream& stream)
        {
            assert(stream.tellp() == 0); // empty stream should come here
            uint64_t cnt = 0;

            if (m_realStart < 3)
            {
                stream << "2,"; // 2,3 and 5 are not included into calc because they are out of n30x formular, adding them manually here
                stream << "1,"; // 1 is a diff between 3 and 2
                stream << "1,"; // 2 is a (diff between 5 and 3) / 2
                lastPrime = 5;
                cnt += 3;
            }
            else if (m_realStart < 4)
            {
                stream << "3,";
                stream << "1,"; // 2 is a (diff between 5 and 3) / 2
                lastPrime = 5;
                cnt += 2;
            }
            else if (m_realStart < 6)
            {
                stream << "5,";
                lastPrime = 5;
                cnt++;
            }
            
            return cnt;
        };

    SavePrimeToStreamPred predTXT = [this](ostream& stream, uint64_t index)
        {
            uint64_t prime;
            prime = (index >> 3) * 30 + BIT_TO_REM[index & 0x07];

            // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
            if ((prime >= m_realStart) && (prime < m_realEnd))
            {
                stream << prime << COMMA;
                return true;
            }
            return false;
        };

    SavePrimeToStreamPred predTXTDiff = [this, &lastPrime](ostream& stream, uint64_t index)
        {
            uint64_t prime;
            prime = (index >> 3) * 30 + BIT_TO_REM[index & 0x07];

            assert(/*prime == 0 ||*/ (prime > lastPrime));
            assert(lastPrime == 0 || (prime - lastPrime < UINT16_MAX));

            // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
            if (prime >= m_realStart && prime < m_realEnd)
            {
                if (lastPrime > 2)
                    stream << (prime - lastPrime) / 2 << COMMA;
                else
                    stream << prime - lastPrime << COMMA;
                //stream << prime - lastPrime << COMMA;
                lastPrime = prime;
                return true;
            }
            return false;
        };


    StartPrimePred predStartBIN = [this, &lastPrime](ostream& stream)
        {
            assert(sizeof(lastPrime) == sizeof(uint64_t));
            assert(stream.tellp() == 0); // empty stream should come here
            uint64_t cnt = 0;

            // 2,3 and 5 are not included into calc because they are out of n30x formular, adding them manually here
            if (m_realStart < 3)
            {
                lastPrime = 2;
                stream.write((char*)&lastPrime, sizeof(lastPrime));
                cnt++;
            }
            if (m_realStart < 4)
            {
                lastPrime = 3;
                stream.write((char*)&lastPrime, sizeof(lastPrime));
                cnt++;
            }
            if (m_realStart < 6)
            {
                lastPrime = 5;
                stream.write((char*)&lastPrime, sizeof(lastPrime));
                cnt++;
            }

            return cnt;
        };

    StartPrimePred predStartBINDiff = [this, &lastPrime](ostream& stream)
        {
            assert(sizeof(lastPrime) == sizeof(uint64_t));
            assert(stream.tellp() == 0); // empty stream should come here
            uint64_t cnt = 0;

            // 2,3 and 5 are not included into calc because they are out of n30x formular, adding them manually here
            if (m_realStart < 3)
            {
                lastPrime = 2;
                stream.write((char*)&lastPrime, sizeof(lastPrime));
                uint16_t diff = 1; // diff between 2 and 3
                stream.write((char*)&diff, sizeof(diff));
                diff = 1; // (diff between 3 and 5) / 2
                stream.write((char*)&diff, sizeof(diff));
                cnt += 3;
                lastPrime = 5;
            }
            else if (m_realStart < 4)
            {
                lastPrime = 3;
                stream.write((char*)&lastPrime, sizeof(lastPrime));
                uint16_t diff = 1; // (diff between 3 and 5) / 2
                stream.write((char*)&diff, sizeof(diff));
                lastPrime = 5;
                cnt += 2;
            }
            else if (m_realStart < 6)
            {
                lastPrime = 5;
                stream.write((char*)&lastPrime, sizeof(lastPrime));
                cnt++;
            }

            return cnt;
        };

    SavePrimeToStreamPred predBIN = [this](ostream& stream, uint64_t index)
        {
            uint64_t prime;
            prime = (index >> 3) * 30 + BIT_TO_REM[index & 0x07];

            // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
            if ((prime >= m_realStart) && (prime < m_realEnd))
            {
                stream.write((char*)&prime, sizeof(prime));
                return true;
            }
            return false;
        };

    SavePrimeToStreamPred predBINDiff = [this, &lastPrime](ostream& stream, uint64_t index)
        {
            uint64_t prime;
            prime = (index >> 3) * 30 + BIT_TO_REM[index & 0x07];

            assert(/*prime == 0 ||*/ (prime > lastPrime));
            assert(lastPrime == 0 || (prime - lastPrime < UINT16_MAX));

            // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
            if (prime >= m_realStart && prime < m_realEnd)
            {
                if (lastPrime == 0)
                {
                    stream.write((char*)&prime, sizeof(prime));
                }
                else
                {
                    uint16_t diff = (uint16_t)(prime - lastPrime);
                    assert(lastPrime < 3 || ((diff & 0x01ULL) == 0ULL)); // check that diff is really even
                    if (lastPrime > 2) diff /= 2;
                    stream.write((char*)&diff, sizeof(diff));
                }

                lastPrime = prime;
                return true;
            }
            return false;
        };

    StartPrimePred predStartBINDiffVar = [this, &lastPrime](ostream& stream)
        {
            assert(stream.tellp() == 0); // empty stream should come here
            uint8_t buf[9];
            uint64_t cnt = 0;

            // 2,3 and 5 are not included into calc because they are out of n30x formular, adding them manually here
            if (m_realStart < 3)
            {
                size_t len = var_len_encode(buf, 2);
                stream.write((char*)buf, len);

                len = var_len_encode(buf, 1); // diff between 2 and 3
                stream.write((char*)buf, len);
                len = var_len_encode(buf, 1); // diff between 3 and 5 divided by 2
                stream.write((char*)buf, len);
                lastPrime = 5;
                cnt += 3;
            }
            else if (m_realStart < 4)
            {
                size_t len = var_len_encode(buf, 3);
                stream.write((char*)buf, len);
                len = var_len_encode(buf, 1); // diff between 3 and 5 divided by 2
                stream.write((char*)buf, len);
                lastPrime = 5;
                cnt += 2;
            }
            else if (m_realStart < 6)
            {
                size_t len = var_len_encode(buf, 5);
                stream.write((char*)buf, len);
                lastPrime = 5;
                cnt++;
            }

            return cnt;
        };

    SavePrimeToStreamPred predBINDiffVar = [this, &lastPrime](ostream& stream, uint64_t index)
        {
            uint64_t prime;
            prime = (index >> 3) * 30 + BIT_TO_REM[index & 0x07];

            assert(/*prime == 0 ||*/ (prime > lastPrime));
            assert(lastPrime == 0 || (prime - lastPrime < UINT16_MAX));

            // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
            if ((prime >= m_realStart) && (prime < m_realEnd))
            {
                uint8_t buf[9];
                uint64_t diff;

                diff = prime - lastPrime; // diff is always even. that is why we can divide it by 2. more values will fit into single byteт.
                assert(lastPrime == 0 || ((diff & 0x01ULL) == 0ULL)); // check that diff is really even
                //TODO remove it?? because assert above
                if (lastPrime > 0 /*> 2*/) diff /= 2; // bypass div by 2 when lastPrime=0 and lastPrime=2 because diff between 2 and 3 = 1

                size_t len = var_len_encode(buf, diff);
                stream.write((char*)buf, len);

                lastPrime = prime;
                return true;
            }
            return false;
        };

  /*  size_t chunk = 10'000'000;

    fstream f;
    f.exceptions(ifstream::failbit | ifstream::badbit);
    f.sync_with_stdio(false);
    f.open(outputFilename, ios::out | ios::binary);

    string s;
    s.reserve(chunk); // store to file by 10M chunks

    uint64_t cntPrimes = 0;
    uint64_t prime = 0;

    if (m_realStart < 3) s.append("2,");
    if (m_realStart < 4) s.append("3,");
    if (m_realStart < 6) s.append("5,");

    for (uint64_t i = m_bitArray->ibegin(); i < m_bitArray->iend(); ++i)
    {
        if (!m_bitArray->get(i))
        {
            prime = (i >> 3) * 30 + BIT_TO_REM[i & 0x07];
            
            //sometimes prime may be out of requested range, add to file only primes that fit into range
            if ((prime >= m_realStart) && (prime < m_realEnd))
            {
                cntPrimes++;
                s.append(to_string(prime));
                s.append(",");
            }

            if (s.size() > chunk - 25) // when we are close to capacity but еще НЕ перепрыгнули ее
            {
                f.write(s.c_str(), s.length());
                s.clear(); // очищаем строку но оставляем capacity
            }
        }
    }
    */

    /*
    for (uint64_t i = 0; i < LENGTH; ++i)
    {
        for (uint8_t bit = 0; bit < 8; bit++)
        {
            if ( (bitArray[i] & (1ll << bit)) == 0 )
            {
                prime = (i + START) * 30 + BIT_TO_REM[bit];

                //sometimes prime may be out of requested range, add to file only primes that fit into range
                if ((prime >= m_realStart) && (prime < m_realEnd))
                {
                    cntPrimes++;
                    s.append(to_string(prime));
                    s.append(",");
                }

                if (s.size() > chunk - 25) // when we are close to capacity but еще НЕ перепрыгнули ее
                {
                    f.write(s.c_str(), s.length());
                    s.clear(); // очищаем строку но оставляем capacity
                }
            }
        }
    }
    */
    //f.write(s.c_str(), s.length());
    //f.close();
    
    if (Parameters::SAVEFILE)
    {
        cout << "Saving file...";

        //auto start2 = chrono::high_resolution_clock::now();
        Ticks::Start("save");
        
        uint64_t primesSaved = 0;
        string outputFilename = getOutputFilename();

        switch (m_OutputFileType)
        {
        case PrimesFIO::PFF::txt:       primesSaved = saveAsTXT(outputFilename, predStartTXT, predTXT); break;
        case PrimesFIO::PFF::txtdiff:   primesSaved = saveAsTXT(outputFilename, predStartTXTDiff, predTXTDiff); break;
        case PrimesFIO::PFF::bin:       primesSaved = saveAsBIN(outputFilename, predStartBIN, predBIN); break;
        case PrimesFIO::PFF::bindiff:   primesSaved = saveAsBIN(outputFilename, predStartBINDiff, predBINDiff); break;
        case PrimesFIO::PFF::bindiffvar:primesSaved = saveAsBIN(outputFilename, predStartBINDiffVar, predBINDiffVar); break;
        }

        cout << " Saved." << endl;

        cout << format("{:<{}}: ","Primes saved to file", F_WIDTH) << primesSaved << endl;
        cout << format("{:<{}}: ", "File size", F_WIDTH) << std::filesystem::file_size(outputFilename) << " bytes" << endl;
        cout << format("{:<{}}: {}", "File saving time", F_WIDTH, MillisecToStr<string>(Ticks::Finish("save"))) << endl;
        cout << format("{:<{}}: {}", "Total time", F_WIDTH, MillisecToStr<string>(Ticks::GetTick("n30xCalc") + Ticks::GetTick("save"))) << endl;
    }

    PrintMemoryUsage(); // it should be BEFORE freeing big arrays such as m_prldPrimes and m_bitArray

    delete[] m_prldPrimes;
    delete m_bitArray;
}

// stores bits only for 6n+1 and 6n+5 numbers which covers all prime numbers
// if current number is 6n+1 or 6n+5 then set bit in bit field to mark this number as compound
// if current number is NOT 6n+1 or 6n+5 then do nothing and go to the next number 
// calculate6n1 works about two times slower than Optimum method BUT requires 1.5 times less memory than Optimum
void EratosthenesSieve::calculate6n1()
{
    uint64_t START = m_realStart / 3ULL; 
    uint64_t END   = m_realEnd / 3ULL + 1;

    defineRanges(START, END);
    m_nextPrimeIndex = 1; // bypass prime '2'

    loadPrimes(); // loading primes from file, result - in m_prldPrimes array. 

    m_bitArray = new SegmentedArray(START, END);

    //auto start1 = chrono::high_resolution_clock::now();
    Ticks::Start("6n1Calc");

    StartProgress(m_readPrimes);

    if (Parameters::THREADS > 1)
    {
        StrikeoutComposedPred predStrikeoutComposed = [this](uint64_t NPrime, uint64_t currPrime, EratosthenesSieve*)
            {
                std::lldiv_t divres;
                uint64_t index = 0;
                bool found = false;

                // we need to lock a range
                // for that we need to call index in compressed array
                // divres.rem =0,2,3,4 means composed number that is not stored in array => no index exists in array for that number
                // we check numbers one-by-one till find first number that has index in array
                while (NPrime < m_realEnd)
                {
                    divres = std::div(NPrime, 6ll);
                    if (divres.rem == 1)
                    {
                        index = (divres.quot << 1);
                        found = true;
                        break;
                    }
                    else if (divres.rem == 5)
                    {
                        index = (divres.quot << 1) + 1;
                        found = true;
                        break;
                    }
                    NPrime += 2 * currPrime;
                }

                if (NPrime >= m_realEnd) return;

                // found first NPrime that has an index in m_bitArray
                // then lock appropriate range
                assert(found);

                auto range = getRange(index);
                if (range != nullptr)
                    range->first->lock();

                m_bitArray->setTrue(index); // setTrue() we can call only after locking the range

                while (NPrime < m_realEnd)
                {
                    divres = std::div(NPrime, 6ll);
                    if (divres.rem == 1)
                    {
                        index = (divres.quot << 1);
                        if (index > range->second.second)
                        {
                            range->first->unlock();
                            range = getRange(index);
                            if (range != nullptr) // range can be null one time when index becomes bigger than iend() in this loop.  
                                range->first->lock(); // we jumped into next range. lock it and continue filtering primes in this range
                        }
                        m_bitArray->setTrue(index);
                    }
                    else if (divres.rem == 5)
                    {
                        index = (divres.quot << 1) + 1;
                        if (index > range->second.second)
                        {
                            range->first->unlock();
                            range = getRange(index);
                            if (range != nullptr) // range can be null one time when index becomes bigger than iend() in this loop.  
                                range->first->lock(); // we jumped into next range. lock it and continue filtering primes in this range
                        }
                        m_bitArray->setTrue(index);
                    }
                    NPrime += 2 * currPrime; // NPrime + currPrime makes NPrime even. we have to bypass cases when NPrime is even. That is why we do NPrime + 2*currPrime
                }

                range->first->unlock(); // unlock last range before taking next prime
            };

        FindPrimesInThreads(Parameters::THREADS, predStrikeoutComposed); // waits until all threads are finished
    }
    else
    {
        uint64_t NPrime, currPrime;
        std::lldiv_t divres;
        for (uint32_t i = 1; i < m_readPrimes; ++i)
        {
            UpdateProgress(i);

            currPrime = m_prldPrimes[i];

            NPrime = currPrime * currPrime; // start to strike out composed nums from p^2 because variants like p*3, p*5, p*7 have already visited in previous iterations 
            
            if (NPrime < m_realStart)
            {
                //NPrime = m_realStart;
                //rem = m_realStart % currPrime;
                //if (rem > 0)
                //    NPrime = m_realStart - rem + currPrime; // calc first number inside specified range that divisible to currPrime
                NPrime = m_realStart + ((currPrime - m_realStart % currPrime) % currPrime);

                if ((NPrime & 0x01ULL) == 0) NPrime += currPrime; // we do not store even numbers in bit array therefore we take next number for NPrime which is odd  
            }

            assert((NPrime & 0x01ULL) != 0); // NPrime must be odd here

            //NPrime must be odd before starting this loop
            while (NPrime < m_realEnd)
            {
                divres = std::div(NPrime, 6ll);
                if (divres.rem == 1)
                {
                    m_bitArray->setTrue(divres.quot << 1);
                }
                else if (divres.rem == 5)
                {
                    m_bitArray->setTrue((divres.quot << 1) + 1);
                }

                NPrime += 2*currPrime; // NPrime + currPrime makes NPrime even. we have to bypass cases when NPrime is even. That is why we do NPrime + 2*currPrime
            }
        }
    }

    if (m_realStart < 3) m_bitArray->setTrue(0); // заглушка если генерим числа начиная с 0. что бы '1' не попало в простые
    
    FinishProgress();

    cout << format("{:<{}}: {}", "Calculation time", F_WIDTH, MillisecToStr<string>(Ticks::Finish("6n1Calc"))) << endl;

    /////////////////////////////////////////////////////////////////////////

    uint64_t lastPrime = 0; // kind of global var between predStartTXT and predTXTDiff

    StartPrimePred predStartTXT = [this, &lastPrime](ostream& stream)
        {   
            assert(stream.tellp() == 0); // empty stream should come here
            uint64_t cnt = 0;
            if (m_realStart < 3)
                stream << "2,", cnt++; // 2 and 3 are not included into calc because they are out of 6n+1/6n+5 formular, adding them manually here
            if (m_realStart < 4)
            {
                lastPrime = 3;
                stream << "3,";
                cnt++;
            }
            
            return cnt;
        };

    StartPrimePred predStartTXTDiff = [this, &lastPrime](ostream& stream)
        {
            assert(stream.tellp() == 0); // empty stream should come here
            uint64_t cnt = 0;
            if (m_realStart < 3)
            {
                stream << "2,"; // 2 and 3 are not included into calc because they are out of 6n+1/6n+5 formular, adding them manually here
                stream << "1,"; // 1 is a diff between 3 and 2
                lastPrime = 3;
                cnt += 2;
            }
            else if (m_realStart == 3)
            {
                stream << "3,";
                lastPrime = 3;
                cnt++;
            }

            return cnt;
        };

    SavePrimeToStreamPred predTXT = [this](ostream& stream, uint64_t index)
        {
            uint64_t prime;
            if ((index & 0x01) == 0)
                prime = index * 3 + 1; // index is even, full formular (index/2)*6+1
            else
                prime = (index - 1) * 3 + 5; // index is odd, full formular ((index-1)/2)*6+5

            // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
            if ((prime >= m_realStart) && (prime < m_realEnd))
            {
                stream << prime << COMMA;
                return true;
            }
            return false;
        };

    SavePrimeToStreamPred predTXTDiff = [this, &lastPrime](ostream& stream, uint64_t index)
        {
            uint64_t prime;
            if ((index & 0x01) == 0)
                prime = index * 3 + 1; // index is even, full formular (index/2)*6+1
            else
                prime = (index - 1) * 3 + 5; // index is odd, full formular ((index-1)/2)*6+5

            assert(prime > lastPrime);
            assert(lastPrime == 0 || (prime - lastPrime < UINT16_MAX));

            // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
            if (prime >= m_realStart && prime < m_realEnd)
            {
                if (lastPrime > 2)
                    stream << (prime - lastPrime) / 2 << COMMA;
                else
                    stream << prime - lastPrime << COMMA;
                //stream << prime - lastPrime << COMMA;
                lastPrime = prime;
                return true;
            }
            return false;
        };

    StartPrimePred predStartBIN = [this, &lastPrime](ostream& stream)
        {
            assert(sizeof(lastPrime) == sizeof(uint64_t));
            assert(stream.tellp() == 0); // empty stream should come here
            uint64_t cnt = 0;
 
            // 2 and 3 are not included into calc because they are out of 6n+1/6n+5 formular, adding them manually here
            if (m_realStart < 3)
            {
                lastPrime = 2;
                stream.write((char*)&lastPrime, sizeof(lastPrime));
                cnt++; 
            }
            if (m_realStart < 4)
            {
                lastPrime = 3;
                stream.write((char*)&lastPrime, sizeof(lastPrime));
                cnt++;
            }

            return cnt;
        };

    StartPrimePred predStartBINDiff = [this, &lastPrime](ostream& stream)
        {
            assert(sizeof(lastPrime) == sizeof(uint64_t));
            assert(stream.tellp() == 0); // empty stream should come here
            uint64_t cnt = 0;
            
            // 2 and 3 are not included into calc because they are out of 6n+1/6n+5 formular, adding them manually here
            if (m_realStart < 3)
            {
                lastPrime = 2;
                stream.write((char*)&lastPrime, sizeof(lastPrime)); 
                uint16_t diff = 1; // diff between 2 and 3
                stream.write((char*)&diff, sizeof(diff));
                lastPrime = 3;
                cnt += 2;
            }
            else if (m_realStart == 3)
            {
                lastPrime = 3;
                stream.write((char*)&lastPrime, sizeof(lastPrime));
                cnt++;
            }

            return cnt;
        };

    SavePrimeToStreamPred predBIN = [this](ostream& stream, uint64_t index)
        {
            return predBIN6n1(stream, index);
        };

    SavePrimeToStreamPred predBINDiff = [this, &lastPrime](ostream& stream, uint64_t index)
        {
            uint64_t prime;
            if ((index & 0x01) == 0)
                prime = index * 3 + 1; // index is even, full formular (index/2)*6+1
            else
                prime = (index - 1) * 3 + 5; // index is odd, full formular ((index-1)/2)*6+5

            assert(prime > lastPrime);
            assert(lastPrime == 0 || (prime - lastPrime < UINT16_MAX));

            // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
            if (prime >= m_realStart && prime < m_realEnd)
            {
                if (lastPrime == 0)
                {
                    stream.write((char*)&prime, sizeof(prime));
                }
                else
                {
                    uint16_t diff = (uint16_t)(prime - lastPrime);
                    assert(lastPrime < 3 || ((diff & 0x01ULL) == 0ULL)); // check that diff is really even
                    if (lastPrime > 2) diff /= 2;
                    stream.write((char*)&diff, sizeof(diff));
                }

                lastPrime = prime;
                return true;
            }
            return false;
        };

    StartPrimePred predStartBINDiffVar = [this, &lastPrime](ostream& stream)
        {
            assert(stream.tellp() == 0); // empty stream should come here
            uint8_t buf[9];
            uint64_t cnt = 0;

            // 2 and 3 are not included into calc because they are out of 6n+1/6n+5 formular, adding them manually here
            if (m_realStart < 3)
            {
                size_t len = var_len_encode(buf, 2);
                stream.write((char*)buf, len);
                
                len = var_len_encode(buf, 1); // diff between 2 and 3
                stream.write((char*)buf, len);
                
                lastPrime = 3;
                cnt += 2;
            }
            else if (m_realStart < 4)
            {
                lastPrime = 3;
                size_t len = var_len_encode(buf, lastPrime);
                stream.write((char*)buf, len);
                cnt++;
            }

            return cnt;
        };

    SavePrimeToStreamPred predBINDiffVar = [this, &lastPrime](ostream& stream, uint64_t index)
        {            
            uint64_t prime;
            if ((index & 0x01) == 0)
                prime = index * 3 + 1; // index is even, full formular (index/2)*6+1
            else
                prime = (index - 1) * 3 + 5; // index is odd, full formular ((index-1)/2)*6+5

            assert(prime > lastPrime);
            assert(lastPrime == 0 || (prime - lastPrime < UINT16_MAX));

            // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
            if ((prime >= m_realStart) && (prime < m_realEnd))
            {
                uint8_t buf[9];
                uint64_t diff;

                diff = prime - lastPrime; // diff is always even. that is why we can divide it by 2. more values will fit into single byteт.
                assert(lastPrime == 0 || ((diff & 0x01ULL) == 0ULL)); // check that diff is really even
                
                if (lastPrime > 2) diff /= 2; // bypass div by 2 when lastPrime=0 and lastPrime=2 because diff between 2 and 3 = 1
            
                size_t len = var_len_encode(buf, diff);
                stream.write((char*)buf, len);

                lastPrime = prime;
                return true;
            }
            return false;
        };

    if (Parameters::SAVEFILE)
    {
        cout << "Saving file...";

        //auto start2 = chrono::high_resolution_clock::now();
        Ticks::Start("save");

        uint64_t primesSaved = 0;
        string outputFilename = getOutputFilename();

        switch (m_OutputFileType)
        {
        case PrimesFIO::PFF::txt:       primesSaved = saveAsTXT(outputFilename, predStartTXT, predTXT); break;
        case PrimesFIO::PFF::txtdiff:   primesSaved = saveAsTXT(outputFilename, predStartTXTDiff, predTXTDiff); break;
        case PrimesFIO::PFF::bin:       primesSaved = saveAsBIN(outputFilename, predStartBIN, predBIN); break;
        case PrimesFIO::PFF::bindiff:   primesSaved = saveAsBIN(outputFilename, predStartBINDiff, predBINDiff); break;
        case PrimesFIO::PFF::bindiffvar:primesSaved = saveAsBIN(outputFilename, predStartBINDiffVar, predBINDiffVar); break;
        }

        cout << " Saved." << endl;

        cout << format("{:<{}}: ", "Primes saved to file", F_WIDTH) << primesSaved << endl;
        cout << format("{:<{}}: ", "File size", F_WIDTH) << std::filesystem::file_size(outputFilename) << " bytes" << endl;
        
        cout << format("{:<{}}: {}", "File saving time", F_WIDTH, MillisecToStr<string>(Ticks::Finish("save"))) << endl;
        cout << format("{:<{}}: {}", "Total time", F_WIDTH, MillisecToStr<string>(Ticks::GetTick("save") + Ticks::GetTick("6n1Calc"))) << endl;
    }

    PrintMemoryUsage(); // it should be BEFORE freeing big arrays such as m_prldPrimes and m_bitArray

    delete[] m_prldPrimes;
    delete m_bitArray;
}

// stores only odd numbers in memory because all even numbers are not primes by default
// uses two times less memory than Simple method
void EratosthenesSieve::calculateOptimum()
{
    uint64_t START = (m_realStart /* - 1ULL*/) / 2ULL; // calc start index of packed array. Do not store even numbers because, no need spending space on them.
    uint64_t END   = m_realEnd / 2ULL; // calc end index of packed array

    defineRanges(START, END);
    m_nextPrimeIndex = 1; //bypass prime '2'

    loadPrimes(); // loading primes from file, result - in m_prldPrimes array. 

    m_bitArray = new SegmentedArray(START, END);

    //auto start1 = chrono::high_resolution_clock::now();
    Ticks::Start("OptCalc");
    
    StartProgress(m_readPrimes);

    if (Parameters::THREADS > 1)
    {
        StrikeoutComposedPred predStrikeoutComposed = [](uint64_t NPrime, uint64_t currPrime, EratosthenesSieve* es)
            {
                uint64_t index = (NPrime - 1) / 2;
                auto range = es->getRange(index);
                if (range != nullptr)
                    range->first->lock();

                //while (NPrime < es->m_realEnd)
                while(index < es->m_bitArray->iend())
                {
                    es->m_bitArray->setTrue(index);
                    //NPrime += 2 * currPrime; // NPrime + currPrime makes NPrime even. we have to bypass cases when NPrime is even. That is why we do NPrime + 2*currPrime
                    //index = (NPrime - 1) / 2;
                    index += currPrime; // this is equivalent of two lines above

                    if (index > range->second.second)
                    {
                        range->first->unlock();
                        range = es->getRange(index);
                        if (range != nullptr) // range can be null one time when j becomes bigger than m_realEnd in this loop.  
                            range->first->lock(); // we jumped into next range. lock it and continue filtering primes in this range
                    }
                }

                if(range != nullptr)
                    range->first->unlock(); // unlock last range
            };

        FindPrimesInThreads(Parameters::THREADS, predStrikeoutComposed); // waits until all threads are finished
    }
    else
    {
        uint64_t NPrime, currPrime, index;
        
        for (uint32_t i = 1; i < m_readPrimes; ++i)  //%%%%% bypass prime number 2 and start from 3
        {
            UpdateProgress(i);

            currPrime = m_prldPrimes[i];

            NPrime = currPrime * currPrime; // start to strike out composed nums from p^2 because variants like p*3, p*5, p*7 have already visited in previous iterations 
            if (NPrime < m_realStart)
            {
                //NPrime = m_realStart;
                //rem = m_realStart % currPrime;
                //if (rem > 0)
                //    NPrime = m_realStart - rem + currPrime; // calc first number inside specified range that divisible to currPrime
                NPrime = m_realStart + ((currPrime - m_realStart % currPrime) % currPrime);

                if ((NPrime & 0x01ULL) == 0) NPrime += currPrime; // even numbers are not in array therefore take next num divisible by currPrime, it will be odd            
            }
            
            assert((NPrime & 0x01ULL) != 0); // NPrime must be odd

            index = (NPrime - 1) / 2;
            //while (NPrime < m_realEnd)
            while(index < m_bitArray->iend())
            {
                // NPrime is always odd, transfer it to index of "packed" array
                m_bitArray->setTrue(index);
                index += currPrime;
                //NPrime += 2*currPrime; // NPrime + currPrime makes begin even. We have to bypass cases when even NPrime is even. That is why we do NPrime + 2*currPrime
            }
        }
    }

    if (m_realStart == 0) m_bitArray->setTrue(0); // 0 and 1 are not primes numbers, mark them compound in array
    if (m_realStart == 1) m_bitArray->setTrue(0); // 0 and 1 are not primes numbers, mark them compound in array

    FinishProgress();
    
    cout << format("{:<{}}: {}", "Calculation time", F_WIDTH, MillisecToStr<string>(Ticks::Finish("OptCalc"))) << endl;

    ///////////////////////////////////////////////////////////////////

    uint64_t lastPrime = 0; // kind of global var between predStartTXT and predTXTDiff

    StartPrimePred predStartTXT = [this, &lastPrime](ostream& stream)
        {
            if (m_realStart < 3)
            {
                lastPrime = 2;
                stream << lastPrime << COMMA; // 2 is not included into calc because it even number, need to add it here     
                return 1;
            }
            return 0;
        };

    //const uint64_t realEnd = m_realStart + m_realLength;
    
    SavePrimeToStreamPred predTXT = [this](ostream& stream, uint64_t index)
        {
            uint64_t prime = index * 2 + 1;
            
            // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
            if ((prime >= m_realStart) && (prime < m_realEnd))
            {
                stream << prime << COMMA;
                return true;
            }
            return false;
        };

    SavePrimeToStreamPred predTXTDiff = [this, &lastPrime](ostream& stream, uint64_t index)
        {
            uint64_t prime = index * 2 + 1;
            assert(prime > lastPrime);
            assert(lastPrime == 0 || (prime - lastPrime < UINT16_MAX));

            // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
            if (prime >= m_realStart && prime < m_realEnd)
            { 
                if (lastPrime > 2)
                    stream << (prime - lastPrime) / 2 << COMMA;
                else
                    stream << prime - lastPrime << COMMA;
                //stream << prime - lastPrime << COMMA;
                lastPrime = prime;
                return true;
            }
            return false;

        };

    StartPrimePred predStartBIN = [this, &lastPrime](ostream& stream)
        {
            assert(sizeof(lastPrime) == sizeof(uint64_t));

            if (m_realStart < 3)
            {
                lastPrime = 2;
                stream.write((char*)&lastPrime, sizeof(lastPrime)); // 2 is not included into calc because it even number, need to add it here     
                return 1;
            }
            return 0;
        };

    SavePrimeToStreamPred predBIN = [this](ostream& stream, uint64_t index)
        {
            uint64_t prime = index * 2 + 1;

            // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
            if ((prime >= m_realStart) && (prime < m_realEnd))
            {
                stream.write((char*)&prime, sizeof(prime)); // write prime in binary format
                return true;
            }
            return false;
        };

    SavePrimeToStreamPred predBINDiff = [this, &lastPrime](ostream& stream, uint64_t index)
        {
            uint64_t prime = index * 2 + 1;
            assert(prime > lastPrime);
            assert(lastPrime == 0 || (prime - lastPrime < UINT16_MAX));

            // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
            if ((prime >= m_realStart) && (prime < m_realEnd))
            {
                if (lastPrime == 0)
                {
                    stream.write((char*)&prime, sizeof(prime)); // this is first prime, needs to be written as is (no diff)
                }
                else
                {
                    uint16_t diff = (uint16_t)(prime - lastPrime);
                    assert(lastPrime < 3 || ((diff & 0x01ULL) == 0ULL)); // check that diff is really even
                    if (lastPrime > 2) diff /= 2;
                    stream.write((char*)&diff, sizeof(diff)); 
                }

                lastPrime = prime;
                return true;
            }
            return false;
        };


    StartPrimePred predStartBINDiffVar = [this, &lastPrime](ostream& stream)
        {   
            if (m_realStart < 3)
            {
                uint8_t buf[9];

                size_t len = var_len_encode(buf, 2);
                stream.write((char*)buf, len);
                lastPrime = 2;
                return 1;
            }
            return 0;
        };

    SavePrimeToStreamPred predBINDiffVar = [this, &lastPrime](ostream& stream, uint64_t index)
        {
            uint64_t prime = index * 2 + 1;
            assert(prime > lastPrime);
            assert(lastPrime == 0 || (prime - lastPrime < UINT16_MAX));

            // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
            if ((prime >= m_realStart) && (prime < m_realEnd))
            {
                uint8_t buf[9];
                uint64_t diff;

                //if (lastPrime == 0) //TODO for simple we might not need this if
                //{
                //    diff = prime;
                //}
                //else
                {
                    diff = prime - lastPrime; // diff is always even. that is why we can divide it by 2. more values will fit into single byte.
                    assert(lastPrime < 3 || ((diff & 0x01ULL) == 0ULL)); // check that diff is really even

                    if (lastPrime > 2) diff /= 2; // bypass div by 2 when lastPrime=0 and lastPrime=2 because diff between 2 and 3 = 1
                }

                size_t len = var_len_encode(buf, diff);
                stream.write((char*)buf, len);

                lastPrime = prime;
                return true;
            }
            return false;
        };

    if (Parameters::SAVEFILE)
    {
        cout << "Saving file...";

        //auto start2 = chrono::high_resolution_clock::now();
        Ticks::Start("save");

        uint64_t primesSaved = 0;
        string outputFilename = getOutputFilename();

        switch (m_OutputFileType)
        {
        case PrimesFIO::PFF::txt:       primesSaved = saveAsTXT(outputFilename, predStartTXT, predTXT);break;
            // predStart adds only one prime, that is why we do not need calculate diff there 
        case PrimesFIO::PFF::txtdiff:   primesSaved = saveAsTXT(outputFilename, predStartTXT, predTXTDiff);break;
        case PrimesFIO::PFF::bin:       primesSaved = saveAsBIN(outputFilename, predStartBIN, predBIN);break;
        case PrimesFIO::PFF::bindiff:   primesSaved = saveAsBIN(outputFilename, predStartBIN, predBINDiff);break;
        case PrimesFIO::PFF::bindiffvar:primesSaved = saveAsBIN(outputFilename, predStartBINDiffVar, predBINDiffVar);break;
        }

        cout << " Saved." << endl;

        cout << format("{:<{}}: ", "Primes saved to file", F_WIDTH) << primesSaved << endl;
        cout << format("{:<{}}: ", "File size", F_WIDTH) << std::filesystem::file_size(outputFilename) << " bytes" << endl;
        cout << format("{:<{}}: {}", "File saving time", F_WIDTH, MillisecToStr<string>(Ticks::Finish("save"))) << endl;
        
        cout << format("{:<{}}: {}", "Total time", F_WIDTH, MillisecToStr<string>(Ticks::GetTick("OptCalc")+ Ticks::GetTick("save"))) << endl;
    }

    PrintMemoryUsage(); // it should be BEFORE freeing big arrays such as m_prldPrimes and m_bitArray

    delete[] m_prldPrimes;
    delete m_bitArray;
}

void EratosthenesSieve::calculateSimple()
{
    defineRanges(m_realStart, m_realLength);
    m_nextPrimeIndex = 0;

    loadPrimes(); // loading primes from file, result - in m_prldPrimes array. 

    m_bitArray = new SegmentedArray(m_realStart, m_realEnd);

    //start1 = chrono::high_resolution_clock::now();
    Ticks::Start("SimpleCalc");
    
    StartProgress(m_readPrimes);

    if (Parameters::THREADS > 1)
    {
        StrikeoutComposedPred predStrikeoutComposed = [this](uint64_t NPrime, uint64_t currPrime, EratosthenesSieve* es)
            {
                auto range = es->getRange(NPrime);
                if (range != nullptr)
                    range->first->lock();

                while (NPrime < es->m_realEnd)
                {
                    es->m_bitArray->setTrue(NPrime);
                    NPrime += currPrime; 

                    if (NPrime > range->second.second)
                    {
                        range->first->unlock();
                        range = es->getRange(NPrime);
                        if (range != nullptr) // range can be null one time when j becomes bigger than m_realEnd in this loop.  
                            range->first->lock(); // we jumped into next range. lock it and continue filtering primes in this range
                    }
                }

               // if (range != nullptr) 
               //     range->first->unlock();
            };

        FindPrimesInThreads(Parameters::THREADS, predStrikeoutComposed); // waits until all threads are finished
    }
    else
    {
        uint64_t len = m_bitArray->iend();
        uint64_t NPrime, currPrime;

        // special strike out of all even numbers 
        assert(2 == m_prldPrimes[0]);

        uint64_t k = m_realStart;
        if (k < 4) k = 4; // leave 2 as prime number 
        else if ((k & 0x01) != 0) k++; // if k is odd then make k even

        while (k < len)
        {
            m_bitArray->setTrue(k);
            k += 2;
        }

        for (uint32_t i = 1; i < m_readPrimes; ++i)
        {
            UpdateProgress(i);

            currPrime = m_prldPrimes[i];

            NPrime = currPrime * currPrime; // start to strike out composed nums from p^2 because variants like p*3, p*5, p*7 have already visited in previous iterations 
            if (NPrime < m_realStart)
            {
                //NPrime = m_realStart;
                //rem = m_realStart % currPrime;
                //if (rem > 0)
                //    NPrime = m_realStart - rem + currPrime; // calc first number inside specified range that divisible to currPrime
                NPrime = m_realStart + ((currPrime - m_realStart % currPrime) % currPrime);

                if ((NPrime & 0x01ULL) == 0) NPrime += currPrime; // even numbers are not in array therefore take next num divisible by currPrime, it will be odd            
            }

            assert((NPrime & 0x01ULL) != 0); // NPrime must be odd

            while (NPrime < m_realEnd)
            {
                m_bitArray->setTrue(NPrime);
                NPrime += (currPrime << 1); // NPrime + currPrime makes even num. We have to bypass cases when even NPrime is even. That is why we do NPrime + 2*currPrime
            }
        }
    }

    if (m_realStart == 0) m_bitArray->setTrue(0), m_bitArray->setTrue(1); // 0 and 1 are not primes numbers, mark them compound in array
    if (m_realStart == 1) m_bitArray->setTrue(1); // if we generate primes starting from 1, mark 1 as not prime.
    if ((m_realStart > 2) && (m_realStart & 0x01) == 0) m_bitArray->setTrue(m_realStart);

    FinishProgress();

    cout << format("{:<{}}: {}", "Calculations time", F_WIDTH, MillisecToStr<string>(Ticks::Finish("SimpleCalc"))) << endl;

    ///////////////////////////////////////////////////////////////

    uint64_t lastPrime = 0; // kind of global var between predStartTXT and predTXTDiff

    StartPrimePred predStart = [this]([[maybe_unused]] ostream& stream)
        {
            assert(stream.tellp() == 0);

            return false; // no any start primes for simple mode
        };

    SavePrimeToStreamPred predTXT = [this](ostream& stream, uint64_t index)
        {                       
            // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
            //if ((index >= m_realStart) && (index < m_realEnd)) //TODO this if can be deleted for simple mode
            //{
                stream << index << COMMA;
                return true;
            //}
            //return false;
        };

    SavePrimeToStreamPred predTXTDiff = [this, &lastPrime](ostream& stream, uint64_t index)
        {
            assert(index > lastPrime);
            assert(lastPrime == 0 || (index - lastPrime < UINT16_MAX));
            assert(lastPrime != 1);

            // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
            //if (index >= m_realStart && index < m_realEnd) //TODO this if can be deleted for simple mode
            //{
          
            if (lastPrime > 2)
                stream << (index - lastPrime) / 2 << COMMA;
            else 
                stream << index - lastPrime << COMMA;
          
            lastPrime = index;
            return true;
            //}
            //return false;
        };


    SavePrimeToStreamPred predBIN = [this](ostream& stream, uint64_t index)
        {
            // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
            //if ((index >= m_realStart) && (index < m_realEnd))
            //{
                stream.write((char*)&index, sizeof(index)); // write primes as is in binary format
                return true;
            //}
            //return false;
        };

    SavePrimeToStreamPred predBINDiff = [this, &lastPrime](ostream& stream, uint64_t index)
        {
            assert(index > lastPrime);
            assert(lastPrime == 0 || (index - lastPrime < UINT16_MAX));
            
            // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
            //if ((index >= m_realStart) && (index < m_realEnd))
            //{               
                if (lastPrime == 0)
                {
                    stream.write((char*)&index, sizeof(index));
                }
                else
                {
                    
                    uint16_t diff = (uint16_t)(index - lastPrime);
                    assert(lastPrime < 3 || ((diff & 0x01ULL) == 0ULL)); // check that diff is really even
                    if (lastPrime > 2) diff /= 2;
                    stream.write((char*)&diff, sizeof(diff));
                }

                lastPrime = index;
                return true;
            //}
            //return false;
        };

    SavePrimeToStreamPred predBINDiffVar = [this, &lastPrime](ostream& stream, uint64_t index)
        {
            assert(index > lastPrime);
            assert(lastPrime == 0 || (index - lastPrime < UINT16_MAX));
            
            // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
            //if ((index >= m_realStart) && (index < m_realEnd))
            //{
                uint8_t buf[9];
                uint64_t diff;

                //if (lastPrime == 0) //TODO for simple we might not need this if
                //{
                //    diff = index;
                //}
                //else
                {
                    diff = index - lastPrime; // diff is always even. that is why we can divide it by 2. more values will fit into single byteт.
                    assert(lastPrime < 3 || ((diff & 0x01ULL) == 0ULL)); // check that diff is really even

                    if (lastPrime > 2) diff /= 2; // bypass div by 2 when lastPrime=0 and lastPrime=2 because diff between 2 and 3 = 1
                }

                size_t len = var_len_encode(buf, diff);
                stream.write((char*)buf, len);

                lastPrime = index;
                return true;
            //}
            //return false;
        };

    if (Parameters::SAVEFILE)
    {
        cout << "Saving file...";

        //auto start2 = chrono::high_resolution_clock::now();
        Ticks::Start("save");

        uint64_t primesSaved = 0;
        string outputFilename = getOutputFilename();

        switch (m_OutputFileType)
        {
        case PrimesFIO::PFF::txt:       primesSaved = saveAsTXT(outputFilename, predStart, predTXT);break;
        case PrimesFIO::PFF::txtdiff:   primesSaved = saveAsTXT(outputFilename, predStart, predTXTDiff); break;
        case PrimesFIO::PFF::bin:       primesSaved = saveAsBIN(outputFilename, predStart, predBIN);break;
        case PrimesFIO::PFF::bindiff:   primesSaved = saveAsBIN(outputFilename, predStart, predBINDiff); break;
        case PrimesFIO::PFF::bindiffvar:primesSaved = saveAsBIN(outputFilename, predStart, predBINDiffVar);break;
        }

        cout << " Saved." << endl;

        cout << format("{:<{}}: ", "Primes saved to file", F_WIDTH) << primesSaved << endl;
        cout << format("{:<{}}: ", "File size", F_WIDTH) << std::filesystem::file_size(outputFilename) << " bytes" << endl;
        
        cout << format("{:<{}}: {}", "File saving time", F_WIDTH, MillisecToStr<string>(Ticks::Finish("save"))) << endl;
        cout << format("{:<{}}: {}", "Total time", F_WIDTH, MillisecToStr<string>(Ticks::GetTick("SimpleCalc") + Ticks::GetTick("save"))) << endl;
    }

    PrintMemoryUsage(); // it should be BEFORE freeing big arrays such as m_prldPrimes and m_bitArray

    delete[] m_prldPrimes;
    delete m_bitArray;
}

// stores in both TXT and TXTDIFF formats
// exact storing is delegated to StartPrimePred and SavePrimeToStreamPred predicates
uint64_t EratosthenesSieve::saveAsTXT(string& outputFilename, StartPrimePred startPred, SavePrimeToStreamPred pred)
{
    constexpr uint64_t CHUNK = 10'000'000; // store primes to file by 10M chunks
    constexpr uint64_t DELTA = 25; // max number of 64bit integer digits - 20. we need Delta to be slightly greater than it.
    constexpr uint64_t THRESHOLD = CHUNK - DELTA;

    fstream f;
    f.exceptions(ifstream::failbit | ifstream::badbit);
    f.sync_with_stdio(false);
    f.open(outputFilename, ios::out | ios::binary);
    
    string s;
    ostringstream oss;
  
    s.reserve(CHUNK);
    oss.str(std::move(s)); // assign big buffer s to stream oss

    uint64_t cntPrimes = 0;
    cntPrimes = startPred(oss);

    for (uint64_t i = m_bitArray->ibegin(); i < m_bitArray->iend(); ++i)
    {
        if (!m_bitArray->get(i))
        {
            if (pred(oss, i)) cntPrimes++;
      
            if ((uint64_t)oss.tellp() > THRESHOLD) // when we are close to capacity but еще НЕ перепрыгнули ее
            {
                auto v = oss.view();
                f.write(v.data(), v.size());
               // f.flush();
                oss.str(""); // clear oss buffer, but leave capacity
                oss.clear(); // clear flags like failbit, eofbit, etc.
            }
        }
    }

    auto v = oss.view();
    f.write(v.data(), v.size());
    f.close();

    return cntPrimes;
}

/*
uint64_t EratosthenesSieve::saveAsTXTOptimumMode(SegmentedArray* sarr, string& outputFilename, StartPrimePred startPred, PrimeDiffFromIndexPred pred)
{
    size_t chunk = 10'000'000;

    fstream f;
    f.exceptions(ifstream::failbit | ifstream::badbit);
    f.sync_with_stdio(false);
    f.open(outputFilename, ios::out | ios::binary);

    string s;
    s.reserve(chunk); // store to file by 10M chunks

    uint64_t cntPrimes = 0;
    uint64_t lastPrime = 0;
    //uint64_t prime = 0;
    std::vector<uint64_t> startPrimes;

    if (startPred(startPrimes, lastPrime))
    {
        for (auto prm : startPrimes)
        {
            cntPrimes++;
            s.append(to_string(prm));
            s.append(",");
        }
    }
    //if ((start == 0) || (start == 1))
    //    s.append("2,"); // add first prime '2' for compressed array if we generate primes starting from 0 or 1.

    for (uint64_t i = sarr->ibegin(); i < sarr->iend(); ++i)
    {
        if (!sarr->get(i))
        {
            //prime = pred(i, lastPrime); //2 * i + 1;
            // sometimes prime may be out of requested range
            // add to file only primes that fit into range
            //if ((prime >= m_realStart) && (prime < m_realStart + m_realLength))
            //{
                cntPrimes++;
                s.append(to_string(pred(i, lastPrime)));
                s.append(",");
            //}

            if (s.size() > chunk - 20) // when we are close to capacity but еще НЕ перепрыгнули ее
            {
                f.write(s.c_str(), s.length());
                // f.flush();
                s.clear(); // clear string but leave capacity
            }
        }
    }

    f.write(s.c_str(), s.length());
    f.close();

    return cntPrimes;
}

uint64_t EratosthenesSieve::saveAsTXTDiffOptimumMode(uint64_t start, SegmentedArray* sarr, string& outputFilename, PrimeFromIndexPred pred)
{
    constexpr uint64_t CHUNK = 10'000'000; // store primes to file by 10M chunks
    constexpr uint64_t DELTA = 25; // max number of 64bit integer digits - 20. we need Delta to be slightly greater than it.

    fstream f;
    f.exceptions(ifstream::failbit | ifstream::badbit);
    f.sync_with_stdio(false);
    f.open(outputFilename, ios::out | ios::binary);

    string s;
    s.reserve(CHUNK); // reserve capacity in advance to minimise re-allocations. 

    uint64_t cntPrimes = 0;
    uint64_t lastPrime = 0;
    uint64_t prime = 0;

    if( ( start == 0) || (start == 1) )
    {
        s.append("2,"); // add first prime '2' for compressed array if we generate primes starting from 0 or 1.
        lastPrime = 2;
    }

    for (uint64_t i = start; i < sarr->iend(); ++i)
    {
        if (!sarr->get(i))
        {
            cntPrimes++;
            prime = pred(i); // 2 * i + 1;

            s.append(to_string(prime - lastPrime));
            s.append(",");
            lastPrime = prime;

            if (s.size() > CHUNK - DELTA) // when we are close to capacity but еще НЕ перепрыгнули ее
            {
                f.write(s.c_str(), s.length());
                // f.flush();
                s.clear(); // clear string but leave capacity
            }
        }
    }

    f.write(s.c_str(), s.length());
    f.close();

    return cntPrimes;
}
*/
uint64_t EratosthenesSieve::saveAsBIN(string& outputFilename, StartPrimePred startPred, SavePrimeToStreamPred pred)
{
    fstream f;
    f.exceptions(ifstream::failbit | ifstream::badbit);
    f.sync_with_stdio(false);
    f.open(outputFilename, ios::out | ios::binary);

    uint64_t cntPrimes = 0;

    cntPrimes = startPred(f);

    for (uint64_t i = m_bitArray->ibegin(); i < m_bitArray->iend(); i++)
    {
        if (!m_bitArray->get(i))
        {
            if (pred(f, i)) cntPrimes++; // increase num of saved primes only if pred returned true (the prime has saved)
        }
    }

    f.close();

    return cntPrimes;
}

/*
uint64_t EratosthenesSieve::saveAsBINOptimumMode(uint64_t start, SegmentedArray* sarr, string& outputFilename, PrimeFromIndexPred pred)
{
    fstream f;
    f.exceptions(ifstream::failbit | ifstream::badbit);
    f.sync_with_stdio(false);
    f.open(outputFilename, ios::out | ios::binary);

    uint64_t prime;
    uint64_t cntPrimes = 0;

    if ((start == 0) || (start == 1))
        prime = 2, f.write((char*)&prime, sizeof(uint64_t)); // заглушка для сжатого массива если генерим числа начиная с 0.

    for (uint64_t i = start; i < sarr->iend(); i++)
    {
        if (!sarr->get(i))
        {
            cntPrimes++;            
            prime = pred(i); // 2 * i + 1;
            f.write((char*)&prime, sizeof(uint64_t)); 
        }
    }

    f.close();

    return cntPrimes;
}

uint64_t EratosthenesSieve::saveAsBINDiffOptimumMode(uint64_t start, SegmentedArray* sarr, string& outputFilename, PrimeFromIndexPred pred)
{
    fstream f;
    f.exceptions(ifstream::failbit | ifstream::badbit);
    f.sync_with_stdio(false);
    f.open(outputFilename, ios::out | ios::binary);

    uint64_t prime;
    uint64_t cntPrimes = 0;
    uint64_t lastPrime = 0;

    if ((start == 0) || (start == 1))
        prime = 2, lastPrime = 2, f.write((char*)&prime, sizeof(uint64_t)); // заглушка для сжатого массива если генерим числа начиная с 0.


    for (uint64_t i = sarr->ibegin(); i < sarr->iend(); i++)
    {
        if (!sarr->get(i))
        {
            cntPrimes++;

            prime = pred(i); //2 * i + 1;

            if (lastPrime == 0)
            {
                f.write((char*)&prime, sizeof(uint64_t)); // save as 8 byte value (full number)
            }
            else
            {
                uint16_t diff = (uint16_t)(prime - lastPrime);
                f.write((char*)&diff, sizeof(uint16_t));  // save a 2 bytes value (difference)
            }

            lastPrime = prime;
        }
    }

    f.close();

    return cntPrimes;
}

uint64_t EratosthenesSieve::saveAsBINDiffVarSimpleMode(uint64_t start, SegmentedArray* sarr, string& outputFilename)
{
    fstream f;
    f.exceptions(ifstream::failbit | ifstream::badbit);
    f.sync_with_stdio(false);
    f.open(outputFilename, ios::out | ios::binary);

    uint64_t cntPrimes = 0;
    uint64_t lastPrime = 0;
    uint8_t buf[9];

    for (uint64_t i = start; i < sarr->iend(); i++)
    {
        if (!sarr->get(i))
        {
            cntPrimes++;

            uint64_t diff = (i - lastPrime); // разница всегда четная. поэтому можем хранить половину значения. больше значений уместится в 1 байт.
            assert((diff & 0x01ULL) == 0ULL); // check that diff is even

            if (lastPrime > 2) diff /= 2; // bypass div by 2 when lastPrime=0 and lastPrime=2 because diff between 2 and 3 = 1

            size_t len = var_len_encode(buf, diff);
            f.write((char*)buf, len);

            lastPrime = i;
        }
    }

    f.close();

    return cntPrimes;
}

uint64_t EratosthenesSieve::saveAsBINDiffVarOptimumMode(uint64_t start, SegmentedArray* sarr, string& outputFilename, PrimeFromIndexPred pred)
{
    fstream f;
    f.exceptions(ifstream::failbit | ifstream::badbit);
    f.sync_with_stdio(false);
    f.open(outputFilename, ios::out | ios::binary);

    uint64_t lastPrime = 0;
    uint64_t prime;

    const uint64_t BUF_LEN = 100'000'000; // записываем в файл блоками по 100М
    uint8_t* buf = new uint8_t[BUF_LEN];

    if ( (start == 0) || (start == 1) ) // заглушка для сжатого массива если генерим числа начиная с 0.
    {
        prime = 2;
        size_t len = var_len_encode(buf, prime);
        f.write((char*)buf, len);
        lastPrime = prime;
    }

    uint64_t cntPrimes = 0;
    uint64_t offset = 0;

    for (uint64_t i = start; i < sarr->iend(); i++)
    {
        if (!sarr->get(i))
        {
            prime = pred(i); // (i << 1) + 1; //2 * i + 1;

            // sometimes prime may be out of requested range, save to file only primes that fit into range
            if ((prime < m_realStart) || (prime >= m_realStart + m_realLength))
                continue;

            cntPrimes++;

            uint64_t diff;
            if (lastPrime == 0)
            {
                diff = prime;
            }
            else
            {
                diff = prime - lastPrime;
                assert((diff & 0x01ULL) == 0ULL); // check that diff is even

                if (lastPrime > 2) diff >>= 1; // bypass div by 2 when lastPrime=0 and lastPrime=2 because diff between 2 and 3 = 1
            }

            size_t len = var_len_encode(buf + offset, diff);            
            assert(len > 0);
            
            offset += len;

            if (offset > BUF_LEN - 3) // encoded len always less than 2^24 (max number that fits into 3 bytes)
            {
                f.write((char*)buf, offset);
                offset = 0;
            }

            lastPrime = prime;
        }
    }

    f.write((char*)buf, offset);

    f.close();

    delete[] buf;

    return cntPrimes;
}
*/

bool EratosthenesSieve::predBIN6n1(ostream& stream, uint64_t index)
{
    uint64_t prime;
    if ((index & 0x01) == 0)
        prime = index * 3 + 1; // index is even, full formular (index/2)*6+1
    else
        prime = (index - 1) * 3 + 5; // index is odd, full formular ((index-1)/2)*6+5

    // some compressed arrays may contain values outside of specified range, add to file only primes that fit into the range
    if ((prime >= m_realStart) && (prime < m_realEnd))
    {
        stream.write((char*)&prime, sizeof(prime));
        return true;
    }
    return false;
};