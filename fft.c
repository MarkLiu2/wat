#include "fft.h"
#include <math.h>
#include <stdlib.h>
#define PI	M_PI
#define TWOPI	(2.0*PI)

#ifdef HAVE_THREADS
#include <pthread.h>

long nb_thread_f = 2;
#endif


typedef struct fft{
        int n, mmax, m, j, istep, i;
        int nn;
        double wtemp, wr, wpr, wpi, wi, theta;
        double tempr, tempi;
        double * data;
        int isign;
} fft;

void * four1_t(void *arg)
{

        fft *f = (fft *)arg;

        int n = f->n; 
        int nn = f->nn; 

        int mmax = f->mmax; 
        int m; 
        /* i e j iteradores */
        int j; 
        int i;

        int istep; 
        double wtemp; 
        double wr; 
        double wpr; 
        double wpi; 
        double wi; 
        double theta;

        /* auxiliares para real e imag */
        double tempr; 
        double tempi;

        double *data = f->data;;
        int isign = f->isign;

        while (n > mmax) {
                istep = 2*mmax;
                theta = TWOPI/(isign*mmax);
                wtemp = sin(0.5*theta);
                wpr = -2.0*wtemp*wtemp;
                wpi = sin(theta);
                wr = 1.0;
                wi = 0.0;
                for (m = 1; m < mmax; m += 2) {
                        for (i = m; i <= nn; i += istep) {
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
        return NULL;
}
        /*
#ifdef HAVE_THREADS
        pthread_t *handle_t;
        
        handle_t = (pthread_t *)malloc(nb_thread_f * sizeof(pthread_t));

        fft * f = (fft *)malloc(sizeof(fft));
        f->mmax = 2;
        f->data = data;
        f->isign = isign;
        f->nn = n;
        f->n = n/2;


        pthread_create(&handle_t[0], NULL, four1_t, f);
        f->n = n;
        f->mmax = n/2;
        pthread_create(&handle_t[1], NULL, four1_t, f);
        pthread_join(handle_t[0], NULL);
        pthread_join(handle_t[1], NULL);
        
#else 
        */
 
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
//#endif
}



void four0(double data[], int nn, int isign)
{
        /* dobro de NFFT */
        int n; 

        int mmax; 
        int m; 
        /* i e j iteradores */
        int j; 
        int i;

        int istep; 
        double wtemp; 
        double wr; 
        double wpr; 
        double wpi; 
        double wi; 
        double theta;

        /* auxiliares para real e imag */
        double tempr; 
        double tempi;

        /* n recebe o dobro de nn (tamanho de NFFT) */
        n = nn << 1; 
        /* j iniciado em 1 */
        j = 1;
        /* for de 1 a 2 * NFFT de 2 em 2 */
        for (i = 1; i < n; i += 2) {
                /* j iniciado em 1 entao nao entra na primeira iteracao */
                if (j > i) {
                        /* em i = 3 e j = NFFT + 1 (it 2) */
                        /* troca a posicao data[3] por data[NFFT + 1] */
                        tempr = data[j];     
                        data[j] = data[i];     
                        data[i] = tempr;

                        /* troca a posicao data[4] por data[NFFT + 2] */
                        tempr = data[j+1]; 
                        data[j+1] = data[i+1]; 
                        data[i+1] = tempr;
                }
                /* m recebe metade de n */
                /* m sempre sera NFFT neste ponto */
                m = n >> 1;

                /* enquanto m maior ou igual a 2 */
                /* enquando j maior que m */

//                if(i < 100)
//                printf("\nwhile m = %d\t j = %d",m, j);
                while (m >= 2 && j > m) {
                        j -= m;
                        m >>= 1;

//                        if(i < 100)
//                                printf("\nnew m = %d\t j = %d",m, j);
                }
                /* j recebe m logo NFFT + 1*/
                j += m;
//                if(i < 100)
//                printf("\nj = %d", j);
        }
        mmax = 2;
        while (n/2 > mmax) {
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
 
                }

                        wr = (wtemp = wr)*wpr - wi*wpi + wr;
                        wi = wi*wpr + wtemp*wpi + wi;
                mmax = istep;
        }

        mmax = n/2;
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
 
                }

                        wr = (wtemp = wr)*wpr - wi*wpi + wr;
                        wi = wi*wpr + wtemp*wpi + wi;
                mmax = istep;
        }

}


