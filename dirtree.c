/*
 * Copyright (c) 2008-2012 l00g33k@gmail.com All rights reserved.
 *
 */

#define LIBCALL
#include <crc32.c>

#include "dirtree.h"



char *threeArgs = 
        "@echo off\n"
        "rem fc /b %%2 %%3 > nul\n"
        "dirtree -= %%2 %%3 > nul\n"
        "if errorlevel 2 goto what\n"
        "if errorlevel 1 goto diff\n"
        ":same\n"
        "echo same\n"
        "goto end\n"
        ":diff\n"
        "echo diff\n"
        "goto end\n"
        ":what\n"
        "echo what\n"
        ":end\n",
    *twoArgs = 
        "@echo off\n"
        "rem fc /b %%1 %%2 > nul\n"
        "dirtree -= %%1 %%2 > nul\n"
        "if errorlevel 2 goto what\n"
        "if errorlevel 1 goto diff\n"
        ":same\n"
        "echo same\n"
        "goto end\n"
        ":diff\n"
        "echo diff\n"
        "goto end\n"
        ":what\n"
        "echo what\n"
        ":end\n",
    *copyArgs = 
        "@echo off\n"
        "rem copy /b /y %%1 %%2\n"
        "dir /b %%1\n"
        "dir /b %%2\n",
    *onlyArgs = 
        "@echo off\n"
        "rem copy /b /y %%1 %%2\n"
        "dir /b %%1\n";

int debug = 0;
int recurlvl = 0, maxrecurlvl = 0, printdirtree = 0, file = 1, calccrc = 0,
    verbose = 0, homepathlen, nodate = 0, noexclude = 0, printmove = 0,
    filter = 0, recurlvllmt = 32767, noexcludedir = 0, nohomepath = 0,
    exclextinvt = 0, excldirinvt = 0;
#if defined(__GNUC__)
/* default to not ignore case on Linux*/
int ignorecase = 0;
#else
int ignorecase = 1;
#endif
long int nofiles = 0, totalbytes = 0, totalGbytes = 0,
    NTtimediff = 0, nooutput;

char qoute = ' ', homepath [2] [2048], datafile [256] = {"dirtree.dir"}, datafile2 [256],
    *padding = "", *exclude [NOEXC], *excludedir [NOEXC];


unsigned long nodirs [2],
fnamesz [2], fnameptr [2],
dnamesz [2], dnameptr [2],
  dirsz [2],   dirptr [2];
char HUGE *fnamebuf [2], HUGE *dnamebuf [2];
struct dirlist HUGE *dirbuf [2];


char procfilebuf [2048];

int (*dirtstrcmp) (char *, char*);








char *numcomma (unsigned long int GBytes, unsigned long int bytes) {
/*****************************************************************************
 *
 *  Print comma separated large number
 *
 *****************************************************************************/
    static char buf [32];
    int needleading0, remainder;

    buf [0] = 0;
    needleading0 = 0;

    if (GBytes > 0) {
        sprintf (buf, "%ld,", GBytes);
        needleading0 = 1;
    }

    remainder = bytes / 1000000;
    if (remainder > 0) {
        if (needleading0 != 0) {
            sprintf (buf + strlen (buf), "%03d,", remainder);
        } else {
            sprintf (buf + strlen (buf), "%d,", remainder);
        }
        bytes -= remainder * 1000000;
        needleading0 = 1;
    }
    remainder = bytes / 1000;
    if ((remainder > 0) || (needleading0 != 0)) {
        if (needleading0 != 0) {
            sprintf (buf + strlen (buf), "%03d,", remainder);
        } else {
            sprintf (buf + strlen (buf), "%d,", remainder);
        }
        bytes -= remainder * 1000;
        needleading0 = 1;
    }

    remainder = bytes;
    if (needleading0 != 0) {
        sprintf (buf + strlen (buf), "%03d", remainder);
    } else {
        sprintf (buf + strlen (buf), "%d", remainder);
    }

    return (buf);
} /* numcomma */



void gadd (
    unsigned long int gbytes,
    unsigned long int bytes,
    unsigned long int *tgbytes,
    unsigned long int *tbytes
) {
    unsigned long int j;

    *tbytes  += bytes;
    *tgbytes += gbytes;

    if (*tbytes >= 1000000000) {
        j = (*tbytes / 1000000000);
        *tbytes -= j * 1000000000;
        *tgbytes += j;
    }

} /* gadd */


