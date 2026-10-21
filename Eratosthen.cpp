// Erastofen.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "Debug.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include "DefaultParser.h"
//#include "HelpFormatter.h"
#include "EratosthenesSieve.h"
#include "Parameters.h"


using namespace std;

#define OPT_S "s"
#define OPT_O "o"
#define OPT_6 "6"
#define OPT_30 "30"
#define OPT_P "p"
#define OPT_T "t"
#define OPT_H "h"
#define OPT_N "n"

struct MyGroupSeparator : numpunct<char>
{
    char do_thousands_sep() const override { return ' '; } // thousands delimiter
    string do_grouping() const override { return "\3"; }   // groupping by 3
};

static void defineOptions(COptionsList& options)
{
    options.AddOption(OPT_S, "simple", "Generate primes using simple Eratosthenes sieve algorithm", 3);
    options.AddOption(OPT_O, "optimum", "Generate primes using optimized Eratosthenes sieve algorithm\n(uses less memory by excluding even numbers from consideration)", 3);
    options.AddOption(OPT_6, "6n1", "Generate primes using 6n1 Eratosthenes sieve algorithm. All primes have 6n+1 or 6n+5 structure.", 3);
    options.AddOption(OPT_30, "30nx", "Generate primes using 30nx Eratosthenes sieve mode. Some extension of 6n1 algorithm", 3);
    options.AddOption(OPT_P, "preload", string("Specify file with primes to preload. If not specified then default '") + Pre_Loaded_Primes_Filename + "' file name is used", 1);
    options.AddOption(OPT_T, "threads", "Use specified number of threads during primes generating", 1);
    options.AddOption(OPT_H, "help", "Show info about command line arguments.", 0);
    options.AddOption(OPT_N, "nofile", "Do not save file, only calculate numbers. Used mostly for measuring calculation metrics.", 0);

    options.MutuallyExclusive(OPT_S, OPT_O, OPT_6);
    options.MutuallyExclusive(OPT_30, OPT_O, OPT_6);
    options.MutuallyExclusive(OPT_30, OPT_S);
}

void PrintUsage()
{
    cout << endl << "Usage: ";
    cout << APP_EXE_NAME << " [-s][-o][-6][-30] <format> <start> <length> [option] <optionarg>" << endl << endl;
   
    cout << "Commands" << endl;
    cout << "  -s, --simple          Generate primes using simple Eratosthenes sieve algorithm." << endl;
    cout << "  -o, --optimum         Generate primes using optimized Eratosthenes sieve algorithm" << endl;
    cout << "                        (uses less memory by excluding even numbers from consideration)." << endl;
    cout << "  -6, --6n1             Generate primes using 6n1 Eratosthenes sieve algorithm. All primes have 6n + 1 or 6n + 5 structure." << endl;
    cout << "  -30, --30nx           Generate primes using 30nx Eratosthenes sieve mode. Logical extension of 6n1 algorithm." << endl;
    
    cout << endl << "Command arguments" << endl;
    cout << "  <format>              Defines output file format. Possible values: txt, txtdiff, bin, bindiff, and bindiffvar." << endl;
    cout << "  <start>               Starting value for generating primes." << endl; 
    cout << "                        Accepts suffixes B(bytes), K(kilo), M(mega), G(giga), T(tera), P(peta) for big values." << endl;
    cout << "  <length>              Length of a range. Primes are generated in a range: start...start+length." << endl;
    cout << "                        Accepts suffixes B, K, M, G, T, P." << endl;

    cout << endl << "Options" << endl;
    cout << "  -p, --preload <fname> Specify file with primes to preload. Two formats are supported: txt and bindiffvar." << endl;
    cout << "                        If not specified then default file 'primes - 0-1G.diffvar.bin' name is used." << endl;
    cout << "  -t, --threads <num>   Use specified number of threads for generating primes." << endl;
    cout << "  -n, --nofile          Do not save file, only calculate numbers. Used mostly for measuring calculation metrics." << endl;

    cout << endl << "Examples" << endl;
    cout << "  " << APP_EXE_NAME << " -s txt 0b 10g" << endl;
    cout << "  " << APP_EXE_NAME << " -o bin 9000G 100G -n" << endl;
    cout << "  " << APP_EXE_NAME << " -6 txtdiff 100T 10G" << endl;
    cout << "  " << APP_EXE_NAME << " -30 bindiff 5T 50M" << endl;
    cout << "  " << APP_EXE_NAME << " -o bindiffvar 20P 1G" << endl;
    cout << "  " << APP_EXE_NAME << " -o txt 1000P 100M -t 5" << endl;
    cout << "  " << APP_EXE_NAME << " -6 1000B 1G" << endl;
    cout << "  " << APP_EXE_NAME << " -s 0M 100M -p pre-generated-primes.txt" << endl;

    cout << endl <<"If you need help on cmd parameters make this call: " << APP_EXE_NAME << " -h" << endl;
}

