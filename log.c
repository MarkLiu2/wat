#include "log.h"

static int log_level = LOG_INFO;

void set_log_level(int arg)
{
        log_level = arg;
}

int get_log_level()
{
        return log_level;
}

void wat_log(int level, char *msg)
{
        if(get_log_level() >= level){
                printf("%s", msg);
        }
}
