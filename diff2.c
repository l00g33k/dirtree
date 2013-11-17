/*
 * Copyright (c) 2008-2012 l00g33k@gmail.com All rights reserved.
 *
 */

#include "dirtree.h"


void outputcpy (
    int condition,
    int cmd,
    char *dname,
    char *fname,
    char *bdir0,
    char *bdir1,
    struct dirlist *d0,
    struct dirlist *d1,
    int cnt
) {
/*****************************************************************************
 *
 *  Prints output text
 *
 *
 *  int condition,      : display condition: same, diff, 1/2 only, newer. etc.
 *  int cmd,            : unused?
 *  char *dname,        : dir name
 *  char *fname,        : file name
 *  struct dirlist *d0, : dir entry in 1
 *  struct dirlist *d1  : dir entry in 2
 *
 *****************************************************************************/
    char *doscmd;
    int fmt;
    static char *bdir0_l, *dname_l, *fname_l;

    if ((cmd & condition) == 0) {
        return;
    }

    switch (condition) {
        default :
            return;

        case CMD_SAME      : fmt = 2; doscmd = "call same____.bat "; break;
        case CMD_DIFF      : fmt = 1; doscmd = "call pause___.bat "; break;
        case CMD_THIS_ONLY : fmt = 2; doscmd = "call thisonly.bat "; break;
        case CMD_THAT_ONLY : fmt = 3; doscmd = "call thatonly.bat "; break;
        case CMD_THIS_NEW  : fmt = 2; doscmd = "call thisnewr.bat "; break;
        case CMD_THAT_NEW  : fmt = 3; doscmd = "call thatnewr.bat "; break;
        case CMD_THIS_MOVE : fmt = 5; doscmd = "call thismved.bat "; break;
        case CMD_THAT_MOVE : fmt = 5; doscmd = "call thatmved.bat "; break;
        case CMD_THIS_DUP  : fmt = 4; doscmd = "call thisdupd.bat "; break;
        case CMD_THAT_DUP  : fmt = 4; doscmd = "call thatdupd.bat "; break;
    }

    if (fmt == 2) {
        /*  2 file sizes and dates */
        printf ("%s %c%s%s%s%c %c%s%s%s%c\n",
            doscmd, 
            qoute, bdir0, dname, fname, qoute, 
            qoute, bdir1, dname, fname, qoute);
    } else if (fmt == 4) {
        if (cnt == 1) {
            bdir0_l = bdir0;
            dname_l = dname;
            fname_l = fname;
        } else {
            printf ("%s %d %c%s%s%s%c %c%s%s%s%c\n",
                    doscmd, cnt, qoute, bdir0_l, dname_l, fname_l, qoute, 
                                 qoute, bdir0, dname, fname, qoute);
        }
    } else if (fmt == 3) {
        /*  2 file sizes and dates */
        printf ("%s %c%s%s%s%c %c%s%s%s%c\n",
            doscmd, qoute, bdir1, dname, fname, qoute, 
                    qoute, bdir0, dname, fname, qoute);
    } else if (fmt == 5) {
        /*  2 file sizes and dates */
        printf ("%s %c%s%s%s%c %c%s%s%s%c\n",
            doscmd, qoute, bdir0, dname, fname, qoute, 
                    qoute, bdir1, 
                    dnamebuf [1] + d1->dirid, 
                    fnamebuf [1] + d1->fnameidx, qoute);
    } else {
        /*  1 file size and date. */
        printf ("%s %c%s%s%s%c\n",
            doscmd, qoute, bdir0, dname, fname, qoute);
    }

    nooutput++;

} /* outputcpy */



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
) {
/*****************************************************************************
 *
 *  Prints output text
 *
 *
 *  int condition,      : display condition: same, diff, 1/2 only, newer. etc.
 *  int cmd,            : unused?
 *  char *dname,        : dir name
 *  char *fname,        : file name
 *  struct dirlist *d0, : dir entry in 1
 *  struct dirlist *d1  : dir entry in 2
 *
 *****************************************************************************/
    char c, date0 [32], date1 [32];
    int fmt;

    if ((cmd & condition) == 0) {
        return;
    }

    /*  convert one or both time_t to ASCII */
    strcpy (date0, xctime (&d0->tt));
    date0 [strlen (date0) - 1] = '\0';
    if (d1 != NULL) {
        strcpy (date1, xctime (&d1->tt));
        date1 [strlen (date1) - 1] = '\0';
    }


    switch (condition) {
        default :
            return;

        case CMD_SAME      : c = '='; fmt = 1; break;
        case CMD_DIFF      : c = '#'; fmt = 2; break;
        case CMD_THIS_ONLY : c = '1'; fmt = 1; break;
        case CMD_THAT_ONLY : c = '2'; fmt = 1; break;
        case CMD_THIS_NEW  : c = '>'; fmt = 2; break;
        case CMD_THAT_NEW  : c = '<'; fmt = 2; break;
        case CMD_THIS_MOVE : c = '}'; fmt = 3; break;
        case CMD_THAT_MOVE : c = '{'; fmt = 3; break;
        case CMD_THIS_DUP  : c = ']'; fmt = 4; break;
        case CMD_THAT_DUP  : c = '['; fmt = 4; break;
    }

    if (calccrc == 0) {
        if (fmt == 3) {
            printf ("%s %c%10ld , %s%s , %s%10ld , %s%s\n",
                date0, c, d0->size, dname, fname, date1, d1->size,
                dnamebuf [1] + d1->dirid, 
                fnamebuf [1] + d1->fnameidx);
        } else if (fmt == 4) {
            /*  1 file size and date. */
            printf ("%s %c%d%10ld %s, %s%s\n",
                date0, c, cnt, d0->size, padding, dname, fname);
        } else if (fmt == 2) {
            /*  2 file sizes and dates */
            printf ("%s %c %10ld , %s%s , %s%10ld\n",
                date0, c, d0->size, dname, fname, date1, d1->size);
        } else {
            /*  1 file size and date. */
            printf ("%s %c %10ld %s, %s%s\n",
                date0, c, d0->size, padding, dname, fname);
        }
    } else {
        if (fmt == 3) {
            printf ("%s %c%10ld , 0x%08lx , %s%s , %s%10ld , 0x%08lx , %s%s\n",
                date0, c, d0->size, d0->crc, dname, fname, date1, d1->size, d1->crc, 
                dnamebuf [1] + d1->dirid, 
                fnamebuf [1] + d1->fnameidx);
        } else if (fmt == 4) {
            /*  1 file size and date. */
            printf ("%s %c%d%10ld , 0x%08lx , %s, %s%s\n",
                date0, c, cnt, d0->size, d0->crc, padding, dname, fname);
        } else if (fmt == 2) {
            /*  2 file sizes and dates */
            printf ("%s %c %10ld , 0x%08lx , %s%s , %s%10ld\n",
                date0, c, d0->size, d0->crc, dname, fname, date1, d1->size);
        } else {
            /*  1 file size and date. */
            printf ("%s %c %10ld , 0x%08lx , %s, %s%s\n",
                date0, c, d0->size, d0->crc, padding, dname, fname);
        }
    }
    nooutput++;

} /* outputtxt */



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
) {
/*****************************************************************************
 *
 *  Prints output CSV
 *
 *
 *  int condition,      : display condition: same, diff, 1/2 only, newer. etc.
 *  int cmd,            : unused?
 *  char *dname,        : dir name
 *  char *fname,        : file name
 *  struct dirlist *d0, : dir entry in 1
 *  struct dirlist *d1  : dir entry in 2
 *
 *****************************************************************************/
    char c, date0 [32], date1 [32];
    int fmt;

    if ((cmd & condition) == 0) {
        return;
    }

    /*  convert one or both time_t to ASCII */
    strcpy (date0, xctime (&d0->tt));
    date0 [strlen (date0) - 1] = '\0';
    if (d1 != NULL) {
        strcpy (date1, xctime (&d1->tt));
        date1 [strlen (date1) - 1] = '\0';
    }


    switch (condition) {
        default :
            return;

        case CMD_SAME      : c = '='; fmt = 1; break;
        case CMD_DIFF      : c = '#'; fmt = 2; break;
        case CMD_THIS_ONLY : c = '1'; fmt = 1; break;
        case CMD_THAT_ONLY : c = '2'; fmt = 1; break;
        case CMD_THIS_NEW  : c = '>'; fmt = 2; break;
        case CMD_THAT_NEW  : c = '<'; fmt = 2; break;
        case CMD_THIS_MOVE : c = '}'; fmt = 3; break;
        case CMD_THAT_MOVE : c = '{'; fmt = 3; break;
        case CMD_THIS_DUP  : c = ']'; fmt = 4; break;
        case CMD_THAT_DUP  : c = '['; fmt = 4; break;
    }


    if (calccrc == 0) {
        /*  just pathname and file name */
        if (fmt == 3) {
            printf ("\"%s\" , \"%c\" , %ld , \"%s\" , %ld , \"%s\" , \"%s\" , \"%s\" , \"%s\"\n",
                date0, c, d0->size, date1, d1->size, (dname [0] == '\0') ?  ":(blank):" : dname, fname,
                (*(dnamebuf [1] + d1->dirid) == '\0') ?  ":(blank):" : dnamebuf [1] + d1->dirid, 
                fnamebuf [1] + d1->fnameidx);
        } else if (fmt == 4) {
            /*  1 file size and date. */
            printf ("\"%s\" , \"%c%d\" , %ld , \"\" ,   \"\" , \"%s\" , \"%s\"\n",
                date0, c, cnt, d0->size, (dname [0] == '\0') ?  ":(blank):" : dname, fname);
        } else if (fmt == 2) {
            /*  2 file sizes and dates */
            printf ("\"%s\" , \"%c\" , %ld , \"%s\" , %ld , \"%s\" , \"%s\"\n",
                date0, c, d0->size, date1, d1->size, (dname [0] == '\0') ?  ":(blank):" : dname, fname);
        } else {
            /*  1 file size and date. */
            printf ("\"%s\" , \"%c\" , %ld , \"\" ,   \"\" , \"%s\" , \"%s\"\n",
                date0, c, d0->size, (dname [0] == '\0') ?  ":(blank):" : dname, fname);
        }
    } else {
        /*  just pathname and file name */
        if (fmt == 3) {
            printf ("\"%s\" , \"%c\" , %ld , 0x%08lx , \"%s\" , %ld , 0x%08lx , \"%s\" , \"%s\" , \"%s\" , \"%s\"\n",
                date0, c, d0->size, d0->crc, date1, d1->size, d1->crc, (dname [0] == '\0') ?  ":(blank):" : dname, fname,
                (*(dnamebuf [1] + d1->dirid) == '\0') ?  ":(blank):" : dnamebuf [1] + d1->dirid, 
                fnamebuf [1] + d1->fnameidx);
        } else if (fmt == 4) {
            /*  1 file size and date. */
            printf ("\"%s\" , \"%c%d\" , %ld , 0x%08lx , \"\" ,   \"\" , \"%s\" , \"%s\"\n",
                date0, c, cnt, d0->size, d0->crc, (dname [0] == '\0') ?  ":(blank):" : dname, fname);
        } else if (fmt == 2) {
            /*  2 file sizes and dates */
            printf ("\"%s\" , \"%c\" , %ld , 0x%08lx , \"%s\" , %ld , 0x%08lx , \"%s\" , \"%s\"\n",
                date0, c, d0->size, d0->crc, date1, d1->size, d1->crc, (dname [0] == '\0') ?  ":(blank):" : dname, fname);
        } else {
            /*  1 file size and date. */
            printf ("\"%s\" , \"%c\" , %ld , 0x%08lx , \"\" ,   \"\" , \"%s\" , \"%s\"\n",
                date0, c, d0->size, d0->crc, (dname [0] == '\0') ?  ":(blank):" : dname, fname);
        }
    }

    nooutput++;

} /* outputcsv */




