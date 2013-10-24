#include "wat.h"
#include "string.h"
#include <stdint.h>
#include <math.h>

#include "dft.h"
#include "fft.h"
#include "benchmark.h"
#include "utils.h"

#ifdef HAVE_THREADS
#include <pthread.h>

long nb_thread = 2;
#endif

WavInput * wav_input = NULL;
uint32_t tempo_total = 0;
int it_tempo = 0;

void help()
{
        printf("\n\tWAT");
        printf("\n\nParameters:");
        printf("\n\t-h                   Help.");
        printf("\n\t-i <imput file>      Imput file.");
        printf("\n\t-o <output file>     Imput file.");
        printf("\n\t-d                   Print data of input file after and"
                        " before dft.");
        printf("\n\t-p <nb secondes>     Print nb seconds of data after dft");
        printf("\n\t-fft                 Run FFT in input file.");
        printf("\n\t-dft                 Run DFT in input file.");
        printf("\n\t-sec <int>           Run DFT only in <int> seconds");
        printf("\n\t-log_level=<level>   Define the level of logging, can be"
                        "INFO (default), ERROR and PANIC.");
        printf("\n\t-eq <float>          Equalize audio with main factor <float>");

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

double bytes_to_double(uint8_t first, uint8_t second)
{
        short data;
        data = (second << 8) + first;
        return data;// / 32768.0;
}

int read_wav_data(WavInput * wi)
{
        wat_log(LOG_PANIC, "\nIn read_wav_data\n");
        FILE *f;
        int ret = 1;
        uint8_t * buffer;

        f = fopen(wi->file_name, "r");
        if (f == NULL){
                wat_log(LOG_INFO, "\nFile Error");
                wat_log(LOG_ERROR, " in read_wav_data");
                return -1;
        }

        buffer = (uint8_t *) malloc(sizeof(uint8_t) * 
                        (wi->wav_header->subchunk2_size));

        fseek(f, HEADER_SIZE, SEEK_SET);
        fread(buffer, sizeof(uint8_t), (wi->wav_header->subchunk2_size), f);

        if(wi->wat_args->print_hexa_data){
                wat_log(LOG_PANIC, "\nprint_hexa_data");
                print_hexa_data(wi, buffer);
        }

        if(wi->wav_header->num_channels == 2){
                wat_log(LOG_INFO, "\nWav with 2 channels.");
                wi->nb_samples = wi->wav_header->subchunk2_size >> 2;

                wi->right_side = (double *)malloc(sizeof(double) * wi->nb_samples);
                wi->left_side = (double *)malloc(sizeof(double) * wi->nb_samples);
                wi->zero_data = (double *)malloc(sizeof(double) * wi->nb_samples);
        }
        else if (wi->wav_header->num_channels == 1){
                wat_log(LOG_INFO, "\nWav with 1 channel.");
                wi->nb_samples = wi->wav_header->subchunk2_size >> 1;

                wi->zero_data = (double *)malloc(sizeof(double) * wi->nb_samples);
                wi->left_side = (double *)malloc(sizeof(double) * wi->nb_samples);
                wi->right_side = NULL;
        }
        else {
                wat_log(LOG_INFO, "\nNumber of channels invalid.");
                return -3;
        }
        printf("\nnb_samples = %d", wi->nb_samples);

        wat_log(LOG_PANIC, "\nGoing to copy the data");

        long int i = 0;
        long int it = 0;
        while(it < wi->wav_header->subchunk2_size){
                wi->left_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                wi->zero_data[i] = 0;
                it += 2;
                if(wi->wav_header->num_channels == 2){
                        wi->right_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                        it+=2;
                }
                i++;
        }


        char msg[50];
        sprintf(msg, "\n read_wav_data DONE, ret = %d", ret);
        wat_log(LOG_PANIC, msg);

        return 1;
}

int fix_data_to_fft(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nIn fix_data_to_fft function");
        wi->left_fixed = (double *)malloc(2 * wi->nb_samples * sizeof(double) + 1);
        if(wi->wav_header->num_channels == 2){
                wi->right_fixed = (double *)malloc(2 * wi->nb_samples * sizeof(double) + 1);
        }

        int i;
        if(wi->left_side){
                wat_log(LOG_PANIC, "\nGoing to copy left data");
                for(i = 0; i < wi->nb_samples; i++){
                        wi->left_fixed[2*i+1] = wi->left_side[i];
                        wi->left_fixed[2*i+2] = 0;
                }
        }
        if(wi->wav_header->num_channels == 2){
                if(wi->right_side){
                        wat_log(LOG_PANIC, "\nGoing to copy right data");
                        for(i = 0; i < wi->nb_samples; i++){
                                wi->right_fixed[2*i+1] = wi->right_side[i];
                                wi->right_fixed[2*i+2] = 0;
                       }
                }
        }
        wat_log(LOG_PANIC, "\nfix_data_to_fft function DONE");
        return 1;
}

int back_data_to_normal(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nIn back_data_to_normal");
        int i;
        wat_log(LOG_PANIC, "\nGoing to copy left data");
        if(wi->left_fixed){
                for(i = 0; i < wi->nb_samples; i++){
                        wi->left_side[i] = wi->left_fixed[2*i+1]; 
                }
        }
        wat_log(LOG_PANIC, "\nGoing to copy right data");
        if(wi->wav_header->num_channels == 2){
                if(wi->right_fixed){
                        for(i = 0; i < wi->nb_samples; i++){
                                wi->right_side[i] = wi->right_fixed[2*i+1]; 
                        }
                }
        }
        wat_log(LOG_PANIC, "\nback_data_to_normal DONE");
        return 1;
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
        else if(strcmp(argv, "-p") == 0){
                wa->print_nb_data = 1;
                return 4;
        }
        else if(strcmp(argv, "-sec") == 0){
                return 5;
        }
        else if(strcmp(argv, "-eq") == 0){
                return 6;
        }
        else if(strcmp(argv, "-dft") == 0){
                wa->dft = 1;
        }
        else if(strcmp(argv, "-fft") == 0){
                wa->fft = 1;
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

int calculate_factors(Factors * fc, float main_fc)
{
        char * msg_fac = (char *)malloc(64 * sizeof(char));
        sprintf(msg_fac, "\nCalculating factors with main factor equal %f", main_fc);
        wat_log(LOG_PANIC, msg_fac);

        fc->fac_0 = 0.7 * main_fc;
        fc->fac_1 = 0.75 * main_fc;
        fc->fac_2 = 0.90 * main_fc;
        fc->fac_3 = 1.0 * main_fc;
        fc->fac_4 = 1.1 * main_fc;
        fc->fac_5 = 1.1 * main_fc;
        fc->fac_6 = 1.0 * main_fc;
        fc->fac_7 = 0.9 * main_fc;
        fc->fac_8 = 0.75 * main_fc;
        fc->fac_9 = 0.7 * main_fc;
        fc->fac_10 = 0.7 * main_fc;

        wat_log(LOG_INFO, "\n\nFactors:\n");
        printf("\n\tfac_0 = %f", fc->fac_0);
        printf("\n\tfac_1 = %f", fc->fac_1);
        printf("\n\tfac_2 = %f", fc->fac_2);
        printf("\n\tfac_3 = %f", fc->fac_3);
        printf("\n\tfac_4 = %f", fc->fac_4);
        printf("\n\tfac_5 = %f", fc->fac_5);
        printf("\n\tfac_6 = %f", fc->fac_6);
        printf("\n\tfac_7 = %f", fc->fac_7);
        printf("\n\tfac_8 = %f", fc->fac_8);
        printf("\n\tfac_9 = %f", fc->fac_9);
        printf("\n\tfac_10 = %f\n", fc->fac_10);
        
        return 1;
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
        wi->wat_args->print_nb_data = 0;
        wi->wat_args->nb_to_print = 0;
        wi->wat_args->dft = 0;
        wi->wat_args->dft_sec = 0;
        wi->wat_args->fft = 0;
        wi->wat_args->equalize = 0;

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
                else if(ret == 4){
                        wi->wat_args->nb_to_print = atoi(argv[i + 1]);
                        i++;
                }
                else if(ret == 5){
                        wi->wat_args->dft_sec = atoi(argv[i + 1]);
                        i++;
                }
                else if(ret == 6){
                        wi->wat_args->equalize = atof(argv[i + 1]);
                        wi->factors = (Factors *)malloc(sizeof(Factors));
                        calculate_factors(wi->factors, wi->wat_args->equalize);
                        i++;
                }
                else if(ret < 1)
                        exit(1);
        }
        wat_log(LOG_PANIC, "\nParse_args DONE\n");

        f = fopen(wi->file_name, "r");
        if(f == NULL){
                char * msg = malloc(64 * sizeof(char));
                sprintf(msg, "\nError while openning input file: \"%s\" \n", 
                                wi->file_name);
                wat_log(LOG_INFO, msg);
                wat_log(LOG_ERROR," in init\n");
                exit(1);
        }

        fseek(f, 0, SEEK_END);
        wi->file_size = ftell(f);
        fclose(f);

        printf("\n\nFile => %s", wi->file_name);
        printf("\nSize => %d bytes", wi->file_size);

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
                wi->short_left = (short int *)malloc(sizeof(short int) 
                                * wi->nb_samples);
                wi->short_right = (short int *)malloc(sizeof(short int) 
                                * wi->nb_samples);
        }
        else {
                wi->short_left = (short int *)malloc(sizeof(short int) 
                                * wi->nb_samples);
        }

        wi->buffer = (unsigned char *)malloc(sizeof(unsigned char) 
                        * wi->wav_header->subchunk2_size);

        wat_log(LOG_PANIC, "\nDouble to short");
        int i;
        for(i = 0; i < wi->nb_samples; i++){
                wi->short_left[i] = (short int)wi->left_side[i];
                if(wi->wav_header->num_channels == 2){
                        wi->short_right[i] = (short int)wi->right_side[i];
                }
        }

        if(wi->wav_header->num_channels == 2){

        }

        wat_log(LOG_PANIC, "\nGoing to copy into buffer");
        int iterator = wi->wav_header->num_channels * 2;
        int count = 0;
        for(i = 0; i < wi->nb_samples * iterator; i += iterator){
                memcpy(&wi->buffer[i], &wi->short_left[count], 2);
                if(wi->wav_header->num_channels == 2){
                        memcpy(&wi->buffer[i + 2], &wi->short_right[count], 2);
                }
                count++;
        }
        wat_log(LOG_PANIC, "\nConverted double to short");

        return 1;
}

int save_file(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nIn save_file");
        char * msg = malloc(128 * sizeof(char));
        FILE *f;

        f = fopen(wi->output_file, "wb");
        if(f == NULL){
                sprintf(msg, "\nError while openning the output file: \"%s\".", 
                                wi->output_file);
                wat_log(LOG_INFO, msg);
                return -1;
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

        wat_log(LOG_PANIC, "\nConverted");
        int i;

        if(wh->num_channels == 2){
                sprintf(msg, "\nSaving data as 2 channels, "
                                "with nb_samples %d in which side", wi->nb_samples);
                wat_log(LOG_PANIC, msg);         
        }
        int iterator = wi->wav_header->num_channels;

        wat_log(LOG_INFO, "\nWriting in file now");
        for(i = 0; i < wi->wav_header->subchunk2_size; i += iterator){
                fwrite(&wi->buffer[i], sizeof(unsigned char), 1, f);
                if(wh->num_channels == 2){
                        fwrite(&wi->buffer[i + 1], sizeof(unsigned char), 1, f);
                }
        }

        fclose(f);
        wat_log(LOG_PANIC, "\nsave_file DONE");
        return 1;        
}

void divides_nfft(double * temp, int size)
{
        wat_log(LOG_PANIC, "\nDivides ");
        int i;
        for(i = 0; i < size; i++)
                temp[i] /= size;

        wat_log(LOG_PANIC, " DONE");
}

void print_array(double *temp, int size, int it)
{
        printf("\n\n\t\tPrinting array");
        int i;
        for(i = 0; i < size; i++){
                printf(" %f ", temp[it * 88200 + i]);
                if(i % 10 == 0 && i != 0)
                        printf("\n");
        }

        printf("\n\n\t\tPrinting array DONE");

}

int array_is_equal(double *vet1, double *vet2, int size, int it)
{
        printf("\n\nTesting if arrays is equals with size => %d \t it => %d", size, it);
        int i;
        int is_equal = 1;
        for(i = 0; i < size; i++){
                if(vet1[i] - vet2[it * size + i] > 1){
                        printf("\n\nArrays diferentes em i = %d", i);
                        printf("\n\nValor de Vet1 = %f \t Vet2 = %f", vet1[i], vet2[i]);
                        is_equal = 0;
                }
        }
        return is_equal == 1 ? 1 : 0;
}

int equalizeXk(WavInput *wi, double *temp, int size)
{
        int i;
        size >>=1;
        if(size < BAND_0)
                return 1;
        for(i = BAND_MIN; i < BAND_0; i++)
                temp[i] *= wi->factors->fac_0;
    
        if(size < BAND_1)
                return 1;
        for(i = BAND_0; i < BAND_1; i++)
                temp[i] *= wi->factors->fac_1;

        if(size < BAND_2)
                return 1;
        for(i = BAND_1; i < BAND_2; i++)
                temp[i] *= wi->factors->fac_2;

        if(size < BAND_3)
                return 1;
        for(i = BAND_2; i < BAND_3; i++)
                temp[i] *= wi->factors->fac_3;

        if(size < BAND_4)
                return 1;
        for(i = BAND_3; i < BAND_4; i++)
                temp[i] *= wi->factors->fac_4;

        if(size < BAND_5)
                return 1;
        for(i = BAND_4; i < BAND_5; i++)
                temp[i] *= wi->factors->fac_5;

        if(size < BAND_6)
                return 1;
        for(i = BAND_5; i < BAND_6; i++)
                temp[i] *= wi->factors->fac_6;

        if(size < BAND_7)
                return 1;
        for(i = BAND_6; i < BAND_7; i++)
                temp[i] *= wi->factors->fac_7;

        if(size < BAND_8)
                return 1;
        for(i = BAND_7; i < BAND_8; i++)
                temp[i] *= wi->factors->fac_8;

        if(size < BAND_9)
                return 1;
        for(i = BAND_8; i < BAND_9; i++)
                temp[i] *= wi->factors->fac_9;

        if(size < BAND_MAX)
                return 1;
        for(i = BAND_9; i < BAND_MAX; i++)
                temp[i] *= wi->factors->fac_10;

        return 1;
}


int equalize44k(WavInput *wi, double *temp, int size)
{
        wat_log(LOG_PANIC, "\nEqualizing audio with 44.1k");
        int i;
        for(i = BAND_MIN; i < BAND_0; i++)
                temp[i] *= wi->factors->fac_0;
    
        for(i = BAND_0; i < BAND_1; i++)
                temp[i] *= wi->factors->fac_1;

        for(i = BAND_1; i < BAND_2; i++)
                temp[i] *= wi->factors->fac_2;

        for(i = BAND_2; i < BAND_3; i++)
                temp[i] *= wi->factors->fac_3;

        for(i = BAND_3; i < BAND_4; i++)
                temp[i] *= wi->factors->fac_4;

        for(i = BAND_4; i < BAND_5; i++)
                temp[i] *= wi->factors->fac_5;

        for(i = BAND_5; i < BAND_6; i++)
                temp[i] *= wi->factors->fac_6;

        for(i = BAND_6; i < BAND_7; i++)
                temp[i] *= wi->factors->fac_7;

        for(i = BAND_7; i < BAND_8; i++)
                temp[i] *= wi->factors->fac_8;

        for(i = BAND_8; i < BAND_9; i++)
                temp[i] *= wi->factors->fac_9;

        for(i = BAND_9; i < BAND_MAX; i++)
                temp[i] *= wi->factors->fac_10;

        return 1;
}


/* FFT ----------------------------------------------------------------------- */
int channel_fft(s_fft *s)
{
//                sprintf(s->msg, "\nFFT %d of %f seconds, in Channel 1, in %d to %d", i, seconds, i * freq, i * freq + freq);
//                wat_log(LOG_PANIC, msg);

                memset(s->temp, 0, sizeof(double) * s->array_size);
                memcpy(s->temp, &s->channel[s->it * s->freq], s->freq * sizeof(double));

                four1(s->temp, s->NFFT, 1);

                //sprintf(msg, "\nIFFT %d of %f seconds, in Channel 1", i, seconds);
//                wat_log(LOG_PANIC, msg);

                if(s->wi->wat_args->equalize != 0){
                        if(s->wi->wav_header->sample_rate == 44100){
                                equalize44k(s->wi, s->temp, s->NFFT);
                        }
                        else{
                                equalizeXk(s->wi, s->temp, s->NFFT);
                        }
                }

                divides_nfft(s->temp, s->NFFT);

                four1(s->temp, s->NFFT, -1);

                memcpy(&s->channel[s->it * s->freq], s->temp, s->freq * sizeof(double));
//                wat_log(LOG_PANIC, "\nmemcpy ch 1 DONE");

//                wat_log(LOG_PANIC, "\n\tchannel 1 DONE\n\n");
                return 1;
}

#ifdef HAVE_THREADS
void *fft_t(void *st)
{
        channel_fft((s_fft *) st);
        return NULL;
}
#endif

int run_fft(WavInput *wi, float seconds)
{
#ifdef HAVE_THREADS
        printf("\nThread Baby");
        pthread_t *fft_thread;
        
        fft_thread = (pthread_t *)malloc(nb_thread * sizeof(pthread_t));
        s_fft **s = (s_fft **)malloc(nb_thread * sizeof(s_fft *));

        s[0] = (s_fft *)malloc(nb_thread * sizeof(s_fft));
        s[1] = (s_fft *)malloc(nb_thread * sizeof(s_fft));

        s[0]->log = s[1]->log = (char *)malloc(128 * sizeof(char));
        s[0]->wi = wi;
        s[1]->wi = wi;

        s[0]->freq = wi->wav_header->sample_rate;
        s[1]->freq = wi->wav_header->sample_rate;

        s[0]->freq <<= 1;
        s[1]->freq <<= 1;
        printf("\nfrequencia => %d", s[0]->freq);
        s[0]->NFFT = (int)pow(2.0, ceil(log((double)s[0]->freq)/log(2.0)));
        s[1]->NFFT = (int)pow(2.0, ceil(log((double)s[0]->freq)/log(2.0)));
        printf("\nNFFT = %d", s[0]->NFFT);
        s[0]->array_size = 2 * s[0]->NFFT + 1;
        s[1]->array_size = 2 * s[0]->NFFT + 1;

        s[0]->temp = s[1]->temp = (double *)malloc(s[0]->array_size * sizeof(double));
        wat_log(LOG_PANIC, "\n\nGoing to apply the FFT");
#else
        s_fft *s = (s_fft *)malloc(sizeof(s_fft));
        s->log = (char *)malloc(128 * sizeof(char));
        s->wi = wi;
        s->freq = wi->wav_header->sample_rate;
        s->freq <<= 1;
        printf("\nfrequencia => %d", s->freq);
        s->NFFT = (int)pow(2.0, ceil(log((double)s->freq)/log(2.0)));
        printf("\nNFFT = %d", s->NFFT);
        s->array_size = 2 * s->NFFT + 1;

        s->temp = (double *)malloc(s->array_size * sizeof(double));
        wat_log(LOG_PANIC, "\n\nGoing to apply the FFT");
#endif
        fix_data_to_fft(wi);

        int i;
        for(i = 0; i < floor(seconds); i++){
#ifdef HAVE_THREADS
                sprintf(s[0]->log, "\nTHREAD FFT %d of %f seconds, in Channel 1, in %d to %d", i, seconds, i * s[0]->freq, i * s[0]->freq + s[0]->freq);
                wat_log(LOG_PANIC, s[0]->log);
                s[0]->it = i;
                s[0]->channel = wi->left_fixed;

                pthread_create(&fft_thread[0], NULL, fft_t, s[0]);

                if(wi->wav_header->num_channels == 2){
                        sprintf(s[1]->log, "\nTHREAD FFT %d of %f seconds, in Channel 2, in %d to %d", i, seconds, i * s[1]->freq, i * s[1]->freq + s[1]->freq);
                        wat_log(LOG_PANIC, s[1]->log);

                        s[1]->channel = wi->right_fixed;

                        pthread_create(&fft_thread[1], NULL, fft_t, s[1]);
                        pthread_join(fft_thread[1], NULL);
                }
                pthread_join(fft_thread[0], NULL);

#else

                sprintf(s->log, "\nFFT %d of %f seconds, in Channel 1, in %d to %d", i, seconds, i * s->freq, i * s->freq + s->freq);
                wat_log(LOG_PANIC, s->log);

                s->it = i;
                s->channel = wi->left_fixed;

                channel_fft(s);

                if(wi->wav_header->num_channels == 2){
                        sprintf(s->log, "\nFFT %d of %f seconds, in Channel 2, in %d to %d", i, seconds, i * s->freq, i * s->freq + s->freq);
                        wat_log(LOG_PANIC, s->log);

                        s->channel = wi->right_fixed;

                        channel_fft(s);
                }
#endif
        }
        back_data_to_normal(wi);
        return 1;
}

/* END FFT -------------------------------------------------------------------- */

int run_dft(WavInput *wi, float seconds)
{
        int i, ret;
        int freq = wi->wav_header->sample_rate;
        double * temp = (double *)malloc(freq * sizeof(double));
        wat_log(LOG_PANIC, "\n\nGoing to apply the DFT");
        char * msg = malloc(64 * sizeof(char));
        for(i = 0; i < seconds; i++){
                if((float)(seconds - i) < 1){
                        int last_part = wi->nb_samples - (i * freq);
                        printf("\nlast part = %d", last_part);

                        sprintf(msg, "\n\nLess than a seconds, %.3f seconds missing, in Channel 1, from %d to %d", (float)(seconds - i), i * freq, i * freq + last_part);
                        wat_log(LOG_PANIC, msg);

                        sprintf(msg, "\nRealloc temp with %d size in Channel 1", last_part);
                        wat_log(LOG_PANIC, msg);

                        realloc(temp, sizeof(double) * last_part);
                        memcpy(temp, &wi->left_side[i * freq], last_part * sizeof(double));
                        ret = dft(last_part, temp, wi->zero_data);
                        ret = inverse_dft(last_part, temp, wi->zero_data);
                        memcpy(&wi->left_side[i * freq], temp, last_part * sizeof(double));

                        if(wi->wav_header->num_channels == 2){
                                sprintf(msg, "\n\nDFT Less than a seconds, %.3f seconds missing, in Channel 2", (seconds - i));
                                wat_log(LOG_PANIC, msg);

                                memcpy(temp, &wi->right_side[i * freq], last_part * sizeof(double));
                                ret = dft(last_part, temp, wi->zero_data);
                                sprintf(msg, "\n\nIDFT Less than a seconds, %.3f seconds missing, in Channel 2", (seconds - i));
                                wat_log(LOG_PANIC, msg);
                                ret = inverse_dft(last_part, temp, wi->zero_data);
                                memcpy(&wi->right_side[i * freq], temp, last_part * sizeof(double));
                        }
                } 
                else {
                        sprintf(msg, "\n%d of %f seconds, in Channel 1, in %d to %d", i, seconds, i * freq, i * freq + freq);
                        wat_log(LOG_PANIC, msg);

                        memcpy(temp, &wi->left_side[i * freq], freq * sizeof(double));
                        ret = dft(freq, temp, wi->zero_data);
                        sprintf(msg, "\nIDFT %d of %f seconds, in Channel 1", i, seconds);
                        wat_log(LOG_PANIC, msg);
                        ret = inverse_dft(freq, temp, wi->zero_data);
                        memcpy(&wi->left_side[i * freq], temp, freq * sizeof(double));

                        if(wi->wav_header->num_channels == 2){
                                sprintf(msg, "\nDFT %d of %f seconds, in Channel 2", i, seconds);
                                wat_log(LOG_PANIC, msg);

                                memcpy(temp, &wi->right_side[i * freq], freq * sizeof(double));
                                ret = dft(freq, temp, wi->zero_data);
                                sprintf(msg, "\nIDFT %d of %f seconds, in Channel 2", i, seconds);
                                wat_log(LOG_PANIC, msg);
                                ret = inverse_dft(freq, temp, wi->zero_data);
                                memcpy(&wi->right_side[i * freq], temp, freq * sizeof(double));
                        }
                }
        }
        return 1;
}

int main(int argc, char **argv)
{
        int ret;
        char * msg = malloc(128 * sizeof(char)); 

        wav_input = (WavInput *)malloc(sizeof(WavInput));

#ifdef HAVE_THREADS
//        wav_input->wat_thread = (WatThread *)malloc(sizeof(WatThread));
#endif

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

        /* getting the duration of audio */
        float seconds = 0;
        if(wav_input->wav_header->num_channels == 1){
                seconds = wav_input->wav_header->subchunk2_size 
                        / wav_input->wav_header->sample_rate;
                seconds /= 2;

                char * msg = malloc(64 * sizeof(char)); 
                sprintf(msg, "\n\nDuration => %.3f seconds 1 ch\n", seconds);
                wat_log(LOG_INFO, msg);
        }
        else if(wav_input->wav_header->num_channels == 2){
                seconds = wav_input->wav_header->subchunk2_size 
                        / wav_input->wav_header->sample_rate;
                seconds /= 4;

                sprintf(msg, "\n\nDuration => %.3f seconds, 2 ch\n", seconds);
                wat_log(LOG_INFO, msg);
        }

        if(wav_input->wat_args->dft_sec > 0){
                if(wav_input->wat_args->dft_sec > seconds){
                        sprintf(msg, "\nArgument -sec %d invalid, should be less than %f\n\n", 
                                        wav_input->wat_args->dft_sec,  seconds);
                        wat_log(LOG_INFO, msg);
                        exit(1);
                }
                seconds = wav_input->wat_args->dft_sec;
                printf("\nUsing only %.2f seconds\n", seconds);
        }
 
        if (ret < 0)
                exit(0);

        ret = read_wav_data(wav_input);

        if(wav_input->wat_args->dft){
                run_dft(wav_input, seconds);
        }
        else if(wav_input->wat_args->fft){
                int last_part = wav_input->nb_samples;
                printf("\nmain nb_samples= %d", last_part);
                uint32_t tempo = wat_gettime();
                run_fft(wav_input, seconds);
                tempo = wat_gettime() - tempo;
                printf("\nTempo em run_fft = %ju", (intmax_t)tempo);
        }


        if(ret < 0){
                exit(4);
        }

        if(wav_input->wat_args->print_data)
                ret = print_wav_data(wav_input);

        if(ret < 0){
                exit(4);
        }

        if(wav_input->wat_args->print_nb_data){
        
        }

        if(ret < 0){
                exit(4);
        }

        if(wav_input->wat_args->has_output)
                save_file(wav_input);

        printf("\n\nNumero de cores => %d", get_number_of_cores());

        printf("\n\nend\n");
        return 1;
}


/* 
 *

//        int last_part = wi->nb_samples * 2;
//        printf("\nnb_samples= %d", last_part);
 *                 if((float)(seconds - i) < 1){
                        last_part -= (i * freq);
                        printf("\nlast part = %d", last_part);
                        NFFT = (int)pow(2.0, ceil(log((double)last_part)/log(2.0)));
                        array_size = 2 * NFFT + 1;
                        printf("\nNFFT = %d", NFFT);

                        sprintf(msg, "\n\nLess than a seconds, %.3f seconds missing, in Channel 1, from %d to %d", (float)(seconds - i), i * freq, i * freq + last_part);
                        wat_log(LOG_PANIC, msg);

                        sprintf(msg, "\nRealloc temp with %d size in Channel 1", last_part);
                        wat_log(LOG_PANIC, msg);

                        realloc(temp, sizeof(double) * array_size);
                        memset(temp, 0, sizeof(double) * array_size);
                        memcpy(temp, &wi->left_fixed[i * freq], last_part * sizeof(double));

                        tempo = wat_gettime();
#ifdef HAVE_THREADS
                        four1(temp, NFFT, 1);
#else
                        four1(temp, NFFT, 1);
#endif
                        tempo = wat_gettime() - tempo;
                        tempo_total += tempo;
                        it_tempo++;
                        printf("\n\nTempo %ju", (intmax_t)tempo);

                        if(wi->wat_args->equalize != 0){
                                if(wi->wav_header->sample_rate == 44100){
                                        equalize44k(wi, temp, NFFT);
                                }
                                else{
                                        equalizeXk(wi, temp, NFFT);
                                }
                        }

                        tempo = wat_gettime();
                        four1(temp, NFFT, -1);
                        tempo = wat_gettime() - tempo;
                        tempo_total += tempo;
                        it_tempo++;
                        printf("\n\nTempo %ju", (intmax_t)tempo);


                        divides_nfft(temp, NFFT);
                        memcpy(&wi->left_fixed[i * freq], temp, last_part * sizeof(double));

                        wat_log(LOG_PANIC, "\nChannel 1 finished");
                        if(wi->wav_header->num_channels == 2){
                                sprintf(msg, "\n\nFFT Less than a seconds, %.3f seconds missing, in Channel 2", (seconds - i));
                                wat_log(LOG_PANIC, msg);

                                memset(temp, 0, sizeof(double) * array_size);
                                memcpy(temp, &wi->right_fixed[i * freq], last_part * sizeof(double));

                        tempo = wat_gettime();
                        four1(temp, NFFT, 1);
                        tempo = wat_gettime() - tempo;
                        tempo_total += tempo;
                        it_tempo++;
                        printf("\n\nTempo %ju", (intmax_t)tempo);


//                                four1(temp, NFFT, 1);
                                sprintf(msg, "\n\nIFFT Less than a seconds, %.3f seconds missing, in Channel 2", (seconds - i));
                                wat_log(LOG_PANIC, msg);

                                if(wi->wat_args->equalize != 0){
                                        if(wi->wav_header->sample_rate == 44100){
                                                equalize44k(wi, temp, NFFT);
                                        }
                                        else{
                                                equalizeXk(wi, temp, NFFT);
                                        }
                                }
                        tempo = wat_gettime();
                        four1(temp, NFFT, -1);
                        tempo = wat_gettime() - tempo;
                        tempo_total += tempo;
                        it_tempo++;
                        printf("\n\nTempo %ju", (intmax_t)tempo);


//                                four1(temp, NFFT, -1);

                                divides_nfft(temp, NFFT);
                                memcpy(&wi->right_fixed[i * freq], temp, last_part * sizeof(double));
                        }
                } 
                else {

*/
