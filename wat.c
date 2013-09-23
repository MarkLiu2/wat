#include "wat.h"
#include "string.h"

WavInput * wav_input = NULL;

int read_header_file(WavHeader * wh, char *file_name)
{
        FILE *f;        
        int ret;
        unsigned char * buffer;

        f = fopen(file_name, "r");
        if(f == NULL){
                printf("\nFile Error in read header");
        }

        buffer = (unsigned char *) malloc(sizeof(unsigned char *) * HEADER_SIZE);
        if(buffer == NULL){
                printf("\nMemory Error");
                return -1;
        }

        ret = fread(buffer, sizeof(unsigned char *), HEADER_SIZE, f);

        fclose(f);

        if(ret < HEADER_SIZE){
                printf("\nSomething got wrong with fread");
                return -1;
        }

        /* getting header information */
        strncpy(wh->chunk_id, (char *)buffer, 4);
        wh->chunk_id[4] = '\0';
        memcpy(&wh->chunk_size, buffer + 4, 4);
        strncpy(wh->format, (char *)buffer + 8, 4);
        wh->format[4] = '\0';

        strncpy(wh->subchunk1_id, (char *)buffer + 12, 4);
        wh->subchunk1_id[4] = '\0';
        memcpy(&wh->subchunk1_size, buffer + 16, 4);
        memcpy(&wh->audio_format, buffer + 20, 2);
        memcpy(&wh->num_channels, buffer + 22, 2);
        memcpy(&wh->sample_rate, buffer + 24, 4);
        memcpy(&wh->byte_rate, buffer + 28, 4);
        memcpy(&wh->block_align, buffer + 32, 2);
        memcpy(&wh->bits_per_sample, buffer + 34, 2);

        strncpy(wh->subchunk2_id, (char *)buffer + 36, 4);
        wh->subchunk2_id[4] = '\0';
        memcpy(&wh->subchunk2_size, buffer + 40, 4);

        /* print header infos */
        printf("\n\nHeader of wav file:\n");
        printf("\nchunk_id => %s", wh->chunk_id);
        printf("\nchunk_size => %d", wh->chunk_size);
        printf("\nformat => %s", wh->format);

        printf("\n\nSubChunk 1:\n");
        printf("\nsubchunk1_id => %s", wh->subchunk1_id);
        printf("\nsubchunk1_size => %d", wh->subchunk1_size);
        printf("\naudio_format => %d", wh->audio_format);
        printf("\nnum_channels => %d", wh->num_channels);
        printf("\nsample_rate => %d", wh->sample_rate);
        printf("\nbyte_rate => %d", wh->byte_rate);
        printf("\nblock_align => %d", wh->block_align);
        printf("\nbits_per_sample => %d", wh->bits_per_sample);

        printf("\n\nSubChunk 2:\n");
        printf("\nsubchunk2_id => %s", wh->subchunk2_id);
        printf("\nsubchunk2_size => %d", wh->subchunk2_size);

        free(buffer);
        return 1;
}

int read_wav_data(WavInput * wi)
{
        FILE *f;
        int ret;
        char * buffer;
        long int data_size;

        data_size = wi->file_size - HEADER_SIZE;
        data_size = data_size / 2 + 1;

        f = fopen(wi->file_name, "r");
        if (f == NULL){
                printf("File Error in read_wav_data");
                return -1;
        }

        buffer = (char *) malloc( sizeof(char)* data_size);

        fseek(f, 44, SEEK_SET);
        fread(buffer, sizeof(char), data_size, f);

        printf("\n\n\n DATA of WAV \n\n");
        short s;
        int i;
        for(i = HEADER_SIZE; i < data_size; i += 2){
                s = (buffer[i+1] << 8) | buffer[i];
                printf(" %+6f ", (double)s/32768.0);
                
                if(i % 13 == 0)
                        printf("\n");
        }

        return 1;
}


int init(WavInput *wi, int argc, char *argv[])
{
        FILE *f;        
        int ret;

        f = fopen(wi->file_name, "r");
        if(f == NULL){
                printf("\nFile Error in init");
                exit(1);
        }

        fseek(f, 0, SEEK_END);
        wi->file_size = ftell(f);

        fclose(f);

        printf("\n\nFile => %s", wi->file_name);
        printf("\nSize => %lu bytes", wi->file_size);

        wav_input->wav_header = malloc(sizeof(WavHeader));
        ret = read_header_file(wav_input->wav_header, wav_input->file_name);

        if (ret < 0)
                exit(0);

        if(argc >= 3 && strcmp(argv[2], "-d") == 0){
                ret = read_wav_data(wav_input);
        }


        return ret;
}

int main(int argc, char *argv[])
{
        int ret;

        wav_input = malloc(sizeof(WavInput));

        if(argc > 1){
                wav_input->file_name = argv[1];
        }
        else{
                printf("\nArgument is required\n\n");
                exit(3);
        }

        ret = init(wav_input, argc, argv);
        if(ret < 0){
                exit(4);
        }

        printf("\n\nend of program\n");
        return 1;
}
