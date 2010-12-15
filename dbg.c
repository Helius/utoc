#include <stdio.h>
#include "dbg.h"

void err_msg (const char * filename, const char * func, const int line)
{
	printf ("*** !!!!! ERROR !!!!!! %s (%s) [%d]: ", filename, func, line);
}

void dbg_msg (const char * filename, const char * func, const int line)
{
	printf ("%s (%s) [%d]: ", filename, func, line);
}