void treesize (int dbidx, char *homepath, char treesizemode, int outputfmt) {
/*****************************************************************************
 *
 *  Print entire dir tree
 *
 *****************************************************************************/
    int k, summarizing, dirinsub, levelinsub;
    unsigned int diridx;
    unsigned long i, j;
    struct dirlist *d, *last;
    struct dirsize_ *dirsize;
    unsigned long int total, gtotal, stotal, sgtotal, fileinsub;

    total  = 0;
    gtotal = 0;

    if (nodirs [dbidx] == 0) {
        printf ("%14s Total bytes\n", "0");
        return;
    }

    dirsize = malloc (sizeof (struct dirsize_) * nodirs [dbidx] + 16);

    if (dirsize == NULL) {
        fprintf (stderr, "malloc failed (%ld bytes)\n",
                          sizeof (struct dirsize_) * nodirs [dbidx] + 16);
        return;
    }




/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    dirsort (dbidx, SORTBYDIR);


/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    diridx = 0;
    dirsize [diridx].dirthisgbytes = 0;
    dirsize [diridx].dirthisbytes  = 0;
    dirsize [diridx].dirsubgbytes  = 0;
    dirsize [diridx].dirsubbytes   = 0;

    for (i = 0; i < dirptr [dbidx]; i++) {
/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
        d = dirbuf [dbidx] + i;

        if (i == 0) {
            d = dirbuf [dbidx];
            dirsize [diridx].dirname = dnamebuf [dbidx] + d->dirid;

            gadd (0L, d->size, &gtotal, &total);
            gadd (0L, d->size, &dirsize [diridx].dirthisgbytes,
                            &dirsize [diridx].dirthisbytes);
            dirsize [diridx].nofiles = 1;
        } else {
            last = dirbuf [dbidx] + i - 1;
            if (dirtstrcmp (dnamebuf [dbidx] + d->dirid,
                            dnamebuf [dbidx] + last->dirid) != 0) {
                /*  different dir now */
                diridx++;
                dirsize [diridx].dirname = dnamebuf [dbidx] + d->dirid;
                dirsize [diridx].dirthisgbytes = 0;
                dirsize [diridx].dirthisbytes  = 0;
                dirsize [diridx].dirsubgbytes  = 0;
                dirsize [diridx].dirsubbytes   = 0;
                dirsize [diridx].nofiles = 1;
            }
            gadd (0L, d->size, &gtotal, &total);
            gadd (0L, d->size, &dirsize [diridx].dirthisgbytes,
                               &dirsize [diridx].dirthisbytes);
            dirsize [diridx].nofiles++;
        }
    }
    diridx++;

    treesizesort (dirsize, diridx, TREESIZESORTBYDIRSIZE);
    if (treesizemode == 'T') {
        treesizesort (dirsize, diridx, TREESIZESORTBYPAHT);
        /*  process base ([0]) too */
        for (i = 0; i < diridx; i++) {
            if ((i % 10) == 0) {
                fprintf (stderr, "+");
            }
            /* start at j to include self in the sub-total */
            for (j = i; j < diridx; j++) {
                if (strncmp (dirsize [i].dirname, dirsize [j].dirname,
                    strlen (dirsize [i].dirname)) == 0) {
                    /*  partial match, add it */
                    gadd (dirsize [j].dirthisgbytes,
                          dirsize [j].dirthisbytes,
                         &dirsize [i].dirsubgbytes,
                         &dirsize [i].dirsubbytes);
                }
            }
        }
        fprintf (stderr, "\n");
        treesizesort (dirsize, diridx, TREESIZESORTBYSUBSIZE);

        /*  loop non empty directories */
        for (i = 0; i < diridx; i++) {
            printf ("%14s; ",
                numcomma (dirsize [i].dirsubgbytes, dirsize [i].dirsubbytes));
            printf ("%14s; %s%s\n",
                numcomma (dirsize [i].dirthisgbytes, dirsize [i].dirthisbytes),
                homepath, (dirsize [i].dirname [0] == 0) ? "(:base dir:)" : dirsize [i].dirname);
        }
        puts ("  bytes in sub;   bytes in dir; dir name");
        printf ("%14s Total bytes\n", numcomma (gtotal, total));
    } else if (treesizemode == 'x') {
        treesizesort (dirsize, diridx, TREESIZESORTBYPAHT);
        /*  loop non empty directories */

        sgtotal = 0;
        stotal  = 0;
        summarizing = 0;
        fileinsub = 0;
        dirinsub = 0;
        levelinsub = 0;
        for (i = 0; i < diridx; i++) {
            k = strlen (dirsize [i].dirname);
            dirsize [i].dirlevel = 0;
            for (k--; k >= 0; k--) {
                if (dirsize [i].dirname [k] == DIRSEPARATOR) {
                    dirsize [i].dirlevel++;
                }
            }

            if (dirsize [i].dirlevel <= recurlvllmt) {
                if (summarizing != 0) {
                    summarizing = 0;
                    printf ("                    %d dirs, %d levels, %ld files, %s bytes\n",
                        dirinsub, levelinsub, fileinsub,
                        numcomma (sgtotal, stotal));
                    sgtotal = 0;
                    stotal  = 0;
                    fileinsub = 0;
                    dirinsub = 0;
                    levelinsub = 0;
                }

                printf ("%14s %2d ; %s%s\n",
                    numcomma (dirsize [i].dirthisgbytes, dirsize [i].dirthisbytes),
                    dirsize [i].dirlevel,
                    homepath, (dirsize [i].dirname [0] == 0) ? "(:base dir:)" : dirsize [i].dirname);
            } else {
                gadd (dirsize [i].dirthisgbytes, dirsize [i].dirthisbytes,
                      &sgtotal, &stotal);
                summarizing = 1;
                fileinsub += dirsize [i].nofiles;
                dirinsub++;
                if (levelinsub < (dirsize [i].dirlevel - recurlvllmt)) {
                    levelinsub = (dirsize [i].dirlevel - recurlvllmt);
                }
            }
        }
        if (summarizing != 0) {
            printf ("                    %d dirs, %d levels, %ld files, %s bytes\n",
                        dirinsub, levelinsub, fileinsub,
                        numcomma (sgtotal, stotal));
        }

        puts ("  bytes in sub;   bytes in dir; dir name");
        printf ("%14s Total bytes\n", numcomma (gtotal, total));
    } else {
        /*  loop non empty directories */
        for (i = 0; i < diridx; i++) {
            printf ("%14s; %s%s\n",
                numcomma (dirsize [i].dirthisgbytes, dirsize [i].dirthisbytes),
                homepath, (dirsize [i].dirname [0] == 0) ? "(:base dir:)" : dirsize [i].dirname);
        }
        puts ("  bytes in dir; dir name");
        printf ("%14s Total bytes\n", numcomma (gtotal, total));
    }


    free (dirsize);

} /* treesize */



