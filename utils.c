#include "utils.h"
#include <sys/param.h>
#include <sys/sysctl.h>

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
