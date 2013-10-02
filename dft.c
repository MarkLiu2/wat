#include "dft.h"
#include <math.h>
#include <stdlib.h>

/*
   Discrete Fourier Transform
   */

int dft(long int length, double real_sample[], double imag_sample[])
{

        wat_log(LOG_INFO, "\nDFT function.\n\n");
        long int i, j;
        double arg;
        double cosarg,sinarg;
        double *temp_real=NULL,*temp_imag=NULL;

        char msg[60];

        temp_real = calloc(length, sizeof(double));
        temp_imag = calloc(length, sizeof(double));
        if (temp_real == NULL || temp_imag == NULL)
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
                temp_imag[i] = 0;
                arg = -1.0 * 2.0 * 3.141592654 * (double)i / (double)length;
                for(j=0; j<length; j+=1) 
                {
                        cosarg = cos(j * arg);
                        sinarg = sin(j * arg);
                        temp_real[i] += (real_sample[j] * cosarg - imag_sample[j] * sinarg);
                        temp_imag[i] += (real_sample[j] * sinarg + imag_sample[j] * cosarg);
                }
        }

        wat_log(LOG_PANIC, "\n\nEnd of dft \n\n");
        /* Copy the data back */
        for (i=0; i<length; i+=1) 
        {
                if(i % 1001 == 0)
                        wat_log(LOG_PANIC, "\nDFT Copying data");
                real_sample[i] = temp_real[i];
                imag_sample[i] = temp_imag[i];
        }

        free(temp_real);
        free(temp_imag);
        return(1);
}


/*
   Inverse Discrete Fourier Transform
   */

int inverse_dft(long int length, double real_sample[], double imag_sample[])
{
        wat_log(LOG_INFO, "\nInverse DFT function.\n\n");
        long int i, j;
        double arg;
        double cosarg,sinarg;
        double *temp_real=NULL,*temp_imag=NULL;

        char msg[60];

        temp_real = calloc(length, sizeof(double));
        temp_imag = calloc(length, sizeof(double));
        if (temp_real == NULL || temp_imag == NULL)
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
                temp_imag[i] = 0;
                arg = 2.0 * 3.141592654 * (double)i / (double)length;
                for(j=0; j<length; j+=1) 
                {
                        cosarg = cos(j * arg);
                        sinarg = sin(j * arg);
                        temp_real[i] += (real_sample[j] * cosarg - imag_sample[j] * sinarg);
                        temp_imag[i] += (real_sample[j] * sinarg + imag_sample[j] * cosarg);
                }
        }

        wat_log(LOG_PANIC, "\n\nEnd of idft \n\n");
        /* Copy the data back */
        for (i=0; i<length; i+=1) 
        {
                if(i % 1001 == 0)
                        wat_log(LOG_PANIC, "\nIDFT Copying data");
                real_sample[i] = temp_real[i] / (double)length;
                imag_sample[i] = temp_imag[i] / (double)length;
        }

        free(temp_real);
        free(temp_imag);
        return(1);
}
