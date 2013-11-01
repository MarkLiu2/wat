#include <math.h>
#include <stdlib.h>
#define PI	M_PI
#define TWOPI	(2.0*PI)

#include "fft.h"
#include "fft_original.c"
#include "fft_fission.c"

void pick_fft(double data[], int nn, int isign)
{
#ifdef FISSION
#ifdef FISSION_V1
        fission_fft(data, nn, isign);
#endif

#ifdef FISSION_V2
        fission_fft_v2(data, nn, isign);
#endif

#else
        original_fft(data, nn, isign);
#endif
}