void dupchk (int dbidx, char *homepath, int equalmask) {
/*****************************************************************************
 *
 *  Print entire dir tree
 *
 *****************************************************************************/
    char buf [30];
    int lastdiff, namelen, j;
    unsigned long i, nodups;
    struct dirlist *d, *last;



/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    dirsort (dbidx, SORTBYDUP);


/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    namelen = 0;
    lastdiff = 1;
    for (i = 1; i < dirptr [dbidx]; i++) {
/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
        d    = dirbuf [dbidx] + i;
        last = dirbuf [dbidx] + i - 1;
        if (((dirtstrcmp (fnamebuf [dbidx] + d->fnameidx,
                          fnamebuf [dbidx] + last->fnameidx) == 0)) &&
            (((equalmask & EQUALMASKSIZE ) == 0) || (d->size == last->size)) &&
            (((equalmask & EQUALMASKCRC  ) == 0) || (d->crc == last->crc)) &&
            (((equalmask & EQUALMASKTIME ) == 0) || (d->tt == last->tt))) {
/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
            if (lastdiff != 0) {
                j = strlen (fnamebuf [dbidx] + last->fnameidx);
                if (namelen < j) {
                    namelen = j;
                }
            }
            j = strlen (fnamebuf [dbidx] + d->fnameidx);
            if (namelen < j) {
                namelen = j;
            }
            lastdiff = 0;
        } else {
            lastdiff = 1;
        }

    }

    lastdiff = 1;
    nodups = 0;
/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    for (i = 1; i < dirptr [dbidx]; i++) {
/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
        d    = dirbuf [dbidx] + i;
        last = dirbuf [dbidx] + i - 1;
        if (((dirtstrcmp (fnamebuf [dbidx] + d->fnameidx,
                          fnamebuf [dbidx] + last->fnameidx) == 0)) &&
            (((equalmask & EQUALMASKSIZE ) == 0) || (d->size == last->size)) &&
            (((equalmask & EQUALMASKCRC  ) == 0) || (d->crc == last->crc)) &&
            (((equalmask & EQUALMASKTIME ) == 0) || (d->tt == last->tt))) {
/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
            if (lastdiff != 0) {
                strcpy (buf, xctime (&last->tt));
                buf [24] = '\0';
                printf ("%10ld %08lx %s * %-*s %s%s\n",
                    last->size,
                    last->crc,
                    buf,
                    namelen, fnamebuf [dbidx] + last->fnameidx,
                    homepath,
                    dnamebuf [dbidx] + last->dirid);
                nodups++;
            }
            strcpy (buf, xctime (&d->tt));
            buf [24] = '\0';
            printf ("%10ld %08lx %s * %-*s %s%s\n",
                d->size,
                d->crc,
                buf,
                namelen, fnamebuf [dbidx] + d->fnameidx,
                homepath,
                dnamebuf [dbidx] + d->dirid);
            lastdiff = 0;
        } else {
            lastdiff = 1;
        }
    }

    printf ("There are %ld files duplicated in various directories\n", nodups);

} /* dupchk */



