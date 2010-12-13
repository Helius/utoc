#ifndef _UTOC_H_
#define _UTOC_H_

#define true	1
#define false 0 

#define _STR_LEN 120

#define _MIN_ARG_CNT 2
#define _MAX_ARG_CNT 10
#define _BKMK_RECORD_LEN 256

#define _HIST_NAME "/.utoc"
#define _BKMK_NAME "/.utoc_bkmk"

typedef struct {
	char * filename;
	char str [_STR_LEN];
	time_t start_time;
	int timeout_m;
	int cursor;
	int char_cnt;
	int err_char_cnt;
	int speed;
	int pererr;
} s_utoc;

#endif

