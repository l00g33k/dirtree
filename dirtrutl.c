/*
 * Copyright (c) 2008-2012 l00g33k@gmail.com All rights reserved.
 *
 */

#include "dirtree.h"


#ifdef __GNUC__
char *strlwr (char *p) {
    char *p2;
    p2 = p;
    while (*p2) {
        *p2 = tolower (*p2);
        p2++;
    }
    return (p);

} /* strlwr */

int stricmp (char *s1, char *s2) {
    char c1, c2;

    do {
        c1 = tolower (*s1);
        c2 = tolower (*s2);
        s1++;
        s2++;
    } while ((c1 == c2) && (c1 != 0));

    return (c1 - c2);

} /* stricmp */

#endif

#if defined(__BORLANDC__) || defined(__TURBOC__)
struct myffblk {
    char          ff_reserved [21];
    char          attrib;
    time_t   time_write;      /*  combine date/time into ftime */
    unsigned long size;
    char          name [13];
};
#endif

#if defined(__TURBOC__)
time_t mktime (const struct tm *tm);
#endif

#if defined(__BORLANDC__) || defined(__TURBOC__)
time_t ftime2tt (struct ftime *ft) {
/*****************************************************************************
 *  Converts from dir's struct ftime to ctime's time_t
 *****************************************************************************/
    struct tm tt;

    tt.tm_sec  = ft->ft_tsec * 2;
    tt.tm_min  = ft->ft_min;
    tt.tm_hour = ft->ft_hour;
    tt.tm_mday = ft->ft_day;
    tt.tm_mon  = ft->ft_month - 1;
    tt.tm_year = ft->ft_year + 80;
    tt.tm_isdst = 0;
    return (mktime (&tt));
}

#endif

char *xctime (time_t *t) {
    static char buf [64];
    char tbuf [64];
    struct tm *tt;

    strcpy (tbuf, ctime (t));
    tt = localtime (t);

    sprintf (buf, "%4d/%02d/%02d %2d:%02d:%02d %c%c%c",
        tt->tm_year + 1900,
        tt->tm_mon  + 1,
        tt->tm_mday ,
        tt->tm_hour ,
        tt->tm_min  ,
        tt->tm_sec  ,
        tbuf [0],  tbuf [1],  tbuf [2]);

    return (buf);

} /* xctime */

int exclude_ext (char *fname) {
/*****************************************************************************
 *
 *  Compare extension against filters.
 *
 *  Returns 1 if matched, 0 otherwise
 *
 *****************************************************************************/
    char *p;
    int i;

    p = strrchr (fname, '.');
    if (p == NULL) {
        /*  no match */
        return (0);
    }
    p++;


    for (i = 0; i < noexclude; i++) {
        if (dirtstrcmp (p, exclude [i]) == 0) {
            /*  match */
            return (exclextinvt ? 0 : 1);
        }
    }

    /*  no match */
    return (exclextinvt ? 1 : 0);
} /* exclude_ext */


int exclude_dir (char *dname) {
/*****************************************************************************
 *
 *  Compare sub-directory against filters.
 *
 *  Returns 1 if matched, 0 otherwise
 *
 *****************************************************************************/
    char *p, buf [256];
    int i, len;

    for (i = 0; i < noexcludedir; i++) {
        if (*excludedir [i] == DIRSEPARATOR) {
            len = strlen (excludedir [i] + 1);
            if (ignorecase != 0) {
                if (strncmp (strlwr (dname), excludedir [i] + 1, len) == 0) {
                    /*  1st level dir */
                    return (excldirinvt ? 0 : 1);
                }
            } else {
                if (strncmp (dname, excludedir [i] + 1, len) == 0) {
                    /*  1st level dir */
                    return (excldirinvt ? 0 : 1);
                }
            }
        }
    }

    for (i = 0; i < noexcludedir; i++) {
        if (*excludedir [i] != DIRSEPARATOR) {
            strcpy (buf, DIRSEPARATORS);
            strcat (buf, excludedir [i]);
            strcat (buf, DIRSEPARATORS);
            len = strlen (excludedir [i]);
            if (ignorecase != 0) {
                if (strncmp (strlwr (dname), buf + 1, len + 1) == 0) {
                    /*  1st level dir */
                    return (excldirinvt ? 0 : 1);
                }
                if (strstr (strlwr (dname), buf) != NULL) {
                    return (excldirinvt ? 0 : 1);
                }
            } else {
                if (strncmp (dname, buf + 1, len + 1) == 0) {
                    /*  1st level dir */
                    return (excldirinvt ? 0 : 1);
                }
                if (strstr (dname, buf) != NULL) {
                    return (excldirinvt ? 0 : 1);
                }
            }
        }
    }

    /*  no match */
    return (excldirinvt ? 1 : 0);
} /* exclude_dir */


