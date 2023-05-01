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
#define OPTIONS "vhi:o:"
int bitlen(uint16_t num) {
    int length = 0;
    while (num != 0) {
        num /= 2;
        length++;
    }
    return length;
}

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
               "    Compresses files using the LZ78 compression algorithm.\n"
               "    Compressed files are decompressed with the corresponding decoder.\n"

               "USAGE\n"
               "    ./arm64/encode [-vh] [-i input] [-o output]\n"

               "OPTIONS\n"
               "    -v          Display compression statistics\n"
               "    -i input    Specify input to compress (stdin by default)\n"
               "    -o output   Specify output of compressed input (stdout by default)\n"
               "    -h          Display program help and usage\n");

    } else {
        if (infile < 0) {
            printf("Can't open file.\n");
            close(outfile);
            exit(0);
        }
        if (outfile < 0) {
            printf("Can't open file.\n");
            exit(0);
        }
        struct stat buffer;
        fstat(infile, &buffer);
        FileHeader *header = calloc(1, sizeof(FileHeader));
        header->magic = MAGIC;
        header->protection = buffer.st_mode;
        fchmod(outfile, buffer.st_mode);
        write_header(outfile, header);

        TrieNode *root = trie_create();
        TrieNode *curr_node = root;
        TrieNode *prev_node = NULL;
        TrieNode *next_node;
        uint8_t curr_sym = 0;
        uint8_t prev_sym = 0;
        uint16_t next_code = START_CODE;

        while (read_sym(infile, &curr_sym)) {
            next_node = trie_step(curr_node, curr_sym);
            if (next_node != NULL) {
                prev_node = curr_node;
                curr_node = next_node;
            } else {
                write_pair(outfile, curr_node->code, curr_sym, bitlen(next_code));
                curr_node->children[curr_sym] = trie_node_create(next_code);
                curr_node = root;
                next_code = next_code + 1;
            }
            if (next_code == MAX_CODE) {
                trie_reset(root);
                curr_node = root;
                next_code = START_CODE;
            }
            prev_sym = curr_sym;
        }
        if (curr_node != root) {
            write_pair(outfile, prev_node->code, prev_sym, bitlen(next_code));
            next_code = (next_code + 1) % MAX_CODE;
        }
        write_pair(outfile, STOP_CODE, 0, bitlen(next_code));
        flush_pairs(outfile);
        trie_delete(root);

        if (statistics) {
            int uncompressed_size = total_syms;
            int compressed_size = 0;
            if (total_bits % 8 == 0) {
                compressed_size = total_bits / 8 + sizeof(FileHeader);
            } else {
                compressed_size = total_bits / 8 + sizeof(FileHeader);
            }
            float percent_difference = (1.00 - (float) compressed_size / uncompressed_size) * 100;
            fprintf(stderr,
                "Compressed file size: %d bytes\nUncompressed file size: %d bytes\nSpace Saving: "
                "%.2f%%\n",
                compressed_size, uncompressed_size, percent_difference);
        }
        free(header);
        close(infile);
        close(outfile);
    }
}
