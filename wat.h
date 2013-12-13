#ifndef WAT_H
#define WAT_H

#include "string.h"
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

/*
 * Factors 
 * for which band of frequence
 * is calculated a factor based on the 
 * value passed by parameter (-eq <value>)
 * to be used in equalization
 */
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

        int argc;
        char **argv;
        int has_input;
        int has_output;
        int fft;
        int n_times;
        int one_channel;
        float equalize;
        int raise;
        int print_header;
        int only_sec;

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

/* 
 * s_fft of struct_fft 
 * 
 * is only used in wat.c and carry on all 
 * data needed to call the FFT function.
 *
 */
struct s_fft{
        WavInput * wi;
        double * temp;
        double * channel;
        int it;
        int freq;
        int NFFT;
        int array_size;
        char *log;
        int it_max;
        int num_channels;
        int this_channel;
} s_fft;


extern WavInput *wav_input;

int convert_double_to_short(WavInput *wi);

#endif
