/*
 * Copyright (c) 2008-2012 l00g33k@gmail.com All rights reserved.
 *
 */

#include "dirtree.h"

#define FACTORING 1

static int *cmpflgl, *cmpflgr, cmpflgsz;

static int qsortdupcmpnodir (int usename, int idx0, const void *a, int idx1, const void *b);
char *strdelslash (char *ptr);

static int numsamename (int idx, unsigned long ptr) {
    char HUGE *first;
    int numsame;
    unsigned long nxt;

    numsame = 1;
    first = fnamebuf [idx] + dirbuf [idx] [ptr].fnameidx;
    for (nxt = ptr + 1; nxt < dirptr [idx]; nxt++) {
        if (0 != dirtstrcmp ((char *) first,
            (char *) (fnamebuf [idx] + dirbuf [idx] [nxt].fnameidx))) {
            break;
        }
        numsame++;
    }

    return (numsame);

} /* numsamename */


static int numnonaidentical (int idx, unsigned long ptr) {
    char HUGE *first;
    int identical;
    unsigned long nxt;

    identical = 1;
    for (nxt = ptr + 1; nxt < dirptr [idx]; nxt++) {
        if (0 != qsortdupcmpnodir (0, idx, dirbuf [idx] + ptr, 
                                      idx, dirbuf [idx] + nxt)) {
            break;
        }
        identical++;
    }

    return (identical);

} /* numnonaidentical */

static int numidentical (int idx, unsigned long ptr) {
    int identical;
    unsigned long nxt;

    identical = 1;
    for (nxt = ptr + 1; nxt < dirptr [idx]; nxt++) {
        if (0 != qsortdupcmpnodir (1, idx, dirbuf [idx] + ptr, 
                                      idx, dirbuf [idx] + nxt)) {
            break;
        }
        identical++;
    }

    return (identical);

} /* numidentical */


static void skipdup (int outputfmt, int idx, struct dirlist HUGE **dptr, unsigned long *ptr) {
    int cmp, cnt = 0;
    unsigned long ptr1st, pnxt;
    struct dirlist HUGE *d1st, *dnxt;


    ptr1st = *ptr;
    d1st = *dptr;

    pnxt = *ptr + 1;

    while (1) {
        if (pnxt >= dirptr [idx]) {
            break;
        }
        dnxt = dirbuf [idx] + pnxt;
        cmp = qsortdupcmpnodir (1, idx, d1st, idx, dnxt);
        if (cmp != 0) {
            break;
        }
        if (cnt == 0) {
            if (outputfmt == OUTPUTCPY) {
                printf ("rem ");
            }
            printf ("%s%d %-s%-s\n", (idx == 0 ? "[ " : " ]"), cnt++,
                dnamebuf [idx] + d1st->dirid, fnamebuf [idx] + d1st->fnameidx);
        }
        if (outputfmt == OUTPUTCPY) {
            printf ("rem ");
        }
        printf ("%s%d %-s%-s\n", (idx == 0 ? "[ " : " ]"), cnt++,
            dnamebuf [idx] + dnxt->dirid, fnamebuf [idx] + dnxt->fnameidx);
        pnxt++;
    }
    if (cnt > 1) {
        *ptr = pnxt - 1;
        *dptr = dirbuf [idx] + *ptr;
    }
} /* skipdup */



void samecmp_samedir (int cmd, int outputfmt, struct dirlist HUGE *d, struct dirlist HUGE *d2) {
    if (cmd & CMD_SAME) {
        if (outputfmt == OUTPUTCPY) {
        } else {
            printf ("==  %-20s %-20s\n",
                fnamebuf [0] + d->fnameidx,
                dnamebuf [1] + d2->dirid);
            nooutput++;
        }
    }
} /* samecmp_samedir */


void samecmp_diffdir (int cmd, char *bdir0, int outputfmt, struct dirlist HUGE *d, struct dirlist HUGE *d2) {
    if (cmd & CMD_DIFF) {
        if (outputfmt == OUTPUTCPY) {
            printf ("call move %s%s%s %s\n",
                    bdir0,
                    dnamebuf [0] + d->dirid,
                    fnamebuf [0] + d->fnameidx,
                    strdelslash (dnamebuf [1] + d2->dirid));
            nooutput++;
        } else {
            printf ("m=  %-20s 1)%-20s -> 2)%-20s\n",
                fnamebuf [0] + d->fnameidx,
                dnamebuf [0] + d->dirid,
                dnamebuf [1] + d2->dirid);
            nooutput++;
        }
    }
} /* samecmp_diffdir */


