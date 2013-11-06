#include "utils.h"
#include <sys/param.h>
#include <sys/sysctl.h>
#include "wat.h"

int get_number_of_cores()
{
        int info[] = {CTL_HW, HW_AVAILCPU};
        size_t len = 4;
        uint32_t nb;

        sysctl(info, 2, &nb, &len, NULL, 0);

        if(nb < 1){
                info[1] = HW_NCPU;
                sysctl(info, 2, &nb, &len, NULL, 0);
                if(nb < 1)
                        nb = 1;
        }

        return nb;
}

int write_header_raised(WavHeader *wh, FILE *f, int nb)
{
        wat_log(LOG_PANIC, "\nwrite_header");

        wh->chunk_size += wh->subchunk2_size * nb;
        wh->subchunk2_size *= nb;
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

        wat_log(LOG_PANIC, "\nwrite_header DONE\n");

        return 1;
}

int raise_wav_file(WavInput *wi)
{
        wat_log(LOG_PANIC, "\nraise_wav_file");
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

        int original_size = wh->subchunk2_size;
        write_header_raised(wh, f, wi->wat_args->raise);
        
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
        int r;
        for(r = 0; r < wi->wat_args->raise; r++){
                for(i = 0; i < original_size; i += iterator){
                        fwrite(&wi->buffer[i], sizeof(unsigned char), 1, f);
                        if(wh->num_channels == 2){
                                fwrite(&wi->buffer[i + 1], sizeof(unsigned char), 1, f);
                        }
                }
        }

        wat_log(LOG_INFO, "\nFile raised\n\n");
        exit(1);
        return 1;
}