void bat (void) {
/*****************************************************************************
 *
 *  Prints extension batch file for difference comparison
 *
 *****************************************************************************/
    FILE *st;



    st = fopen ("xds.bat", "wt");

    fprintf (st, "rem xds.bat, dir size sub-total; in directory : xds\n");
    fprintf (st, "\n");
    fprintf (st, "if \"%%DB_DEL%%\"==\"\" SET DB_DEL=f:\\a\\del\\del\\del\\\n");
    fprintf (st, "if \"%%DB_EDT%%\"==\"\" SET DB_EDT=notepad\n");
    fprintf (st, "rem echo DB_DEL = \"%%DB_DEL%%\"\n");
    fprintf (st, "dirtree -s%%DB_DEL%%db_this.dir -p %%1 %%2\n");
    fprintf (st, "\n");
    fprintf (st, "del %%DB_DEL%%db_del.out\n");
    fprintf (st, "\n");
    fprintf (st, "echo *-*-*-*>> %%DB_DEL%%db_del.out\n");
    fprintf (st, "dirtree -r%%DB_DEL%%db_this.dir -p -T   >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "echo . >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "echo --------------------------------------------------------------- >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "echo --------------------------------------------------------------- >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "echo *-*-*-*>> %%DB_DEL%%db_del.out\n");
    fprintf (st, "echo --------------------------------------------------------------- >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "echo --------------------------------------------------------------- >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "echo . >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "dirtree -r%%DB_DEL%%db_this.dir -p -TT >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "echo %%DB_DEL%%db_del.out\n");
    fprintf (st, "%%DB_EDT%% %%DB_DEL%%db_del.out\n");
    fprintf (st, "pause\n");
    fprintf (st, "del %%DB_DEL%%db_del.out\n");
    fprintf (st, "del %%DB_DEL%%db_this.dir\n");
    fprintf (st, ":end\n");



    fclose (st);

    st = fopen ("xdc.bat", "wt");


    fprintf (st, "rem xdc.bat, compare directories; in this : xdc f:.\n");
    fprintf (st, "\n");
    fprintf (st, "if \"%%DB_DEL%%\"==\"\" SET DB_DEL=f:\\a\\del\\del\\del\\\n");
    fprintf (st, "if \"%%DB_EDT%%\"==\"\" SET DB_EDT=notepad\n");
    fprintf (st, "rem echo DB_DEL = \"%%DB_DEL%%\"\n");
    fprintf (st, "if \"%%1\"==\"\" goto end\n");
    fprintf (st, "\n");
    fprintf (st, "dirtree -s%%DB_DEL%%db_this.dir -p %%2 %%3\n");
    fprintf (st, "dirtree -s%%DB_DEL%%db_that.dir -p %%1 %%2 %%3\n");
    fprintf (st, "\n");
    fprintf (st, "rem dirtree -r%%DB_DEL%%db_this.dir -2%%DB_DEL%%db_that.dir -c63 > %%DB_DEL%%db_del.out\n");
    fprintf (st, "rem notepad  %%DB_DEL%%db_del.out\n");
    fprintf (st, "\n");
    fprintf (st, "del %%DB_DEL%%db_del.out\n");
    fprintf (st, "\n");
    fprintf (st, "@echo '-                                                >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-   *-*-* Identical (same date, same CRC/size)   >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-                                                >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo 'vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "dirtree -r%%DB_DEL%%db_this.dir -2%%DB_DEL%%db_that.dir -t1 %%2 %%3 %%4 %%5 -C -c1 >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "\n");
    fprintf (st, "@echo '^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-   *-*-* Identical                              >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '------------------------------------------------ >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-                                                >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-         Same (any date, same CRC/size)         >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-                                                >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo 'vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "dirtree -r%%DB_DEL%%db_this.dir -2%%DB_DEL%%db_that.dir -t1 %%2 %%3 %%4 %%5 -C -D -c1 >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "\n");
    fprintf (st, "@echo '^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-   *-*-* Same                                   >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '------------------------------------------------ >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-                                                >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-         Newer in this                          >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-                                                >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo 'vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "dirtree -r%%DB_DEL%%db_this.dir -2%%DB_DEL%%db_that.dir -t1 %%2 %%3 %%4 %%5 -C -c16 >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "\n");
    fprintf (st, "@echo '^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-   *-*-* Newer in this                          >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '------------------------------------------------ >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-                                                >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-         Newer in that                          >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-                                                >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo 'vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "dirtree -r%%DB_DEL%%db_this.dir -2%%DB_DEL%%db_that.dir -t1 %%2 %%3 %%4 %%5 -C -c32 >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "\n");
    fprintf (st, "@echo '^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-   *-*-* Newer in that                          >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '------------------------------------------------ >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-                                                >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-         This only                              >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-                                                >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo 'vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "dirtree -r%%DB_DEL%%db_this.dir -2%%DB_DEL%%db_that.dir -t1 %%2 %%3 %%4 %%5 -C -c4 >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "\n");
    fprintf (st, "@echo '^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-   *-*-* This only                              >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '------------------------------------------------ >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-                                                >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-         That only                              >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-                                                >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo 'vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "dirtree -r%%DB_DEL%%db_this.dir -2%%DB_DEL%%db_that.dir -t1 %%2 %%3 %%4 %%5 -C -c8 >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "\n");
    fprintf (st, "@echo '^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-   *-*-* That only                              >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '------------------------------------------------ >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '-         End                                    >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "@echo '------------------------------------------------ >> %%DB_DEL%%db_del.out\n");
    fprintf (st, "\n");
    fprintf (st, "%%DB_EDT%% %%DB_DEL%%db_del.out\n");
    fprintf (st, "pause\n");
    fprintf (st, "del %%DB_DEL%%db_del.out\n");
    fprintf (st, "del %%DB_DEL%%db_this.dir\n");
    fprintf (st, "del %%DB_DEL%%db_that.dir\n");
    fprintf (st, "\n");
    fprintf (st, ":end\n");




    fclose (st);

    st = fopen ("xdb.bat", "wt");






    fprintf (st, "rem xdb.bat, make both the same; in this : xdb f:.\n");
    fprintf (st, "\n");
    fprintf (st, "if \"%%DB_DEL%%\"==\"\" SET DB_DEL=f:\\a\\del\\del\\del\\\n");
    fprintf (st, "if \"%%DB_EDT%%\"==\"\" SET DB_EDT=notepad\n");
    fprintf (st, "rem echo DB_DEL = \"%%DB_DEL%%\"\n");
    fprintf (st, "if \"%%1\"==\"\" goto end\n");
    fprintf (st, "\n");
    fprintf (st, "dirtree -s%%DB_DEL%%db_this.dir -p %%2 %%3\n");
    fprintf (st, "dirtree -s%%DB_DEL%%db_that.dir -p %%1 %%2 %%3\n");
    fprintf (st, "\n");
    fprintf (st, "rem dirtree -r%%DB_DEL%%db_this.dir -2%%DB_DEL%%db_that.dir -c63 > %%DB_DEL%%db_del.bat\n");
    fprintf (st, "rem notepad  %%DB_DEL%%db_del.bat\n");
    fprintf (st, "\n");
    fprintf (st, "del %%DB_DEL%%db_del.bat\n");
    fprintf (st, "\n");
    fprintf (st, "@echo @rem -                                                >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem -   *-*-* Newer in this                          >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem -                                                >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "dirtree -r%%DB_DEL%%db_this.dir -2%%DB_DEL%%db_that.dir -t1 %%2 %%3 %%4 %%5 -C -c16 -y >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "\n");
    fprintf (st, "@echo @rem ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem -   *-*-* Newer in this                          >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem ------------------------------------------------ >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem -                                                >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem -         Newer in that                          >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem -                                                >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "dirtree -r%%DB_DEL%%db_this.dir -2%%DB_DEL%%db_that.dir -t1 %%2 %%3 %%4 %%5 -C -c32 -y >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "\n");
    fprintf (st, "@echo @rem ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem -   *-*-* Newer in that                          >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem ------------------------------------------------ >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem -                                                >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem -         This only                              >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem -                                                >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "dirtree -r%%DB_DEL%%db_this.dir -2%%DB_DEL%%db_that.dir -t1 %%2 %%3 %%4 %%5 -C -c4 -y >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "\n");
    fprintf (st, "@echo @rem ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem -   *-*-* This only                              >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem ------------------------------------------------ >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem -                                                >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem -         That only                              >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem -                                                >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "dirtree -r%%DB_DEL%%db_this.dir -2%%DB_DEL%%db_that.dir -t1 %%2 %%3 %%4 %%5 -C -c8 -y >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "\n");
    fprintf (st, "@echo @rem ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem -   *-*-* That only                              >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem ------------------------------------------------ >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem -         End                                    >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "@echo @rem ------------------------------------------------ >> %%DB_DEL%%db_del.bat\n");
    fprintf (st, "\n");
    fprintf (st, "\n");
    fprintf (st, "%%DB_EDT%% %%DB_DEL%%db_del.bat\n");
    fprintf (st, "pause\n");
    fprintf (st, "call %%DB_DEL%%db_del.bat\n");
    fprintf (st, "pause\n");
    fprintf (st, "del %%DB_DEL%%db_del.bat\n");
    fprintf (st, "del %%DB_DEL%%db_this.dir\n");
    fprintf (st, "del %%DB_DEL%%db_that.dir\n");
    fprintf (st, "\n");
    fprintf (st, ":end\n");



    fclose (st);

    st = fopen ("xdm.bat", "wt");



    fprintf (st, "rem xdm.bat, move files in this to match that : xdm f:\\that\n");
    fprintf (st, "\n");
    fprintf (st, "if \"%%DB_DEL%%\"==\"\" SET DB_DEL=f:\\a\\del\\del\\del\\\n");
    fprintf (st, "if \"%%DB_EDT%%\"==\"\" SET DB_EDT=notepad\n");
    fprintf (st, "rem echo DB_DEL = \"%%DB_DEL%%\"\n");
    fprintf (st, "if \"%%1\"==\"\" goto end\n");
    fprintf (st, "dirtree -s%%DB_DEL%%db_this.dir -p %%2 %%3\n");
    fprintf (st, "dirtree -s%%DB_DEL%%db_that.dir -p %%1  %%2 %%3\n");
    fprintf (st, "dirtree -r%%DB_DEL%%db_this.dir -2%%DB_DEL%%db_that.dir -m -c63 -y > %%DB_DEL%%db_del.bat\n");
    fprintf (st, "pause\n");
    fprintf (st, "%%DB_EDT%% %%DB_DEL%%db_del.bat\n");
    fprintf (st, "pause\n");
    fprintf (st, "call %%DB_DEL%%db_del.bat\n");
    fprintf (st, "pause\n");
    fprintf (st, "del %%DB_DEL%%db_this.dir\n");
    fprintf (st, "del %%DB_DEL%%db_that.dir\n");
    fprintf (st, "del %%DB_DEL%%db_del.bat\n");
    fprintf (st, ":end\n");


    fclose (st);

    puts ("xdb.bat, xdc.bat, xdm.bat, and xds.bat generated");;

} /* bat */



unsigned long procdir (int dbidx, char HUGE *path) {
/*****************************************************************************
 *
 *
 *
 *****************************************************************************/
    unsigned long retval;
    int i;
    unsigned long l;
    void HUGE *p;

    /*  + homepathlen to save only partial path */

    retval = dnameptr [dbidx];


    i = strlen (path + homepathlen) + 1;

    if ((dnameptr [dbidx] + i + 16) > dnamesz [dbidx]) {
        l = dnamesz [dbidx] + DNAMEALLOCSZ;
        p = realloc ((void *) dnamebuf [dbidx], l);

        if (p == NULL) {
            printf ("OUT OF MEMORY:    dir: %s %d\n", __FILE__, __LINE__);
            return (1);
        }
        dnamesz  [dbidx] = l;
        dnamebuf [dbidx] = p;
    }


    if (ignorecase != 0) {
        strcpy (dnamebuf [dbidx] + dnameptr [dbidx], strlwr (path + homepathlen));
    } else {
        strcpy (dnamebuf [dbidx] + dnameptr [dbidx], path + homepathlen);
    }
    dnameptr [dbidx] += i;


    return (retval);

} /* procdir */



struct dirlist save;


int procfile (
    int dbidx,
    unsigned long diridx,
    char HUGE *path,
    char HUGE *name,
    unsigned long size,
    time_t time_write
) {
/*****************************************************************************
 *
 *
 *
 *****************************************************************************/
    int i;
    unsigned long l;
    void HUGE *p;
    struct dirlist HUGE *d;
#ifdef __TURBOC__
    time_t tt;

    tt = ftime2tt ((struct ftime *) &time_write);
    time_write = (unsigned long) tt;
#endif



    if (dirptr [dbidx] + 1 >= dirsz [dbidx]) {
        /*  need more memory */
        l = dirsz [dbidx] + DIRALLOCSZ;
        p = realloc ((void *) dirbuf [dbidx], l * sizeof (struct dirlist));

        if (p == NULL) {
            printf ("OUT OF MEMORY:    dir: %s %d\n", __FILE__, __LINE__);
            return (1);
        }
        dirsz [dbidx]  = l;
        dirbuf [dbidx] = (struct dirlist *) p;
    }

    if (time_write == 0xffffffff) {
        /*  0 may be intepreted as Wed Dec 31 16:00:00 1969 (MS)
            or Thu Jan 01 -8:00:00 1970 (Turbo C) */
        time_write = 0;
    }

    if ((strlen (path) + strlen (name) + 2) < 2048) {
        strcpy (procfilebuf, path);
        strcat (procfilebuf, name);
    } else {
        /*  full path name longer than 2048, give up */
        procfilebuf [0] = 0;
    }


    d = dirbuf [dbidx] + dirptr [dbidx];
    d->tt       = time_write;
    d->size     = size;
    if (calccrc != 0) {
        d->crc  = crc32file (procfilebuf);
    } else {
        d->crc  = 0;
    }
    d->fnameidx = fnameptr [dbidx];
    d->dirid    = diridx;


    if (debug >= 5) {
        d = dirbuf [dbidx] + dirptr [dbidx];
        printf ("dirbuf [%d] [%ld] = %08lx org %08lx ",
            dbidx, dirptr [dbidx], d, dirbuf [dbidx]);
    }

    i = strlen (name) + 1;


    if ((fnameptr [dbidx] + i + 16) > fnamesz [dbidx]) {
        l = fnamesz [dbidx] + FNAMEALLOCSZ;
        p = realloc ((void *) fnamebuf [dbidx], l);

        if (p == NULL) {
            printf ("OUT OF MEMORY:  f    : %s %d\n", __FILE__, __LINE__);
            return (1);
        }
        fnamesz  [dbidx] = l;
        fnamebuf [dbidx] = p;
    }


    /*  count if memory available */
    dirptr [dbidx]++;

    if (calccrc != 0) {
        if ((dirptr [dbidx] & 0x3f) == 0) {
            fprintf (stderr, ".");
        }
    }

    if (debug >= 5) {
        printf ("fixaddr (%08lx, %lx) = %08lx org %08lx\n",
            fnamebuf [dbidx], fnameptr [dbidx], fnamebuf [dbidx] + fnameptr [dbidx], fnamebuf [dbidx]);
    }

    if (ignorecase != 0) {
        strcpy (fnamebuf [dbidx] + fnameptr [dbidx], strlwr (name));
    } else {
        strcpy (fnamebuf [dbidx] + fnameptr [dbidx], name);
    }
    fnameptr [dbidx] += i;


    return (0);


} /* procfile */



int dtscandir (int dbidx, char HUGE *path) {
/*****************************************************************************
 *
 *
 *
 *****************************************************************************/
    int i, j, pathend;
    unsigned long ldirnameidx;
#if defined(_MSC_VER)
    long hdl;
    struct _finddata_t ff;
#elif defined(__BORLANDC__) || defined(__TURBOC__)
    /*struct ffblk ff;*/
    struct myffblk ff;
    time_t tt;
#else
    /* __GNUC__ */
    char fullpath [2048];
    DIR             *dip;
    struct dirent   *dit;
    struct stat buf;
#endif

    if (debug == 0) {
        static int progress = 0;
        if ((progress++ % 4) == 0) {
            fprintf (stderr, ".");
        }
    }

    if (recurlvl == 0) {
        /*  start counting from current directory */
        nodirs [dbidx] = 1;
    }
    if (++recurlvl > maxrecurlvl) {
        maxrecurlvl = recurlvl;
    }

    pathend = strlen (path);
    if (ignorecase != 0) {
        strlwr (path);
    } else {
    }

    if (file == 0) {
        puts (path);
    } else {
    /* } else if (exclude_dir (path) == 0) {*/
        /*  not in exclude directory */
        path [pathend] = '\0';
        ldirnameidx = procdir (dbidx, path);
        if (debug >= 1) {
            printf ("%d dir %s\n", dbidx, path);
        }
#if defined(_MSC_VER)
        strcpy (path + pathend, "*.*");
        hdl = _findfirst (path, &ff);
        path [pathend] = '\0';

        if (hdl >= 0) {
            do {
                if (((ff.attrib & 0x10) == 0) && (exclude_ext (ff.name) == 0)) {
                    /*  not directory and not exclude extension */
                    if (debug >= 3) {
                        printf ("file %s%s\n", path, ff.name);
                    }
                    procfile (dbidx, ldirnameidx, path, ff.name, ff.size,
                                                                 ff.time_write);
                    nofiles++;
                    totalbytes += ff.size;
                    if (totalbytes >= 1000000000) {
                        j = (totalbytes / 1000000000);
                        totalbytes -= j * 1000000000;
                        totalGbytes += j;
                    }
                }
                i = _findnext (hdl, &ff);
            } while (i == 0);
        }
        _findclose (hdl);
#elif defined(__BORLANDC__) || defined(__TURBOC__)
        strcpy (path + pathend, "*.*");
        i = findfirst (path, (struct ffblk *) &ff, WILDCARDS);
        path [pathend] = '\0';

        if (i == 0) {
            do {
                if (((ff.attrib & 0x10) == 0) && (exclude_ext (ff.name) == 0)) {
                    /*  not directory and not exclude extension */
                    if (debug >= 3) {
                        printf ("file %s%s\n", path, ff.name);
                    }
                    procfile (dbidx, ldirnameidx, path, ff.name, ff.size,
                                                           ff.time_write);

                    nofiles++;
                    totalbytes += ff.size;
                    if (totalbytes >= 1000000000) {
                        j = (totalbytes / 1000000000);
                        totalbytes -= j * 1000000000;
                        totalGbytes += j;
                    }
                }
                i = findnext ((struct ffblk *) &ff);
            } while (i == 0);
        }
#else
        /* __GNUC__ */
        if (path [0] == 0) {
            dip = opendir (".");
        } else {
            dip = opendir (path);
        }
        if (dip != NULL) {
            while ((dit = readdir (dip)) != NULL) {
                strcpy (fullpath, path);
                strcat (fullpath, dit->d_name);
                if (debug >= 3) {
                    printf ("file %s%s\n", path, dit->d_name);
                }
                i = lstat (fullpath, &buf);
                if ((i == 0) &&
                    ((buf.st_mode & __S_IFMT) == __S_IFREG) && 
                    (exclude_ext (dit->d_name) == 0)) {
                    /*  not directory and not exclude extension */
                    procfile (dbidx, ldirnameidx, path, dit->d_name, 
                        buf.st_size, buf.st_mtim.tv_sec);
                    nofiles++;
                    totalbytes += buf.st_size;
                    if (totalbytes >= 1000000000) {
                        j = (totalbytes / 1000000000);
                        totalbytes -= j * 1000000000;
                        totalGbytes += j;
                    }
                }
            }
            closedir (dip);
        }

#endif
    }



    if (recurlvl <= recurlvllmt) {
        /*  recurse directory */
#if defined(_MSC_VER)
        strcpy (path + pathend, "*.*");
        hdl = _findfirst (path, &ff);
        if (hdl >= 0) {
            do {
                if (((ff.attrib & 0x10) != 0) &&
                    (dirtstrcmp (ff.name, ".") != 0) &&
                    (dirtstrcmp (ff.name, "..") != 0)) {
                    /*  is directory */
                    strcpy (path + pathend, ff.name);
                    strcat (path + pathend, "\\");
                    if (exclude_dir (path) == 0) {
                        nodirs [dbidx]++;
                        dtscandir (dbidx, path);
                    }
                }
                i = _findnext (hdl, &ff);
            } while (i == 0);
        }
        _findclose (hdl);
#elif defined(__BORLANDC__) || defined(__TURBOC__)
        strcpy (path + pathend, "*.*");
        i = findfirst (path, (struct ffblk *) &ff, DRIVE);
        if (i == 0) {
            do {
                if (((ff.attrib & 0x10) != 0) &&
                    (dirtstrcmp (ff.name, ".") != 0) &&
                    (dirtstrcmp (ff.name, "..") != 0)) {
                    /*  directory listing, process */
                    strcpy (path + pathend, ff.name);
                    strcat (path + pathend, "\\");
                    if (exclude_dir (path) == 0) {
                        nodirs [dbidx]++;
                        dtscandir (dbidx, path);
                    }
                }
                i = findnext ((struct ffblk *) &ff);
            } while (i == 0);
        }
#else
    /* __GNUC__ */
        if (path [0] == 0) {
            dip = opendir (".");
        } else {
            dip = opendir (path);
        }
        if (dip != NULL) {
            while ((dit = readdir (dip)) != NULL) {
                strcpy (fullpath, path);
                strcat (fullpath, dit->d_name);
                /*printf ("%s %x\n", fullpath, buf.st_mode);*/
                i = lstat (fullpath, &buf);
                if ((i == 0) &&
                    ((buf.st_mode & __S_IFMT) == __S_IFDIR) &&
                    (dirtstrcmp (dit->d_name, ".") != 0) &&
                    (dirtstrcmp (dit->d_name, "..") != 0)) {
                    /*  directory listing, process */
                    strcpy (fullpath, path);
                    strcat (fullpath, dit->d_name);
                    strcat (fullpath, "/");
                    if (exclude_dir (fullpath) == 0) {
                        nodirs [dbidx]++;
                        dtscandir (dbidx, fullpath);
                    }
                }
            }
            closedir (dip);
        }
#endif
    }

    path [pathend] = '\0';
    --recurlvl;

    return (0);
} /* dtscandir */



#ifdef __TURBOC__

time_t mktime (const struct tm *tm) {
    int max_loop;
    time_t time_gs;
    struct tm *tm_gs;


    max_loop = 100;

    time (&time_gs);


    while (1) {
        if (max_loop-- <= 0) {
            return (0);
        }

        tm_gs = localtime (&time_gs);

        if (tm_gs->tm_year != tm->tm_year) {
            time_gs -= (tm_gs->tm_year - tm->tm_year) *
                       365L * 24L * 60L * 60L;/* +
                       (tm_gs->tm_mon - tm->tm_mon) *
                       30L * 24L * 60L * 60L;*/
            continue;
        }

        if (tm_gs->tm_mon != tm->tm_mon) {
            time_gs -= (tm_gs->tm_mon - tm->tm_mon) *
                       28L * 24L * 60L * 60L;
            continue;
        }

        if (tm_gs->tm_mday != tm->tm_mday) {
            time_gs -= (tm_gs->tm_mday - tm->tm_mday) *
                       24L * 60L * 60L;/* +
                       (tm_gs->tm_hour - tm->tm_hour) *
                             60L * 60L +
                       (tm_gs->tm_min - tm->tm_min) *
                                   60L +
                       (tm_gs->tm_sec - tm->tm_sec);*/
            continue;
        }

        if (tm_gs->tm_hour != tm->tm_hour) {
            time_gs -= (tm_gs->tm_hour - tm->tm_hour) *
                       60L * 60L;
            continue;
        }
        if (tm_gs->tm_min != tm->tm_min) {
            time_gs -= (tm_gs->tm_min - tm->tm_min) *
                       60L;
            continue;
        }
        if (tm_gs->tm_sec != tm->tm_sec) {
            time_gs -= (tm_gs->tm_sec - tm->tm_sec);
            continue;
        }
        break;
    }


    return (time_gs);
}


#endif



void report (unsigned long int nodir, unsigned long int nofil) {
/*****************************************************************************
 *
 *
 *
 *****************************************************************************/
    int i, j;

    printf ("%ld directories %ld files\n", nodir, nofil);

    printf ("recurlvl    = %d\n",    recurlvl);
    printf ("maxrecurlvl = %d\n", maxrecurlvl);


    i = 0;
    if (totalGbytes > 0) {
        printf ("%ld,", totalGbytes);
        i = 1;
    }
    j = totalbytes / 1000000;
    if (j > 0) {
        if (i != 0) {
            printf ("%03d,", j);
        } else {
            printf ("%d,", j);
        }
        totalbytes -= j * 1000000;
        i = 1;
    }
    j = totalbytes / 1000;
    if ((j > 0) || (i != 0)) {
        if (i != 0) {
            printf ("%03d,", j);
        } else {
            printf ("%d,", j);
        }
        totalbytes -= j * 1000;
        i = 1;
    }

    j = totalbytes;
    if ((j > 0) || (i != 0)) {
        printf ("%03d", j);
    } else {
        printf ("%d", j);
    }
    puts (" bytes");

} /* report */



int retrieveit (char *fname, int dbidx, char *basedir) {
/*****************************************************************************
 *
 *  Retrieve directory tree from disk
 *
 *****************************************************************************/
    unsigned long l, offset;
    unsigned int size;
    struct dirlist HUGE *d;
    char HUGE *last, HUGE *q;
    FILE *st;

/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    st = fopen (fname, "rb");
    if (st == NULL) {
        failed :
        printf ("Failed to open '%s'\n", fname);
        fprintf (stderr, "Failed to open '%s'\n", fname);
        if (st != NULL) {
            fclose (st);
        }

        fnameptr [dbidx] = 0;
        dnameptr [dbidx] = 0;
          dirptr [dbidx] = 0;

        return (1);
    }


/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    /*  0xcafe2000, */
    if (fread (&l, sizeof (l), 1, st)  != 1) {
        printf ("Read error: %s %d\n", __FILE__, __LINE__);
        goto failed;
    }
    if (l != 0xcafe2000) {
        if (verbose != 0) {
            printf ("ERROR %s %d\n", __FILE__, __LINE__);
            goto end;
        }
    }

/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    /*  0xcafe2001, <dir name size in bytes>, <dir names ...> */
    if (fread (&l, sizeof (l), 1, st) != 1) {
        printf ("Read error: %s %d\n", __FILE__, __LINE__);
        goto failed;
    }
    if (l != 0xcafe0001) {
        if (verbose != 0) {
            printf ("ERROR %s %d\n", __FILE__, __LINE__);
            goto end;
        }
    }
    if (fread (&dnameptr [dbidx], sizeof (dnameptr [dbidx]), 1, st) != 1) {
        printf ("Read error: %s %d\n", __FILE__, __LINE__);
        goto failed;
    }
    /*  (void *) dnamebuf [dbidx], */
    dnamebuf [dbidx] = malloc (dnameptr [dbidx] + 16);
    if (dnamebuf [dbidx] == NULL) {
        printf ("OUT OF MEMORY: %s %d\n", __FILE__, __LINE__);
        return (1);
    }
    l = dnameptr [dbidx];
    offset = 0;
    while (l > 0) {
        if (l > 64000) {
            size = 64000;
        } else {
            size = l;
        }
        l -= size;
        if (fread (((char HUGE *) dnamebuf [dbidx]) + offset, 1, size, st) != size) {
            printf ("Read error: %s %d\n", __FILE__, __LINE__);
            goto failed;
        }
        offset += size;
    }
    if (verbose != 0) {
        printf ("dname size %ld bytes\n", dnameptr [dbidx]);
    }

    /*  fixup Linux / vs. Windows \ */
    for (l = 0; l < dnameptr [dbidx]; l++) {
        if (dnamebuf [dbidx] [l] == DIRSEPARATOR2) {
            dnamebuf [dbidx] [l] =  DIRSEPARATOR;
        }
    }


/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    /*  0xcafe2002, <file name size in bytes>, <file names ...> */
    if (fread (&l, sizeof (l), 1, st) != 1) {
        printf ("Read error: %s %d\n", __FILE__, __LINE__);
        goto failed;
    }
    if (l != 0xcafe0002) {
        if (verbose != 0) {
            printf ("ERROR %s %d\n", __FILE__, __LINE__);
            goto end;
        }
    }
    if (fread (&fnameptr [dbidx], sizeof (fnameptr [dbidx]), 1, st) != 1) {
        printf ("Read error: %s %d\n", __FILE__, __LINE__);
        goto failed;
    }
    /*  (void *) fnamebuf [dbidx], */
    fnamebuf [dbidx] = malloc (fnameptr [dbidx] + 16);
    if (fnamebuf [dbidx] == NULL) {
        printf ("OUT OF MEMORY: %s %d\n", __FILE__, __LINE__);
        return (1);
    }
    l = fnameptr [dbidx];
    offset = 0;
    while (l > 0) {
        if (l > 64000) {
            size = 64000;
        } else {
            size = l;
        }
        l -= size;
        if (fread (((char HUGE *) fnamebuf [dbidx]) + offset, 1, size, st) != size) {
            printf ("Read error: %s %d\n", __FILE__, __LINE__);
            goto failed;
        }
        offset += size;
    }
    if (verbose != 0) {
        printf ("fname size %ld bytes\n", fnameptr [dbidx]);
    }

/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    /*  0xcafe2003, <dir entry size in bytes>, <dir entry ...> */
    if (fread (&l, sizeof (l), 1, st) != 1) {
        printf ("Read error: %s %d\n", __FILE__, __LINE__);
        goto failed;
    }
    if (l != 0xcafe0003) {
        if (verbose != 0) {
            printf ("ERROR %s %d\n", __FILE__, __LINE__);
            goto end;
        }
    }
    if (fread (&l, sizeof (l), 1, st) != 1) {
        printf ("Read error: %s %d\n", __FILE__, __LINE__);
        goto failed;
    }
    dirptr [dbidx] = l / sizeof (struct dirlist);
    /*  (void *) dirbuf [dbidx], */
    dirbuf [dbidx] = malloc (l + 16);
    if (dirbuf [dbidx] == NULL) {
        printf ("OUT OF MEMORY: %s %d\n", __FILE__, __LINE__);
        return (1);
    }
    offset = 0;
    while (l > 0) {
        if (l > 64000) {
            size = 64000;
        } else {
            size = l;
        }
        l -= size;
        if (fread (((char HUGE *) dirbuf [dbidx]) + offset, 1, size, st) != size) {
            printf ("Read error: %s %d\n", __FILE__, __LINE__);
            goto failed;
        }
        offset += size;
    }


/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    /*  0xcafe2004, <base dir size in bytes>, <base dir name> */
    if (fread (&l, sizeof (l), 1, st) == 1) {
        if (l == 0xcafe0004) {
            if (fread (&l, sizeof (l), 1, st) == 1) {
                fread (basedir, 1, l, st);
                if (verbose != 0) {
                    printf ("Read base dir '%s' %s %d\n", basedir, __FILE__, __LINE__);
                }
            }
        }
    }


/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    /*  count number of dir */
    last = NULL;
    dirsort (dbidx, SORTBYDIR);
    nodirs [dbidx] = 0;
    for (l = 0; l < dirptr [dbidx]; l++) {
        d = dirbuf [dbidx] + l;
        q = dnamebuf [dbidx] + d->dirid;
        if (last != NULL) {
            /*  check dir name of this entry against current dir */
            if (dirtstrcmp (q, last) == 0) {
                /*  same dir, goto next entry */
                continue;
            }
        }
        nodirs [dbidx]++;
        /*  make this dir as current */
        last = q;
    }


    if (verbose != 0) {
        printf ("dir size %ld bytes; %ld entries; %ld directories\n",
            l, dirptr [dbidx], nodirs [dbidx]);
    }

    end :

    fclose (st);

    return (0);

} /* retrieveit */



void saveit (char *fname, int dbidx, char *basedir) {
/*****************************************************************************
 *
 *  Save directory tree to disk
 *
 *****************************************************************************/
    unsigned long l, offset;
    unsigned int size;
    FILE *st;

/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    st = fopen (fname, "wb");
    if (st == NULL) {
        printf ("Unable to write to '%s'\n", fname);
        return;
    }

/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    /*  0xcafe2000, */
    l = 0xcafe2000;
    fwrite (&l, 1, sizeof (l), st);


/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    /*  0xcafe2001, <dir name size in bytes>, <dir names ...> */
    l = 0xcafe0001;
    fwrite (&l, 1, sizeof (l), st);

    fwrite ((void *) &dnameptr [dbidx], 1, sizeof (dnameptr [dbidx]), st);
    l = dnameptr [dbidx];
    offset = 0;
    while (l > 0) {
        if (l > 64000) {
            size = 64000;
        } else {
            size = l;
        }
        l -= size;
        fwrite (((char HUGE *) dnamebuf [dbidx]) + offset, 1, size, st);
        offset += size;
    }
    printf ("dname size %ld bytes\n", dnameptr [dbidx]);


/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    /*  0xcafe2002, <file name size in bytes>, <file names ...> */
    l = 0xcafe0002;
    fwrite (&l, 1, sizeof (l), st);

    fwrite ((void *) &fnameptr [dbidx], 1, sizeof (fnameptr [dbidx]), st);
    l = fnameptr [dbidx];
    offset = 0;
    while (l > 0) {
        if (l > 64000) {
            size = 64000;
        } else {
            size = l;
        }
        l -= size;
        fwrite (((char HUGE *) fnamebuf [dbidx]) + offset, 1, size, st);
        offset += size;
    }
    printf ("fname size %ld bytes\n", fnameptr [dbidx]);

/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    /*  0xcafe2003, <dir entry size in bytes>, <dir entry ...> */
    l = 0xcafe0003;
    fwrite (&l, 1, sizeof (l), st);

    l = dirptr [dbidx] * sizeof (struct dirlist);
    fwrite ((void *) &l, 1, sizeof (l), st);
    offset = 0;
    while (l > 0) {
        if (l > 64000) {
            size = 64000;
        } else {
            size = l;
        }
        l -= size;
        fwrite (((char HUGE *) dirbuf [dbidx]) + offset, 1, size, st);
        offset += size;
    }
    printf ("dir size %ld bytes\n", l);

/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    /*  0xcafe2004, <base dir size in bytes>, <base dir name> */
    l = 0xcafe0004;
    fwrite (&l, 1, sizeof (l), st);

    l = strlen (basedir) + 1;
    fwrite ((void *) &l, 1, sizeof (l), st);
    fwrite (basedir, 1, l, st);

    if (verbose != 0) {
        printf ("Write base dir '%s' %s %d\n", basedir, __FILE__, __LINE__);
    }


    fclose (st);
} /* saveit */