void diffcmp_samedir_samefile (int cmd, int outputfmt, struct dirlist HUGE *d, struct dirlist HUGE *d2) {

    if (d->tt == d2->tt) {
/*--------------------------------------------------------------------------
-  in both diff name, size, CRC, time but same name same dir, same date
---------------------------------------------------------------------------*/
        if (cmd & CMD_DIFF) {
            if (outputfmt == OUTPUTCPY) {
            } else {
                printf ("#C  %-20s %-20s\n",
                    fnamebuf [0] + d->fnameidx,
                    dnamebuf [0] + d->dirid);
                nooutput++;
            }
        }
    } else if (d->tt > d2->tt) {
/*--------------------------------------------------------------------------
-  in both diff name, size, CRC, time but same name same dir, this newer
---------------------------------------------------------------------------*/
        if (cmd & CMD_THIS_NEW) {
            if (outputfmt == OUTPUTCPY) {
            } else {
                printf ("d>  %-20s %-20s\n",
                    fnamebuf [0] + d->fnameidx,
                    dnamebuf [0] + d->dirid);
                nooutput++;
            }
        }
    } else {
/*--------------------------------------------------------------------------
-  in both diff name, size, CRC, time but same name same dir, that newer
---------------------------------------------------------------------------*/
        if (cmd & CMD_THAT_NEW) {
            if (outputfmt == OUTPUTCPY) {
            } else {
                printf ("d<  %-20s %-20s\n",
                    fnamebuf [0] + d->fnameidx,
                    dnamebuf [0] + d->dirid);
                nooutput++;
            }
        }
    }
} /* diffcmp_samedir_samefile */

#if 0
#if FACTORING
                            printf ("%s %d: impossible to be here\n", __FILE__, __LINE__);
                            exit (1);
#else
                            /* must have been modified */
                            if (outputfmt == OUTPUTCPY) {
                            } else {
                                printf ("??  %-20s %-20s %-20s\n",
                                    fnamebuf [0] + d->fnameidx,
                                    dnamebuf [0] + d->dirid,
                                    dnamebuf [1] + d2->dirid);
                                nooutput++;
                            }
#endif
#endif


void diffcmp_diffdir_samefile (int cmd, int outputfmt, struct dirlist HUGE *d, struct dirlist HUGE *d2) {
    if (d->tt == d2->tt) {
        if (outputfmt == OUTPUTCPY) {
        } else {
            printf ("m#  %-20s 1)%-20s -> 2)%-20s\n",
                fnamebuf [0] + d->fnameidx,
                dnamebuf [0] + d->dirid,
                dnamebuf [1] + d2->dirid);
            nooutput++;
        }
    } else if (d->tt < d2->tt) {
        if (outputfmt == OUTPUTCPY) {
        } else {
            printf ("m<  %-20s 1)%-20s -> 2)%-20s\n",
                fnamebuf [0] + d->fnameidx,
                dnamebuf [0] + d->dirid,
                dnamebuf [1] + d2->dirid);
            nooutput++;
        }
    } else {
        if (outputfmt == OUTPUTCPY) {
        } else {
            printf ("m>  %-20s 1)%-20s -> 2)%-20s\n",
                fnamebuf [0] + d->fnameidx,
                dnamebuf [0] + d->dirid,
                dnamebuf [1] + d2->dirid);
            nooutput++;
        }
    }
} /* diffcmp_diffdir_samefile */


void highcmp__difffile (int cmd, int outputfmt, struct dirlist HUGE *d, struct dirlist HUGE *d2) {
    /*  diff name, that only */
    if (cmd & CMD_THAT_ONLY) {
        if (outputfmt == OUTPUTCPY) {
        } else {
            printf (" 2  %-20s %-20s\n",
                fnamebuf [1] + d2->fnameidx,
                dnamebuf [1] + d2->dirid);
            nooutput++;
        }
    }
} /* highcmp__difffile */


void lowcmp__difffile (int cmd, char *bdir0, int outputfmt, struct dirlist HUGE *d, struct dirlist HUGE *d2) {
    /*  diff name, this only */
    if (cmd & CMD_THIS_ONLY) {
        if (outputfmt == OUTPUTCPY) {
            printf ("del       %s%s%s\n",
                    bdir0,
                    dnamebuf [0] + d->dirid,
                    fnamebuf [0] + d->fnameidx);
            nooutput++;
        } else {
            printf ("1   %-20s %-20s\n",
                fnamebuf [0] + d->fnameidx,
                dnamebuf [0] + d->dirid);
            nooutput++;
        }
    }
} /* lowcmp__difffile */


