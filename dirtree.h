/*
 * Copyright (c) 2008-2012 l00g33k@gmail.com All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <time.h>

#if defined(__TURBOC__)
#include <io.h>
#include <dir.h>
#include <alloc.h>
#define malloc farmalloc
#define realloc farrealloc
#elif defined(_MSC_VER)
#include <io.h>
#include <direct.h>
#include <malloc.h>
#elif defined(__GNUC__)
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <malloc.h>
#else
#endif


#if defined (__TURBOC__)
#define HUGE huge
#define PATHLEN 256
#elif defined (_MSC_VER)
#define HUGE
#define PATHLEN 1024
#else
#define HUGE
#define PATHLEN 1024
#endif

#ifdef __GNUC__
#define DIRSEPARATOR '/'
#define DIRSEPARATORS "/"
#define DIRSEPARATOR2 '\\'
#define DIRSEPARATORS2 "\\"
#else
#define DIRSEPARATOR '\\'
#define DIRSEPARATORS "\\"
#define DIRSEPARATOR2 '/'
#define DIRSEPARATORS2 "/"
#endif


#define USE_STRLWR 0
#define MDSMSG 0


#define FNAMEALLOCSZ 50000
#define DNAMEALLOCSZ 50000
#define DIRALLOCSZ    3000

#define CMD_SAME         1
#define CMD_DIFF         2
#define CMD_THIS_ONLY    4
#define CMD_THAT_ONLY    8
#define CMD_THIS_NEW    16
#define CMD_THAT_NEW    32
#define CMD_ALL         63
#define CMD_THIS_MOVE   64
#define CMD_THAT_MOVE  128
#define CMD_THIS_DUP   256
#define CMD_THAT_DUP   512
#define CMD_ALL_DUP   1023
#define CMD_MD        1024
#define CMD_RD        2048


#define SORTBYDIR 0
/*  sort order: file, size, crc, date/time, dirname */
#define SORTBYDUP 1
#define SORTBYDUPNONM 2

#define EQUALMASKSIZE   1
#define EQUALMASKCRC    2
#define EQUALMASKTIME   4


#define OUTPUTTXT 0
#define OUTPUTCSV 1
#define OUTPUTCPY 2

#define TREESIZESORTBYDIRSIZE 0
#define TREESIZESORTBYSUBSIZE 1
#define TREESIZESORTBYPAHT    2


#define NOEXC 30


#ifdef __GNUC__
int stricmp (char *s1, char *s2);
#define max(a,b) ((a)>(b)?(a):(b))
#endif


struct dirlist {
    time_t tt;
    unsigned long int size, crc, fnameidx, dirid;
};
struct dirsize_ {
    char HUGE *dirname;
    int dirlevel;
    unsigned long int dirthisgbytes, dirthisbytes, dirsubgbytes, dirsubbytes, nofiles;
};


extern int debug, qsortidx0, qsortidx1;
extern int recurlvl, maxrecurlvl, printdirtree, file, calccrc,
    verbose, homepathlen, nodate, noexclude, noexcludedir, printmove,
    filter, recurlvllmt, exclextinvt, excldirinvt, ignorecase;
extern long int nofiles, totalbytes, totalGbytes,
    NTtimediff, nooutput;

extern char qoute, homepath [2] [2048], datafile [256], datafile2 [256],
    *padding, *exclude [NOEXC], *excludedir [NOEXC];


extern unsigned long nodirs [2],
fnamesz [2], fnameptr [2],
dnamesz [2], dnameptr [2],
  dirsz [2],   dirptr [2];
extern char HUGE *fnamebuf [2], HUGE *dnamebuf [2];
extern struct dirlist HUGE *dirbuf [2];


extern char procfilebuf [2048];


extern int (*dirtstrcmp) (char *, char*);

/*
void output (
    int condition,
    int cmd,
    int csv,
    char *dname,
    char *fname,
    struct dirlist *d0,
    struct dirlist *d1
);
*/
int
exclude_ext (char *fname);
void bat (void);
void diff2 (int cmd, char *bdir0, char *bdir1, int outputfmt);
void diff2mv (int cmd, char *bdir0, char *bdir1, int outputfmt);
unsigned long procdir (int dbidx, char HUGE *path);
int procfile (
    int dbidx,
    unsigned long diridx,
    char HUGE *path,
    char HUGE *name,
    unsigned long size,
    time_t time_write
);
int dtscandir (int dbidx, char HUGE *path);
void report (unsigned long int nodir, unsigned long int nofil);
int retrieveit (char *fname, int dbidx, char *basedir);
void saveit (char *fname, int dbidx, char *basedir);
void mysort (
    char HUGE *base,
    long int noelem,
    int size,
    int (*dircmp) (const void *a, const void *b)
);
void printit (int dbidx, int csv);
void treesize (int dbidx, char *homepath, char treesizemode, int csv);
char *numcomma (unsigned long int GBytes, unsigned long int bytes);
void gadd (
    unsigned long int gbytes,
    unsigned long int bytes,
    unsigned long int *tgbytes,
    unsigned long int *tbytes
);
void dirsort (int dbidx, int sorttype);

void treesizesort (struct dirsize_ *dirsize, unsigned int nodir, int sortby);

char *xctime (time_t *t);


int qsortdircmp (const void *a, const void *b);
int qsortdupcmp (const void *a, const void *b);
int qsortdirsizecmp (const void *a, const void *b);
int qsortttldirsizecmp (const void *a, const void *b);
int qsortpathdirsizecmp (const void *a, const void *b);

int exclude_dir (char *dname);
int exclude_ext (char *fname);

/*  in crc32.c */
unsigned long int crc32file (char *fname);

void outputcpy (
    int condition,
    int cmd,
    char *dname,
    char *fname,
    char *bdir0,
    char *bdir1,
    struct dirlist *d0,
    struct dirlist *d1 ,
    int cnt
);
void outputtxt (
    int condition,
    int cmd,
    char *dname,
    char *fname,
    char *bdir0,
    char *bdir1,
    struct dirlist *d0,
    struct dirlist *d1,
    int cnt
);
void outputcsv (
    int condition,
    int cmd,
    char *dname,
    char *fname,
    char *bdir0,
    char *bdir1,
    struct dirlist *d0,
    struct dirlist *d1,
    int cnt
);