void printit (int dbidx, int outputfmt) {
/*****************************************************************************
 *
 *  Print entire dir tree
 *
 *****************************************************************************/
    char buf [30], lbuf [30];
    int lastdiff;
    unsigned long i;
    struct dirlist *d, *last;

    if (outputfmt == OUTPUTCSV) {
        printf ("\"Filename\", \"Size\", \"Date\", \"Date\", \"CRC\", \"Path\"\n");
    }


/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    if (filter != 0) {
        dirsort (dbidx, SORTBYDUP);
    }


/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
    for (i = 0; i < dirptr [dbidx]; i++) {
/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
        d = dirbuf [dbidx] + i;
        strcpy (buf, xctime (&d->tt));
        buf [24] = '\0';
        if (debug >= 5) {
            printf ("dirbuf [dbidx] [%ld] = %08lx (%08lx %ld) %08lx (%08lx %ld) %08lx\n",
                i, d,
                dnamebuf [dbidx], d->dirid, dnamebuf [dbidx] + d->dirid,
                fnamebuf [dbidx], d->fnameidx, fnamebuf [dbidx] + d->fnameidx);
        }
        if (outputfmt == OUTPUTTXT) {
/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
            if ((exclude_ext (fnamebuf [dbidx] +  d->fnameidx) == 0) &&
                (exclude_dir (dnamebuf [dbidx] +  d->dirid) == 0)) {
                printf ("%s%10ld %08lx * %s%s\n",
                    buf,
                    d->size,
                    d->crc,
                    dnamebuf [dbidx] + d->dirid,
                    fnamebuf [dbidx] + d->fnameidx);
            }
        } else {
/*---------------------------------------------------------------------------
 -
 -  puts ("-F# : filter: 1 sort, 2 -path, 3 fname, 4 +size, 5 +date");
 ----------------------------------------------------------------------------*/
            if (filter >= 3) {
/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
                if (i == 0) {
                    last = d;
                    continue;
                }
                if (dirtstrcmp (fnamebuf [dbidx] + d->fnameidx,
                                fnamebuf [dbidx] + last->fnameidx) != 0) {
                    lastdiff = 1;
                    last = d;
                    continue;
                }
                if ((filter >= 4) &&
                    ((d->size != last->size) ||
                     (d->crc  != last->crc )
                   )) {
                    lastdiff = 1;
                    last = d;
                    continue;
                }
                if ((filter >= 5) && (d->tt != last->tt)) {
                    lastdiff = 1;
                    last = d;
                    continue;
                }
                if (lastdiff != 0) {
                    strcpy (lbuf, xctime (&last->tt));
                    lbuf [24] = '\0';
                    printf ("\"%s\", %ld, %ld, \"%s\", \"%08lx\", \"%s\"\n",
                        fnamebuf [dbidx] + last->fnameidx,
                        last->size,
                        last->tt,
                        lbuf,
                        last->crc,
                        dnamebuf [dbidx] + last->dirid
                    );
                }
                last = d;
                lastdiff = 0;
            }
            if (filter != 2) {
/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
                printf ("\"%s\", %ld, %ld, \"%s\", \"%08lx\", \"%s\"\n",
                    fnamebuf [dbidx] + d->fnameidx,
                    d->size,
                    d->tt,
                    buf,
                    d->crc,
                    dnamebuf [dbidx] + d->dirid
                );
            } else {
/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
                printf ("\"%s\", %ld, %ld, \"%s\", \"%08lx\"\n",
                    fnamebuf [dbidx] + d->fnameidx,
                    d->size,
                    d->tt,
                    buf,
                    d->crc
                );
            }
        }
    }

} /* printit */



