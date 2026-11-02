# Eratosthen primes generating and verifying tool

This is a tool that runs several Eratosthen algorithms for generating prime numbers in specified range, checks numbers for primality and converts files with primes from one format to another.

## Generating primes
### Command line arguments

```
Usage:
Eratosthen.exe [-g <alg> <format> <start> <len>] [-v (@fname)|(<p1>...<pN>)] [-c <from-to> <fname>] [opt] <arg>" 

Commands:
   -g, --generate        Generates primes using one of four available Eratosthenes sieve algorithms: simple, optimum, 6n1, 30nx.
   -v, --verify          Verifies that provided numbers are primes (or not). You can either provide list of primes in command line or provide a file name from what primes will be read.
                         Primes in command line should be separated by spaces. In case of file, filename should be provided in first argument and preceeded with @ (not a part of filename).
   -c, --convert         Converts file with primes from one format to another. First argument should define 'from' and 'to' formats (in format 'from-to'). 
                         Second argument is a 'from' file name. File name 'to' is build by adding appropriate suffix to 'from' filename.
                         Accepted format ids: txt, txdiff, bin, bindiff, bindiffvar. Special cases: 'bin-bin' and 'bindiffvar-bindiffvar'.

Command arguments:
   <alg>                 Defines algorithm to use to generate primes. Possible values: simple, optimum, 6n1, 30nx.
   <format>              Defines output file format. Possible values: txt, txtdiff, bin, bindiff, bindiffvar.
   <start>               Starting value for generating primes.
                          Accepts suffixes B(bytes), K(kilo), M(mega), G(giga), T(tera), P(peta) for big values.
   <length>              Length of a range. Primes are generated in a range: start...start+length.
                         Accepts suffixes B, K, M, G, T, P.
   <@fname>              File name (without @) from where primes will be read and checked.
                         File format is determined automatically by file extention(s).
   <p1>...<pN>           List of prime numbers specified directly in command line and separated by spaces.
   <from-to>             Two formats for converting files. Examples: txt-bin, bindiff-txt, bindiffvar-bin, etc.

Options:
   -p, --preload <fname> Specifies file with primes to preload. Two formats are supported: txt and bindiffvar.
                         If not specified then default name 'primes - 0-1G.diffvar.bin' is used.
   -t, --threads <num>   Use specified number of threads for generating primes.
   -l, --limit   <num>   Only <num> primes from file will be checked for primality.
   -i, --ignore  <num>   First <num> primes will be ignored (bypassed) in a file before start checking for primality.
   -n, --nofile          Do not save file, only generate numbers. Used mostly for measuring generating metrics.
   -o, --outfolder       Output folder where resulting file will be placed. Folder should exist before running app.

   If you need help on cmd parameters type: Eratosthen.exe -h

```
You can specify only one command (either `-g`, `-v` or `-c`) with required arguments and any number of options related to the command.

### Prime generation

There are four primes generating algorithms: simple, optimum, 6n1 and 30nx.
Eratosthen algorithm itself requires much memory for execution.
Simple mode is a straightforward and simple implementation of Eratosthen algorithm. 
Optimum mode requires about two times less memory than simple mode because it does not store even numbers.
6n1 mode uses fact that any prime number may have only reminders either 1 or 5 when dividing by 6.
It does not store numbers that have other reminders. This way it saves memory.
30nx this is extension of 6n1 mode to divider 30. It uses even less memory than 6n1 and slightly faster then 6n1.

### Output file type

Generated primes numbers can be stored into 5 different file formats.
You can specify any suitable for you format in command line.

Supported formats: txt, txtdiff, bin, bindiff, bindiffvar.

* `txt` - standard txt format where all prime numbers are stored as comma separated decimal numbers and can be easily viewed in notepad. Files of this format have the biggest size.
* `txtdiff` - this is also text format that can be viewed in notepad. Each prime number starting from second is written as difference with previous one. First prime number is written as is.
That allows to make output file much smaller. It becomes especially important is you need to generate big primes number and in a big range (let say 100G range or even more).
* `bin` - numbers are stored in binary format. Each prime number in a file takes exacly 8 bytes (long long in C++). To work with this format you probably need to write your code to read this format. This is easy.
Please pay attention that some other programming languages and/or platforms may use other sequence of bytes when saving/reading binary integers from file.
* `bindiff` - idea is the same as `txtdiff` but in binary format. Only 2 bytes (short type in C++) instead of 8 are used for saving diff values. It makes bindiff file four times smaller than .bin file.
* `bindiffvar` - main idea is the following: prime numbers usually stay not too far from each other. For even really big numbers (for numbers that fit into C++ long long type) maximum distance between two consequitive primes does not exceed 800.
Is most cases distance is even less than 255. It means that if distance is less than 255 we can code this difference with 1 byte, and only for rare cases when the difference is larger than 255 we can code such value with 2 bytes.
In addition to above, distance between two primes numbers is always even. So we can store diff/2.
That idea allows us to create even more 'compressed' format of storing such numbers. `bindiffvar` format uses 'variable length' coding to store difference between two consequitive primes numbers.
it makes file with primes numbers 10 times smaller size than txt format.

