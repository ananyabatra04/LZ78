#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include "io.h"
#include "trie.h"
#include "code.h"
#include "word.h"
#define OPTIONS "vhi:o:"

int main(int argc, char **argv) {
    int opt = 0;
    int statistics = 0;
    int infile = 0;
    int outfile = 1;
    int help = 0;

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'i': infile = open(optarg, O_RDONLY); break;
        case 'o': outfile = open(optarg, O_RDWR | O_CREAT); break;
        case 'v': statistics = 1; break;
        case 'h': help = 1; break;
        }
    }
    if (help) {
        printf("SYNOPSIS\n"
               "    Decompresses files with the LZ78 decompression algorithm.\n"
               "    Used with files compressed with the corresponding encoder.\n"

               "USAGE\n"
               "    ./arm64/decode [-vh] [-i input] [-o output]\n"

               "OPTIONS\n"
               "    -v          Display decompression statistics\n"
               "    -i input    Specify input to decompress (stdin by default)\n"
               "    -o output   Specify output of decompressed input (stdout by default)\n"
               "    -h          Display program usage\n");

    } else {
        if (infile < 0) {
            close(outfile);
            printf("Can't open file.\n");
            exit(0);
        }
        FileHeader header;
        read_header(infile, &header);

        fchmod(outfile, header.protection);
        if (outfile < 0) {
            printf("Can't open file.\n");
            exit(0);
        }

        WordTable *table = wt_create();
        uint8_t curr_sym = 0;
        uint16_t curr_code = 0;
        uint16_t next_code = START_CODE;

        while (read_pair(infile, &curr_code, &curr_sym, log2(next_code) + 1)) {
            table[next_code] = word_append_sym(table[curr_code], curr_sym);
            write_word(outfile, table[next_code]);
            next_code++;
            if (next_code == MAX_CODE) {
                wt_reset(table);
                next_code = START_CODE;
            }
        }
        flush_words(outfile);

        if (statistics) {
            int uncompressed_size = total_syms;
            int compressed_size = 0;

            if (total_bits % 8 == 0) {
                compressed_size = total_bits / 8 + sizeof(FileHeader);
            } else {
                compressed_size = total_bits / 8 + sizeof(FileHeader) + 1;
            }
            float percent_difference = (1.00 - (float) compressed_size / uncompressed_size) * 100;
            fprintf(stderr,
                "Compressed file size: %d bytes\nUncompressed file size: %d bytes\nSpace Saving: "
                "%.2f%%\n",
                compressed_size, uncompressed_size, percent_difference);
        }
        wt_delete(table);

        close(infile);
        close(outfile);
    }
}
