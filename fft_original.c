/************************************************
 * FFT code from the book Numerical Recipes in C *
 * Visit www.nr.com for the licence.             *
 ************************************************/

#include "fft.h"
#include <math.h>
#include <stdlib.h>
#include "benchmark.h"
#define PI	M_PI
#define TWOPI	(2.0*PI)

 
uint32_t original_fft(double data[], int nn, int isign)
{
        int n, mmax, m, j, istep, i;
        double wtemp, wr, wpr, wpi, wi, theta;
        double tempr, tempi;
        uint32_t sample;

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

        sample = wat_gettime();
        int snk;
        int inner;
        int max = n/2;
        mmax = 2;
        while (n > mmax) {
                istep = 2*mmax;
                theta = TWOPI/(isign*mmax);
                wtemp = sin(0.5*theta);
                wpr = -2.0*wtemp*wtemp;
                wpi = sin(theta);
                wr = 1.0;
                wi = 0.0;
                snk = 4 * istep;

                for (m = 1; m < mmax; m += 2) {
                        for (i = m; i <= max; i += istep) {
                                j = i + mmax;
                                tempr = wr*data[j]   - wi*data[j+1];
                                tempi = wr*data[j+1] + wi*data[j];
                                data[j]   = data[i] - tempr;
                                data[j+1] = data[i+1] - tempi;
                                data[i] += tempr;
                                data[i+1] += tempi;
                        }
                }
                        wr = (wtemp = wr)*wpr - wi*wpi + wr;
                        wi = wi*wpr + wtemp*wpi + wi;
                mmax = istep;
        }
        sample = wat_gettime() - sample; 

        return sample;
}