void diff2mv (int cmd, char *bdir0, char *bdir1, int outputfmt) {
/*****************************************************************************
 *
 *  Compares two directory trees, finds duplicate too
 *
 *****************************************************************************/
    int cmp, dircmp, filecmp, found, database, ii,
        nolt, nort, lidx, ridx;
    unsigned long ptr0, ptr1;
    long int lastnooutput;
    struct dirlist HUGE *d, HUGE *d2;
    char HUGE *dname0, HUGE *dname1;
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


    /*  sort by filename for duplicate check */
    /*  sort order: file, size, crc, date/time, dirname */
    if (printmove == 2) {
        dirsort (0, SORTBYDUPNONM);
    } else {
        dirsort (0, SORTBYDUP);
        dirsort (1, SORTBYDUP);
    }


    /*  allocate compare flag buffer */
    if (cmpflgsz == 0) {
        cmpflgsz = 100;
        cmpflgl = malloc (cmpflgsz * 2 * sizeof (int));
        if (cmpflgl == NULL) {
            fprintf (stderr, "*** failed to malloc (%d)\n",
                cmpflgsz * 2 * sizeof (int));
            return;
        }
        cmpflgr = cmpflgl + cmpflgsz;
    }

    if (cmd == -2) {
        goto deldir;
    }
    if (cmd == -1) {
        if (debug >= 5) {
            printf ("Database dump:\n"
                "database,index,filename,size,CRC,date/time stamp,directory name\n");
            for (database = 0; database < 2; database++) {
                for (ptr0 = 0; ptr0 < dirptr [database]; ptr0++) {
                    d  = dirbuf [database] + ptr0;
                    printf ("%d,%lu,%s,%lu,%08lx,%08lx,%s\n",
                        database, 
                        ptr0, 
                        fnamebuf [database] + d->fnameidx,
                        d->size,
                        d->crc,
                        d->tt,
                        dnamebuf [database] + d->dirid
                    );
                }
            }
        }

/*--------------------------------------------------------------------------
 -  print dir in 1 only
 ---------------------------------------------------------------------------*/
        /*  for all dirname in 1 */
        for (dname1 = dnamebuf [1];
             dname1 < dnamebuf [1] + dnameptr [1];
             dname1 += strlen (dname1) + 1) {
            found = 0;
            /*  for all dirname in 0 */
            for (dname0 = dnamebuf [0];
                 dname0 < dnamebuf [0] + dnameptr [0];
                 dname0 += strlen (dname0) + 1) {
                /*  compare dirname */
                if (dirtstrcmp (dname0, dname1) == 0) {
                    /*  dirnames equal */
                    found = 1;
                    break;
                }
            }
            if (found == 0) {
                /*  dname1 is in 1 but not in 0 */
                if (outputfmt == OUTPUTCPY) {
                    /*  create dir so 0 looks like 1 */
                    printf ("md \"%s\"\n", strdelslash (dname1));
                } else {
                    printf ("dir '%-20s' is not in this\n", dname1);
                }
            }
        }
        return;
    }

    if ((outputfmt == OUTPUTTXT) || (outputfmt == OUTPUTCPY)) {
        if (outputfmt == OUTPUTCPY) {
            printf ("@echo off\nrem ");
        }
        puts ("  *****************************************************************");
        if (outputfmt == OUTPUTCPY) {
            printf ("rem ");
        }
        switch (cmd) {
            default            : 
                puts ("    unknown");
                break;
            case CMD_SAME      : 
                printf ("    same [%d]\n", CMD_SAME);
                break;
            case CMD_DIFF      : 
                printf ("    different [%d]\n", CMD_DIFF);
                break;
            case CMD_THIS_ONLY : 
                printf ("    this only [%d]\n", CMD_THIS_ONLY);
                break;
            case CMD_THAT_ONLY : 
                printf ("    that only [%d]\n", CMD_THAT_ONLY);
                break;
            case CMD_THIS_NEW  : 
                printf ("    this new [%d]\n", CMD_THIS_NEW);
                break;
            case CMD_THAT_NEW  : 
                printf ("    that new [%d]\n", CMD_THAT_NEW);
                break;
            case CMD_THIS_MOVE  : 
                printf ("    this moved to that [%d]\n", CMD_THIS_MOVE);
                break;
            case CMD_THAT_MOVE  : 
                printf ("    that moved from this [%d]\n", CMD_THAT_MOVE);
                break;
            case CMD_THIS_DUP  : 
                printf ("    duplicated in this [%d]\n", CMD_THIS_DUP);
                break;
            case CMD_THAT_DUP  : 
                printf ("    duplicated in that [%d]\n", CMD_THAT_DUP);
                break;
        }
    }

    if ((cmd & CMD_THIS_DUP) != 0) {
        ptr0 = 0;
        if (printmove == 2) {
            /*  no name dup */
            while (ptr0 < dirptr [0]) {
                nolt = numnonaidentical (0, ptr0);
                d  = dirbuf [0] + ptr0;
                if (exclude_ext (fnamebuf [0] +  d->fnameidx) == 0) {
                    /*  this fname is not excluded */
                    if (nolt > 1) {
                        for (lidx = 0; lidx < nolt; lidx++) {
                            output (CMD_THIS_DUP, cmd, 
                                dnamebuf [0] + dirbuf [0] [ptr0 + lidx].dirid, 
                                fnamebuf [0] + dirbuf [0] [ptr0 + lidx].fnameidx,
                                bdir0, bdir1, &dirbuf [0] [ptr0 + lidx], NULL, lidx + 1);
                        }
                    }
                }
                ptr0 += nolt;
            }
            goto skip;
        } else {
            while (ptr0 < dirptr [0]) {
                nolt = numidentical (0, ptr0);
                d  = dirbuf [0] + ptr0;
                if (exclude_ext (fnamebuf [0] +  d->fnameidx) == 0) {
                    /*  this fname is not excluded */
                    if (nolt > 1) {
                        for (lidx = 0; lidx < nolt; lidx++) {
                            output (CMD_THIS_DUP, cmd, 
                                dnamebuf [0] + dirbuf [0] [ptr0 + lidx].dirid, 
                                fnamebuf [0] + dirbuf [0] [ptr0 + lidx].fnameidx,
                                bdir0, bdir1, &dirbuf [0] [ptr0 + lidx], NULL, lidx + 1);
                        }
                    }
                }
                ptr0 += nolt;
            }
        }
    }
    if ((cmd & CMD_THAT_DUP) != 0) {
        ptr1 = 0;
        while (ptr1 < dirptr [1]) {
            nort = numidentical (1, ptr1);
            d2 = dirbuf [1] + ptr1;
            if (exclude_ext (fnamebuf [1] + d2->fnameidx) == 0) {
                /*  this fname is not excluded */
                if (nort > 1) {
                    for (ridx = 0; ridx < nort; ridx++) {
                        output (CMD_THAT_DUP, cmd, 
                            dnamebuf [1] + dirbuf [1] [ptr1 + ridx].dirid, 
                            fnamebuf [1] + dirbuf [1] [ptr1 + ridx].fnameidx,
                            bdir1, bdir0, &dirbuf [1] [ptr1 + ridx], NULL, ridx + 1);
                    }
                }
            }
            ptr1 += nort;
        }
    }

    ptr0 = 0;
    ptr1 = 0;
    /*  loops thruogh files in both dirs in pairs */
/*--------------------------------------------------------------------------
 -  iterate both lists
 ---------------------------------------------------------------------------*/
    while ((ptr0 < dirptr [0]) || (ptr1 < dirptr [1])) {
        d  = dirbuf [0] + ptr0;
        d2 = dirbuf [1] + ptr1;

/*--------------------------------------------------------------------------
 -  check for extension or directory exclusion in 0
 ---------------------------------------------------------------------------*/
        if (ptr0 < dirptr [0]) {
            /*  still dirnames in 0 */
            if (exclude_ext (fnamebuf [0] +  d->fnameidx) != 0) {
                /*  this fname is excluded */
                nolt = numsamename (0, ptr0);
                ptr0 += nolt;
                continue;
            }
        }
/*--------------------------------------------------------------------------
 -  check for extension or directory exclusion in 1
 ---------------------------------------------------------------------------*/
        if (ptr1 < dirptr [1]) {
            /*  still dirnames in 1 */
            if (exclude_ext (fnamebuf [1] + d2->fnameidx) != 0) {
                /*  this fname is excluded */
                nort = numsamename (1, ptr1);
                ptr1 += nort;
                continue;
            }
        }


        if ((ptr0 < dirptr [0]) && (ptr1 < dirptr [1])) {
/*--------------------------------------------------------------------------
 -  in both
 ---------------------------------------------------------------------------*/
            filecmp = dirtstrcmp (fnamebuf [0] +  d->fnameidx,
                                  fnamebuf [1] + d2->fnameidx);
            nolt = numsamename (0, ptr0);
            nort = numsamename (1, ptr1);
            if (filecmp < 0) {
                /*  left unique filename */
                if ((cmd & CMD_THIS_ONLY) != 0) {
                    for (lidx = 0; lidx < nolt; lidx++) {
                        output (CMD_THIS_ONLY, cmd, 
                            dnamebuf [0] + dirbuf [0] [ptr0 + lidx].dirid, 
                            fnamebuf [0] + dirbuf [0] [ptr0 + lidx].fnameidx,
                            bdir0, bdir1, &dirbuf [0] [ptr0 + lidx], NULL, 0);
                    }
                }
                ptr0 += nolt;
            } else if (filecmp > 0) {
                /*  right unique filename */
                if ((cmd & CMD_THAT_ONLY) != 0) {
                    for (ridx = 0; ridx < nort; ridx++) {
                        output (CMD_THAT_ONLY, cmd, 
                            dnamebuf [1] + dirbuf [1] [ptr1 + ridx].dirid, 
                            fnamebuf [1] + dirbuf [1] [ptr1 + ridx].fnameidx,
                            bdir0, bdir1, &dirbuf [1] [ptr1 + ridx], NULL, 0);
                    }
                }
                ptr1 += nort;
            } else {
                ii = max (nolt, nort);
                if (cmpflgsz < ii) {
                    cmpflgsz += ii;
                    cmpflgl = realloc (cmpflgl, cmpflgsz * 2 * sizeof (int));
                    if (cmpflgl == NULL) {
                        fprintf (stderr, "*** failed to malloc (%d)\n",
                            cmpflgsz * 2 * sizeof (int));
                        return;
                    }
                    cmpflgr = cmpflgl + cmpflgsz;
                }
                memset (cmpflgl, 0xff, nolt * sizeof (int));
                memset (cmpflgr, 0xff, nort * sizeof (int));
                /*  filename in both */
                /*  find moved */
                for (lidx = 0; lidx < nolt; lidx++) {
                    for (ridx = 0; ridx < nort; ridx++) {
                        d  = dirbuf [0] + ptr0 + lidx;
                        d2 = dirbuf [1] + ptr1 + ridx;
                        /*  cmp: filename, size, crc, date/time */
                        cmp = qsortdupcmpnodir (1, 0, d, 1, d2);
                        dircmp = dirtstrcmp (dnamebuf [0] + d->dirid,
                                 dnamebuf [1] + d2->dirid);
                        if ((dircmp != 0) && (cmp == 0)) {
                            /*  same sig, diff dir */
                            cmpflgl [lidx] = ridx;
                            cmpflgr [ridx] = lidx;
                        }
                    }
                }
                /*  find sam dir */
                for (lidx = 0; lidx < nolt; lidx++) {
                    for (ridx = 0; ridx < nort; ridx++) {
                        d  = dirbuf [0] + ptr0 + lidx;
                        d2 = dirbuf [1] + ptr1 + ridx;
                        /*  cmp: filename, size, crc, date/time */
                        cmp = qsortdupcmpnodir (1, 0, d, 1, d2);
                        dircmp = dirtstrcmp (dnamebuf [0] + d->dirid,
                                 dnamebuf [1] + d2->dirid);
                        if ((dircmp == 0) && (cmp == 0)) {
                            /*  same dir, sig */
                            cmpflgl [lidx] = -2;
                            cmpflgr [ridx] = -2;
                            if ((cmd & CMD_SAME) != 0) {
                                output (CMD_SAME, cmd, 
                                    dnamebuf [0] + d->dirid, 
                                    fnamebuf [0] + d->fnameidx,
                                    bdir0, bdir1, d, NULL, 0);
                            }
                        } else if (dircmp == 0) {
                            /*  same dir, diff sig */
                            cmpflgl [lidx] = -2;
                            cmpflgr [ridx] = -2;
                            if (d->tt > d2->tt) {
                                if ((cmd & CMD_THIS_NEW) != 0) {
                                    output (CMD_THIS_NEW, cmd, 
                                        dnamebuf [0] + d->dirid, 
                                        fnamebuf [0] + d->fnameidx,                            
                                        bdir0, bdir1, 
                                        d, d2, 0);
                                }
                            } else {
                                if ((cmd & CMD_THAT_NEW) != 0) {
                                    output (CMD_THAT_NEW, cmd, 
                                        dnamebuf [0] + d->dirid, 
                                        fnamebuf [0] + d->fnameidx,                            
                                        bdir0, bdir1, 
                                        d, d2, 0);
                                }
                            }
                        }
                    }
                }


                for (lidx = 0; lidx < nolt; lidx++) {
                    if (cmpflgl [lidx] == -1) {
                        if ((cmd & CMD_THIS_ONLY) != 0) {
                            output (CMD_THIS_ONLY, cmd, 
                                dnamebuf [0] + dirbuf [0] [ptr0 + lidx].dirid, 
                                fnamebuf [0] + dirbuf [0] [ptr0 + lidx].fnameidx,                            
                                bdir0, bdir1, 
                                &dirbuf [0] [ptr0 + lidx], NULL, 0);
                        }
                    } else if (cmpflgl [lidx] >= 0) {
                        ridx = cmpflgl [lidx];
                        if ((cmd & CMD_THIS_MOVE) != 0) {
                            output (CMD_THIS_MOVE, cmd, 
                                dnamebuf [0] + dirbuf [0] [ptr0 + lidx].dirid, 
                                fnamebuf [0] + dirbuf [0] [ptr0 + lidx].fnameidx,                            
                                bdir0, bdir1, 
                                &dirbuf [0] [ptr0 + lidx], 
                                &dirbuf [1] [ptr1 + ridx], 0);
                        }
                    }
                }
                for (ridx = 0; ridx < nort; ridx++) {
                    if (cmpflgr [ridx] == -1) {
                        if ((cmd & CMD_THAT_ONLY) != 0) {
                            output (CMD_THAT_ONLY, cmd, 
                                dnamebuf [1] + dirbuf [1] [ptr1 + ridx].dirid, 
                                fnamebuf [1] + dirbuf [1] [ptr1 + ridx].fnameidx,                            
                                bdir0, bdir1, 
                                &dirbuf [1] [ptr1 + ridx], NULL, 0);
                        }
                    } else if (cmpflgr [ridx] >= 0) {
                        lidx = cmpflgr [ridx];
                        if ((cmd & CMD_THAT_MOVE) != 0) {
                            output (CMD_THAT_MOVE, cmd, 
                                dnamebuf [0] + dirbuf [0] [ptr0 + lidx].dirid, 
                                fnamebuf [0] + dirbuf [0] [ptr0 + lidx].fnameidx,                            
                                bdir0, bdir1, 
                                &dirbuf [0] [ptr0 + lidx], 
                                &dirbuf [1] [ptr1 + ridx], 0);
                        }
                    }
                }
                ptr0 += nolt;
                ptr1 += nort;
            }
continue;

            skipdup (outputfmt, 0, &d , &ptr0);
            skipdup (outputfmt, 1, &d2, &ptr1);

            /*  cmp: filename, size, crc, date/time */
            cmp = qsortdupcmpnodir (0, 0, d, 1, d2);
            dircmp = dirtstrcmp (dnamebuf [0] + d->dirid,
                                 dnamebuf [1] + d2->dirid);
            filecmp = dirtstrcmp (fnamebuf [0] +  d->fnameidx,
                                  fnamebuf [1] + d2->fnameidx);

            if (debug >= 3) {
                if (ptr0 == 0) {
                    puts ("dirname    # filename cp dcp fcm dirname  # fname");
                }
                printf ("%-9s %2ld %-8s c%c d%c f%c %-9s %2ld %-8s |\n",
                    dnamebuf [0] +  d->dirid,
                    ptr0,
                    fnamebuf [0] +  d->fnameidx,
                    (cmp == 0) ? '=' : ((cmp > 0) ? '>' : '<'),
                    (dircmp == 0) ? '=' : ((dircmp > 0) ? '>' : '<'),
                    (filecmp == 0) ? '=' : ((filecmp > 0) ? '>' : '<'),
                    dnamebuf [1] + d2->dirid,
                    ptr1,
                    fnamebuf [1] + d2->fnameidx);
            }

/*--------------------------------------------------------------------------
 -  comparing
 ---------------------------------------------------------------------------*/
            if (cmp == 0) {
                /*  in both same name, size, CRC, time */
                if (dircmp == 0) {
                    /*  in both same name, size, CRC, time and same dir */
                    samecmp_samedir (cmd, outputfmt, d, d2);
                } else {
                    /*  in both same name, size, CRC, time but different dir */
                    samecmp_diffdir (cmd, bdir0, outputfmt, d, d2);
                }
                ptr0++;
                ptr1++;
            } else {
                /*  in both diff name, size, CRC, time */
                if ((dircmp == 0) && (filecmp == 0)) {
                    /*  in both diff name, size, CRC, time but same name same dir*/
                    diffcmp_samedir_samefile (cmd, outputfmt, d, d2);

                    /* normally in cmp != 0 case wee advance just
                       one side since we need to find match. Here we
                       have just print moved so both need to be
                       advanced.  The default cmp != 0 advance one side,
                       the other side is aadvanced here.
                     */
                    if (cmp < 0) {
                        ptr1++;
                    } else {
                        ptr0++;
                    }
                } else {
                    /*  in both diff name, size, CRC, time, diff name or dir */
                    if (filecmp == 0) {
                        if (dircmp == 0) {
                            /*  in both diff name, size, CRC, time, same name & dir (??) */
                            printf ("%s %d: impossible to be here\n", __FILE__, __LINE__);
                            exit (1);
                        } else {
                            /*  in both diff name, size, CRC, time, same name diff dir : moved */
                            diffcmp_diffdir_samefile (cmd, outputfmt, d, d2);

                            /* normally in cmp != 0 case wee advance just
                               one side since we need to find match. Here we
                               have just print moved so both need to be
                               advanced.  The default cmp != 0 advance one side,
                               the other side is aadvanced here.
                             */
                            if (cmp < 0) {
                                ptr1++;
                            } else {
                                ptr0++;
                            }
                        }
                    } else {
                        /*  in both diff name, size, CRC, time, diff name, this or that only */
                        if (cmp > 0) {
                            highcmp__difffile (cmd, outputfmt, d, d2);
                        } else {
                            lowcmp__difffile (cmd, bdir0, outputfmt, d, d2);
                        }
                    }
                }
    /*-------------------------------------------------*/
                if (cmp > 0) {
                    ptr1++;
                } else {
                    ptr0++;
                }
            }
        } else if (ptr0 < dirptr [0]) {
/*--------------------------------------------------------------------------
 -  file in 0/this only
 ---------------------------------------------------------------------------*/
            skipdup (outputfmt, 0, &d , &ptr0);
            if (cmd & CMD_THIS_ONLY) {
                if (outputfmt == OUTPUTCPY) {
                } else {
                    printf ("11  %-20s %-20s %-20s\n",
                        fnamebuf [0] + d->fnameidx,
                        dnamebuf [0] + d->dirid,
                        "   this only");
                    nooutput++;
                }
            }
            ptr0++;
        } else if (ptr1 < dirptr [1]) {
/*--------------------------------------------------------------------------
 -  file in 1/that only
 ---------------------------------------------------------------------------*/
            skipdup (outputfmt, 1, &d2, &ptr1);
            if (cmd & CMD_THAT_ONLY) {
                if (outputfmt == OUTPUTCPY) {
                } else {
                    printf ("22  %-20s %-20s %-20s\n",
                        fnamebuf [1] + d2->fnameidx,
                        dnamebuf [1] + d2->dirid,
                        "   that only");
                    nooutput++;
                }
            }
            ptr1++;
        }
    }
/*--------------------------------------------------------------------------
 -  end of iteration loop
 ---------------------------------------------------------------------------*/

    skip :
    if ((outputfmt == OUTPUTTXT) || (outputfmt == OUTPUTCPY)) {
        if (outputfmt == OUTPUTCPY) {
            printf ("rem ");
        }
        printf ("    %ld files ", nooutput - lastnooutput);
        switch (cmd) {
            default            : 
                puts ("unknown");
                break;
            case CMD_SAME      : 
                printf ("same in both [%d]\n", CMD_SAME);
                break;
            case CMD_DIFF      : 
                printf ("different but same date/time? [%d]\n", CMD_DIFF);
                break;
            case CMD_THIS_ONLY : 
                printf ("exist only in this (%s) [%d]\n", datafile, CMD_THIS_ONLY);
                break;
            case CMD_THAT_ONLY : 
                printf ("exist only in that (%s) [%d]\n", datafile2, CMD_THAT_ONLY);
                break;
            case CMD_THIS_NEW  : 
                printf ("this is newer (%s) [%d]\n", datafile, CMD_THIS_NEW);
                break;
            case CMD_THAT_NEW  : 
                printf ("that is newer (%s) [%d]\n", datafile2, CMD_THAT_NEW);
                break;
            case CMD_THIS_MOVE  : 
                printf ("this moved to that (%s) [%d]\n", datafile, CMD_THIS_MOVE);
                break;
            case CMD_THAT_MOVE  : 
                printf ("that moved from this (%s) [%d]\n", datafile2, CMD_THAT_MOVE);
                break;
            case CMD_THIS_DUP  : 
                printf ("duplicated in this (%s) [%d]\n", datafile, CMD_THIS_DUP);
                break;
            case CMD_THAT_DUP  : 
                printf ("duplicated in that (%s) [%d]\n", datafile2, CMD_THAT_DUP);
                break;
        }
    }


    deldir :

    if (cmd == -2) {
/*--------------------------------------------------------------------------
 -  delete directory in 1/that only
 ---------------------------------------------------------------------------*/
        /*  delete blank dirextories */

        /*  for all dirname in 0 */
        for (dname0 = dnamebuf [0];
             dname0 < dnamebuf [0] + dnameptr [0];
             dname0 += strlen (dname0) + 1) {
            found = 0;
            /*  for all dirname in 1*/
            for (dname1 = dnamebuf [1];
                 dname1 < dnamebuf [1] + dnameptr [1];
                 dname1 += strlen (dname1) + 1) {
                /*  compare dirname */
                if (dirtstrcmp (dname0, dname1) == 0) {
                    /*  dirnames equal */
                    found = 1;
                    break;
                }
            }
            if (found == 0) {
                /*  dname0 is in 0 but not in 1 */
                if (outputfmt == OUTPUTCPY) {
                    /*  delete dir in 0 that is not in 1 */
                    printf ("rd \"%s\"\n", strdelslash (dname0));
                } else {
                    printf ("dir '%-20s' is not in that\n", dname0);
                    nooutput++;
                }
            }
        }

#if 0
        /*  print legends */
        puts ("  [  dup in this");
        puts ("   ] dup in that");
        puts ("  == in both same name, size, CRC, time and same dir");
        puts ("  m= in both same name, size, CRC, time but different dir");
        puts ("  #C in both diff name, size, CRC, time but same name same dir, same date");
        puts ("  d> in both diff name, size, CRC, time but same name same dir, this newer");
        puts ("  d< in both diff name, size, CRC, time but same name same dir, that newer");
        puts ("  ?? in both diff name, size, CRC, time, same name & dir (??)");
        puts ("  m# in both diff name, size, CRC, time, same name diff dir : same date/time");
        puts ("  m< in both diff name, size, CRC, time, same name diff dir : that newer");
        puts ("  m> in both diff name, size, CRC, time, same name diff dir : this newer");
        puts ("  1  diff name, this only");
        puts ("   2 diff name, that only");
        puts ("  11 file in 0/this only");
        puts ("  22 file in 1/that only");
#endif

    }


} /* diff2mv */




