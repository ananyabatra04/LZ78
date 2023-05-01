# Assignment 6
**Ananya Batra - abatra5**

In assignment 6, I wrote programs to perform LZ78 compression on any text or binary file and a program to decompress any file compressed with the previous algorithm, respectively. Both programs operate on big and little endian systems. A user can compress a file with encode.c, which implements LZ78 compression. LZ78 compression is a lossless compression that compresses a file by storing a sequence of bytes (words) into a dictionary that gets built during the compression process. Words are denoted by codes in the dictionary. The compression algorithm works by replacing any future occurrence of a word that exists in a dictionary with its code. These words are stored in a Trie ADT, implemented by trie.c. Each TrieNode contains a symbol and n child nodes (256 in our case). In addition to trie.c, encode.c also calls upon functions in io.c. For this assignment, both encode and decode programs perform read and writes in blocks of 4KB. io.c contains functions in order to facilitate efficient reading and writing. A user can decompress a file (compressed with encode.c) with decode.c. Decompression works by translating codes to words (using a lookup table called a Word Table which is just an array of words). The Word ADT is implemented in word.c. Makefile compiles all the c program files and builds the encode and decode executables.


To build the program:
1. In your terminal, set the file path to wherever all the files for this assignment are stored
2. Call make encode to build the encode executable and call make decode to build the decode executable. Calling make or make all will build both programs and make clean will remove all compiler generated files.
3. If running encode: type ./encode -[vhi:o:] [-i infile] [-o outfile]
4. If running decode: type ./decode -[vhi:o:] [-i infile] [-o outfile]

Command-line options for encode: 
-   -v              Print compression statistics to stderr
-   -i infile       Specifies input to compress (default: stdin)
-   -o outfile      Specifies output of compressed input (default: stdout)
-   -h              Prints help message and program usage

Command-line options for encode: 
-   -v              Print decompression statistics to stderr
-   -i infile       Specifies input to decompress (default: stdin)
-   -o outfile      Specifies output of decompressed input (default: stdout)
-   -h              Prints help message and program usage