int main(int argc, char* argv[])
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtMemState s1, s2, s3;
    _CrtMemCheckpoint(&s1); // Take a snapshot at the start of main()

    cout << endl;
    cout << "Application for calculating prime numbers with different Eratosthenes algorithms." << endl << endl;

    cout.imbue(locale(cout.getloc(), new MyGroupSeparator));

    CDefaultParser defaultParser;
    CCommandLine cmd;
    COptionsList options;

    defineOptions(options);

    if (argc < 2)
    {
        //cout << "Error: No command line arguments found." << endl << endl;
        PrintUsage();
        //cout << CHelpFormatter::Format(APP_NAME, &options) << endl;

        return 1;
    }

    if (!defaultParser.Parse(&options, &cmd, argv, argc))
    {
        cout << defaultParser.GetLastError() << endl;
        PrintUsage();
        //cout << CHelpFormatter::Format(APP_NAME, &options) << endl;

        return 1;
    }

    if (cmd.HasOption(OPT_P))
    {
        Parameters::PRIMES_FILENAME = cmd.GetOptionValue("p", 0, Pre_Loaded_Primes_Filename);
    }
    if (cmd.HasOption(OPT_N))
    {
        Parameters::SAVEFILE = false; // do not save file, only generate primes
    }
    if (cmd.HasOption(OPT_T))
    {
        try
        {
            Parameters::THREADS = stoi(cmd.GetOptionValue(OPT_T));
            if ( (Parameters::THREADS == 0) || (Parameters::THREADS > 25) )
            {
                cout << "Number of threads is out of bounds (1...25). Using default value " << DEFAULT_THREADS << "." << endl;
                Parameters::THREADS = DEFAULT_THREADS;
            }
        }
        catch (...) 
        {
            cout << "Incorrect number of threads specified '" << cmd.GetOptionValue(OPT_T) <<"'. Using default value " << DEFAULT_THREADS << "." << endl;
            Parameters::THREADS = DEFAULT_THREADS;
        }
    }

    if (cmd.HasOption(OPT_H))
    {
        PrintUsage();
        //cout << CHelpFormatter::Format(APP_NAME, &options) << endl;
    }

    if ( (cmd.HasOption(OPT_O) && cmd.HasOption(OPT_S)) || // incorrect case when two mutually exclusive commands present in command line
         (cmd.HasOption(OPT_O) && cmd.HasOption(OPT_6)) ||
         (cmd.HasOption(OPT_O) && cmd.HasOption(OPT_30))||
         (cmd.HasOption(OPT_S) && cmd.HasOption(OPT_6)) ||
         (cmd.HasOption(OPT_S) && cmd.HasOption(OPT_30))||
         (cmd.HasOption(OPT_6) && cmd.HasOption(OPT_30))
        ) 
    {
        cout << "Only one of four commands -s, -o, -6, -30 is allowed in command line." << endl;
        PrintUsage();
        //cout << CHelpFormatter::Format(APP_NAME, &options) << endl;

        return 1;
    }

  
    if (cmd.HasOption(OPT_O) || cmd.HasOption(OPT_S) || cmd.HasOption(OPT_6) || cmd.HasOption(OPT_30))
    {
        try
        {
            string opt = cmd.HasOption(OPT_O) ? OPT_O : cmd.HasOption(OPT_S)? OPT_S : cmd.HasOption(OPT_6) ? OPT_6: OPT_30;
            
            string ftype  = cmd.GetOptionValue(opt, 0);
            string start  = cmd.GetOptionValue(opt, 1);
            string length = cmd.GetOptionValue(opt, 2);

            EratosthenesSieve sieve;
            sieve.SetParams(opt, ftype, start, length);
            sieve.Calculate();


            _CrtMemCheckpoint(&s2); // Take a snapshot at the end of main()
            _CrtMemCheckpoint(&s2); // Take a snapshot at the end of main()
            if (_CrtMemDifference(&s3, &s1, &s2)) _CrtMemDumpStatistics(&s3); // Dump memory statistics excluding global variables
        }
        catch (invalid_cmd_option& e)
        {
            cout << endl << e.what() << endl << endl;
            PrintUsage();
            return 1;
        }
        catch (ios_base::failure& e)
        {
            cout << endl;
            cout << e.what() << endl << endl;
            cout << "Error code: " << e.code() << endl;

        }
        catch (exception& e)
        {
            cout << endl << e.what() << endl << endl;
            cout << "Run " << APP_EXE_NAME << " -h to get more info about command line parameters." << endl;
            return 1;
        }
        catch (...)
        {
            cout << "UNKNOWN exception has thrown" << endl;
            return 1;
        }
    }

    return 0;
}
