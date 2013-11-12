#include "wat.h"

#include <stdint.h>
#include <math.h>

#include "fft.h"
#include "benchmark.h"
#include "utils.h"

#ifdef HAVE_THREADS

#include <pthread.h>

#endif

WavInput * wav_input = NULL;

int bench_nb = 1;

uint32_t bench_equalize = 0;
uint32_t bench_fix_data = 0;
uint32_t bench_read_data = 0;
uint32_t bench_back_data = 0;
uint32_t bench_convert_double = 0;
uint32_t bench_save_file = 0;
uint32_t bench_fft = 0;
uint32_t bench_wat = 0;
uint32_t bench_sum = 0;

uint32_t *bench_fft_1;


int equalize44k(WavInput *wi, double *temp, int size);

int fix_data_to_fft(WavInput *wi);

int back_data_to_normal(WavInput *wi);

int convert_double_to_short(WavInput *wi);

int save_file(WavInput *wi);

void help()
{
        printf("\n\tWAT\n");
        printf("\n\thelp                 Help.");
        printf("\n\t-i <imput file>      Imput file.");
        printf("\n\t-o <output file>     Imput file.");
        printf("\n\t-b <int>             Run <int> times to benchmark.");
        printf("\n\t-fft                 Run FFT in input file.");
        printf("\n\t-times <int>         Run FFT <int> times");
        printf("\n\t-sec <int>           Run FFT only in <int> first seconds");
        printf("\n\t-log_level=<level>   Define the logging level, can be"
                        "BENCH, INFO (default), ERROR and PANIC.");
        printf("\n\t-eq <float>          Equalize audio with main factor <float>");
        printf("\n\t-one-channel         Will apply fft only in left channel");
        printf("\n\t-r <int>             Raise the size of wav file in <int> times");

        printf("\n\n\tUsing threads");
        printf("\n\tYou have to compile WAT using \"make thread\".\n");
        printf("\n\t-nt <int>            Choose the number of threads is using it");

        printf("\n\n");
}

void print_header(WavHeader * wh)
{
        wat_log(LOG_PANIC, "\nprint_header");

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

        wat_log(LOG_PANIC, "\n\nprint_header, DONE.");
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

        /*
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
        */
        
        return 1;
}


