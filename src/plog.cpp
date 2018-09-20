#include <stdio.h>
#include <string.h>

#include "plog.h"



const char* get_src_name(const char* fpath)
{
    const char* pos = strrchr(fpath, '/');
    if(pos) return pos+1;
    return fpath;
}


