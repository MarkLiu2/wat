#include "dft.h"
#include <math.h>
#include <stdlib.h>

/*
   Discrete Fourier Transform
   */

int dft(long int length, double real_sample[])
{

        wat_log(LOG_INFO, "\nDFT function.\n\n");
        long int i, j;
        double arg;
        double cosarg,sinarg;
        double *temp_real = NULL;

        char msg[60];

        temp_real = calloc(length, sizeof(double));
        if (temp_real == NULL)
        {
                wat_log(LOG_INFO, "Null data in dft function.");
                return(-1);
        }

        for(i=0; i<length; i+=1) 
        {
                if(i % 1001 == 0){
                        sprintf(msg, "\nDFT Out loop with iteration = %lu", i);
                        wat_log(LOG_PANIC, msg);
                }
                temp_real[i] = 0;
                arg = -1.0 * 2.0 * 3.141592654 * (double)i / (double)length;
                for(j=0; j<length; j+=1) 
                {
                        cosarg = cos(j * arg);
                        sinarg = sin(j * arg);
                        temp_real[i] += (real_sample[j] * cosarg);
                        /*
                        if(i < 22050){
                                if(i < 11025)
                                        temp_real[i] *= 1.5;
                                else 
                                        temp_real[i] *= 0.5;
                        }
                        */
                }
        }

        wat_log(LOG_PANIC, "\n\nEnd of dft \n\n");
        /* Copy the data back */
        for (i=0; i<length; i+=1) 
        {
                if(i % 1001 == 0)
                        wat_log(LOG_PANIC, "\nDFT Copying data");
                real_sample[i] = temp_real[i];
        }

        free(temp_real);
        return(1);
}


/*
   Inverse Discrete Fourier Transform
   */

int inverse_dft(long int length, double real_sample[])
{
        wat_log(LOG_INFO, "\nInverse DFT function.\n\n");
        long int i, j;
        double arg;
        double cosarg,sinarg;
        double *temp_real = NULL;

        char msg[60];

        temp_real = calloc(length, sizeof(double));
        if (temp_real == NULL)
        {
                wat_log(LOG_INFO, "\n NULL data in Inverse DFT function.");
                return(-1);
        }

        for(i=0; i<length; i+=1) 
        {
                if(i % 1001 == 0){
                        sprintf(msg, "\nIDFT Out loop with iteration = %lu", i);
                        wat_log(LOG_PANIC, msg);
                }
                temp_real[i] = 0;
                arg = 2.0 * 3.141592654 * (double)i / (double)length;
                for(j=0; j<length; j+=1) 
                {
                        cosarg = cos(j * arg);
                        temp_real[i] += (real_sample[j] * cosarg);
                }
        }

        wat_log(LOG_PANIC, "\n\nEnd of idft \n\n");
        /* Copy the data back */
        for (i=0; i<length; i+=1) 
        {
                if(i % 1001 == 0)
                        wat_log(LOG_PANIC, "\nI;DFT Copying data");
                real_sample[i] = temp_real[i] / (double)length;
        }

        free(temp_real);
        return(1);
}
