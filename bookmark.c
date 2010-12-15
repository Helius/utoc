#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utoc.h"
#include "dbg.h"

#define _BKMK_DEBUG 0

#if _BKMK_DEBUG
#define DBG(...) DBG_MSG(__VA_ARGS__)
#else
#define DBG(...) ;
#endif

//*****************************************************************************
//
static int get_hash (char * str, char * hash)
{
	FILE * f;
	const int STRSIZE = 256;
	char cmd [STRSIZE];
	sprintf (cmd, "echo %s | md5sum", str);
	DBG ("cmd: [%s]\n", cmd);

		f = popen( cmd, "r" );
		if ( f == 0 ) {
				fprintf( stderr, "Could not execute\n" );
				return 1;
		}
		const int BUFSIZE = 256;
		char buf[ BUFSIZE ];
		if (fgets (buf, BUFSIZE, f)) {
			strncpy (hash, buf, 32);
			pclose (f);
			return 0;
		}
		pclose (f);
		return 1;
}



//*****************************************************************************
//
int load_bookmark (s_utoc * utoc)
{
	FILE * f;
	char * homepath;
	char * fname;
	char str [_BKMK_RECORD_LEN];
	int i = 0;

	homepath = getenv("HOME");
	fname = malloc (sizeof (char) * (strlen (homepath) + strlen (_BKMK_NAME)));
	strcpy (fname, homepath);
	strcat (fname, _BKMK_NAME);

	f = fopen (fname, "r");
	if (f == NULL) {
		DBG ("Can't open bookmark file [%s]\n", fname);
		return 1;
	} else {
		// found saved record for 'filename'
		while (fgets (str, _BKMK_RECORD_LEN, f) != 0 ) {
			i++;
		}
		DBG ("bookmark count %d, malloc %d bytes\n", i, i * sizeof (hist_rec));
		utoc->hist_mem = malloc (i * sizeof (hist_rec)); // +1 for last byte will be '\0' to avoid problem with string function
		utoc->hist_size = i * sizeof (hist_rec);
	
		rewind (f); //set file stream position indicator to begin

		memset (utoc->hist_mem, ' ', utoc->hist_size);
		memset (str, ' ', _BKMK_RECORD_LEN);

		i = 0;
		while (fgets (str, _BKMK_RECORD_LEN, f) != 0 ) {
		//	DBG ("read from file, write to %d: %s", ((sizeof (hist_rec)*i)) ,str);
			memcpy (utoc->hist_mem + ((sizeof (hist_rec)*i)), str, strlen (str));
			i++;
		}
	}

	free (fname);
	fclose (f);

#if _BKMK_DEBUG == 1
	int k,j;
	printf ("\n-----------\nmemory:\n");
	for (j = 0; j < i*sizeof (hist_rec); j++) {
		printf ("%c", *(utoc->hist_mem + j));
	}
	printf ("\n-----------\nmemory:\n");
	for (j = 0; j < utoc->hist_size + 1; j++) {
		printf ("%d ", *(utoc->hist_mem + j));
	}
	printf ("\n");
#endif

	return 0;
}

//*****************************************************************************
//
int find_bookmark (s_utoc * utoc)
{
	char hash [33];
	char pos [8];
	utoc->hist_ind = NULL;
	if (get_hash (utoc->filename, hash)) {
		DBG ("ERROR: calc hash error\n");
		return 0;
	}
	hash[32] = 0;
	DBG ("utoc.hist_mem %d\n", strlen (utoc->hist_mem));
	DBG ("find substring [%s]\n", hash);
	char * sbstr = strstr (utoc->hist_mem, hash);
	DBG ("sbstr: [%s]\n", sbstr);
	if ((sbstr == NULL) || (strlen (sbstr) < sizeof (hist_rec))) {
		DBG ("No bookmark found\n");
		return 0;
	}
	utoc->hist_ind = sbstr;
	get_dig (pos, sbstr + 33, 0);
	DBG ("pos found: [%s] %d\n", pos, atoi(pos));
	int ret = atoi (pos);
	if (ret > 0) {
		utoc->start_position = ret;
	}
	return 0;
}


//*****************************************************************************
//
int save_bookmark (s_utoc * utoc)
{
	
	char * homepath;
	char * fname;
	char str [_BKMK_RECORD_LEN];
	int i = 0;

	homepath = getenv("HOME");
	fname = malloc (sizeof (char) * (strlen (homepath) + strlen (_BKMK_NAME)));
	strcpy (fname, homepath);
	strcat (fname, _BKMK_NAME);

	if (utoc->hist_ind != NULL)	{
		FILE * f;
		// file history exist, update history record and write memory to file
		int len = sprintf (str, "%d;", utoc->start_position + utoc->char_cnt);
		memcpy ((utoc->hist_ind + 33), str, len);
		memset ((utoc->hist_ind + 33 + len), ' ', 33+8-len);
		*(utoc->hist_ind + 33 + 7) = '\n';
		
#if _BKMK_DEBUG == 1
		printf ("\nrecord: ");
		for (i = 0; i < 33 + 8; i++)
			printf ("%c", *(utoc->hist_ind + i));
		printf ("\nrecord: ");
		for (i = 0; i < 33 + 8; i++)
			printf ("%d ", *(utoc->hist_ind + i));
		printf ("\n\n");
#endif

		f = fopen (fname, "w+");
		if (f == NULL) {
			printf ("Can't open bookmark file [%s]\n", fname);
			return 1;
		}
		DBG ("update file\n");
		int wr = fwrite (utoc->hist_mem, sizeof(char), utoc->hist_size, f);
		DBG ("file wrote %d\n", wr);
	} else {
		FILE * f;
		f = fopen (fname, "a+");
		if (f == NULL) {
			printf ("Can't open bookmark file [%s]\n", fname);
			return 1;
		}
		char hash [33];
		// new file, not in history, just add 'hist_rec' at end of history file
		get_hash (utoc->filename, hash);
		hash [32] = '\0';
		int len = sprintf (str, "%s %d;", hash, utoc->char_cnt + utoc->start_position);
		memset ((str +  len),' ', 33+8-len);
		*(str + 33 + 7) = '\n';
		*(str + 33 + 8) = '\0';
		DBG ("New rec is [%s]\n", str);
		fputs (str, f);
		fclose (f);
	}
	free (fname);
	return 0;
}

