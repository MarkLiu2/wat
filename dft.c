#include "dft.h"
#include <math.h>
#include <stdlib.h>

#define PI	M_PI	/* pi to machine precision, defined in math.h */
#define TWOPI	(2.0*PI)


/*
   Discrete Fourier Transform
   */

int dft(long int length, double real_sample[], double imag_sample[])
{

        wat_log(LOG_INFO, "\nDFT function.\n\n");
        long int i, j;
        int freq = length;
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
                if(i < freq/2){
                        if(i < freq/4){
                                temp_real[i] *= 2.0;
                        }
                        else if( i > freq/4){
                                temp_real[i] *= 2.0;
                        }
                }
        }
        sprintf(msg, "\nLast iteration in i => %lu", i);
        wat_log(LOG_PANIC, msg);


        wat_log(LOG_PANIC, "\n\nEnd of dft \n\n");
        /* Copy the data back */
        for (i=0; i<length; i+=1) 
        {
                if(i % 1001 == 0){
                        sprintf(msg, "\nDFT Copying data  %lu", i);
                        wat_log(LOG_PANIC, msg);
                }
                real_sample[i] = temp_real[i];
                imag_sample[i] = temp_imag[i];
        }
        sprintf(msg, "\nLast Copy in i => %lu", i);
        wat_log(LOG_PANIC, msg);


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
        sprintf(msg, "\nLast iteration in i => %lu", i);
        wat_log(LOG_PANIC, msg);

        wat_log(LOG_PANIC, "\n\nEnd of idft \n\n");
        /* Copy the data back */
        for (i=0; i<length; i+=1) 
        {
                if(i % 1001 == 0){
                        sprintf(msg, "\nIDFT Copying data  %lu", i);
                        wat_log(LOG_PANIC, msg);
                }
                real_sample[i] = temp_real[i] / (double)length;
                imag_sample[i] = temp_imag[i] / (double)length;
        }
        sprintf(msg, "\nLast copy in i => %lu", i);
        wat_log(LOG_PANIC, msg);


        free(temp_real);
        free(temp_imag);
        return(1);
}


void four1(double data[], int nn, int isign)
{
        int n, mmax, m, j, istep, i;
        double wtemp, wr, wpr, wpi, wi, theta;
        double tempr, tempi;

        n = nn << 1;
        j = 1;
        for (i = 1; i < n; i += 2) {
                if (j > i) {
                        tempr = data[j];     
                        data[j] = data[i];     
                        data[i] = tempr;
                        
                        tempr = data[j+1]; 
                        data[j+1] = data[i+1]; 
                        data[i+1] = tempr;
                }
                m = n >> 1;
                while (m >= 2 && j > m) {
                        j -= m;
                        m >>= 1;
                }
                j += m;
        }
        mmax = 2;
        while (n > mmax) {
                istep = 2*mmax;
                theta = TWOPI/(isign*mmax);
                wtemp = sin(0.5*theta);
                wpr = -2.0*wtemp*wtemp;
                wpi = sin(theta);
                wr = 1.0;
                wi = 0.0;
                for (m = 1; m < mmax; m += 2) {
                        for (i = m; i <= n; i += istep) {
                                j =i + mmax;
                                tempr = wr*data[j]   - wi*data[j+1];
                                tempi = wr*data[j+1] + wi*data[j];
                                data[j]   = data[i]   - tempr;
                                data[j+1] = data[i+1] - tempi;
                                data[i] += tempr;
                                data[i+1] += tempi;
                        }
                        wr = (wtemp = wr)*wpr - wi*wpi + wr;
                        wi = wi*wpr + wtemp*wpi + wi;
                }
                mmax = istep;
        }
}


