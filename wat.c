#include "wat.h"
#include "string.h"
#include <stdint.h>
#include "dft.h"

WavInput * wav_input = NULL;


void help()
{
        printf("\n\tWAT");
        printf("\n\nParameters:");
        printf("\n\t-h                   Help.");
        printf("\n\t-i <imput file>      Imput file.");
        printf("\n\t-o <output file>     Imput file.");
        printf("\n\t-d                   Print data of input file after and before dft.");
        printf("\n\t-dft                 Run DFT in input file.");
        printf("\n\t-idft                Run IDFT in input file.");
        printf("\n\t-log_level=<level>   Define the level of logging, can be"
                        "INFO (default), ERROR and PANIC.");

        printf("\n\n");
}

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
                char msg[50];
                sprintf(msg, "\nSomething got wrong with fread, ret = %d\n", ret);
                wat_log(LOG_ERROR, msg);
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
        printf("\nsubchunk2_size => %d", wh->subchunk2_size);

        free(buffer);
        wat_log(LOG_PANIC, "\n\nHeader printed.");
        return 1;
}

int print_hexa_data(WavInput *wi, unsigned char *buffer)
{
        wat_log(LOG_PANIC, "\n\nPrinting hexa data.");
        int i;
        for(i = 0; i < wi->wav_header->subchunk2_size; i++){
                printf(" %.2X ", buffer[i]);
                if(i % 10 == 0)
                        printf("\n");
        }
        wat_log(LOG_PANIC, "\n\nPrinting hexa data DONE.");

        return 1;
}

int print_wav_data(WavInput *wi)
{
        wat_log(LOG_PANIC, "\n\nPrinting all data");
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
        return data;// / 32768.0;
}

int read_wav_data(WavInput * wi)
{
        wat_log(LOG_PANIC, "\nIn read_wav_data\n");
        FILE *f;
        int ret = 1;
        unsigned char * buffer;
        long int data_size;

        data_size = wi->file_size - HEADER_SIZE;
        //data_size = data_size/2;

        f = fopen(wi->file_name, "r");
        if (f == NULL){
                wat_log(LOG_INFO, "\nFile Error");
                wat_log(LOG_ERROR, " in read_wav_data");
                return -1;
        }

        buffer = (unsigned char *) malloc(sizeof(unsigned char) * data_size);

        fseek(f, HEADER_SIZE, SEEK_SET);
        fread(buffer, sizeof(unsigned char), data_size, f);

        if(wi->wat_args->print_hexa_data){
                wat_log(LOG_PANIC, "\nprint_hexa_data");
                print_hexa_data(wi, buffer);
        }

        long int nb_samples;

        if(wi->wav_header->num_channels == 2){
                wat_log(LOG_INFO, "\nWav with 2 channels.");
                nb_samples = data_size / 4;

                wi->right_side = (double *)malloc(sizeof(double) * nb_samples);
                wi->left_side = (double *)malloc(sizeof(double) * nb_samples);
                wi->zero_data = (double *)malloc(sizeof(double) * nb_samples);
        }
        else if (wi->wav_header->num_channels == 1){
                wat_log(LOG_INFO, "\nWav with 1 channel.");
                nb_samples = data_size / 2;

                wi->left_side = (double *)malloc(sizeof(double) * nb_samples);
                wi->right_side = NULL;
                wi->zero_data = (double *)malloc(sizeof(double) * nb_samples);
        }
        else {
                wat_log(LOG_INFO, "\nNumber of channels invalid.");
                return -3;
        }
        wi->nb_samples = nb_samples;

        long int i = 0;
        long int it = 0;
        while(it < data_size){
                wi->left_side[i] = bytes_to_double(buffer[it], buffer[it+1]);
                wi->zero_data[i] = 0;
                it += 2;
                if(wi->wav_header->num_channels == 2){
                        wi->right_side[i] = bytes_to_double(buffer[i * 4 + 2], 
                                        buffer[i * 4 + 3]);
                        it += 2;
                }
                i++;
        }


        char msg[50];
        sprintf(msg, "\n read_wav_data DONE, ret = %d", ret);
        wat_log(LOG_PANIC, msg);

        return ret;
}

