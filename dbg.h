#ifndef __DBG_H__
#define __DBG_H__


void err_msg (const char * filename, const char * func, const int line);
void dbg_msg (const char * filename, const char * func, const int line);

#define ERR(...) err_msg (__FILE__, __func__, __LINE__); printf(__VA_ARGS__);  printf ("\n");

#define DBG_MSG(...) dbg_msg (__FILE__, __func__, __LINE__); printf(__VA_ARGS__); printf ("\n");

#endif
