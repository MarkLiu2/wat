#include <math.h>
#include <stdlib.h>
#define PI	M_PI
#define TWOPI	(2.0*PI)

#include "fft.h"
#include "fft_original.c"
#include "fft_fission.c"

uint32_t pick_fft(double data[], int nn, int isign)
{
        /*
#if defined(FISSION) || defined(OPT)
        return fission_fft(data, nn, isign);
#endif

#if defined(ORIG) || defined(ORIG_B)
        return original_fft(data, nn, isign);
#endif
*/
        return original_fft(data, nn, isign);
        return -1;
}
