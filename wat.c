#include "wat.h"
#include "string.h"
#include <stdint.h>

WavInput * wav_input = NULL;

static int print_data = 0; 

int read_header_file(WavHeader * wh, char *file_name)
{
        FILE *f;        
        int ret;
        unsigned char * buffer;

        f = fopen(file_name, "r");
        if(f == NULL){
                wat_log(LOG_INFO, "\nFile Error");
                wat_log(LOG_ERROR, " in read_header_file");
                return -1;
        }

        buffer = (unsigned char *)malloc(sizeof(unsigned char *) * HEADER_SIZE);

        if(buffer == NULL){
                wat_log(LOG_ERROR, "\nMemory Error in read_header_file");
                return -1;
        }

        ret = fread(buffer, sizeof(unsigned char *), HEADER_SIZE, f);
        fclose(f);

        if(ret < HEADER_SIZE){
                wat_log(LOG_ERROR, "\nSomething got wrong with fread");
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

        wat_log(LOG_PANIC, "\nRead all header infos, printing than now.");

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
        printf("\nsubchunk2_size => %lu", wh->subchunk2_size);

        free(buffer);
        wat_log(LOG_PANIC, "\nHeader printed.");
        return 1;
}

int print_wav_data(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nPrinting all data");
        if(wi->left_side){
                int i;
                printf("\nLEFT SIDE\n\n");
                for(i = 0; i < wi->nb_samples; i++){
                        printf(" %+7f ", wi->left_side[i]);
                        if(i % 11 == 0){
                                printf("\n");
                        }
                }
        }
        if(wi->right_side){
                int i;
                printf("\nRIGHT SIDE\n\n");
                for(i = 0; i < wi->nb_samples; i++){
                        printf(" %+7f ", wi->right_side[i]);
                        if(i % 11 == 0){
                                printf("\n");
                        }
                }
        }
        wat_log(LOG_PANIC, "\n\nAll data printed");
        return 1;
}

double bytes_to_double(char first, char second)
{
        short data;
        data = (second << 8) | first;
        return data / 32768.0;
}

int read_wav_data(WavInput * wi)
{
        FILE *f;
        int ret;
        char * buffer;
        long int data_size;

        data_size = wi->file_size - HEADER_SIZE;
        //data_size = data_size/2;

        f = fopen(wi->file_name, "r");
        if (f == NULL){
                wat_log(LOG_INFO, "\nFile Error");
                wat_log(LOG_ERROR, " in read_wav_data");
                return -1;
        }

        buffer = (char *) malloc( sizeof(char)* data_size);

        fseek(f, HEADER_SIZE, SEEK_SET);
        fread(buffer, sizeof(char), data_size, f);

        long int nb_samples;

        if(wi->wav_header->num_channels == 2){
                nb_samples = data_size / 4;

                wi->right_side = (double *)malloc(sizeof(double) * nb_samples);
                wi->left_side = (double *)malloc(sizeof(double) * nb_samples);
        }
        else if (wi->wav_header->num_channels == 1){
                nb_samples = data_size / 2;
                wi->left_side = (double *)malloc(sizeof(double) * nb_samples);
                wi->right_side = NULL;
        }
        else {
                return -3;
        }
        wi->nb_samples = nb_samples;

        long int i = 0;
        long int it = 0;
        while(it < data_size){
                wi->left_side[i] = bytes_to_double(buffer[it], buffer[it+1]);
                it += 2;
                if(wi->wav_header->num_channels == 2){
                        wi->right_side[i] = bytes_to_double(buffer[i * 4 + 2], 
                                        buffer[i * 4 + 3]);
                        it += 2;
                }
                i++;
        }
        if(print_data)
                ret = print_wav_data(wav_input);

        return ret;
}

int parse_args(char *argv)
{
        int ret = 1;

        if(strcmp(argv, "-d") == 0){
                print_data = 1;
        }
        else if(strcmp(argv, "-log_level=INFO") == 0){
                set_log_level(LOG_ERROR);
        }
        else if(strcmp(argv, "-log_level=ERROR") == 0){
                set_log_level(LOG_ERROR);
        }
        else if(strcmp(argv, "-log_level=PANIC") == 0){
                set_log_level(LOG_PANIC);
        }
        else{
                char msg[60];
                sprintf(msg, "\nParametro \"%s\" nao encontrado\n", argv);
                wat_log(LOG_INFO, msg);
                ret = -1;
        }

        return ret;
}

int init(WavInput *wi, int argc, char *argv[])
{
        FILE *f;        
        int ret;

        f = fopen(wi->file_name, "r");
        if(f == NULL){
                wat_log(LOG_INFO,"\nErro ao abrir o arquivo\n");
                wat_log(LOG_ERROR," em init\n");
                exit(1);
        }

        fseek(f, 0, SEEK_END);
        wi->file_size = ftell(f);
        fclose(f);

        printf("\n\nFile => %s", wi->file_name);
        printf("\nSize => %lu bytes", wi->file_size);

        wat_log(LOG_PANIC, "\nLeu arquivo\n");

        int i;
        for(i = 2; i < argc; i++){
                ret = parse_args(argv[i]);
                if(ret < 1)
                        break;
        }

        wat_log(LOG_PANIC, "\nFim do parser dos argumentos\n");

        return ret;
}

int main(int argc, char **argv)
{
        int ret;

        wav_input = (WavInput *)malloc(sizeof(WavInput));

        if(argc > 1){
                wav_input->file_name = argv[1];
        }
        else{
                wat_log(LOG_INFO, "\nArgument is required\n\n");
                exit(3);
        }

        ret = init(wav_input, argc, argv);

        if(ret < 0){
                exit(4);
        }

        wav_input->wav_header = malloc(sizeof(WavHeader));
        ret = read_header_file(wav_input->wav_header, wav_input->file_name);

        if (ret < 0)
                exit(0);


        ret = read_wav_data(wav_input);

        if(ret < 0){
                exit(4);
        }

        printf("\n\nend\n");
        return 1;
}
