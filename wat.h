#ifndef WAT_H
#define WAT_H

#include <stdio.h>
#include <stdlib.h>

#define HEADER_SIZE 44

typedef struct WavHeader{
        /* 4 bytes (0 - 4) */
        char chunk_id[4];
        /* 4 bytes (4 - 8) */
        int chunk_size;
        /* 4 bytes (8 - 12) */
        char format[4];

        /* 4 bytes (12 - 16) */
        char subchunk1_id[4];
        /* 4 bytes (16 - 20) */
        int subchunk1_size;
        /* 2 bytes (20 - 22) */
        int audio_format;
        /* 2 bytes (22 - 24) */
        int num_channels;
        /* 4 bytes (24 - 28) */
        int sample_rate;
        /* 4 bytes (28 - 32) */
        int byte_rate;
        /* 2 bytes (32 - 34) */
        int block_align;
        /* 2 bytes (34 - 36) */
        int bits_per_sample;

        /* 4 bytes (36 - 40) */
        char subchunk2_id[4];
        /* 4 bytes (40 - 44) */
        int subchunk2_size;

} WavHeader;


extern WavHeader *wav_header;


#endif
