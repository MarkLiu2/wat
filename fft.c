#include <math.h>
#include <stdlib.h>
#define PI	M_PI
#define TWOPI	(2.0*PI)

#include "fft.h"
#include "fft_original.c"

/* pick_fft isn't necessary anymore, was used during the optimization.
 * will stay here, just in case 
 */
uint32_t pick_fft(double data[], int nn, int isign)
{
        return original_fft(data, nn, isign);
}
