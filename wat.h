#ifndef WAT_H
#define WAT_H

#include <stdio.h>
#include <stdlib.h>
#include "log.h"

#define HEADER_SIZE 44

#define BAND_MIN 0
#define BAND_0  140
#define BAND_1  360
#define BAND_2  640
#define BAND_3 1200
#define BAND_4 2000
#define BAND_5 6000
#define BAND_6 12000
#define BAND_7 24000
#define BAND_8 28000
#define BAND_9 32000
#define BAND_MAX 44100

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

typedef struct Factors{
        float fac_0;
        float fac_1;
        float fac_2;
        float fac_3;
        float fac_4;
        float fac_5;
        float fac_6;
        float fac_7;
        float fac_8;
        float fac_9;
        float fac_10;
} Factors;

typedef struct Arguments{

        int print_data;
        int dft;
        int dft_sec;
        int print_help;
        int has_input;
        int has_output;
        int print_hexa_data;
        int print_nb_data;
        int nb_to_print;
        int fft;
        float equalize;
} Arguments;

typedef struct WavInput{

        WavHeader *wav_header;

        int file_size;
        int nb_samples;

        char * file_name;
        char * output_file;

        double *left_side;
        double *right_side;

        double *left_fixed;
        double *right_fixed;

        double *zero_data;

        short *short_left;
        short *short_right;

        unsigned char * buffer;

        Arguments *wat_args;
        Factors *factors;

} WavInput;

typedef struct s_fft{
        WavInput * wi;
        double * temp;
        double * channel;
        int it;
        int freq;
        int NFFT;
        int array_size;
        char *log;
} s_fft;


extern WavInput *wav_input;


#endif