void diff2 (int cmd, char *bdir0, char *bdir1, int outputfmt) {
/*****************************************************************************
 *
 *  Compares two directory trees
 *
 *****************************************************************************/
    char HUGE *p, HUGE *last, HUGE *q, HUGE *flags [2], HUGE *thisdir [2], HUGE *fname [2], HUGE *dname [2],
        date1 [32], date2 [32];
    int dbidx, dbidxst, dbidxen, cmprst, endflag, k, title_printed = 0;
    unsigned long i, j, nodirs [2], HUGE *dirs [2], HUGE *inboth, noinboth, ptr [2];
    long int lastnooutput;
    struct dirlist HUGE *d, HUGE *d2;
    void (*output) (
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


    lastnooutput = nooutput;

    if (outputfmt == OUTPUTCSV) {
        output = (long) outputcsv;
        if ((long) output != (long) outputcsv) {
            fprintf (stderr, "%s %d fn error %08lx %08lx\n",
                __FILE__, __LINE__, (long) output, (long) outputcsv);
        }
    } else if (outputfmt == OUTPUTCPY) {
        output = (long) outputcpy;
        if ((long) output != (long) outputcpy) {
            fprintf (stderr, "%s %d fn error %08lx %08lx\n",
                __FILE__, __LINE__, (long) output, (long) outputcpy);
        }
    } else {
        output = (long) outputtxt;
        if ((long) output != (long) outputtxt) {
            fprintf (stderr, "%s %d fn error %08lx %08lx\n",
                __FILE__, __LINE__, (long) output, (long) outputtxt);
        }
    }


    dirs [0] = NULL;
    dirs [1] = NULL;
    flags [0] = NULL;
    flags [1] = NULL;
    inboth = NULL;

    if (debug >= 3) {
        printf ("%d: %ld files %ld files\n", __LINE__, dirptr [0], dirptr [1]);
    }


    /*  loops through both dir lists twice:
        1st time to count number of dir names
        2nd time to save pointer to dir names in
                    dirs [dbidx] [nodirs [dbidx]++]
    */
    for (dbidx = 0; dbidx < 4; dbidx++) {
        /*  counter for dirs */
        nodirs [dbidx & 1] = 0;
        last = NULL;
        /*  loop entries in each list */
        for (i = 0; i < dirptr [dbidx & 1]; i++) {
            /*  get pointer to dir entry */
            d = dirbuf [dbidx & 1] + i;
            /*  get pointer to name of dir of this entry */
            q = dnamebuf [dbidx & 1] + d->dirid;
            if (debug >= 5) {
                printf ("%d: dirbuf [%d]=%08lx, d=%08lx, dnamebuf []=%08lx, q=%08lx\n", __LINE__,
                    dbidx & 1, dirbuf [dbidx & 1], d,
                    dnamebuf [dbidx & 1], q);

            }
            if (last != NULL) {
                /*  check dir name of this entry against current dir */
                if (dirtstrcmp (q, last) == 0) {
                    /*  same dir, goto next entry */
                    continue;
                }
            }

            /*  count dir if 1st pass, remember index to dir entry if 2nd pass */
            if (dbidx < 2) {
                /*  count */
                nodirs [dbidx & 1]++;
            } else {
                /*  remember it */
                dirs [dbidx & 1] [nodirs [dbidx & 1]++] = i;
            }

            if (debug >= 4) {
                printf ("%d: nodirs [%d] = %ld q '%30s' last '%30s'\n", __LINE__,
                    dbidx & 1, nodirs [dbidx & 1], q, last);
            }
            /*  make this dir as current */
            last = q;
        }
        if (dbidx < 2) {
            /*  on first pass, allocate memory for array to remember dir */
            /*  allocate memory */
            dirs [dbidx] = malloc (nodirs [dbidx] * sizeof (unsigned long *));
            if (dirs [dbidx] == NULL) {
                goto end;
            }
            flags [dbidx] = malloc (nodirs [dbidx]);
            if (flags [dbidx] == NULL) {
                goto end;
            }
        }
    }
    if (debug >= 3) {
        printf ("%d: %ld dirs %ld dirs\n", __LINE__, nodirs [0], nodirs [1]);
    }

    /*  now we have the number of directories in each dirs [] in
        nodirs [dbidx & 1];
        index (in nodirs []) to start of each group of directory entries
        in dirs [dbidx & 1] [] */


    /*  allocate an array for a pair of index into the same dir in both lists */
    inboth = malloc (nodirs [0] * 2 * sizeof (unsigned long));
    if (inboth == NULL) {
        /*  out of memory */
        printf ("%d: malloc (%ld) failed\n", __LINE__,
            nodirs [0] * 2 * sizeof (unsigned long));
        goto end;
    }

    /*  clear flags [] [] to mean dir not in the other list */
    for (dbidx = 0; dbidx < 2; dbidx++) {
        for (i = 0; i < nodirs [dbidx]; i++) {
            flags [dbidx] [i] = 0;  /*  default to not in */
        }
    }

    /*  number of directory common in both set to zero */
    noinboth = 0;
    /*  for each dir in this list */
    for (i = 0; i < nodirs [0]; i++) {
        /*  assume this dir is not in that */
        flags [0] [i] = 0;  /*  not in 1 */
        /*  loops through every dir in that */
        for (j = 0; j < nodirs [1]; j++) {
            /*  get address of both dir entry */
            d  = dirbuf [0] + dirs [0] [i];
            d2 = dirbuf [1] + dirs [1] [j];

            if (debug >= 5) {
                printf ("%d: %ld %ld\n", __LINE__, i, j);
            }

            /*  are dir name the same? */
            if (dirtstrcmp (dnamebuf [0] +  d->dirid,
                        dnamebuf [1] + d2->dirid) == 0) {
                /*  dir exist in both */
                /*  make as in both */
                flags [0] [i] = 1;  /*  in 1 */
                flags [1] [j] = 1;  /*  in 1 */
                /*  record index to both dir in both */
                inboth [noinboth++] = dirs [0] [i];
                inboth [noinboth++] = dirs [1] [j];
            }
        }
    }

    if (outputfmt == OUTPUTCPY) {
        /*  print directory commands for copy mode */
        for (dbidx = 0; dbidx <= 1; dbidx++) {
            /*  looping starting from 1; 0 is base, must exist in both */
            for (i = 1; i < nodirs [dbidx]; i++) {
                if (flags [dbidx] [i] == 0) {
                    /*  in dnidx only */
                    /*  access directories */
                    d  = dirbuf [dbidx] + dirs [dbidx] [i];
                    /*  output directory copy command */
                    if (dbidx == 0) {
                        /*  if in this only, delete it. */
                        printf ("del %s%s*.*\n", bdir0, dnamebuf [dbidx] + d->dirid);
                        printf ("rd  %s%s.\n", bdir0, dnamebuf [dbidx] + d->dirid);
                    } else {
                        /*  if in that only, make it. */
                        printf ("md %s%s.\n", bdir0, dnamebuf [dbidx] + d->dirid);
                    }
                }
            }
        }
    }

    /*  inboth [noinboth++] has pairs of index to common dir in both */

    if (outputfmt == OUTPUTTXT) {
        puts ("  *****************************************************************");
        switch (cmd) {
            default            : 
                puts ("    unknown");
                break;
            case CMD_SAME      : 
                puts ("    same");
                break;
#if MDSMSG
            case CMD_DIFF      : 
                puts ("    different, date date/time stamp buf different size or CRC");
                break;
            case CMD_THIS_ONLY : 
                puts (
                    "    The following files are present in the reference but not in your directory:\n"
                    "    - This implies that an update is available and should be applied!"
                );
                break;
            case CMD_THAT_ONLY : 
                puts (
                    "    The following files are present in your directory but not in the update.\n"
                    "    - An update may have deleted these files.  Please investigate."
                );
                break;
            case CMD_THIS_NEW  : 
                puts (
                    "    The following files are present in both directories but have newer date\n"
                    "    stamps in the reference.\n"
                    "    - This implies that an update is available and should be applied!"
                );
                break;
            case CMD_THAT_NEW  : 
                puts (
                    "    The following files are present in both directories but have newer date \n"
                    "    stamps in your code.\n"
                    "    - This implies you have updated your copy, and you should investigate \n"
                    "    whether the modification remains appropriate."
                );
                break;
#else
            case CMD_DIFF      : 
                puts ("    different");
                break;
            case CMD_THIS_ONLY : 
                puts ("    this only");
                break;
            case CMD_THAT_ONLY : 
                puts ("    that only");
                break;
            case CMD_THIS_NEW  : 
                puts ("    this new");
                break;
            case CMD_THAT_NEW  : 
                puts ("    that new");
                break;
#endif
        }
    }
    for (i = 0; i < noinboth; i += 2) {
        ptr [0] = inboth [i];
        ptr [1] = inboth [i + 1];

        /*  access directories */
        d  = dirbuf [0] + ptr [0];
        d2 = dirbuf [1] + ptr [1];

        /*  save name of directories */
        d = dirbuf [0] + inboth [i];
        thisdir [0] = dnamebuf [0] + d->dirid;
        d = dirbuf [1] + inboth [i + 1];
        thisdir [1] = dnamebuf [1] + d->dirid;
        if (debug >= 3) {
            printf ("%d: thisdir '%s' thatdir '%s'\n", __LINE__, thisdir [0], thisdir [1]);
        }

        /*  scan until both directories are completely covered */
        endflag = 0;
        while (endflag != 3) {
            /*  check for end condition for both directories */
            for (dbidx = 0; dbidx < 2; dbidx++) {
                /*  perform checks if not at the end */
                if ((endflag & (1 << dbidx)) == 0) {
                    /*  still to go, check end */
                    /*  at the end of dirbuf? */
                    if (ptr [dbidx] >= dirptr [dbidx]) {
                        /*  reached the end of the end of dirbuf [], stop */
                        endflag |= 1 << dbidx;
                        /*  continue does not work because of for loop */
                        goto cont;
                    }
                    /*  not at the end of dirbuf */
                    /*  get to dir entry */
                    d  = dirbuf [dbidx] + ptr [dbidx];
                    /*  p is name of dir of this entry */
                    p = dnamebuf [dbidx] + d->dirid;
                    if (dirtstrcmp (p, thisdir [dbidx]) != 0) {
                        /*  reached different dname */
                        endflag |= 1 << dbidx;
                        /*printf ("diff dir: >%s< >%s< endflag %d %s %d\n", p, q, endflag, __FILE__, __LINE__);*/
                        /*  continue does not work because of for loop */
                        goto cont;
                    }
                }
            }
            /*  we are pointing to two new entry, possibly the end */
            if ((endflag & 1) == 0) {
                d  = dirbuf [0] + ptr [0];
                fname [0] = fnamebuf [0] +  d->fnameidx;
                dname [0] = dnamebuf [0] +  d->dirid;
                strcpy (date1, xctime (&d->tt));
                date1 [strlen (date1) - 1] = '\0';
            }
            if ((endflag & 2) == 0) {
                d2 = dirbuf [1] + ptr [1];
                fname [1] = fnamebuf [1] + d2->fnameidx;
                dname [1] = dnamebuf [1] + d2->dirid;
                strcpy (date2, xctime (&d2->tt));
                date2 [strlen (date2) - 1] = '\0';
            }

/*printf ("%d %ld %ld\n", __LINE__, ptr [0], ptr [1]);*/
            if (endflag == 0) {
                /*  neither directory has been exhausted */
                /*  look at next entry in both directory */
                cmprst = dirtstrcmp (fname [0], fname [1]);
                if (cmprst == 0) {
/*---------------------------------------------------------------------------
 -  output below
 -  exist in both
 ----------------------------------------------------------------------------*/
                    if ((d->tt == d2->tt) ||
                        (d->tt == (d2->tt + NTtimediff)) ||
                        (nodate != 0)) {
                        /*  same crc + date/time too? */
                        if ((d->size != d2->size) ||
                            ((calccrc != 0) && (d->crc != d2->crc))) {
                            /*  diff */
                            if (((cmd & (CMD_DIFF)) != 0) ||
                                ((nodate != 0) && ((cmd & (CMD_THIS_NEW | CMD_THAT_NEW)) != 0))) {
                                if ((exclude_ext (fname [0]) == 0) &&
                                    (exclude_dir (dname [0]) == 0)) {
                                    /*  did not match exclusion list */
                                    /*  output: # */
#if 0
                                    output (CMD_DIFF, cmd, dname [0], fname [0], bdir0, bdir1, d, d2);
#else
                                    if (nodate == 0) {
                                        /* check date, output normally */
                                        output (CMD_DIFF, cmd, dname [0], fname [0], bdir0, bdir1, d, d2, 0);
                                    } else {
                                        /* no date, still out date symbolically */
                                        if ((d->tt == d2->tt) ||
                                            (d->tt == (d2->tt + NTtimediff))) {
                                            output (CMD_DIFF, cmd, dname [0], fname [0], bdir0, bdir1, d, d2, 0);
                                        } else if (d->tt > d2->tt) {
                                            output (CMD_THIS_NEW, cmd, dname [0], fname [0], bdir0, bdir1, d, d2, 0);
                                        } else {
                                            output (CMD_THAT_NEW, cmd, dname [0], fname [0], bdir0, bdir1, d, d2, 0);
                                        }
                                    }
#endif
                                }
                            }
                        } else {
                            /*  identical */
                            /*  same */
                            if ((cmd & (CMD_SAME)) != 0) {
                                if ((exclude_ext (fname [0]) == 0) &&
                                    (exclude_dir (dname [0]) == 0)) {
                                    /*  did not match exclusion list */
                                    /*  output: = */
                                    output (CMD_SAME, cmd, dname [0], fname [0], bdir0, bdir1, d, NULL, 0);
                                }
                            }
                        }
                    } else
                    if (d->tt > d2->tt) {
                        /*  this newer */
                        if ((cmd & (CMD_THIS_NEW)) != 0) {
                            if ((exclude_ext (fname [0]) == 0) &&
                                (exclude_dir (dname [0]) == 0)) {
                                /*  did not match exclusion list */
                                /*  output: > */
                                output (CMD_THIS_NEW, cmd, dname [0], fname [0], bdir0, bdir1, d, d2, 0);
                            }
                        }
                    } else
                    if (d->tt < d2->tt) {
                        /*  that newer */
                        if ((cmd & (CMD_THAT_NEW)) != 0) {
                            if ((exclude_ext (fname [0]) == 0) &&
                                (exclude_dir (dname [0]) == 0)) {
                                /*  did not match exclusion list */
                                /*  output: < */
                                output (CMD_THAT_NEW, cmd, dname [0], fname [0], bdir0, bdir1, d, d2, 0);
                            }
                        }
                    }


/*---------------------------------------------------------------------------
 -  output above
 ----------------------------------------------------------------------------*/
                    ptr [0]++;
                    ptr [1]++;
                } else if (cmprst > 0) {
/*---------------------------------------------------------------------------
 -  output below
 ----------------------------------------------------------------------------*/
                    /*  this not in that */
                    if ((cmd & (CMD_THAT_ONLY)) != 0) {
                        if ((exclude_ext (fname [1]) == 0) &&
                            (exclude_dir (dname [1]) == 0)) {
                            /*  did not match exclusion list */
                            /*  output: 2 */
                            output (CMD_THAT_ONLY, cmd, dname [1], fname [1], bdir0, bdir1, d2, NULL, 0);
                        }
                    }
/*---------------------------------------------------------------------------
 -  output above
 ----------------------------------------------------------------------------*/
                    ptr [1]++;
                } else {
/*---------------------------------------------------------------------------
 -  output below
 ----------------------------------------------------------------------------*/
                    /*  that not in this */
                    if ((cmd & (CMD_THIS_ONLY)) != 0) {
                        if ((exclude_ext (fname [0]) == 0) &&
                            (exclude_dir (dname [0]) == 0)) {
                            /*  did not match exclusion list */
                            /*  output: 1 */
                            output (CMD_THIS_ONLY, cmd, dname [0], fname [0], bdir0, bdir1, d, NULL, 0);
                        }
                    }
/*---------------------------------------------------------------------------
 -  output above
 ----------------------------------------------------------------------------*/
                    ptr [0]++;
                }
            } else {
/*---------------------------------------------------------------------------
 -  output below
 -  These are this or that only
 ----------------------------------------------------------------------------*/
                if ((endflag & 1) == 0) {
                    if ((cmd & (CMD_THIS_ONLY)) != 0) {
                        if ((exclude_ext (fname [0]) == 0) &&
                            (exclude_dir (dname [0]) == 0)) {
                            /*  did not match exclusion list */
                            /*  output: 1 */
                            output (CMD_THIS_ONLY, cmd, dname [0], fname [0], bdir0, bdir1, d, NULL, 0);
                        }
                    }
                    ptr [0]++;
                }
                if ((endflag & 2) == 0) {
                    if ((cmd & (CMD_THAT_ONLY)) != 0) {
                        if ((exclude_ext (fname [1]) == 0) &&
                            (exclude_dir (dname [1]) == 0)) {
                            /*  did not match exclusion list */
                            /*  output: 2 */
                            output (CMD_THAT_ONLY, cmd, dname [1], fname [1], bdir0, bdir1, d2, NULL, 0);
                        }
                    }
                    ptr [1]++;
                }
/*---------------------------------------------------------------------------
 -  output above
 ----------------------------------------------------------------------------*/
            }
            cont :
                ;
        }
    }


/*---------------------------------------------------------------------------
 -  selection below
 -  These are this or that only
 ----------------------------------------------------------------------------*/
    /*  list directories not common to both */
    k = cmd & (CMD_THIS_ONLY | CMD_THAT_ONLY);
    if (k == (CMD_THIS_ONLY | CMD_THAT_ONLY)) {
        dbidxst = 0;
        dbidxen = 1;
    } else
    if (k == CMD_THIS_ONLY) {
        dbidxst = 0;
        dbidxen = 0;
    } else
    if (k == CMD_THAT_ONLY) {
        dbidxst = 1;
        dbidxen = 1;
    } else {
        dbidxst = 0;
        dbidxen = -1;
    }
    if (debug >= 3) {
        printf ("%d: dbidxst %d dbidxen %d\n", __LINE__, dbidxst, dbidxen);
    }

/*---------------------------------------------------------------------------
 -  selection above
 ----------------------------------------------------------------------------*/
    for (dbidx = dbidxst; dbidx <= dbidxen; dbidx++) {
        for (i = 0; i < nodirs [dbidx]; i++) {
            if (flags [dbidx] [i] == 0) {
                /*  in dnidx only */
                /*  access directories */
                d  = dirbuf [dbidx] + dirs [dbidx] [i];
                /*  dirout */
                if (debug > 0) {
                    printf ("Sub-directory '%s' is in '%s' but not in '%s'\n",
                        dnamebuf [dbidx] + d->dirid,
                        (dbidx == 0) ? datafile  : datafile2,
                        (dbidx == 0) ? datafile2 : datafile);
                }

                /*  save name of directories */
                thisdir [0] = dnamebuf [dbidx] + d->dirid;

                j = dirs [dbidx] [i];
                while (j < dirptr [dbidx]) {
                    /*  get to dir entry */
                    d  = dirbuf [dbidx] + j;
                    /*  p is name of dir of this entry */
                    p = dnamebuf [dbidx] + d->dirid;
                    if (dirtstrcmp (p, thisdir [0]) != 0) {
                        /*  reached different dname */
                        break;
                    }

                    /*  we are pointing to two new entry */
                    fname [0] = fnamebuf [dbidx] + d->fnameidx;
                    dname [0] = dnamebuf [dbidx] + d->dirid;
                    strcpy (date1, xctime (&d->tt));
                    date1 [strlen (date1) - 1] = '\0';

/*---------------------------------------------------------------------------
 -  output below
 -  These are this or that only
 ----------------------------------------------------------------------------*/
                    if ((cmd & (CMD_THIS_ONLY | CMD_THAT_ONLY)) != 0) {
                        if ((exclude_ext (fname [0]) == 0) &&
                            (exclude_dir (dname [0]) == 0)) {
                            /*  did not match exclusion list */
                            /*  output: 1 */
                            /*  output: 2 */
                            output ((dbidx == 0) ? CMD_THIS_ONLY : CMD_THAT_ONLY,
                                cmd, dname [0], fname [0], bdir0, bdir1, d, NULL, 0);
                        }
                    }
/*---------------------------------------------------------------------------
 -  output above
 ----------------------------------------------------------------------------*/
                    j++;
                }
            }
        }
    }


    if (outputfmt == OUTPUTTXT) {
        printf ("    %ld common directories checked.\n    %ld files ", noinboth / 2, nooutput - lastnooutput);
        switch (cmd) {
            default            : 
                puts ("unknown");
                break;
            case CMD_SAME      : 
                puts ("same in both");
                break;
            case CMD_DIFF      : 
                puts ("different but same date/time?");
                break;
            case CMD_THIS_ONLY : 
                printf ("exist only in this (%s)\n", datafile);
                break;
            case CMD_THAT_ONLY : 
                printf ("exist only in that (%s)\n", datafile2);
                break;
            case CMD_THIS_NEW  : 
                printf ("this is newer (%s)\n", datafile);
                break;
            case CMD_THAT_NEW  : 
                printf ("that is newer (%s)\n", datafile2);
                break;
        }
    }

    end :
    if (dirs [0] != NULL) {
        free (dirs [0]);
    }
    if (dirs [1] != NULL) {
        free (dirs [1]);
    }
    if (flags [0] != NULL) {
        free (flags [0]);
    }
    if (flags [1] != NULL) {
        free (flags [1]);
    }
    if (inboth != NULL) {
        free (inboth);
    }

} /* diff2 */


