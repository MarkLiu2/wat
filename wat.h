#ifndef WAT_H
#define WAT_H

#include <stdio.h>
#include <stdlib.h>
#include "log.h"

#define HEADER_SIZE 44

typedef struct WavHeader{

        /* 4 bytes (0 - 4) */
        char chunk_id[5];
        /* 4 bytes (4 - 8) */
        int chunk_size;
        /* 4 bytes (8 - 12) */
        char format[5];

        /* 4 bytes (12 - 16) */
        char subchunk1_id[5];
        /* 4 bytes (16 - 20) */
        int subchunk1_size;
        /* 2 bytes (20 - 22) */
        short int audio_format;
        /* 2 bytes (22 - 24) */
        short int num_channels;
        /* 4 bytes (24 - 28) */
        int sample_rate;
        /* 4 bytes (28 - 32) */
        int byte_rate;
        /* 2 bytes (32 - 34) */
        short int block_align;
        /* 2 bytes (34 - 36) */
        short int bits_per_sample;

        /* 4 bytes (36 - 40) */
        char subchunk2_id[5];
        /* 4 bytes (40 - 44) */
        int subchunk2_size;

} WavHeader;

typedef struct Arguments{

        int print_data;
        int dft;
        int idft;
        int print_help;
        int has_input;
        int has_output;
        int print_hexa_data;
} Arguments;


typedef struct WavInput{

        WavHeader *wav_header;

        int file_size;
        int nb_samples;

        char * file_name;
        char * output_file;

        double *left_side;
        double *right_side;

        short *short_left;
        short *short_right;

        double *zero_data;

        Arguments *wat_args;

} WavInput;

extern WavInput *wav_input;


#endif
