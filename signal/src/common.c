
#include "common.h"

void SigHandler(int signum)
{
    SIG_LAST_NUM = signum;
}