int parse_args(WavInput *wi, int argc, char **argv)
{

        wi->wat_args->argc = argc;
        wi->wat_args->argv = argv;
        wi->wat_args->has_input = 0;
        wi->wat_args->has_output = 0;
        wi->wat_args->benchmark = 0;
        wi->wat_args->fft = 0;
        wi->wat_args->n_times = 1;
        wi->wat_args->equalize = 0;
        wi->wat_args->one_channel = 0;
        wi->wat_args->raise = 0;
        wi->wat_args->print_header = 0;

#ifdef HAVE_THREADS
        wi->nb_thread = get_number_of_cores();
#endif

        int i;
        for(i = 1; i < argc; i++){
                if(strcmp(argv[i], "-ph") == 0){
                        wi->wat_args->print_header = 1;
                }
                else if(strcmp(argv[i], "help") == 0){
                        help();
                        exit(1);
                }
                else if(strcmp(argv[i], "-i") == 0){
                        wi->wat_args->has_input = 1;
                        wi->file_name = argv[i + 1];
                        i++;
                }
                else if(strcmp(argv[i], "-o") == 0){
                        wi->wat_args->has_output = 1;
                        wi->output_file = argv[i + 1];
                        i++;
                }
                else if(strcmp(argv[i], "-b") == 0){
                        wi->wat_args->benchmark = atoi(argv[i + 1]);
                        bench_nb = atoi(argv[i + 1]);
                        i++;
                }
                else if(strcmp(argv[i], "-sec") == 0){
                        wi->wat_args->only_sec = atoi(argv[i + 1]);
                        i++;
                }
                else if(strcmp(argv[i], "-one-channel") == 0){
                        wi->wat_args->one_channel = 1;
                }
                else if(strcmp(argv[i], "-r") == 0){
                        wi->wat_args->raise = atoi(argv[i + 1]);
                        i++;
                }
                else if(strcmp(argv[i], "-eq") == 0){
                        wi->wat_args->equalize = atof(argv[i + 1]);
                        wi->factors = (Factors *)malloc(sizeof(Factors));
                        calculate_factors(wi->factors, wi->wat_args->equalize);
                        i++;
                }
                else if(strcmp(argv[i], "-fft") == 0){
                        wi->wat_args->fft = 1;
                }
                else if(strcmp(argv[i], "-times") == 0){
                        wi->wat_args->n_times = atoi(argv[i + 1]);
                        i++;
                }
                else if(strcmp(argv[i], "-log_level=INFO") == 0){
                        set_log_level(LOG_ERROR);
                }
                else if(strcmp(argv[i], "-log_level=ERROR") == 0){
                        set_log_level(LOG_ERROR);
                }
                else if(strcmp(argv[i], "-log_level=PANIC") == 0){
                        set_log_level(LOG_PANIC);
                }
                else if(strcmp(argv[i], "-log_level=BENCH") == 0){
                        set_log_level(LOG_BENCH);
                }

                else if(strcmp(argv[i], "-nt") == 0){
#ifdef HAVE_THREADS
                        wi->nb_thread = atol(argv[i + 1]);
                        if(wi->nb_thread < 0){
                                wat_log(LOG_INFO, "\n\nNumber of threads invalid\n\n");
                                exit(1);
                        }
                        i++;
#else 
                        wat_log(LOG_BENCH, "Error on parameter \"-nt <int>\".\n"
                                        "This options is only available if"
                                        " WAT was compiled to use threads, and"
                                        " wasn't.\n"
                                        "You can use \"make thread\" for that.\n");
                        return -1;
#endif
                }
                else{
                        char msg[60];
                        sprintf(msg, "\nParametro \"%s\" nao encontrado\n", argv[i]);
                        wat_log(LOG_INFO, msg);
                        return -1;
                }
        }
        wat_log(LOG_PANIC, "parse_args, DONE.\n");
        return 1;
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

#ifdef ORIG
int equalize44k(WavInput *wi, double *temp, int size)
{
        wat_log(LOG_PANIC, "\nEqualizing audio with 44.1k");
        int i;
        
        for(i = BAND_MIN; i < BAND_MAX; i++){
                if(i < BAND_0)
                        temp[i] *= wi->factors->fac_0;
                else if(i < BAND_1)
                        temp[i] *= wi->factors->fac_1;
                else if(i < BAND_2)
                        temp[i] *= wi->factors->fac_2;
                else if(i < BAND_3)
                        temp[i] *= wi->factors->fac_3;
                else if(i < BAND_4)
                        temp[i] *= wi->factors->fac_4;
                else if(i < BAND_5)
                        temp[i] *= wi->factors->fac_5;
                else if(i < BAND_6)
                        temp[i] *= wi->factors->fac_6;
                else if(i < BAND_7)
                        temp[i] *= wi->factors->fac_7;
                else if(i < BAND_8)
                        temp[i] *= wi->factors->fac_8;
                else if(i < BAND_9)
                        temp[i] *= wi->factors->fac_9;
                else
                        temp[i] *= wi->factors->fac_10;
        }
        return 1;    
}
#endif //ORIG

#ifdef ORIG_B

int equalize44k(WavInput *wi, double *temp, int size)
{
        wat_log(LOG_PANIC, "\nEqualizing audio with 44.1k, orig_b");
        int i;
        uint32_t sample;

        sample = wat_gettime();

        for(i = BAND_MIN; i < BAND_MAX; i++){
                if(i < BAND_0)
                        temp[i] *= wi->factors->fac_0;
                else if(i < BAND_1)
                        temp[i] *= wi->factors->fac_1;
                else if(i < BAND_2)
                        temp[i] *= wi->factors->fac_2;
                else if(i < BAND_3)
                        temp[i] *= wi->factors->fac_3;
                else if(i < BAND_4)
                        temp[i] *= wi->factors->fac_4;
                else if(i < BAND_5)
                        temp[i] *= wi->factors->fac_5;
                else if(i < BAND_6)
                        temp[i] *= wi->factors->fac_6;
                else if(i < BAND_7)
                        temp[i] *= wi->factors->fac_7;
                else if(i < BAND_8)
                        temp[i] *= wi->factors->fac_8;
                else if(i < BAND_9)
                        temp[i] *= wi->factors->fac_9;
                else
                        temp[i] *= wi->factors->fac_10;
        }
        sample = wat_gettime() - sample;

        bench_equalize += sample;
        return 1;    
}
#endif //ORIG_B

#if defined(UNROLL) || defined(UNROFIS)
int equalize44k(WavInput *wi, double *temp, int size)
{
        wat_log(LOG_PANIC, "\nEqualizing audio with 44.1k, fission");
        int i;
        uint32_t samples;

        samples = wat_gettime();

        for(i = BAND_MIN; i < BAND_0; i += 2){
                temp[i] *= wi->factors->fac_0;
                temp[i + 1] *= wi->factors->fac_0;
        }
        for(i = BAND_0; i < BAND_1; i += 2){
                temp[i] *= wi->factors->fac_1;
                temp[i + 1] *= wi->factors->fac_1;
        }
        for(i = BAND_1; i < BAND_2; i += 2){
                temp[i] *= wi->factors->fac_2;
                temp[i + 1] *= wi->factors->fac_2;
        }
        for(i = BAND_2; i < BAND_3; i += 2){
                temp[i] *= wi->factors->fac_3;
                temp[i + 1] *= wi->factors->fac_3;
        }
        for(i = BAND_3; i < BAND_4; i += 2){
                temp[i] *= wi->factors->fac_4;
                temp[i + 1] *= wi->factors->fac_4;
        }
        for(i = BAND_4; i < BAND_5; i += 2){
                temp[i] *= wi->factors->fac_5;
                temp[i + 1] *= wi->factors->fac_5;
        }
        for(i = BAND_5; i < BAND_6; i += 2){
                temp[i] *= wi->factors->fac_6;
                temp[i + 1] *= wi->factors->fac_6;
        }
        for(i = BAND_6; i < BAND_7; i += 2){
                temp[i] *= wi->factors->fac_7;
                temp[i + 1] *= wi->factors->fac_7;
        }
        for(i = BAND_7; i < BAND_8; i += 2){
                temp[i] *= wi->factors->fac_8;
                temp[i + 1] *= wi->factors->fac_8;
        }
        for(i = BAND_8; i < BAND_9; i += 2){
                temp[i] *= wi->factors->fac_9;
                temp[i + 1] *= wi->factors->fac_9;
        }
        for(i = BAND_9; i < BAND_MAX; i += 2){
                temp[i] *= wi->factors->fac_10;
                temp[i + 1] *= wi->factors->fac_10;
        }

        samples = wat_gettime() - samples;

        bench_equalize += samples;
        return 1;
}
#endif //UNROLL


#if defined(FISSION) || defined(OPT)
int equalize44k(WavInput *wi, double *temp, int size)
{
        wat_log(LOG_PANIC, "\nEqualizing audio with 44.1k, fission");
        int i;
        uint32_t samples;

        samples = wat_gettime();

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

        samples = wat_gettime() - samples;

        bench_equalize += samples;
        return 1;
}
#endif //FISSION

int read_header_file(WavHeader * wh, char *file_name)
{
        wat_log(LOG_PANIC, "\nread_header_file");
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
                sprintf(msg, "\nSomething got wrong with fread\n");
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

        wat_log(LOG_PANIC, ", DONE");

        return 1;
}

int write_header(WavHeader *wh, FILE *f)
{
        wat_log(LOG_PANIC, "\nwrite_header");

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

        wat_log(LOG_PANIC, ", DONE");

        return 1;
}

double bytes_to_double(uint8_t first, uint8_t second)
{
        short data;
        data = (second << 8) + first;
        return data;// / 32768.0;
}

/* snk */
int read_wav_data(WavInput * wi)
{
        wat_log(LOG_PANIC, "\nread_wav_data");
        FILE *f;
        uint8_t * buffer;
        char * log = (char *)malloc(128 * sizeof(char));

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

        if(wi->wav_header->num_channels == 2){
                wat_log(LOG_PANIC, ", 2 channels");
                wi->nb_samples = wi->wav_header->subchunk2_size >> 2;

                wi->right_side = (double *)malloc(sizeof(double) * wi->nb_samples);
                wi->left_side = (double *)malloc(sizeof(double) * wi->nb_samples);
                wi->zero_data = (double *)malloc(sizeof(double) * wi->nb_samples);
        }
        else if (wi->wav_header->num_channels == 1){
                wat_log(LOG_PANIC, ", 1 channel");
                wi->nb_samples = wi->wav_header->subchunk2_size >> 1;

                wi->zero_data = (double *)malloc(sizeof(double) * wi->nb_samples);
                wi->left_side = (double *)malloc(sizeof(double) * wi->nb_samples);
                wi->right_side = NULL;
        }
        else {
                wat_log(LOG_ERROR, "\nNumber of channels is invalid.");
                return -3;
        }
        sprintf(log, ", nb_samples = %d", wi->nb_samples);
        wat_log(LOG_PANIC, log);

        wat_log(LOG_PANIC, ", copying");

        long int i;
        long int it;


#ifdef ORIG 
        i = it = 0;
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
#endif //ORIG

#if defined(ORIG_B)
        bench_read_data = wat_gettime();
        i = it = 0;
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
        bench_read_data = wat_gettime() - bench_read_data;

#endif //ORIG_B
        
#if defined(UNROLL) //|| defined(FISSION)
        bench_read_data = wat_gettime();
        i = it = 0;
        if(wi->wav_header->num_channels == 1){
                while(it < wi->wav_header->subchunk2_size){
                        wi->left_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                        wi->zero_data[i] = 0;
                        it += 2;
                        i++;
                        wi->left_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                        wi->zero_data[i] = 0;
                        it += 2;
                        i++;

                }
        }
        if(wi->wav_header->num_channels == 2){
                i = 0;
                it = 0;
                while(it < wi->wav_header->subchunk2_size){
                        wi->left_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                        wi->zero_data[i] = 0;
                        it += 2;
                        wi->right_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                        it += 2;
                        i++;

                        wi->left_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                        wi->zero_data[i] = 0;
                        it += 2;
                        wi->right_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                        it += 2;
                        i++;
                }
        }
        bench_read_data = wat_gettime() - bench_read_data;

#endif // UNROLL


#if defined(OPT) //|| defined(FISSION)
        bench_read_data = wat_gettime();
        i = it = 0;
        if(wi->wav_header->num_channels == 1){
                while(it < wi->wav_header->subchunk2_size){
                        wi->left_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                        wi->zero_data[i] = 0;
                        it += 2;
                        i++;
                }
        }
        else if(wi->wav_header->num_channels == 2){
                while(it < wi->wav_header->subchunk2_size){
                        wi->left_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                        wi->zero_data[i] = 0;
                        it += 2;
                        wi->right_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                        it+=2;
                        i++;
                }
        }
        bench_read_data = wat_gettime() - bench_read_data;

#endif //OPT

#ifdef FISSION
        bench_read_data = wat_gettime();
        i = it = 0;
        if(wi->wav_header->num_channels == 1){
                while(it < wi->wav_header->subchunk2_size){
                        wi->left_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                        wi->zero_data[i] = 0;
                        it += 2;
                        i++;
                }
        }
        else if(wi->wav_header->num_channels == 2){
                while(it < wi->wav_header->subchunk2_size){
                        wi->zero_data[i] = 0;
                        it+=4;
                        i++;
                }
                i = it = 0;
                while(it < wi->wav_header->subchunk2_size){
                        wi->left_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                        it += 4;
                        i++;
                }
                i = 0;
                it = 2; 
                while(it < wi->wav_header->subchunk2_size){
                        wi->right_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                        it+=4;
                        i++;
                }
        }
        bench_read_data = wat_gettime() - bench_read_data;
#endif //R_FISSION

#ifdef UNROFIS
        bench_read_data = wat_gettime();
        i = it = 0;
        if(wi->wav_header->num_channels == 1){
                while(it < wi->wav_header->subchunk2_size){
                        wi->left_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                        wi->zero_data[i] = 0;
                        it += 2;
                        i++;
                        wi->left_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                        wi->zero_data[i] = 0;
                        it += 2;
                        i++;
                }
        }
        else if(wi->wav_header->num_channels == 2){
                while(it < wi->wav_header->subchunk2_size){
                        wi->zero_data[i] = 0;
                        it+=4;
                        i++;
                        wi->zero_data[i] = 0;
                        it+=4;
                        i++;
                }
                i = it = 0;
                while(it < wi->wav_header->subchunk2_size){
                        wi->left_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                        it += 4;
                        i++;
                        wi->left_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                        it += 4;
                        i++;
                }
                i = 0;
                it = 2; 
                while(it < wi->wav_header->subchunk2_size){
                        wi->right_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                        it+=4;
                        i++;
                        wi->right_side[i] = bytes_to_double(buffer[it], buffer[it + 1]);
                        it+=4;
                        i++;
                }
        }
        bench_read_data = wat_gettime() - bench_read_data;
#endif // UNROFIS


        wat_log(LOG_PANIC, ", DONE.");
        return 1;
}

#ifdef ORIG
int fix_data_to_fft(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nfix_data_to_fft");

        wi->left_fixed = (double *)malloc(2 * wi->nb_samples * sizeof(double) + 1);
        if(wi->left_fixed == NULL){
                wat_log(LOG_ERROR, "\nMemory error in left_fixed");
        }
        if(wi->wav_header->num_channels == 2){
                wi->right_fixed = (double *)malloc(2 * wi->nb_samples * sizeof(double) + 1);
                if(wi->right_fixed == NULL){
                        wat_log(LOG_ERROR, "\nMemory error in right_fixed");
                }
        }

        wat_log(LOG_PANIC, ", allocated");
        int i;
        for(i = 0; i < wi->nb_samples; i++){
                wi->left_fixed[2*i+1] = wi->left_side[i];
                wi->left_fixed[2*i+2] = 0;
                if(wi->wav_header->num_channels == 2){
                        wi->right_fixed[2*i+1] = wi->right_side[i];
                        wi->right_fixed[2*i+2] = 0;
                }
        }
        wat_log(LOG_PANIC, ", DONE");
        return 1;
}
#endif //ORIG

#if defined(ORIG_B)
int fix_data_to_fft(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nfix_data_to_fft");
        bench_fix_data = wat_gettime();

        wi->left_fixed = (double *)malloc(2 * wi->nb_samples * sizeof(double) + 1);
        if(wi->left_fixed == NULL){
                wat_log(LOG_ERROR, "\nMemory error in left_fixed");
        }
        if(wi->wav_header->num_channels == 2){
                wi->right_fixed = (double *)malloc(2 * wi->nb_samples * sizeof(double) + 1);
                if(wi->right_fixed == NULL){
                        wat_log(LOG_ERROR, "\nMemory error in right_fixed");
                }
        }

        wat_log(LOG_PANIC, ", allocated");
        int i;
        for(i = 0; i < wi->nb_samples; i++){
                wi->left_fixed[2*i+1] = wi->left_side[i];
                wi->left_fixed[2*i+2] = 0;
                if(wi->wav_header->num_channels == 2){
                        wi->right_fixed[2*i+1] = wi->right_side[i];
                        wi->right_fixed[2*i+2] = 0;
                }
        }
        bench_fix_data = wat_gettime() - bench_fix_data;
        wat_log(LOG_PANIC, ", DONE");
        return 1;
}
#endif //ORIG_B

#if defined(UNROLL) 
int fix_data_to_fft(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nfix_data_to_fft");

        wi->left_fixed = (double *)malloc(2 * wi->nb_samples * sizeof(double) + 1);
        if(wi->left_fixed == NULL){
                wat_log(LOG_ERROR, "\nMemory error in left_fixed");
        }
        if(wi->wav_header->num_channels == 2){
                wi->right_fixed = (double *)malloc(2 * wi->nb_samples * sizeof(double) + 1);
                if(wi->right_fixed == NULL){
                        wat_log(LOG_ERROR, "\nMemory error in right_fixed");
                }
        }

        wat_log(LOG_PANIC, ", allocated");
        bench_fix_data = wat_gettime();
        int i;
        if(wi->wav_header->num_channels == 1){
                for(i = 0; i < wi->nb_samples; i+=2){
                        wi->left_fixed[2*i+1] = wi->left_side[i];
                        wi->left_fixed[2*i+2] = 0;

                        wi->left_fixed[2*i+3] = wi->left_side[i + 1];
                        wi->left_fixed[2*i+4] = 0;
                }
        }
        else if(wi->wav_header->num_channels == 2){
                for(i = 0; i < wi->nb_samples; i+=2){
                        wi->left_fixed[2*i+1] = wi->left_side[i];
                        wi->left_fixed[2*i+2] = 0;
                        wi->right_fixed[2*i+1] = wi->right_side[i];
                        wi->right_fixed[2*i+2] = 0;

                        wi->left_fixed[2*i+3] = wi->left_side[i+1];
                        wi->left_fixed[2*i+4] = 0;
                        wi->right_fixed[2*i+3] = wi->right_side[i+1];
                        wi->right_fixed[2*i+4] = 0;
                        /*
                        wi->left_fixed[2*i+5] = wi->left_side[i+2];
                        wi->left_fixed[2*i+6] = 0;
                        wi->right_fixed[2*i+5] = wi->right_side[i+2];
                        wi->right_fixed[2*i+6] = 0;
                         
                        wi->left_fixed[2*i+7] = wi->left_side[i+3];
                        wi->left_fixed[2*i+8] = 0;
                        wi->right_fixed[2*i+7] = wi->right_side[i+3];
                        wi->right_fixed[2*i+8] = 0;
                        */
                 }
        }
        bench_fix_data = wat_gettime() - bench_fix_data;

        wat_log(LOG_PANIC, ", DONE");
        return 1;
}
#endif //UNROLL

#if defined(OPT) 
int fix_data_to_fft(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nfix_data_to_fft");

        wi->left_fixed = (double *)malloc(2 * wi->nb_samples * sizeof(double) + 1);
        if(wi->left_fixed == NULL){
                wat_log(LOG_ERROR, "\nMemory error in left_fixed");
        }
        if(wi->wav_header->num_channels == 2){
                wi->right_fixed = (double *)malloc(2 * wi->nb_samples * sizeof(double) + 1);
                if(wi->right_fixed == NULL){
                        wat_log(LOG_ERROR, "\nMemory error in right_fixed");
                }
        }

        wat_log(LOG_PANIC, ", allocated");
        bench_fix_data = wat_gettime();
        int i;
        if(wi->wav_header->num_channels == 1){
                for(i = 0; i < wi->nb_samples; i++){
                        wi->left_fixed[2*i+1] = wi->left_side[i];
                        wi->left_fixed[2*i+2] = 0;
                }
        }
        else if(wi->wav_header->num_channels == 2){
                for(i = 0; i < wi->nb_samples; i++){
                        wi->left_fixed[2*i+1] = wi->left_side[i];
                        wi->left_fixed[2*i+2] = 0;
                        wi->right_fixed[2*i+1] = wi->right_side[i];
                        wi->right_fixed[2*i+2] = 0;
                 }
        }
        bench_fix_data = wat_gettime() - bench_fix_data;

        wat_log(LOG_PANIC, ", DONE");
        return 1;
}
#endif //OPT

#if defined(UNROFIS) 
int fix_data_to_fft(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nfix_data_to_fft");

        wi->left_fixed = (double *)malloc(2 * wi->nb_samples * sizeof(double) + 1);
        if(wi->left_fixed == NULL){
                wat_log(LOG_ERROR, "\nMemory error in left_fixed");
        }
        if(wi->wav_header->num_channels == 2){
                wi->right_fixed = (double *)malloc(2 * wi->nb_samples * sizeof(double) + 1);
                if(wi->right_fixed == NULL){
                        wat_log(LOG_ERROR, "\nMemory error in right_fixed");
                }
        }

        wat_log(LOG_PANIC, ", allocated");
        bench_fix_data = wat_gettime();
        int i;
        if(wi->wav_header->num_channels == 1){
                for(i = 0; i < wi->nb_samples; i+=2){
                        wi->left_fixed[2*i+1] = wi->left_side[i];
                        wi->left_fixed[2*i+2] = 0;
                        wi->left_fixed[2*i+3] = wi->left_side[i+1];
                        wi->left_fixed[2*i+4] = 0;
                }
        }
        else if(wi->wav_header->num_channels == 2){
                for(i = 0; i < wi->nb_samples; i+=2){
                        wi->left_fixed[2*i+1] = wi->left_side[i];
                        wi->left_fixed[2*i+2] = 0;

                        wi->left_fixed[2*i+3] = wi->left_side[i+1];
                        wi->left_fixed[2*i+4] = 0;
                }
                for(i = 0; i < wi->nb_samples; i+=2){
                        wi->right_fixed[2*i+1] = wi->right_side[i];
                        wi->right_fixed[2*i+2] = 0;

                        wi->right_fixed[2*i+3] = wi->right_side[i+1];
                        wi->right_fixed[2*i+4] = 0;
                }
        }
        bench_fix_data = wat_gettime() - bench_fix_data;

        wat_log(LOG_PANIC, ", DONE");
        return 1;
}
#endif //UNROFIS


#if defined(FISSION) 
int fix_data_to_fft(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nfix_data_to_fft");

        wi->left_fixed = (double *)malloc(2 * wi->nb_samples * sizeof(double) + 1);
        if(wi->left_fixed == NULL){
                wat_log(LOG_ERROR, "\nMemory error in left_fixed");
        }
        if(wi->wav_header->num_channels == 2){
                wi->right_fixed = (double *)malloc(2 * wi->nb_samples * sizeof(double) + 1);
                if(wi->right_fixed == NULL){
                        wat_log(LOG_ERROR, "\nMemory error in right_fixed");
                }
        }

        wat_log(LOG_PANIC, ", allocated");
        bench_fix_data = wat_gettime();
        int i;
        if(wi->wav_header->num_channels == 1){
                for(i = 0; i < wi->nb_samples; i++){
                        wi->left_fixed[2*i+1] = wi->left_side[i];
                        wi->left_fixed[2*i+2] = 0;
                }
        }
        else if(wi->wav_header->num_channels == 2){
                for(i = 0; i < wi->nb_samples; i++){
                        wi->left_fixed[2*i+1] = wi->left_side[i];
                        wi->left_fixed[2*i+2] = 0;
                }
                for(i = 0; i < wi->nb_samples; i++){
                        wi->right_fixed[2*i+1] = wi->right_side[i];
                        wi->right_fixed[2*i+2] = 0;
                }
        }
        bench_fix_data = wat_gettime() - bench_fix_data;

        wat_log(LOG_PANIC, ", DONE");
        return 1;
}
#endif //FISSION


#ifdef ORIG
int back_data_to_normal(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nback_data_to_normal");
        int i;

        for(i = 0; i < wi->nb_samples; i++){
                wi->left_side[i] = wi->left_fixed[2*i+1]; 
                if(wi->wav_header->num_channels == 2){
                        wi->right_side[i] = wi->right_fixed[2*i+1]; 
                }
        }

        wat_log(LOG_PANIC, ", DONE");
        return 1;
}
#endif // ORIG

#ifdef ORIG_B
int back_data_to_normal(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nback_data_to_normal");
        int i;

        bench_back_data = wat_gettime();
        for(i = 0; i < wi->nb_samples; i++){
                wi->left_side[i] = wi->left_fixed[2*i+1]; 
                if(wi->wav_header->num_channels == 2){
                        wi->right_side[i] = wi->right_fixed[2*i+1]; 
                }
        }
        bench_back_data = wat_gettime() - bench_back_data;

        wat_log(LOG_PANIC, ", DONE");
        return 1;
}
#endif //ORIG_B

#if defined(UNROLL) 
int back_data_to_normal(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nback_data_to_normal");
        int i;

        bench_back_data = wat_gettime();
        if(wi->wav_header->num_channels == 1){
                for(i = 0; i < wi->nb_samples; i+=2){
                        wi->left_side[i] = wi->left_fixed[2*i+1]; 
                        wi->left_side[i+1] = wi->left_fixed[2*(i+1)+1]; 
                }
        }
        else if(wi->wav_header->num_channels == 2){
                for(i = 0; i < wi->nb_samples; i+=2){
                        wi->left_side[i] = wi->left_fixed[2*i+1]; 
                        wi->right_side[i] = wi->right_fixed[2*i+1]; 

                        wi->left_side[i + 1] = wi->left_fixed[2*(i+1)+1]; 
                        wi->right_side[i + 1] = wi->right_fixed[2*(i+1)+1]; 
 
                }
        }
        bench_back_data = wat_gettime() - bench_back_data;

        wat_log(LOG_PANIC, ", DONE");
        return 1;
}
#endif // UNROLL


#if defined(OPT) 
int back_data_to_normal(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nback_data_to_normal");
        int i;

        bench_back_data = wat_gettime();
        if(wi->wav_header->num_channels == 1){
                for(i = 0; i < wi->nb_samples; i++){
                        wi->left_side[i] = wi->left_fixed[2*i+1]; 
                }
        }
        else if(wi->wav_header->num_channels == 2){
                for(i = 0; i < wi->nb_samples; i++){
                        wi->left_side[i] = wi->left_fixed[2*i+1]; 
                        wi->right_side[i] = wi->right_fixed[2*i+1]; 
                }
        }
        bench_back_data = wat_gettime() - bench_back_data;

        wat_log(LOG_PANIC, ", DONE");
        return 1;
}
#endif //OPT

#if defined(UNROFIS)
int back_data_to_normal(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nback_data_to_normal");
        int i;

        bench_back_data = wat_gettime();
        if(wi->wav_header->num_channels == 1){
                for(i = 0; i < wi->nb_samples; i+=2){
                        wi->left_side[i] = wi->left_fixed[2*i+1]; 
                        wi->left_side[i+1] = wi->left_fixed[2*(i+1)+1]; 
                }
        }
        else if(wi->wav_header->num_channels == 2){
                for(i = 0; i < wi->nb_samples; i+=2){
                        wi->left_side[i] = wi->left_fixed[2*i+1]; 
                        wi->left_side[i+1] = wi->left_fixed[2*(i+1)+1]; 
                }
                for(i = 0; i < wi->nb_samples; i+=2){
                        wi->right_side[i] = wi->right_fixed[2*i+1]; 
                        
                        wi->right_side[i+1] = wi->right_fixed[2*(i+1)+1]; 
                }
        }
        bench_back_data = wat_gettime() - bench_back_data;

        wat_log(LOG_PANIC, ", DONE");
        return 1;
}
#endif //UNROFIS



#if defined(FISSION)
int back_data_to_normal(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nback_data_to_normal");
        int i;

        bench_back_data = wat_gettime();
        if(wi->wav_header->num_channels == 1){
                for(i = 0; i < wi->nb_samples; i++){
                        wi->left_side[i] = wi->left_fixed[2*i+1]; 
                }
        }
        else if(wi->wav_header->num_channels == 2){
                for(i = 0; i < wi->nb_samples; i++){
                        wi->left_side[i] = wi->left_fixed[2*i+1]; 
                }
                for(i = 0; i < wi->nb_samples; i++){
                        wi->right_side[i] = wi->right_fixed[2*i+1]; 
                }
        }
        bench_back_data = wat_gettime() - bench_back_data;

        wat_log(LOG_PANIC, ", DONE");
        return 1;
}
#endif //FISSION


int init(WavInput *wi, int argc, char *argv[])
{
        FILE *f;        
        int ret;
        char * msg = malloc(128 * sizeof(char));

        wi->wat_args = (Arguments *)malloc(sizeof(Arguments));

        ret = parse_args(wi, argc, argv);
        if(ret < 1)
                return -1;


        f = fopen(wi->file_name, "r");
        if(f == NULL){
                sprintf(msg, "\nError while openning input file: \"%s\" \n", 
                                wi->file_name);
                wat_log(LOG_INFO, msg);
                wat_log(LOG_ERROR," in init\n");
                exit(1);
        }

        fseek(f, 0, SEEK_END);
        wi->file_size = ftell(f);
        fclose(f);

        sprintf(msg, "init, Input: %s (%d bytes)", wi->file_name, wi->file_size);
        wat_log(LOG_PANIC, msg);

        wat_log(LOG_PANIC, ", DONE.");

        return ret;
}

#ifdef ORIG
int convert_double_to_short(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nconvert_double_to_short");

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

        wat_log(LOG_PANIC, ", short arrays allocated");
        int i;
        for(i = 0; i < wi->nb_samples; i++){
                wi->short_left[i] = (short int)wi->left_side[i];
                if(wi->wav_header->num_channels == 2){
                        wi->short_right[i] = (short int)wi->right_side[i];
                }
        }

        wat_log(LOG_PANIC, ", copying into buffer");
        int iterator = wi->wav_header->num_channels * 2;
        int count = 0;
        for(i = 0; i < wi->nb_samples * iterator; i += iterator){
                memcpy(&wi->buffer[i], &wi->short_left[count], 2);
                if(wi->wav_header->num_channels == 2){
                        memcpy(&wi->buffer[i + 2], &wi->short_right[count], 2);
                }
                count++;
        }
        wat_log(LOG_PANIC, ", DONE.");

        return 1;
}
#endif //ORIG

#ifdef ORIG_B
int convert_double_to_short(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nconvert_double_to_short");

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

        wat_log(LOG_PANIC, ", short arrays allocated");
        int i;
        bench_convert_double = wat_gettime();
        for(i = 0; i < wi->nb_samples; i++){
                wi->short_left[i] = (short int)wi->left_side[i];
                if(wi->wav_header->num_channels == 2){
                        wi->short_right[i] = (short int)wi->right_side[i];
                }
        }

        wat_log(LOG_PANIC, ", copying into buffer");
        int iterator = wi->wav_header->num_channels * 2;
        int count = 0;
 
        for(i = 0; i < wi->nb_samples * iterator; i += iterator){
                memcpy(&wi->buffer[i], &wi->short_left[count], 2);
                if(wi->wav_header->num_channels == 2){
                        memcpy(&wi->buffer[i + 2], &wi->short_right[count], 2);
                }
                count++;
        }
        bench_convert_double = wat_gettime() - bench_convert_double;
        wat_log(LOG_PANIC, ", DONE.");

        return 1;
}
#endif // ORIG_B

#if defined(UNROLL) 
int convert_double_to_short(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nconvert_double_to_short");

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

        wat_log(LOG_PANIC, ", short arrays allocated");
        int i;
        bench_convert_double = wat_gettime();
        if(wi->wav_header->num_channels == 1){
                for(i = 0; i < wi->nb_samples; i+=2){
                        wi->short_left[i] = (short int)wi->left_side[i];
                        wi->short_left[i+1] = (short int)wi->left_side[i+1];
                }
        }
        else if(wi->wav_header->num_channels == 2){
                for(i = 0; i < wi->nb_samples; i+=2){
                        wi->short_left[i] = (short int)wi->left_side[i];
                        wi->short_right[i] = (short int)wi->right_side[i];

                        wi->short_left[i+1] = (short int)wi->left_side[i+1];
                        wi->short_right[i+1] = (short int)wi->right_side[i+1];
                }
        }

        wat_log(LOG_PANIC, ", copying into buffer");
        int iterator = wi->wav_header->num_channels * 2;
        int count = 0;
        if(wi->wav_header->num_channels == 1){
                for(i = 0; i < wi->nb_samples * iterator; i += 4){
                        memcpy(&wi->buffer[i], &wi->short_left[count], 2);
                        count++;
                        memcpy(&wi->buffer[i + 2], &wi->short_left[count], 2);
                        count++;
                }
        }
        if(wi->wav_header->num_channels == 2){
                for(i = 0; i < wi->nb_samples * iterator; i +=  8){
                        memcpy(&wi->buffer[i], &wi->short_left[count], 2);
                        memcpy(&wi->buffer[i + 2], &wi->short_right[count], 2);
                        count++;
                        memcpy(&wi->buffer[i + 4], &wi->short_left[count], 2);
                        memcpy(&wi->buffer[i + 6], &wi->short_right[count], 2);
                        count++;

                }
        }

        bench_convert_double = wat_gettime() - bench_convert_double;
        wat_log(LOG_PANIC, ", DONE.");

        return 1;
}
#endif // UNROLL

#if defined(OPT) 
int convert_double_to_short(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nconvert_double_to_short");

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

        wat_log(LOG_PANIC, ", short arrays allocated");
        int i;
        bench_convert_double = wat_gettime();
        if(wi->wav_header->num_channels == 1){
                for(i = 0; i < wi->nb_samples; i++){
                        wi->short_left[i] = (short int)wi->left_side[i];
                }
        }
        else if(wi->wav_header->num_channels == 2){
                for(i = 0; i < wi->nb_samples; i++){
                        wi->short_left[i] = (short int)wi->left_side[i];
                        wi->short_right[i] = (short int)wi->right_side[i];
                }
        }

        wat_log(LOG_PANIC, ", copying into buffer");
        int iterator = wi->wav_header->num_channels * 2;
        int count = 0;
        if(wi->wav_header->num_channels == 1){
                for(i = 0; i < wi->nb_samples * iterator; i += iterator){
                        memcpy(&wi->buffer[i], &wi->short_left[count], 2);
                        count++;
                }
        }
        if(wi->wav_header->num_channels == 2){
                for(i = 0; i < wi->nb_samples * iterator; i += iterator){
                        memcpy(&wi->buffer[i], &wi->short_left[count], 2);
                        memcpy(&wi->buffer[i + 2], &wi->short_right[count], 2);
                        count++;
                }
        }

        bench_convert_double = wat_gettime() - bench_convert_double;
        wat_log(LOG_PANIC, ", DONE.");

        return 1;
}
#endif // OPT

#if defined(UNROFIS)
int convert_double_to_short(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nconvert_double_to_short");

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

        wat_log(LOG_PANIC, ", short arrays allocated");
        int i;

        wat_log(LOG_PANIC, ", copying into buffer");
        bench_convert_double = wat_gettime();
        int iterator = wi->wav_header->num_channels * 2;
        int count = 0;
        if(wi->wav_header->num_channels == 1){
                for(i = 0; i < wi->nb_samples * 2; i += 4){
                        wi->short_left[i / 2] = (short int)wi->left_side[i / 2];
                        memcpy(&wi->buffer[i], &wi->short_left[count], 2);
                        count++;
                        wi->short_left[(i+2) / 2] = (short int)wi->left_side[(i+2) / 2];
                        memcpy(&wi->buffer[i + 2], &wi->short_left[count], 2);
                        count++;
                }
        }
        if(wi->wav_header->num_channels == 2){
                for(i = 0; i < wi->nb_samples * 4; i += 8){
                        wi->short_left[i / 4] = (short int)wi->left_side[i / 4];
                        memcpy(&wi->buffer[i], &wi->short_left[count], 2);
                        count++;
                        wi->short_left[(i+4) / 4] = (short int)wi->left_side[(i+4) / 4];
                        memcpy(&wi->buffer[i + 4], &wi->short_left[count], 2);
                        count++;
                }
                count = 0;
                for(i = 0; i < wi->nb_samples * 4; i += 8){
                        wi->short_right[i / 4] = (short int)wi->right_side[i / 4];
                        memcpy(&wi->buffer[i + 2], &wi->short_right[count], 2);
                        count++;
                        wi->short_right[(i+4) / 4] = (short int)wi->right_side[(i+4) / 4];
                        memcpy(&wi->buffer[i + 4], &wi->short_right[count], 2);
                        count++;
                }
        }

        bench_convert_double = wat_gettime() - bench_convert_double;
        wat_log(LOG_PANIC, ", DONE.");

        return 1;
}
#endif // UNROFIS


#if defined(FISSION)
int convert_double_to_short(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nconvert_double_to_short");

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

        wat_log(LOG_PANIC, ", short arrays allocated");
        int i;

        wat_log(LOG_PANIC, ", copying into buffer");
        bench_convert_double = wat_gettime();
        int iterator = wi->wav_header->num_channels * 2;
        int count = 0;
        if(wi->wav_header->num_channels == 1){
                for(i = 0; i < wi->nb_samples * 2; i += 2){
                        wi->short_left[i / 2] = (short int)wi->left_side[i / 2];
                        memcpy(&wi->buffer[i], &wi->short_left[count], 2);
                        count++;
                }
        }
        if(wi->wav_header->num_channels == 2){
                for(i = 0; i < wi->nb_samples * 4; i += 4){
                        wi->short_left[i / 4] = (short int)wi->left_side[i / 4];
                        memcpy(&wi->buffer[i], &wi->short_left[count], 2);
                        count++;
                }
                count = 0;
                for(i = 0; i < wi->nb_samples * 4; i += 4){
                        wi->short_right[i / 4] = (short int)wi->right_side[i / 4];
                        memcpy(&wi->buffer[i + 2], &wi->short_right[count], 2);
                        count++;
                }
        }

        bench_convert_double = wat_gettime() - bench_convert_double;
        wat_log(LOG_PANIC, ", DONE.");

        return 1;
}
#endif // FISSION

#ifdef ORIG
int save_file(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nsave_file");

        if(wi->wat_args->raise != 0){
                raise_wav_file(wi);
        }

        char * msg = malloc(128 * sizeof(char));
        FILE *f;

        f = fopen(wi->output_file, "wb");
        if(f == NULL){
                sprintf(msg, "\nError while openning the output file: \"%s\".", 
                                wi->output_file);
                wat_log(LOG_INFO, msg);
                return -1;
        }

        write_header(wi->wav_header, f);

        convert_double_to_short(wi);

        int i;

        wat_log(LOG_PANIC, "\n, writing");

        for(i = 0; i < wi->wav_header->subchunk2_size; i += wi->wav_header->num_channels){
                fwrite(&wi->buffer[i], sizeof(unsigned char), 1, f);
                if(wi->wav_header->num_channels == 2){
                        fwrite(&wi->buffer[i + 1], sizeof(unsigned char), 1, f);
                }
        }

        fclose(f);
        wat_log(LOG_PANIC, ", DONE");
        return 1;        
}
#endif //ORIG

#if defined(ORIG_B) 
int save_file(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nsave_file");

        if(wi->wat_args->raise != 0){
                raise_wav_file(wi);
        }

        char * msg = malloc(128 * sizeof(char));
        FILE *f;

        f = fopen(wi->output_file, "wb");
        if(f == NULL){
                sprintf(msg, "\nError while openning the output file: \"%s\".", 
                                wi->output_file);
                wat_log(LOG_INFO, msg);
                return -1;
        }

        write_header(wi->wav_header, f);

        convert_double_to_short(wi);

        int i;

        wat_log(LOG_PANIC, "\n, writing");

        bench_save_file = wat_gettime();
        for(i = 0; i < wi->wav_header->subchunk2_size; i += wi->wav_header->num_channels){
                fwrite(&wi->buffer[i], sizeof(unsigned char), 1, f);
                if(wi->wav_header->num_channels == 2){
                        fwrite(&wi->buffer[i + 1], sizeof(unsigned char), 1, f);
                }
        }
        bench_save_file = wat_gettime() - bench_save_file;

        fclose(f);
        wat_log(LOG_PANIC, ", DONE");
        return 1;        
}
#endif //ORIG_B

#if defined(OPT)
int save_file(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nsave_file");

        if(wi->wat_args->raise != 0){
                raise_wav_file(wi);
        }

        char * msg = malloc(128 * sizeof(char));
        FILE *f;

        f = fopen(wi->output_file, "wb");
        if(f == NULL){
                sprintf(msg, "\nError while openning the output file: \"%s\".", 
                                wi->output_file);
                wat_log(LOG_INFO, msg);
                return -1;
        }

        write_header(wi->wav_header, f);

        convert_double_to_short(wi);

        int i;

        wat_log(LOG_PANIC, "\n, writing");

        bench_save_file = wat_gettime();
        if(wi->wav_header->num_channels == 1){
                for(i = 0; i < wi->wav_header->subchunk2_size; i += 1){
                        fwrite(&wi->buffer[i], sizeof(unsigned char), 1, f);
                }

        }
        else if(wi->wav_header->num_channels == 2){
                for(i = 0; i < wi->wav_header->subchunk2_size; i += 2){
                        fwrite(&wi->buffer[i], sizeof(unsigned char), 1, f);
                        fwrite(&wi->buffer[i + 1], sizeof(unsigned char), 1, f);
                }
        }
        bench_save_file = wat_gettime() - bench_save_file;

        fclose(f);
        wat_log(LOG_PANIC, ", DONE");
        return 1;        
}
#endif // OPT


#if defined(UNROLL) || defined(UNROFIS)
int save_file(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nsave_file");

        if(wi->wat_args->raise != 0){
                raise_wav_file(wi);
        }

        char * msg = malloc(128 * sizeof(char));
        FILE *f;

        f = fopen(wi->output_file, "wb");
        if(f == NULL){
                sprintf(msg, "\nError while openning the output file: \"%s\".", 
                                wi->output_file);
                wat_log(LOG_INFO, msg);
                return -1;
        }

        write_header(wi->wav_header, f);

        convert_double_to_short(wi);

        int i;

        wat_log(LOG_PANIC, "\n, writing");

        bench_save_file = wat_gettime();
        if(wi->wav_header->num_channels == 1){
                for(i = 0; i < wi->wav_header->subchunk2_size; i += 2){
                        fwrite(&wi->buffer[i], sizeof(unsigned char), 1, f);
                        fwrite(&wi->buffer[i + 1], sizeof(unsigned char), 1, f);
                }

        }
        else if(wi->wav_header->num_channels == 2){
                for(i = 0; i < wi->wav_header->subchunk2_size; i += 2){
                        fwrite(&wi->buffer[i], sizeof(unsigned char), 1, f);
                        fwrite(&wi->buffer[i + 1], sizeof(unsigned char), 1, f);

//                        fwrite(&wi->buffer[i + 2], sizeof(unsigned char), 1, f);
//                        fwrite(&wi->buffer[i + 3], sizeof(unsigned char), 1, f);
                }
        }
        bench_save_file = wat_gettime() - bench_save_file;

        fclose(f);
        wat_log(LOG_PANIC, ", DONE");
        return 1;        
}
#endif //UNROLL

#ifdef FISSION
int save_file(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nsave_file");

        if(wi->wat_args->raise != 0){
                raise_wav_file(wi);
        }

        char * msg = malloc(128 * sizeof(char));
        FILE *f;

        f = fopen(wi->output_file, "wb");
        if(f == NULL){
                sprintf(msg, "\nError while openning the output file: \"%s\".", 
                                wi->output_file);
                wat_log(LOG_INFO, msg);
                return -1;
        }

        write_header(wi->wav_header, f);

        convert_double_to_short(wi);

        int i;

        wat_log(LOG_PANIC, "\n, writing");

        bench_save_file = wat_gettime();
        for(i = 0; i < wi->wav_header->subchunk2_size/2; i += wi->wav_header->num_channels){
                fwrite(&wi->buffer[i], sizeof(unsigned char), 1, f);
                fwrite(&wi->buffer[i + 1], sizeof(unsigned char), 1, f);
        }
        for(i = wi->wav_header->subchunk2_size/2; i < wi->wav_header->subchunk2_size; i += wi->wav_header->num_channels){
                fwrite(&wi->buffer[i], sizeof(unsigned char), 1, f);
                fwrite(&wi->buffer[i + 1], sizeof(unsigned char), 1, f);
        }
 
        bench_save_file = wat_gettime() - bench_save_file;

        fclose(f);
        wat_log(LOG_PANIC, ", DONE");
        return 1;        
}
#endif //FISSION



void divides_nfft(double * temp, int size)
{
//        wat_log(LOG_PANIC, "\nDivides ");
        int i;
        for(i = 0; i < size; i++)
                temp[i] /= size;

//        wat_log(LOG_PANIC, " DONE");
}

/* FFT ----------------------------------------------------------------------- */
void equalize(struct s_fft *s)
{
        if(s->wi->wat_args->equalize != 0){
                if(s->wi->wav_header->sample_rate == 44100){
                        equalize44k(s->wi, s->temp, s->NFFT);
                }
                else{
                        equalizeXk(s->wi, s->temp, s->NFFT);
                }
        }
}

int channel_fft(struct s_fft *s)
{
        int pos;
//        uint32_t * samples = calloc(2, sizeof(uint32_t));
#ifdef HAVE_THREADS
//        printf("\ns->it = %d\tthread = %d", s->it, s->tid);
#endif

        memset(s->temp, 0, sizeof(double) * s->array_size);
        memcpy(s->temp, &s->channel[s->it * s->freq], s->freq * sizeof(double));

//        samples[0] = wat_gettime();

        pos = ((s->it_max * s->num_channels) * (s->this_channel - 1)) + (2 * s->it);
        bench_fft_1[pos] = pick_fft(s->temp, s->NFFT, 1);
//        samples[0] = wat_gettime() - samples[0];

        equalize(s);

//        samples[1] = wat_gettime();
        bench_fft_1[pos + 1] = pick_fft(s->temp, s->NFFT, -1);
//        samples[1] = wat_gettime() - samples[1];

        divides_nfft(s->temp, s->NFFT);

        memcpy(&s->channel[s->it * s->freq], s->temp, s->freq * sizeof(double));

//        statistics(samples, 2);

        return 1;
}

#ifdef HAVE_THREADS
        
void * run_fft_t(void *st)
{
        channel_fft((struct s_fft *) st);
        return NULL;
}
#endif

struct s_fft* set_data(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nset_data(struct s_fft)");
        struct s_fft *st = (struct s_fft *)malloc(sizeof(struct s_fft));

        st->wi = wi;

        st->log = (char *)malloc(128 * sizeof(char));

        st->freq = wi->wav_header->sample_rate * 2;

        st->NFFT = (int)pow(2.0, ceil(log((double)st->freq)/log(2.0)));

        st->array_size = 2 * st->NFFT + 1;

        st->temp = (double *)calloc(st->array_size,  sizeof(double));
 
        wat_log(LOG_PANIC, ", DONE.");
        return st;
}

int run_fft(WavInput *wi, float seconds)
{
        wat_log(LOG_PANIC, "\nrun_fft");
        int i;
        int sec = floor(seconds);

#ifdef HAVE_THREADS

        wat_log(LOG_PANIC, "\n with threads");

        if(wi->nb_thread > sec){
                wi->nb_thread = sec;
                wat_log(LOG_INFO, "\nNumber of threads is more than a number of seconds\n");
        }
        pthread_t *fft_thread;

        /* if nb_thread is odd will be added 1 */
        wi->nb_thread += ((wi->nb_thread & 1) == 1);

        fft_thread = (pthread_t *)malloc(wi->nb_thread * sizeof(pthread_t));

        struct s_fft **st_t = (struct s_fft **)malloc(
                        wi->nb_thread * sizeof(struct s_fft *));

        for(i = 0; i < wi->nb_thread; i++)
        {
                st_t[i] = set_data(wi);
                st_t[i]->tid = i;
        }

        fix_data_to_fft(wi);

        int sec_done = 0;
        int k;

        while(sec_done < sec){
                for(k = 0; k < wi->nb_thread; k++){
                        sprintf(st_t[k]->log, "\nTHREAD %d running FFT %d of %d seconds,"
                                        " in Channel 1, in %d to %d", 
                                        k,
                                        sec_done, 
                                        sec, 
                                        k * st_t[k]->freq, 
                                        k * st_t[k]->freq + st_t[k]->freq);
                        wat_log(LOG_PANIC, st_t[k]->log);


                        st_t[k]->it = sec_done;
                        st_t[k]->channel = wi->left_fixed;

                        pthread_create(&fft_thread[k], NULL, run_fft_t, st_t[k]);

                        if(wi->wav_header->num_channels == 2 && wi->wat_args->one_channel == 0){

                                k++;

                                sprintf(st_t[k]->log, "\nTHREAD FFT %d running %d of %d seconds,"
                                                " in Channel 2, in %d to %d", 
                                                k,
                                                sec_done, 
                                                sec, 
                                                k * st_t[k]->freq, 
                                                k * st_t[k]->freq + st_t[k]->freq);
                                wat_log(LOG_PANIC, st_t[k]->log);

                                st_t[k]->it = sec_done;
                                st_t[k]->channel = wi->right_fixed;

                                pthread_create(&fft_thread[k], NULL, run_fft_t, st_t[k]);
                        }

                        sec_done++;
                        if(sec_done >= sec)
                                break;
                }

                for(k = 0; k < wi->nb_thread; k++){
                        pthread_join(fft_thread[k], NULL);
                }
        }
        back_data_to_normal(wi);
        return 1;

#else //HAVE_THREADS

        struct s_fft *s = set_data(wi);

        fix_data_to_fft(wi);

        int nb_samples = sec;

        s->it_max = nb_samples;

        uint32_t *samples_ch1 = calloc(nb_samples, sizeof(uint32_t));
        uint32_t * samples_ch2 = NULL;

        if(wi->wav_header->num_channels == 1 || wi->wat_args->one_channel == 1){
                bench_fft_1 = (uint32_t *)calloc(nb_samples * 2, sizeof(uint32_t));
                s->num_channels = 1;
        }
        else if(wi->wav_header->num_channels == 2 && wi->wat_args->one_channel == 0){
                samples_ch2 = calloc(nb_samples, sizeof(uint32_t));
                bench_fft_1 = (uint32_t *)calloc(nb_samples * 4, sizeof(uint32_t));
                s->num_channels = 2;
        }

        for(i = 0; i < floor(sec); i++){

                sprintf(s->log, "\nFFT %d of %d seconds, in Channel 1,"
                                " in %d to %d", 
                                i, 
                                sec, 
                                i * s->freq, 
                                i * s->freq + s->freq);
                wat_log(LOG_PANIC, s->log);

                s->it = i;
                s->this_channel = 1;
                s->channel = wi->left_fixed;

                samples_ch1[i] = wat_gettime();
                channel_fft(s);
                samples_ch1[i] = wat_gettime() - samples_ch1[i];

                if(wi->wav_header->num_channels == 2 && wi->wat_args->one_channel == 0){
                        sprintf(s->log, "\nFFT %d of %d seconds, in Channel 2,"
                                        " in %d to %d", 
                                        i, 
                                        sec, 
                                        i * s->freq, 
                                        i * s->freq + s->freq);
                        wat_log(LOG_PANIC, s->log);

                        s->channel = wi->right_fixed;
                        s->this_channel = 2;

                        samples_ch2[i] = wat_gettime();
                        channel_fft(s);
                        samples_ch2[i] = wat_gettime() - samples_ch2[i];
                }
        }

//        wat_log(LOG_BENCH, "\nStatistics of run_fft");
        /*
        statistics(samples_ch1, nb_samples, "run_fft_ch1");
        if(wi->wav_header->num_channels == 1 || wi->wat_args->one_channel == 1){
                statistics(bench_fft_1, nb_samples * 2, "bench_fft_1");
        }
        else if(wi->wav_header->num_channels == 2 && wi->wat_args->one_channel == 0){
                statistics(samples_ch2, nb_samples, "run_fft_ch2");
                statistics(bench_fft_1, nb_samples * 4, "bench_fft_1");
        }
        */


        back_data_to_normal(wi);
        wat_log(LOG_PANIC, "\nrun_fft, DONE.");
        return 1;
#endif //HAVE_THREADS
}

/* END FFT -------------------------------------------------------------------- */

float get_seconds(WavHeader *wh)
{
        wat_log(LOG_PANIC, "\nget_seconds ,");
        int sec = 0;
        char * msg = (char *)malloc(128 * sizeof(char));

        sec = wh->subchunk2_size / wh->sample_rate;
        sec >>= 1;

        if(wh->num_channels == 2)
                sec >>= 1;

        sprintf(msg, "Duration: %d seconds", sec);
        wat_log(LOG_PANIC, msg);
        wat_log(LOG_PANIC, ", DONE.");
        return sec;
}

int main(int argc, char **argv)
{
        int i = 1;
        int n_times = 1;
        char bench_type[20];

#if defined(ORIG) || defined(ORIG_B)
        sprintf(bench_type, "ORIG_B");
#endif

#ifdef OPT
        sprintf(bench_type, "OPT");
#endif

#ifdef UNROLL
        sprintf(bench_type, "UNROLL");
#endif
        
#ifdef UNROFIS
        sprintf(bench_type, "UNROFIS");
#endif



#ifdef FISSION
        sprintf(bench_type, "FISSION");
#endif


        printf("\t\t\t\t<-------------\n");
        do{
                bench_wat = wat_gettime();
                char * msg = malloc(128 * sizeof(char)); 
                printf("\n---------------------------------------------------");
                sprintf(msg, "\nRound = %d about %s", i, bench_type);
                wat_log(LOG_BENCH, msg);
                printf("\n---------------------------------------------------");
 
                int ret;

                wav_input = (WavInput *)malloc(sizeof(WavInput));

                if(argc < 2){
                        help();
                        exit(3);
                }

                ret = init(wav_input, argc, argv);

                if(ret < 0){
                        exit(4);
                }

                wav_input->wav_header = (WavHeader *)malloc(sizeof(WavHeader));
                ret = read_header_file(wav_input->wav_header, wav_input->file_name);

                if(wav_input->wat_args->print_header)
                        print_header(wav_input->wav_header);

                /* getting the duration of audio */
                float seconds = get_seconds(wav_input->wav_header);

                if(wav_input->wat_args->only_sec){
                        if(wav_input->wat_args->only_sec > seconds || seconds <= 0){
                                sprintf(msg, "\nArgument -sec %d invalid, should be less than %f,"
                                                "and more than 0\n\n", 
                                                wav_input->wat_args->only_sec,  seconds);
                                wat_log(LOG_INFO, msg);
                                exit(1);
                        }
                        seconds = wav_input->wat_args->only_sec;
                        sprintf(msg, "\nUsing only %.2f seconds", seconds);
                        wat_log(LOG_INFO, msg);
                }

                ret = read_wav_data(wav_input);

                if(wav_input->wat_args->fft){
                        bench_fft = wat_gettime();
                        run_fft(wav_input, seconds);
                        bench_fft = wat_gettime() - bench_fft;
                }

                if(ret < 0){
                        exit(4);
                }
                if(wav_input->wat_args->has_output)
                        save_file(wav_input);

                bench_wat = wat_gettime() - bench_wat;

                sprintf(msg, "\n\nbench_read_data= %d", bench_read_data);
                wat_log(LOG_BENCH, msg);

                sprintf(msg, "\n\nbench_fix_data = %d", bench_fix_data);
                wat_log(LOG_BENCH, msg);

                sprintf(msg, "\n\nbench_equalize = %d", bench_equalize);
                wat_log(LOG_BENCH, msg);

                sprintf(msg, "\n\nbench_back_data= %d", bench_back_data);
                wat_log(LOG_BENCH, msg);

                sprintf(msg, "\n\nbench_fft= %d", bench_fft);
                wat_log(LOG_BENCH, msg);

                sprintf(msg, "\n\nbench_convert_double= %d", bench_convert_double);
                wat_log(LOG_BENCH, msg);

                sprintf(msg, "\n\nbench_save_file = %d", bench_save_file);
                wat_log(LOG_BENCH, msg);

                sprintf(msg, "\n\nbench_wat = %d", bench_wat);
                wat_log(LOG_BENCH, msg);

                bench_sum = 0;
                bench_sum += bench_equalize;
                bench_sum += bench_fix_data;
                bench_sum += bench_read_data;
                bench_sum += bench_back_data;
                bench_sum += bench_convert_double;
                bench_sum += bench_save_file;

                sprintf(msg, "\n\nbench_sum = %d", bench_sum);
                wat_log(LOG_BENCH, msg);



                printf("\n\n---------------------------------------------------");

                n_times = wav_input->wat_args->n_times;
                i++;
        }while(n_times >= i);
        /*
        sprintf(msg, "\n:)\n");
        wat_log(LOG_BENCH, msg);
        sprintf(msg, "\t\t\t\t<-------------\n");
        wat_log(LOG_BENCH, msg);
        */
        printf("\n\n");
        return 1;
}
/*

   uint32_t sample = wat_gettime();
   metodo
   sample = wat_gettime() - sample;
   printf("\n\ntempo = %d\n\n", sample);

   Com otimização => 9368.38

   Sem otimização => 9655.30

*/