int parse_args(Arguments *wa, char *argv)
{
        int ret = 1;

        if(strcmp(argv, "-d") == 0){
                wa->print_data = 1;
        }
        else if(strcmp(argv, "-h") == 0){
                help();
                exit(1);
        }
        else if(strcmp(argv, "-x") == 0){
                wa->print_hexa_data = 1;
        }
        else if(strcmp(argv, "-i") == 0){
                wa->has_input = 1;
                return 2;
        }
        else if(strcmp(argv, "-o") == 0){
                wa->has_output = 1;
                return 3;
        }
        else if(strcmp(argv, "-dft") == 0){
                wa->dft = 1;
        }
        else if(strcmp(argv, "-idft") == 0){
                wa->idft = 1;
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
        int i;

        wi->wat_args = (Arguments *)malloc(sizeof(Arguments));

        wi->wat_args->has_input = 0;
        wi->wat_args->has_output = 0;
        wi->wat_args->print_data = 0;
        wi->wat_args->print_help = 0;
        wi->wat_args->dft = 0;
        wi->wat_args->idft = 0;

        for(i = 1; i < argc; i++){
                ret = parse_args(wi->wat_args, argv[i]);
                if(ret == 2){
                        wi->file_name = argv[i + 1];
                        i++;
                }
                else if (ret == 3){
                        wi->output_file = argv[i + 1];
                        i++;
                }
                else if(ret < 1)
                        exit(1);
        }
        wat_log(LOG_PANIC, "\nParse_args DONE\n");

        f = fopen(wi->file_name, "r");
        if(f == NULL){
                char * msg = malloc(64 * sizeof(char));
                sprintf(msg, "\nError while openning input file: \"%s\" \n", wi->file_name);
                wat_log(LOG_INFO, msg);
                wat_log(LOG_ERROR," in init\n");
                exit(1);
        }

        fseek(f, 0, SEEK_END);
        wi->file_size = ftell(f);
        fclose(f);

        printf("\n\nFile => %s", wi->file_name);
        printf("\nSize => %lu bytes", wi->file_size);

        wat_log(LOG_PANIC, "\nRead file DONE\n");

        char msg[50];
        sprintf(msg, "\nInit DONE. ret => %d", ret);
        wat_log(LOG_PANIC, msg);
        return ret;
}

int convert_double_to_short(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nConverting double to short");

        if(wi->wav_header->num_channels == 2){
                wi->short_left = (short int *)malloc(sizeof(short int) * wi->nb_samples);
                wi->short_right = (short int *)malloc(sizeof(short int) * wi->nb_samples);
        }
        else
        {
                wi->short_left = (short int *)malloc(sizeof(short int) * wi->nb_samples);
        }

        int i;
        for(i = 0; i < wi->nb_samples; i++){
                wi->short_left[i] = (short int)wi->left_side[i];
                if(wi->wav_header->num_channels == 2){
                        wi->short_right[i] = (short int)wi->right_side[i];
                }
        }
        wat_log(LOG_PANIC, "\nConverted double to short");

        return 1;
}

int save_file(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nIn save_file");
        FILE *f;

        f = fopen(wi->output_file, "wb");
        if(f == NULL){
                char msg[50];
                sprintf(msg, "\nError while openning the output file: \"%s\".", 
                                wi->output_file);
                wat_log(LOG_INFO, msg);
        }

        WavHeader * wh = wi->wav_header;

        fwrite(wh->chunk_id, sizeof(char), 4, f);
        fwrite(&wh->chunk_size, sizeof(int), 1, f);
        fwrite(wh->format, sizeof(char), 4, f);

        fwrite(wh->subchunk1_id, sizeof(char), 4, f);
        fwrite(&wh->subchunk1_size, sizeof(int), 1, f);
        fwrite(&wh->audio_format, sizeof(short int), 1, f);
        fwrite(&wh->num_channels, sizeof(short int), 1, f);
        fwrite(&wh->sample_rate, sizeof(int), 1, f);
        fwrite(&wh->byte_rate, sizeof(int), 1, f);
        fwrite(&wh->block_align, sizeof(short int), 1, f);
        fwrite(&wh->bits_per_sample, sizeof(short int), 1, f);

        fwrite(wh->subchunk2_id, sizeof(char), 4, f);
        fwrite(&wh->subchunk2_size, sizeof(int), 1, f);

        wat_log(LOG_PANIC, "\nheader saved");

        convert_double_to_short(wi);

        int i;
        for(i = 0; i < wi->nb_samples; i++){
                fwrite(&wi->short_left[i], sizeof(short int), 1, f);
                if(wh->num_channels == 2)
                        fwrite(&wi->short_right[i], sizeof(short int), 1, f);
        }

        fclose(f);

        wat_log(LOG_PANIC, "\nsave_file DONE");

        return 1;        
}

int main(int argc, char **argv)
{
        int ret;

        wav_input = (WavInput *)malloc(sizeof(WavInput));

        if(argc < 2){
                wat_log(LOG_INFO, "\nArgument is required\n\n");
                exit(3);
        }

        ret = init(wav_input, argc, argv);

        if(ret < 0){
                exit(4);
        }

        wav_input->wav_header = (WavHeader *)malloc(sizeof(WavHeader));
        ret = read_header_file(wav_input->wav_header, wav_input->file_name);

        if (ret < 0)
                exit(0);


        ret = read_wav_data(wav_input);

        if(wav_input->wat_args->dft){
                ret = dft(wav_input->nb_samples, wav_input->left_side, 
                                wav_input->zero_data);
                if(wav_input->wav_header->num_channels == 2){
                        ret = dft(wav_input->nb_samples, wav_input->right_side, 
                                        wav_input->zero_data);
                }
        }

        if(ret < 0){
                exit(4);
        }

        if(wav_input->wat_args->print_data)
                ret = print_wav_data(wav_input);

        if(ret < 0){
                exit(4);
        }


        if(wav_input->wat_args->idft){
                ret = inverse_dft(wav_input->nb_samples, wav_input->left_side, 
                                wav_input->zero_data);
                if(wav_input->wav_header->num_channels == 2){
                        ret = inverse_dft(wav_input->nb_samples, wav_input->right_side, 
                                        wav_input->zero_data);
                }
        }

        if(ret < 0){
                exit(4);
        }

        if(wav_input->wat_args->print_data 
                        && wav_input->wat_args->idft)
                ret = print_wav_data(wav_input);

        if(ret < 0){
                exit(4);
        }

        if(wav_input->wat_args->has_output)
                save_file(wav_input);

        printf("\n\nend\n");
        return 1;
}
