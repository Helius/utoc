#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>   
#include <stdlib.h>
#include <ctype.h>
#include "utoc.h"
#include "bookmark.h"

#define _MAIN_DEBUG 0

#if _MAIN_DEBUG
#define DBG(...) DBG_MSG(__VA_ARGS__)
#else
#define DBG(...) ;
#endif

s_utoc utoc;

//*****************************************************************************
void print_usage (char * name) 
{
	printf ("%s - Unicod Type On Console by Helius (ghelius@gmail.com)\n", name);
	printf ("Simple console keyboard type trainer\n");
	printf ("Usage:\n");
	printf ("\t%s [-t] txtfile\n\n", name);
	printf ("\t -t [1..60]\ttime for training in minuts, after which application will exit\n", name);
	printf ("\t -s --stat \tptint train history and statictics (from ~/.utoc file)\n", name);
	printf ("\t -h --help \tthis message\n", name);
}

//*****************************************************************************
//
void start_timer (void)
{
	static int first_call = true;
	if (first_call) {
		time (&utoc.start_time);
		first_call = false;
	}
}

//*****************************************************************************
// get char user pressed, no waiting Enter input
char get_char (void)
{
	struct termios oldt, newt;
	int ch;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}

//*****************************************************************************
// delete str
void delete_str (void) 
{
	int len = strlen (utoc.str);
	putchar (0x08);
	if (len) {
		putchar (0x20);
		while (len--){
			putchar (8); 
		}
	}
}

//*****************************************************************************
//
void update_str (int err)
{
	int i;
	if (err) {
		printf ("\033[0;31m");
	} else {
		printf ("\033[0;32m");
	}
	for (i = 0; i < utoc.cursor; i++) 
		putchar (utoc.str[i]);
	printf ("\033[0m");
	printf ("%s", &(utoc.str [utoc.cursor]));
}

//*****************************************************************************
// remove or replace all non text symbols, and remove more then one space and
// new line symbol
int prepare_srt (char * str)
{
	//DBG ("input: [%s]\n", str);
	int i;
	
	while ((str[0] == ' ') || (str[0] == '\n')) {
		strcpy (&str[0], &str[1]);
		if (strlen (str) == 0)
			break;
	}

	i = 0;
	while (strlen (str) && (i != strlen (str))) {
		if ( (str[i] == '\n') ) {
			strcpy (&str[i], &str[i+1]);
		} else {
			i++;
		}
	}
	
	i = 0;
	while (strlen (str) && (i != strlen (str))) {
		if ( (str[i] == ' ') && (str[i+1] == ' ') ) {
			strcpy (&str[i], &str[i+1]);
		} else {
			i++;
		}
	}
}

//*****************************************************************************
// recursive find all digit chars starts from 'ch' and copy it to 'arr' 
// in right order 
int get_dig (char * arr, char * ch, int i)
{
	if (isdigit (*ch)) {
		i++;
		get_dig (arr, ++ch, i);
		arr [i-1] = *(--ch);
	} else {
		arr [i--] = '\0';
	}
}

//*****************************************************************************
// find and obtain digits value from string
void parse_stat_line (char * str, int * ch, int * sp, int * err)
{
	char tmp[8];
	char * sbstr = strstr(str, "chars");
	get_dig (tmp, sbstr + 6, 0);
	*ch = atoi (tmp);	
	
	sbstr = strstr(str, "speed");
	get_dig (tmp, sbstr + 6, 0);
	*sp = atoi (tmp);	

	sbstr = strstr(str, "err");
	get_dig (tmp, sbstr + 4, 0);
	*err = atoi (tmp);	
}

//*****************************************************************************
// print ctatistics value for all saved trains, ptint also pseudo-graphics bar
int show_all_statictic (void)
{
	#define _STR_LEN 120
  FILE * f;
	char * homepath;
	char * filename;
	char str [_STR_LEN];
	int allchar = 0;

	homepath = getenv("HOME");
	filename = malloc (sizeof (char) * (strlen (homepath) + strlen (_HIST_NAME)));
	strcpy (filename, homepath);
	strcat (filename, _HIST_NAME);

	f = fopen (filename, "r");
	free (filename);
	if (f == NULL) {
		DBG ("Can't open file [%s]\n", filename);
		return 1;
	}
	printf ("You statistic:\n");
	while (fgets (str, _STR_LEN, f) != NULL) {
		int chnmb, speed, err, i;
		parse_stat_line (str, &chnmb, &speed, &err);
		allchar += chnmb;
		printf ("chars [%d], speed [%d], err [%d]\t", chnmb, speed, err);
		for (i = 0; i < speed/10; i++)
			printf ("#");
		for (i = 0; i < err/10; i++)
			printf ("-");
		printf ("\n");
	}
	printf ("All You chars typed is %d!\n", allchar);
	
	return 0;
}



