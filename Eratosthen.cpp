// Erastofen.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "Debug.h"
#include <iostream>
#include <fstream>
#include <iomanip>

#include "string_utils.h"
#include "Utils.h"
#include "DefaultParser.h"
#include "EratosthenesSieve.h"
#include "Parameters.h"
#include "FileConverter.h"
#include "CheckPrimes.h"
#include "Version.h"

using namespace std;

#define OPT_G "g"  // generate
#define OPT_V "v"  // verify primes
#define OPT_C "c"  // convert
#define OPT_P "p"  // file to preload
#define OPT_O "o"  // output folder
#define OPT_L "l"  // limit
#define OPT_T "t"  // threads
#define OPT_H "h"  // help
#define OPT_N "n"  // do not save generated primes to file
#define OPT_B "b"  // force BigInt
#define OPT_I "i"  // ignore first X primes

static void DefineOptions(COptionsList& options);
static void PrintUsage();

int main(int argc, char* argv[])
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtMemState s1, s2, s3;
    _CrtMemCheckpoint(&s1); // Take a snapshot at the start of main()

    ERA::APP_EXE_NAME = argv[0]; // this statement needs to be before first possible call of PrintUsage().
    ERA::APP_NAME = StripFileExt(argv[0]);

    cout.imbue(ERA::GetLoc());
    //cout.imbue(locale(cout.getloc(), new MyGroupSeparator));

    CDefaultParser defaultParser;
    CCommandLine cmd;
    COptionsList options;

    DefineOptions(options);

    if (argc < 2)
    {
        PrintUsage();
        return 1;
    }

    if (!defaultParser.Parse(&options, &cmd, argv, argc))
    {
        cout << defaultParser.GetLastError() << endl;
        PrintUsage();
        return 1;
    }

    cout << endl << format("{} prime numbers generator/verifier {}.{}.{} x64 Copyright(c) 2024 - 2026 Andrey Romanchenko {}",
        ERA::APP_NAME, ERA_VER_MAJOR, ERA_VER_MINOR, ERA_VER_PATCH, ERA_BUILD_DATE) << endl << endl;

    if (cmd.HasOption(OPT_P))
    {
        Parameters::PRIMES_FILENAME = cmd.GetOptionValue(OPT_P, 0, Pre_Loaded_Primes_Filename);
    }
    if (cmd.HasOption(OPT_N))
    {
        Parameters::SAVEFILE = false; // do not save file, only generate primes
    }
    if (cmd.HasOption(OPT_O))
    {
        Parameters::OUTPUT_FOLDER = cmd.GetOptionValue(OPT_O);
    }

    // default threads value depends on requested operation: generate=1thr, verify=4thrs
    Parameters::THREADS = cmd.HasOption(OPT_G) ? DEFAULT_G_THREADS : cmd.HasOption(OPT_V) ? DEFAULT_V_THREADS : DEFAULT_G_THREADS;
    if (cmd.HasOption(OPT_T))
    {
        try
        {
            uint8_t threads = (uint8_t)stoul(cmd.GetOptionValue(OPT_T));
            if ( (threads == 0) || (threads > 25) )
                cout << "Number of threads (" << cmd.GetOptionValue(OPT_T) << ") is out of bounds (1...25). Using default value " << Parameters::THREADS << "." << endl;
            else
                Parameters::THREADS = threads;
        }
        catch (...) 
        {
            cout << "Incorrect number of threads specified '" << cmd.GetOptionValue(OPT_T) <<"'. Using default value " << Parameters::THREADS << "." << endl;
        }
    }

   // uint64_t limitPrimes = 0; // unlimited by default. //CheckPrimes::DEFAULT_LIMIT_PRIMES; //TODO shall we allow suffixes K,M,G,T,P here?
    if (cmd.HasOption(OPT_L))
        try
        {
            Parameters::LIMIT_PRIMES = stoull(cmd.GetOptionValue(OPT_L));
        }
        catch (...)
        {
            cout << "Incorrect limit of numbers is specified '" << cmd.GetOptionValue(OPT_L) << "'. Using unlimited value as default." /* << CheckPrimes::DEFAULT_LIMIT_PRIMES << "." */ << endl << endl;
            Parameters::LIMIT_PRIMES = DEFAULT_LIMIT_PRIMES; //CheckPrimes::DEFAULT_LIMIT_PRIMES;
        }

        //uint64_t offsetPrimes = CheckPrimes::DEFAULT_OFFSET; //TODO shall we allow suffixes K,M,G,T,P here?
        if (cmd.HasOption(OPT_I))
            try
            {
                Parameters::OFFSET_PRIMES = stoull(cmd.GetOptionValue(OPT_I));
            }
            catch (...)
            {
                cout << "Incorrect primes offset is specified '" << cmd.GetOptionValue(OPT_I) << "'. Using default value " << DEFAULT_OFFSET_PRIMES << "." << endl << endl;
                Parameters::OFFSET_PRIMES = DEFAULT_OFFSET_PRIMES;
            }

    if (cmd.HasOption(OPT_H))
    {
        PrintUsage();
    }

    //////////////////////////////////////
    // Processing commands here
    //////////////////////////////////////

    try
    {
        if (cmd.HasOption(OPT_G))
        {
            string alg    = cmd.GetOptionValue(OPT_G, 0);
            string ftype  = cmd.GetOptionValue(OPT_G, 1);
            string start  = cmd.GetOptionValue(OPT_G, 2);
            string length = cmd.GetOptionValue(OPT_G, 3);

            EratosthenesSieve sieve;
            sieve.SetParams(alg, ftype, start, length);
            sieve.Calculate();
        }
        else if (cmd.HasOption(OPT_V))
        {
            string firstArg = cmd.GetOptionValue(OPT_V, 0);
            assert(firstArg.size() > 0);

            if (firstArg[0] == '@') // file name specified, checking primes from file then
            {
                // size of array to be allocated for primes loaded from a file
                // if file contains more primes, only DEF_PRIMES_TO_CHECK will be checked still.
                //TODO think how to avoid this constraint
                //const uint64_t DEF_PRIMES_TO_CHECK = 200'000'000;

                if (cmd.GetOption(OPT_V)->GetArgs().size() > 1) // cannot mix file and primes in one command line for -v, + only one file can be specified
                    throw invalid_argument("Command -v: Cannot mix file name and primes in one command line. also only one file can be specified for this command.");
            
                //uint64_t numPrimesToCheck = Parameters::LIMIT_PRIMES == 0 ? DEF_PRIMES_TO_CHECK : Parameters::LIMIT_PRIMES;
                CheckPrimes pr(Parameters::LIMIT_PRIMES, Parameters::OFFSET_PRIMES);
                pr.Start(firstArg.substr(1), Parameters::THREADS);

            }
            else // check list of primes from command line
            {
                auto& vs = cmd.GetOptionValues(OPT_V);

                CheckPrimes::CheckPrimesArray(vs, cmd.HasOption(OPT_B));
            }
        }
        else if (cmd.HasOption(OPT_C))
        {
            string fromTo = cmd.GetOptionValue(OPT_C, 0);
            string filename = cmd.GetOptionValue(OPT_C, 1);

            FileConverter fc;
            fc.Convert(fromTo, filename, Parameters::LIMIT_PRIMES, Parameters::OFFSET_PRIMES);
        }
        else
        {
            // unknown option, show help
            cout << "No any commands specified in command line. Exiting." << endl;
            PrintUsage();
            return 1;
        }

        cout << "Done" << endl;

        _CrtMemCheckpoint(&s2); // Take a snapshot at the end of main()
        _CrtMemCheckpoint(&s2); // Take a snapshot at the end of main()
        if (_CrtMemDifference(&s3, &s1, &s2)) _CrtMemDumpStatistics(&s3); // Dump memory statistics excluding global variables

        return 0;
    }
    /*catch (invalid_cmd_option& e)
    {
        cout << endl << e.what() << endl << endl;
        PrintUsage();
    }*/
    catch (ios_base::failure& e)
    {
        cout << endl;
        cout << e.what() << endl << endl;
        cout << "Error code: " << e.code() << endl;
    }
    catch (invalid_argument& e)
    {
        cout << endl;
        cout << e.what() << endl << endl;
    }
    catch (exception& e)
    {
        cout << endl << e.what() << endl << endl;
        cout << "Run " << ERA::APP_EXE_NAME << " -h to get more info about command line parameters." << endl;
    }
    catch (...)
    {
        cout << "UNKNOWN error has been met. Exiting" << endl;
        cout << "Run " << ERA::APP_EXE_NAME << " -h to get more info about command line parameters." << endl;
    }

    return 1;
}


