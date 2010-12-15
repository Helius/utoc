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
#define _BKMK_NAME_1 "/.utoc_bkmk_1"

typedef struct {
	char * filename;
	char str [_STR_LEN];
	time_t start_time;
	int timeout_m;						//user time out for train
	int cursor;
	int char_cnt;
	int err_char_cnt;
	int speed;
	int pererr;
	int start_position;
	char * hist_mem;					
	int hist_size;		
	char * hist_ind;							//if record exist, it point to record, everwise NULL;
} s_utoc;


typedef struct {
	char hash [33]; // 32 char of hash + one whidechar
	char pos [8];
} hist_rec;

#endif