//*****************************************************************************
//
int parse_cmdline (int argc, char ** argv)
{
	utoc.filename = NULL; 
	if ((argc < _MIN_ARG_CNT) || (argc > _MAX_ARG_CNT)) {
		printf ("arg count error\n");
		print_usage (argv[0]);
		return false;
	}
	int i = 1;
	
	while (i < argc) {
		if ((strcmp (argv[i], "-h") == 0) || (strcmp (argv[i], "--help") == 0)) {
			print_usage (argv[0]);
			return false;
		}	else if (strcmp (argv[i], "-t") == 0) {
			i++;
			utoc.timeout_m = atoi (argv[i]);
			if (!((utoc.timeout_m > 0) && (utoc.timeout_m < 61))) {
				printf ("ERROR: time must be from 1 to 60 min\n");
				return false;
			}
		}	else if ((strcmp (argv[i], "-s") == 0) || (strcmp (argv[i], "--stat") == 0)) {
			show_all_statictic ();
			return false;
		} else {
			if (utoc.filename != NULL) {
				printf ("ERROR: bad option [%s], try --help for usage\n", argv[i]);
				return false;
			}
			int len = strlen (argv[i]);
			utoc.filename = malloc (sizeof(char) * (len + 1));
			strcpy (utoc.filename, argv[i]);
		}
		i++;
	}

	return true;
}

//*****************************************************************************
//
void calc_statistic (void)
{
	time_t cur;
	time (&cur);
	utoc.speed = (unsigned int)(utoc.char_cnt * 60) / (unsigned int)(cur - utoc.start_time);
	utoc.pererr = (100*utoc.err_char_cnt)/utoc.char_cnt;
}
//*****************************************************************************
//
void print_statistic (void)
{
	delete_str ();
	printf ("\nRelax! You're doing fine!\n");
	printf ("You speed %d, error %d %%\n", utoc.speed, utoc.pererr);
}
//*****************************************************************************
//
int time_is_over (int m_time) 
{
	time_t cur;
	time (&cur);
	if ((cur - utoc.start_time) > m_time * 1/*60*/)
		return true;
	return false;
}
//*****************************************************************************
//
int save_statistic (void)
{
  FILE * f;
	char * homepath;
	char * filename;
	char str [60];

	homepath = getenv("HOME");
	filename = malloc (sizeof (char) * (strlen (homepath) + strlen (_HIST_NAME)));
	strcpy (filename, homepath);
	strcat (filename, _HIST_NAME);

	f = fopen (filename, "a+");
	if (f == NULL) {
		DBG ("Can't open history file [%s]\n", filename);
	} else {
		sprintf (str, "chars[%d] speed[%d] err[%d]\n",utoc.char_cnt, utoc.speed, utoc.pererr);
		fwrite (str, sizeof (char), strlen (str), f);
		fclose (f);
	}
	free (filename);
	return 0;
}

//*****************************************************************************
//
int main (int argc, char ** argv)
{
  FILE * fl, fb;
	int err, len;
	int pos = 0;

	utoc.filename = NULL;
	utoc.start_position = 0;

	if (!parse_cmdline (argc, argv)) {
		return 1;
	}

	if (utoc.filename == NULL) {
		printf ("ERROR: missing file name\n");
		return 0;
	}

	if (load_bookmark (&utoc)) {
		printf ("can't load bookmark\n");
	} else {
		find_bookmark (&utoc);
		DBG ("found pos from bookmark: %d\n", utoc.start_position);
	}

// open text file
	fl = fopen (utoc.filename, "r");
	if (fl == NULL) {
		printf ("Can't open file %s\n", utoc.filename);
		return 1;
	}
	fseek (fl, utoc.start_position, SEEK_SET);

	len = fread (utoc.str, sizeof (char), _STR_LEN-2, fl);
	utoc.str [len] = '\0';
	if (!strlen (utoc.str)) {
		printf ("Can't read file\n");
		fclose (fl);
		return 1;
	}

	prepare_srt (utoc.str);

	utoc.cursor = 0;
	printf ("Just type!\n"); 

	while (1) {
		update_str (err);
		err = false;
		char ch = get_char ();
		start_timer ();
		if (ch == utoc.str[utoc.cursor]) {
			utoc.cursor++;
			utoc.char_cnt++;
		} else {
			utoc.err_char_cnt++;
			err = true;
		}
		delete_str ();
		if (utoc.cursor > strlen (utoc.str) - 1) {
			if (time_is_over (utoc.timeout_m)) {
				calc_statistic ();
				print_statistic ();
				save_statistic ();
				save_bookmark (&utoc);
				break;
			}
			len = fread (utoc.str, sizeof (char), _STR_LEN-2, fl);
			utoc.str [len] = '\0';
			if (!strlen (utoc.str)) {
				printf ("Can't read text\n");
				break;
			}
			prepare_srt (utoc.str);
			utoc.cursor = 0;
		}
	}
	DBG ("app exit\n");
	//fclose (fl);
}
 