static int qsortdupcmpnodir (int usename, int idx0, const void *a, int idx1, const void *b) {
/*****************************************************************************
 *
 *  Sort for file duplication search
 *
 *  Sort by file name, size, CRC, time_t, dir name
 *
 *****************************************************************************/
    int i;


    if (usename != 0) {
        /*  file name */
        i = dirtstrcmp (fnamebuf [idx0] + ((struct dirlist *) a)->fnameidx,
                        fnamebuf [idx1] + ((struct dirlist *) b)->fnameidx);
        if (i != 0) {
            return (i);
        }
    }

    /*  size */
    if (((struct dirlist *) a)->size != ((struct dirlist *) b)->size) {
        if (((struct dirlist *) a)->size > ((struct dirlist *) b)->size) {
            return (1);
        } else {
            return (-1);
        }
    }
    /*  crc */
    if (((struct dirlist *) a)->crc != ((struct dirlist *) b)->crc) {
        if (((struct dirlist *) a)->crc > ((struct dirlist *) b)->crc) {
            return (1);
        } else {
            return (-1);
        }
    }

    /*  date/time stampe*/
/*
    if (((struct dirlist *) a)->tt != ((struct dirlist *) b)->tt) {
        if (((struct dirlist *) a)->tt > ((struct dirlist *) b)->tt) {
            return (1);
        } else {
            return (-1);
        }
    }
*/

    return (0);

} /* qsortdupcmpnodir */

char *strdelslash (char *ptr) {
    int i;
    static char buf [1024];

    i = strlen (ptr);
    if (i >= 1024) {
        printf ("Exiting (%s %d: dir too long)\n", __FILE__, __LINE__);
        exit (1);
    }

    strcpy (buf, ptr);

    if (--i >= 0) {
        if (buf [i] == '\\') {
            buf [i]  = 0;
        }
    }


    return (buf);

} /* strdelslash */
