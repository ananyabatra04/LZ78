#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "code.h"
#include "endian.h"
#include "io.h"

uint64_t total_syms = 0; // To count the symbols processed.
uint64_t total_bits = 0; // To count the bits processed.

static uint8_t bit_buffer[BLOCK] = { 0 };
static uint8_t sym_buffer[BLOCK] = { 0 };
static int bit_curr_index = 0;
static int sym_curr_index = 0;
static int max_sym_index = -1;
static int max_bit_index = -1;

int read_bytes(int infile, uint8_t *buf, int to_read) {
    int total_bytes_read = 0;
    size_t bytes_read_in = 0;

    while (total_bytes_read <= to_read) {
        bytes_read_in = read(infile, buf + total_bytes_read, to_read - total_bytes_read);
        if (bytes_read_in == 0) {
            return total_bytes_read;
        }
        total_bytes_read += bytes_read_in;
    }
    return total_bytes_read;
}

int write_bytes(int outfile, uint8_t *buf, int to_write) {
    int total_bytes_written = 0;
    size_t bytes_written = 0;

    while (total_bytes_written <= to_write) {
        bytes_written = write(outfile, buf + total_bytes_written, to_write - total_bytes_written);
        if (bytes_written == 0) {
            return total_bytes_written;
        }
        total_bytes_written += bytes_written;
    }
    return total_bytes_written;
}

void read_header(int infile, FileHeader *header) {
    read_bytes(infile, (uint8_t *) header,
        sizeof(FileHeader)); //reading header from infile directly into header

    if (big_endian()) {
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }
    if (header->magic != MAGIC) {
        printf("Magic number is incorrect; exiting...\n");
        exit(0);
    }
}

void write_header(int outfile, FileHeader *header) {
    if (big_endian()) {
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }
    write_bytes(outfile, (uint8_t *) header, sizeof(FileHeader));
}

bool read_sym(int infile, uint8_t *sym) {
    int end_of_buf = 0;
    if (max_sym_index == -1) { //if it's the first time reading into the buffer
        max_sym_index = read_bytes(infile, sym_buffer, BLOCK);
        sym_curr_index = 0;
        total_syms += max_sym_index;
    }
    *sym = sym_buffer[sym_curr_index];
    if (sym_curr_index < max_sym_index - 1) {
        sym_curr_index++;
        return true;
    } else if (max_sym_index == BLOCK) {
        //end_of_buf = 0;
        sym_curr_index = 0;
        max_sym_index = read_bytes(infile, sym_buffer, BLOCK);
        total_syms += max_sym_index;
        if (max_sym_index == 0) { //this means there are no more bytes to be read
            return false;
        }
        return true;
    } else if (end_of_buf == 0 && sym_curr_index == max_sym_index - 1) {
        end_of_buf++;
        sym_curr_index++;
        return true;
    }
    return false;
}

void write_pair(int outfile, uint16_t code, uint8_t sym, int bitlen) {
    for (int i = 0; i < bitlen; i++) {
        if (bit_curr_index >= BLOCK * 8) {
            flush_pairs(outfile);
            //total_bits += bitlen;
        }
        if ((code >> i)
            & 1) { //getting the ith bit from code to the lowest bit position -> this returns true only if the ith bit is 1
            bit_buffer[bit_curr_index / 8]
                = bit_buffer[bit_curr_index / 8]
                  | (1 << (bit_curr_index
                           % 8)); //setting the bit in bit_buffer's current byte index
        }
        bit_curr_index++;
    }

    for (int i = 0; i < 8; i++) {
        if (bit_curr_index >= BLOCK * 8) {
            flush_pairs(outfile);
            //total_bits += 8;
        }
        if ((sym >> i) & 1) {
            bit_buffer[bit_curr_index / 8]
                = bit_buffer[bit_curr_index / 8] | (1 << (bit_curr_index % 8));
        }
        bit_curr_index++;
    }
}

void flush_pairs(int outfile) {
    int curr_index = 0;
    if (bit_curr_index % 8 == 0) {
        curr_index = bit_curr_index / 8;
    } else {
        curr_index = bit_curr_index / 8 + 1; //to round up
    }
    total_bits += write_bytes(outfile, bit_buffer, curr_index) * 8;
    bit_curr_index = 0;
    memset(bit_buffer, 0, BLOCK);
}

bool read_pair(int infile, uint16_t *code, uint8_t *sym, int bitlen) {
    *code = 0;
    *sym = 0;
    if (max_bit_index == -1) {
        max_bit_index = read_bytes(infile, bit_buffer, BLOCK) * 8;
        bit_curr_index = 0;
    }
    for (int i = 0; i < bitlen; i++) {
        if (bit_curr_index >= max_bit_index) {
            memset(bit_buffer, 0, BLOCK);
            max_bit_index = read_bytes(infile, bit_buffer, BLOCK) * 8;
            bit_curr_index = 0;
        }
        if (bit_buffer[bit_curr_index / 8] >> (bit_curr_index % 8) & 1) {
            *code = *code | (1 << i);
        }
        bit_curr_index++;
    }
    total_bits += bitlen;

    for (int i = 0; i < 8; i++) {
        if (bit_curr_index >= max_bit_index) {
            memset(bit_buffer, 0, BLOCK);
            max_bit_index = read_bytes(infile, bit_buffer, BLOCK) * 8;
            bit_curr_index = 0;
        }
        if (bit_buffer[bit_curr_index / 8] >> (bit_curr_index % 8) & 1) {
            *sym = *sym | (1 << i);
        }
        bit_curr_index++;
    }
    total_bits += 8;

    return (*code != STOP_CODE);
}

void write_word(int outfile, Word *w) {
    for (uint32_t i = 0; i < w->len; i++) {
        if (sym_curr_index >= BLOCK) {
            flush_words(outfile);
        }
        sym_buffer[sym_curr_index] = w->syms[i];
        sym_curr_index++;
    }
}

void flush_words(int outfile) {
    total_syms += write_bytes(outfile, sym_buffer, sym_curr_index);
    sym_curr_index = 0;
    memset(sym_buffer, 0, BLOCK);
}
