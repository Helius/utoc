#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utoc.h"


//*****************************************************************************
//
static int get_hash (char * str, char * hash)
{
	FILE * f;
	const int STRSIZE = 256;
	char cmd [STRSIZE];
	sprintf (cmd, "echo %s | md5sum", str);
	printf ("cmd: [%s]\n", cmd);

		f = popen( cmd, "r" );
		if ( f == 0 ) {
				fprintf( stderr, "Could not execute\n" );
				return 1;
		}
		const int BUFSIZE = 256;
		char buf[ BUFSIZE ];
		if (fgets (buf, BUFSIZE, f)) {
			strncpy (hash, buf, 32);
//			printf ("hash: %s\n", hash);
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
		printf ("Can't open bookmark file [%s]\n", fname);
		return 1;
	} else {
		// found saved record for 'filename'
		while (fgets (str, _BKMK_RECORD_LEN, f) != 0 ) {
			i++;
		}
		printf ("bookmark count %d, malloc %d bytes\n", i, i * sizeof (hist_rec));
		utoc->hist_mem = malloc (i * sizeof (hist_rec) + 1); // +1 for last byte will be '\0' to avoid problem with string function
		utoc->hist_size = i * sizeof (hist_rec);
	
		rewind (f); //set file stream position indicator to begin

		memset (utoc->hist_mem, ' ', utoc->hist_size);
		memset (str, ' ', _BKMK_RECORD_LEN);

		i = 0;
		while (fgets (str, _BKMK_RECORD_LEN, f) != 0 ) {
			printf ("read from file, write to %d: %s", ((sizeof (hist_rec)*i)) ,str);
			memcpy (utoc->hist_mem + ((sizeof (hist_rec)*i)), str, strlen (str));
			i++;
		}
		utoc->hist_mem [utoc->hist_size] = '\0';
		fclose (f);
	}

	free (fname);
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

	return 0;
}

//*****************************************************************************
//
int find_bookmark (s_utoc * utoc)
{
	char hash [33];
	char pos [8];
	if (get_hash (utoc->filename, hash)) {
		printf ("ERROR: calc hash error\n");
		return 0;
	}
	hash[32] = 0;
	printf ("utoc.hist_mem %d\n", strlen (utoc->hist_mem));
	printf ("find substring [%s]\n", hash);
	char * sbstr = strstr (utoc->hist_mem, hash);
	printf ("sbstr: [%s]\n", sbstr);
	if ((sbstr == NULL) || (strlen (sbstr) < sizeof (hist_rec))) {
		printf ("ERROR: while find bookmark, substring too short\n");
		return 0;
	}
	get_dig (pos, sbstr + 33, 0);
	printf ("pos found: [%s] %d\n", pos, atoi(pos));
	int ret = atoi (pos);
	if (ret > 0) {
		return ret;
	}
	return 0;
}