int main (int noarg, char *argv []) {
/*****************************************************************************
 *
 *  main
 *
 *****************************************************************************/
    char treesizemode = ' ';
    int i, save = 0, retrieve = 0, dbidx = 0, sort = 1, retrive2nd = 0,
        outputfmt = 0, dotreesize = 0, equalmask = 0, cmpfile = 0, cnt1, cnt2,
        cmd = CMD_SAME      |
              CMD_DIFF      |
              CMD_THIS_ONLY |
              CMD_THAT_ONLY |
              CMD_THIS_NEW  |
              CMD_THAT_NEW  |
              CMD_THIS_MOVE |
              CMD_THAT_MOVE |
              CMD_THIS_DUP  |
              CMD_THAT_DUP  |
              CMD_ALL_DUP   |
              CMD_MD        |
              CMD_RD;
    double d;
    FILE *st, *st2;


/*---------------------------------------------------------------------------
 -  Allocate the three buffers (file name, dir name, dir entry for both trees
 ----------------------------------------------------------------------------*/
    dbidx = 1;
    fnamesz [dbidx] = FNAMEALLOCSZ;
    dnamesz [dbidx] = DNAMEALLOCSZ;
    dirsz   [dbidx] =   DIRALLOCSZ;

    fnamebuf [dbidx] = malloc (dnamesz [dbidx]);
    dnamebuf [dbidx] = malloc (fnamesz [dbidx]);
    dirbuf   [dbidx] = malloc (dirsz [dbidx] * sizeof (struct dirlist));

    dbidx = 0;
    fnamesz [dbidx] = FNAMEALLOCSZ;
    dnamesz [dbidx] = DNAMEALLOCSZ;
    dirsz   [dbidx] =   DIRALLOCSZ;

    fnamebuf [dbidx] = malloc (dnamesz [dbidx]);
    dnamebuf [dbidx] = malloc (fnamesz [dbidx]);
    dirbuf   [dbidx] = malloc (dirsz [dbidx] * sizeof (struct dirlist));


    /*  default */
    timezone = 3600l * 8;

/*---------------------------------------------------------------------------
 -  Process command line arguments
 ----------------------------------------------------------------------------*/
    for (i = 1; i < noarg; i++) {
        if (argv [i] [0] == '-') {
            switch (argv [i] [1]) {
                    case '?' :
                    case 'h' :
                    case 'H' :
                        printf ("Usage: %s [options] [target dir] [2nd target dir]\n", argv [0]);
#ifdef __TURBOC__
                        puts ("-z# : timezone, -z-8 for PST, -z-7 for PDT");
#endif
                        puts ("-p  : surpress/generate print out");
                        puts ("-f- : surpress file scan");
                        puts ("-s[filename] : save");
                        puts ("-r[filename] : retrieve");
                        puts ("-2{filename} : retrieve 2nd file");
                        puts ("-c# : all=63 same=1 diff=2 this=4 that=8 this-newer=16 that-newer=32");
                        puts ("      this-moved-to-that=64 that-moved-from-this=128");
                        puts ("      this-duplicated=256 that-duplicated=512");
                        puts ("      md=1024 rd=2048");
                        puts ("      this-newer-or-only=20 that-newer-or-only=40 diff=62");
                        puts ("-C  : compute CRC 32/use CRC in difference");
                        puts ("-D[#]: ignore date/time for file difference; 0=don't (default)");
                        puts ("-i[#]: case sensitive compare; default: 1=ignore");
                        puts ("-P  : padding for column alignment");
                        puts ("-q  : no sort sort");
                        puts ("-u[#]  : 1 tree duplicate check: 1=size 2=CRC 4=time");
                        puts ("-v  : verbose");
                        puts ("-!  : my special notes");
                        puts ("-m# : move to match; qouting spaces 1='\"'");
                        puts ("-y# : copy files; qouting spaces 1='\"'");
                        puts ("-Y  : make batch files");
                        puts ("    : -Y makes batch files for files processing");
                        puts ("    : 1) cd to an empty directory");
                        puts ("    : 2) generate .dir");
                        puts ("    :    dirtree -sthis.dir \"C:\\Program Files\\MailToNews\\error\\older\"");
                        puts ("    :    dirtree -sthat.dir \"D:\\mail2news_more\\error\"");
                        puts ("    : 3) generate support batch files: dirtree -Y");
                        puts ("    : 4) generate main batch file:");
                        puts ("    :    dirtree -y1 -c1 -rthis.dir -2that.dir > dirtreebat.bat");
                        puts ("    : 5) edit and run dirtreebat.bat");
                        puts ("-N  : find reNamed files");
                        puts ("-R# : maximun recursion limit");
                        puts ("-E  : Excel .csv");
                        //puts ("-b  : create DOS batch file > dt.bat");
                        puts ("-d# : debug level: 1,3,5");
                        puts ("-F# : filter: 1 sort, 2 -path, 3 fname, 4 +size, 5 +date");
                        puts ("-x$ : exclude extensions");
                        puts ("-X$ : exclude sub-directories");
                        puts ("-t#.# : treat time difference of #.# hours as same; for Win2k");
                        puts ("-T[?]  : treesize [?=T sub total for each sub-dir;x=no sort] {-R#}");
                        puts ("Examples:");
                        printf ("%s -T[{T|x}]\n", argv [0]);
                        printf ("%s -sdtc.dir -p    t:\\pa\n", argv [0]);
                        printf ("%s -sdtc.dir -p -C t:\\pa\n", argv [0]);
                        printf ("%s -rdtc.dir \n", argv [0]);
                        printf ("%s -rdtc.dir -2dtc2.dir -c63\n", argv [0]);
                        printf ("%s -rdtc.dir -2dtc2.dir -c63 -C\n", argv [0]);
                        printf ("%s -rdtc.dir -2dtc2.dir -c63 -C -D -P\n", argv [0]);
                        printf ("%s sav\\d1 sav\\e1 -c63\n", argv [0]);
                        printf ("%s sav\\d1 sav\\e1 -c63 -m\n", argv [0]);
                        printf ("built %s %s -- Copyrighted freeware - l00g33k@gmail.com\n", __DATE__, __TIME__);
                        return (1);

                    case '=' :
                        cmpfile = 1;
                        break;

                    case '!' :
                        puts ("dirtree -E -Xalbums -Xinfo -Xthumbs -xhtml -xhtm -xavi -xbat -xpl -xalb -xpro -xdir -xscc -xgif -xmpg -u4 -p");
                        puts ("");
                        puts ("");
                        return (1);

                    case 'N' :
                        printmove = 2;                        
                        break;

                    case 'F' :
                        filter = 1;
                        sscanf (argv [i] + 2, "%d", &filter);
                        break;

                    case 'Y' :
                        st = fopen ("same____.bat", "wt");
                        fprintf (st, twoArgs);
                        fclose (st);
                        st = fopen ("pause___.bat", "wt");
                        fprintf (st, "pause\n");
                        fclose (st);
                        st = fopen ("thisonly.bat", "wt");
                        fprintf (st, onlyArgs);
                        fclose (st);
                        st = fopen ("thatonly.bat", "wt");
                        fprintf (st, onlyArgs);
                        fclose (st);
                        st = fopen ("thisnewr.bat", "wt");
                        fprintf (st, copyArgs);
                        fclose (st);
                        st = fopen ("thatnewr.bat", "wt");
                        fprintf (st, copyArgs);
                        fclose (st);
                        st = fopen ("thismved.bat", "wt");
                        fprintf (st, twoArgs);
                        fclose (st);
                        st = fopen ("thatmved.bat", "wt");
                        fprintf (st, twoArgs);
                        fclose (st);
                        st = fopen ("thisdupd.bat", "wt");
                        fprintf (st, threeArgs);
                        fclose (st);
                        st = fopen ("thatdupd.bat", "wt");
                        fprintf (st, threeArgs);
                        fclose (st);
                        return (1);

                    case 'y' :
                        outputfmt = OUTPUTCPY;
                        printmove = 1;
                        if (argv [i] [2] == '1') {
                            qoute = '\"';
                        }
                        break;

                    case 'i' :
                        ignorecase = !ignorecase;
                        sscanf (argv [i] + 2, "%d", &ignorecase);
                        break;

                    case 'E' :
                        outputfmt = OUTPUTCSV;
                        printf ("\"Date this\" , \"Status\" , \"Size this\" , \"Date that\" ,   \"Size that\" , \"Path\" , \"Filename\"\n");
                        break;

                    case 'u' :
                        equalmask = 15;
                        sscanf (argv [i] + 2, "%d", &equalmask);
                        equalmask |= 0x8000;    /*  non zero to run */
                        break;

                    case 'R' :
                        sscanf (argv [i] + 2, "%d", &recurlvllmt);
                        break;

                    case 'b' :
                        bat ();
                        return (0);

                    case 'x' :
                        if (argv [i] [2] == '!') {
                            exclextinvt = 1;
                        } else if (noexclude < NOEXC) {
                            exclude [noexclude++] = argv [i] + 2;
                        }
                        break;

                    case 'X' :
                        if (argv [i] [2] == '!') {
                            excldirinvt = 1;
                        } else if (noexcludedir < NOEXC) {
                            excludedir [noexcludedir++] = argv [i] + 2;
                        }
                        break;

                    case 'D' :
                        nodate = 1;
                        sscanf (argv [i] + 2, "%c", &nodate);
                        if (nodate) {
                            fprintf (stderr, "-D masks this-newer and that-newer\n");
                        }
                        break;

                    case 'P' :
                        padding = "                                  ";
                        break;

                    case 'v' :
                        verbose = 1;
                        break;

                    case 'T' :
                        dotreesize = 1;
                        sscanf (argv [i] + 2, "%c", &treesizemode);
                        break;

                    case 't' :
                        d = 0;
                        sscanf (argv [i] + 2, "%lf", &d);
                        NTtimediff = (long int) (d * 3600);
                        break;

                    case 'C' :
                        calccrc = !calccrc;
                        sscanf (argv [i] + 2, "%c", &calccrc);
                        break;

                    case 'q' :
                        sort = 0;
                        break;

                    case 's' :
                        save = 1;
                        retrieve = 0;
                        if (argv [i] [2] != '\0') {
                            sscanf (argv [i] + 2, "%s", datafile);
                            if (verbose != 0) {
                                printf ("Using '%s'\n", datafile);
                            }
                        }
                        break;

                    case 'm' :
                        printmove = 1;
                        if (argv [i] [2] == '1') {
                            qoute = '\"';
                        }
                        break;

                    case '2' :
                        if (sscanf (argv [i] + 2, "%s", datafile2) == 1) {
                            retrive2nd = 1;
                            if (verbose != 0) {
                                printf ("Using 2 '%s'\n", datafile2);
                            }
                            /*  inverse -p meaning for diff */
                            /*printdirtree = !printdirtree;*/
                        }
                        break;

                    case 'd' :
                        debug = 1;
                        sscanf (argv [i] + 2, "%d", &debug);
                        break;

                    case 'c' :
                        sscanf (argv [i] + 2, "%d", &cmd);
                        break;

                    case 'r' :
                        save = 0;
                        retrieve = 1;
                        if (argv [i] [2] != '\0') {
                            sscanf (argv [i] + 2, "%s", datafile);
                            if (verbose != 0) {
                                printf ("Using '%s'\n", datafile);
                            }
                        }
                        break;

                    case 'z' :
                        timezone = -8;
                        sscanf (argv [i] + 2, "%ld", &timezone);
                        timezone *= -3600l;
                        break;

                    case 'p' :
                        printdirtree = 1;
                        break;

                    case 'f' :
                        if (argv [i] [2] == '-') {
                            file = 0;
                        }
                        break;

            }
        } else {
            /*  homepath */
            if (nohomepath == 0) {
                nohomepath = 1;
                strcpy (homepath [0], argv [i]);
            } else if (nohomepath == 1) {
                nohomepath = 2;
                strcpy (homepath [1], argv [i]);
            }
        }
    }

    /*  fc for DOS 5.0 with exit value */
    if (cmpfile == 1) {
        if (nohomepath == 2) {
            cmpfile = 0;  /* 0=same, 1=diff */
            st  = fopen (homepath [0], "rb");
            st2 = fopen (homepath [1], "rb");
            if ((st != NULL) && (st2 != NULL)) {
                for (;;) {
                    cnt1 = fread (procfilebuf,        1, 1024, st);
                    cnt2 = fread (procfilebuf + 1024, 1, 1024, st2);
                    if (cnt1 != cnt2) {
                        cmpfile = 1;
                        break;
                    }
                    if (cnt1 <= 0) {
                        break;
                    }
                    if (memcmp (procfilebuf, procfilebuf + 1024, cnt1) != 0) {
                        cmpfile = 1;
                        break;
                    }
                }
            } else {
                cmpfile = 1;
            }
            if (st != NULL) {
                fclose (st);
            }
            if (st2 != NULL) {
                fclose (st2);
            }
        }
        printf ("'%s' and '%s' are %s; return (%d)\n",
            homepath [0], homepath [1], 
            cmpfile ? "diff" : "same", cmpfile);
        return (cmpfile);
    }


    if (ignorecase != 0) {
        dirtstrcmp = stricmp;
    } else {
        dirtstrcmp = strcmp;
    }

/*---------------------------------------------------------------------------
 -  Prints exclude extension filters
 ----------------------------------------------------------------------------*/
    for (i = 0; i < noexclude; i++) {
        fprintf (stderr, "Excluding extension: '%s'\n", exclude [i]);
    }
    for (i = 0; i < noexcludedir; i++) {
        fprintf (stderr, "Excluding sub-directory: '%s'\n", excludedir [i]);
    }

/*
    if (printmove == 2) {
        retrive2nd = 1;
        if (nohomepath == 1) {
            nohomepath = 2;
            strcpy (homepath [1], homepath [0]);
        }
    }
*/


/*---------------------------------------------------------------------------
 -
 ----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
 -  Append '\' if homepath is not "\" nor ""
 ----------------------------------------------------------------------------*/
    if ((dirtstrcmp (homepath [0], DIRSEPARATORS) != 0) &&
        (homepath [0] [0] != '\0') &&
        (homepath [0] [strlen (homepath [0]) - 1] != DIRSEPARATOR)) {
        /*  append '\' if not ending with except "\" or "" */
        strcat (homepath [0], DIRSEPARATORS);
    }

    if (nohomepath == 2) {
        if ((dirtstrcmp (homepath [1], DIRSEPARATORS) != 0) &&
            (homepath [1] [0] != '\0') &&
            (homepath [1] [strlen (homepath [1]) - 1] != DIRSEPARATOR)) {
            /*  append '\' if not ending with except "\" or "" */
            strcat (homepath [1], DIRSEPARATORS);
        }
        strcpy (datafile,  homepath [0]);
        strcpy (datafile2, homepath [1]);
    }

    if (retrieve != 0) {
/*---------------------------------------------------------------------------
 -  Retrieve one or both dir tree from disk
 ----------------------------------------------------------------------------*/
        retrieveit (datafile, dbidx, homepath [0]);
        if (verbose != 0) {
            printf ("retrived for '%s'\n", homepath [0]);
        }

        if (retrive2nd != 0) {
            /*  retrieve second tree */
            retrieveit (datafile2, 1, homepath [1]);
        }
    } else {
/*---------------------------------------------------------------------------
 -  Scan homepath dir tree
 ----------------------------------------------------------------------------*/
        if (verbose != 0) {
            printf ("scanning '%s'\n", homepath [0]);
        }

        /*  homepath is used for opening dir but not saved to disk */
        homepathlen = strlen (homepath [0]);

        /*  scand homepath */
        dtscandir (dbidx, homepath [0]);

        fprintf (stderr, "\n");

        if (sort) {
            /*  sort if not disabled */
            dirsort (dbidx, SORTBYDIR);
        }

        if (save != 0) {
/*---------------------------------------------------------------------------
 -  Save to disk if asked
 ----------------------------------------------------------------------------*/
            saveit (datafile, dbidx, homepath [0]);
        }

        if (nohomepath == 2) {
            dbidx = 1;
            /*  scan the second directory. does not save to file. */
            if (verbose != 0) {
                printf ("scanning '%s'\n", homepath [1]);
            }

            /*  homepath is used for opening dir but not saved to disk */
            homepathlen = strlen (homepath [1]);

            /*  scand homepath */
            dtscandir (dbidx, homepath [1]);

            fprintf (stderr, "\n");

            if (sort) {
                /*  sort if not disabled */
                dirsort (dbidx, SORTBYDIR);
            }
            retrive2nd = 1;
        }
    }


    if (printdirtree != 0) {
/*---------------------------------------------------------------------------
 -  Print dir tree contents
 ----------------------------------------------------------------------------*/
        printit (0, outputfmt);
        if (retrive2nd != 0) {
            printf ("\n\nListing content of '%s'\n\n\n", datafile2);
            printit (1, outputfmt);
        }
    }

/*---------------------------------------------------------------------------
 ----------------------------------------------------------------------------
 -
 -  Start processing
 -
 ----------------------------------------------------------------------------
 ----------------------------------------------------------------------------*/

    if ((retrive2nd == 0) && (printmove != 2)) {
/*---------------------------------------------------------------------------
 -  If retrieved one dir tree, report tree size
 ----------------------------------------------------------------------------*/
        if (dotreesize != 0) {
            treesize (0, homepath [0], treesizemode, outputfmt);
        }
        if (equalmask != 0) {
            dupchk (0, homepath [0], equalmask);
        }
    } else {
/*---------------------------------------------------------------------------
 -  If retrieved two dir trees, compare them
 ----------------------------------------------------------------------------*/
        if (outputfmt == OUTPUTTXT) {
            puts ("  *****************************************************************");
            printf ("    Comparing directories and files in two directory trees:\n");
#if MDSMSG
            printf ("    \"this\" = %s (reference)\n", datafile);
            printf ("    \"that\" = %s\n", datafile2);
#else
            printf ("    \"this\" = %s (1)\n", datafile);
            printf ("    \"that\" = %s (2)\n", datafile2);
#endif
        }
        if (printmove == 0) {
            for (i = 1; i & CMD_ALL; i <<= 1) {
                if (i & cmd) {
                    diff2 (i, homepath [0], homepath [1], outputfmt);
                }
            }
        } else if (printmove == 2) {
            diff2mv (CMD_THIS_DUP, homepath [0], homepath [1], outputfmt);
        } else {
            /*  list directory in this only */
            if (CMD_MD & cmd) {
                diff2mv ( -1, homepath [0], homepath [1], outputfmt);
            }
            /*  cp files */
            for (i = 1; i & CMD_ALL_DUP; i <<= 1) {
                if (i & cmd) {
                    diff2mv (i, homepath [0], homepath [1], outputfmt);
                }
            }
            /*  list directory in that only */
            /*  rd dirs */
            if (CMD_RD & cmd) {
                diff2mv ( -2, homepath [0], homepath [1], outputfmt);
            }
        }
        if (outputfmt == OUTPUTTXT) {
            puts ("  *****************************************************************");
            printf ("    *** %ld output lines printed\n", nooutput);
            puts ("= same  1 this only  > this newer  } this moved  ] this dup\n"
                  "# diff  2 that only  < that newer  { that moved  [ that dup\n");
        }
        if (outputfmt == OUTPUTCPY) {
            puts ("rem  *****************************************************************");
            printf ("rem    *** %ld output lines printed\n", nooutput);
        }
        if (outputfmt == OUTPUTCSV) {
            printf ("*** %ld output lines printed\n", nooutput);
            puts ("= same  1 this only  > this newer  } this moved  ] this dup\n"
                  "# diff  2 that only  < that newer  { that moved  [ that dup\n");
        }
    }



/*---------------------------------------------------------------------------
 -  Final report
 ----------------------------------------------------------------------------*/
    fprintf (stderr, "%ld directories %ld files\n", nodirs [0], dirptr [0]);
    if (verbose != 0) {
        report (nodirs [0], dirptr [0]);
        if (retrive2nd != 0) {
            report (nodirs [1], dirptr [1]);
        }
    }

    return (0);
} /* main */