static void DefineOptions(COptionsList& options)
{
    COption gg;
    gg.ShortName(OPT_G).LongName("generate").Descr("Generate primes using one of the Eratosthenes sieve algorithms.").RequiredArgs(4);
    options.AddOption(gg);
    //options.AddOption(OPT_G, "generate", "Generate primes using one of the Eratosthenes sieve algorithms.", 4);

    COption vv;
    vv.Descr("Verify that provided numbers are primes.Primes should be separated by spaces in cmd line.\n"
        "If first argument starts from @ then primes will be read from a file that goes right after @").
        ShortName(OPT_V).LongName("verify").RequiredArgs(1).NumArgs(1000);
    options.AddOption(vv);
    //options.AddOption(OPT_V, "verify", "Verify that provided numbers are primes. Primes should be separated by spaces in cmd line.\n" 
    //                         "If first argument starts from @ then primes will be read from a file that goes right after @", 1000);

    options.AddOption(OPT_C, "convert", "Convert file from one format to another.First argument should be in format 'from-to', \n"
        "second argument is a 'from' file name. File name 'to' is build by adding suffix.\n"
        "Accepted 'from-to' formats: txt, txdiff, bin, bindiff, bindiffvar.\n"
        "Special cases: 'bin-bin' and 'bindiffvar-bindiffvar'.", 2);
    options.AddOption(OPT_P, "preload", string("Specify file with primes to preload. If not specified then default '") + Pre_Loaded_Primes_Filename + "' file name is used", 1);
    options.AddOption(OPT_I, "ignore", "Ignore first <num> primes before starting checking for primality.\n"
        "Offset is a number of primes to bypass before start processing.\n"
        "Works with both -c and -v commands.", 1);
    options.AddOption(OPT_L, "limit", "Process specified number of prime numbers only. Works with both -c and -v commands.", 1);
    options.AddOption(OPT_T, "threads", "Use specified number of threads during primes generating and verifying", 1);
    options.AddOption(OPT_H, "help", "Show info about command line arguments.", 0);
    options.AddOption(OPT_N, "nofile", "Do not save file, generate prime numbers only. Works with -g command and used mostly for measuring calculation metrics.", 0);
    options.AddOption(OPT_B, "bigint", "Force use BigInt arithmetic for verifying numbers.", 0);
    options.AddOption(OPT_O, "outfolder", "Output folder where resulting file will be placed. Folder should exist before running app.", 1);

    options.MutuallyExclusive(OPT_G, OPT_V, OPT_C);
}

