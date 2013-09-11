#ifndef WAT_H
#define WAT_H

#include <stdio.h>
#include <stdlib.h>

#define HEADER_SIZE 44

typedef struct WavHeader{
        char *chunk_id;
        int chunk_size;
        char format[4];

        char subchunk1_id[4];
        int subchunk1_size;
        int audio_format;
        int num_channels;
        int sample_rate;
        int byte_rate;
        int block_align;
        int bits_per_sample;

        char subchunk2_id[4];
        int subchunk2_size;

} WavHeader;


extern WavHeader *wav_header;


#endif
