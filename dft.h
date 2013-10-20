#ifndef DFT_H
#define DFT_H

#include "log.h"

int dft(long int, double *, double *);

int inverse_dft(long int, double *, double *);

void four1(double data[], int nn, int isign);

#endif