static void PrintUsage()
{
    cout << endl << "Usage: ";
    cout << ERA::APP_EXE_NAME << " [-g <alg> <fmt> <start> <len>] [-v (@fname)|(<p1>...<pN>)] [-c <from-to> <fname>] [opt] <arg>" << endl << endl;

    cout << "Commands" << endl;
    cout << "  -g, --generate        Generates primes using one of four available Eratosthenes sieve algorithms.\n"
            "                        Algorithms are: simple, optimum, 6n1, 30nx." << endl;
    cout << "  -v, --verify          Verifies that provided numbers are primes (or not). You can either provide list of \n"
            "                        primes in command line or provide a file name from what primes will be read.\n"
            "                        Primes in command line should be separated by spaces. In case of file, filename should\n"
            "                        be provided in first argument and preceeded with @ (not a part of filename)." << endl;
    cout << "  -c, --convert         Converts file with primes from one format to another. First argument should define\n"
            "                        'from' and 'to' formats (in format 'from-to'). Second argument is a 'from' file name.\n"
            "                        File name 'to' is build by adding appropriate suffix to 'from' filename.\n"
            "                        Accepted format ids: txt, txdiff, bin, bindiff, bindiffvar.\n"
            "                        Special cases: 'bin-bin' and 'bindiffvar-bindiffvar'." << endl;

    cout << endl << "Command arguments" << endl;
    cout << "  <alg>                 Defines algorithm to use to generate primes. Possible values: simple, optimum, 6n1, 30nx." << endl;
    cout << "  <format>              Defines output file format. Possible values: txt, txtdiff, bin, bindiff, bindiffvar." << endl;
    cout << "  <start>               Starting value for generating primes." << endl;
    cout << "                        Accepts suffixes B(bytes), K(kilo), M(mega), G(giga), T(tera), P(peta) for big values." << endl;
    cout << "  <length>              Length of a range. Primes are generated in a range: start...start+length." << endl;
    cout << "                        Accepts suffixes B, K, M, G, T, P." << endl;
    cout << "  <@fname>              File name (without @) from where primes will be read and checked." << endl;
    cout << "                        File format is determined automatically by file extention(s)." << endl;
    cout << "  <p1>...<pN>           List of prime numbers specified directly in command line and separated by spaces." << endl;
    cout << "  <from-to>             Two formats for converting files. Examples: txt-bin, bindiff-txt, bindiffvar-bin, etc." << endl;

    cout << endl << "Options" << endl;
    cout << "  -p, --preload <fname> Specifies file with primes to preload. Two formats are supported: txt and bindiffvar." << endl;
    cout << "                        If not specified then default name 'primes - 0-1G.diffvar.bin' is used." << endl;
    cout << "  -t, --threads <num>   Use specified number of threads for generating primes." << endl;
    cout << "  -l, --limit   <num>   Only <num> primes from file will be checked for primality." << endl;
    cout << "  -i, --ignore  <num>   First <num> primes will be ignored (bypassed) in a file before start checking for primality. " << endl;
    cout << "  -n, --nofile          Do not save file, only generate numbers. Used mostly for measuring generating metrics." << endl;
    cout << "  -o, --outfolder       Output folder where resulting file will be placed. Folder should exist before running app." << endl;

    cout << endl << "Examples" << endl;
    cout << "  " << ERA::APP_EXE_NAME << " -g optimum txt 0 10g" << endl;
    cout << "  " << ERA::APP_EXE_NAME << " -g 6n1 bin 9000G 100G -n" << endl;
    cout << "  " << ERA::APP_EXE_NAME << " -g 30nx bindiffvar 100T 10G" << endl;
    cout << "  " << ERA::APP_EXE_NAME << " -g simple txtdiff 1000B 10G -t 5" << endl;
    cout << "  " << ERA::APP_EXE_NAME << " -g optimum bindiff 0M 100M -p pre-generated-primes.txt" << endl;
    cout << "  " << ERA::APP_EXE_NAME << " -v 1235 8747 89741 1111 777771 8573859" << endl;
    cout << "  " << ERA::APP_EXE_NAME << " -v @primesX.bin" << endl;
    cout << "  " << ERA::APP_EXE_NAME << " -c txt-bin myprimes.txt" << endl;
    cout << "  " << ERA::APP_EXE_NAME << " -c bin-txtdiff myprimes2.bin" << endl;

    cout << endl << "If you need help on cmd parameters type: " << ERA::APP_EXE_NAME << " -h" << endl;
}