To specify file format use predefined names: txt, txtdiff, bindiff, bindiffvar as the second argument (<format>) of `-g` option.

### Start and Length parameters

Third and fours arguments of `-g` command define START and LENGTH range parameters for generating primes.
You can use factor modificators (case insensitive) to make it more convenient to define range. <br>
B - bytes

K - kilobytes

M - megabytes

G - gigabytes

T - terabytes

P - petabytes

Suffix B can be omitted, in both cases value will threated as value in bytes.

Examples for command lines arguments:

```
Erastofen.exe -g simple txt 0B 10g          - 'simple' algorithm, txt file format, seeks primes in a range 0...10G 
Erastofen.exe -g optimum bin 900G 100G      - 'optimum' algorithm, bin file format, seeks primes from 900G to 1000G
Erastofen.exe -g 6n1 txtdif 100T 10G        - '6n1' algorithm, txtdiff file format, seeks primes from 100 terabytes =100'000'000'000'000 to 100T+10G =100'010'000'000'000
Erastofen.exe -g 6n1 bindiff 5T 50g         - '6n1' algorithm, bindiff file format, START=5T and LENGTH=50G (5T...5T+50G)
Erastofen.exe -g optimum bindiffvar 20P 1G  - 'optimum' algorithm, bindiffvar file format, START=20T and LENGTH=1G
Erastofen.exe -g 30nx bin 0P 1G             - if you would like to generate primes starting from zero you can use any modificator for START argument, or without modificator.
Erastofen.exe -t 5 -g 30nx bindiffvar 20P 1G    - the same as above but primes generation will be performed in 5 threads + 1 main and controling thread.
Erastofen.exe -t 5 -p "primes - 3G.diffvar.bin" -g 30nx bindiffvar 20P 1G   - the same as above but primes will be preloaded from specified file instead of default one.
```

### Options `threads` and `preload`

It is possible to do primes generation in several threads.
Specify option `-t` followed by number of threads you need.
Number of threads can be between 1 and 24.
Please note that N threads do not increase speed in N times because of synchronization expenses.

`Eratosthen.exe` uses pre-generated prime numbers for calculation other primes.
Pre-generated prime numbers are stored in `primes - 0-1G.diffvar.bin` file that is used by default.
Pre-generated prime numbers of 0-1G range allow us to generate other prime numbers from 1G to 1e18.

If you would like to use another file with pre-generated prime numbers you can use option `-p` to tell `Eratosthen.exe` to read primes from that file.

## Check numbers for primality

Application can verify number of primality.
It is done using simple deterministic algoritm by dividing number to all numbers from 2 till sqrt(number).
Number to verify can be specified in command line as arguments of `-v` command or loaded from file. All 5 file formats mentioned above are supported.
In case of loading numbers from file, file name should be specified as first argument of `-v` command preceeded with '@'. '@' is not a part of file name. 
Only one such file can be specified this way.

If you speicifed a file to read numbers from, you can specify two additional options: `-i` and `-l` 
Option `-i` tells how many numbers need to be skipped (ignored) before starting verification.
Option `-l` tells how many numbers to verify. Skipped numbers are not counted to numbers to verify.


Examples:

```
Erastofen.exe -v 101 1234007 55739 485737593  - verify 4 numbers provided in command line 
Erastofen.exe -v @mynumbers.txt               - verify all numbers read from file
Erastofen.exe -v @nums.txt -i 1000 -l 100     - will skip first 1000 numbers, then verify 100 numbers from a file nums.txt

```

## Convert files with primes

Application has a function to convery files from one format to another (option `-c`).
All 5 formats are supported with any conversions between them.
`-c` requires two arguments 'from-to' and input file name.
Output file name will be build from input file name by adding extension of 'to' format accordingly.

Examples:

```
Erastofen.exe -c txt-bin mynums.txt               - convert mynums.txt from txt format to bin. Resulting file will have name mynums.txt.bin 
Erastofen.exe -c bindiff-txtdiff mynums.diff.bin  - convert mynums.diff.bin into txtbon format. Resulting file will have name mynums.diff.bin.diff.txt
Erastofen.exe -c txt-bindiffvar nums.txt          - convert nums.txt into bindiffvar format. Resulting file will have name nums.txt.diffvar.bin

```

