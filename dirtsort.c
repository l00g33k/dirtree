/*
 * Copyright (c) 2008-2012 l00g33k@gmail.com All rights reserved.
 *
 */

#include "dirtree.h"


int qsortidx0, qsortidx1;




int qsortdircmp (const void *a, const void *b) {
/*****************************************************************************
 *
 *  Sort function by dir name then file name
 *
 *****************************************************************************/
    int i;

    i = dirtstrcmp (dnamebuf [qsortidx0] + ((struct dirlist *) a)->dirid,
                    dnamebuf [qsortidx1] + ((struct dirlist *) b)->dirid);
    if (i == 0) {
        i = dirtstrcmp (fnamebuf [qsortidx0] + ((struct dirlist *) a)->fnameidx,
                        fnamebuf [qsortidx1] + ((struct dirlist *) b)->fnameidx);
    }

    return (i);

} /* qsortdircmp */


int qsortdupcmp (const void *a, const void *b) {
/*****************************************************************************
 *
 *  Sort for file duplication search
 *
 *  Sort function by file name, size, CRC, time_t, dir name
 *
 *****************************************************************************/
    int i;



    /*  file name */
    i = dirtstrcmp (fnamebuf [qsortidx0] + ((struct dirlist *) a)->fnameidx,
                    fnamebuf [qsortidx1] + ((struct dirlist *) b)->fnameidx);
    if (i != 0) {
        return (i);
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
    if (((struct dirlist *) a)->tt != ((struct dirlist *) b)->tt) {
        if (((struct dirlist *) a)->tt > ((struct dirlist *) b)->tt) {
            return (1);
        } else {
            return (-1);
        }
    }

    /*  directory name */
    i = dirtstrcmp (dnamebuf [qsortidx0] + ((struct dirlist *) a)->dirid,
                    dnamebuf [qsortidx1] + ((struct dirlist *) b)->dirid);

    return (i);

} /* qsortdupcmp */


int qsortnonamedupcmp (const void *a, const void *b) {
/*****************************************************************************
 *
 *  Sort for file duplication search
 *
 *  Sort function by file name, size, CRC, time_t, dir name
 *
 *****************************************************************************/
    int i;


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
    if (((struct dirlist *) a)->tt != ((struct dirlist *) b)->tt) {
        if (((struct dirlist *) a)->tt > ((struct dirlist *) b)->tt) {
            return (1);
        } else {
            return (-1);
        }
    }

    /*  directory name */
    i = dirtstrcmp (dnamebuf [qsortidx0] + ((struct dirlist *) a)->dirid,
                    dnamebuf [qsortidx1] + ((struct dirlist *) b)->dirid);

    return (i);

} /* qsortnonamedupcmp */


void mysort (
    char HUGE *base,
    long int noelem,
    int size,
    int (*dircmp) (const void *a, const void *b)
) {
/*****************************************************************************
 *
 *  > 64 kbytes sort for 16-bit compiler
 *
 *****************************************************************************/
    void *tmp;
    long int i, j;


    tmp = malloc (size);
    if (tmp == NULL) {
        fprintf (stderr, "%s %d: malloc failed\n", __FILE__, __LINE__);
        return;
    }


    for (i = noelem - 2; i >= 0; i--) {
        if ((i % 10) == 0) {
            fprintf (stderr, ".");
        }
        for (j = 0; j <= i; j++) {
            if (qsortdirsizecmp (base + size * j, base + size * (j + 1)) > 0) {
                memcpy (tmp, (void *) (base + size * j), size);
                memcpy ((void *) (base + size * j),
                        (void *) (base + size * (j + 1)), size);
                memcpy ((void *) (base + size * (j + 1)), tmp, size);
            }
        }
    }
    fprintf (stderr, "\n");

    free (tmp);
} /* mysort */



void dirsort (int dbidx, int sorttype) {
    /* int (*dircmpfn) (const void *a, const void *b);*/
    unsigned long int dircmpfn;

    qsortidx0 = dbidx;
    qsortidx1 = dbidx;


    switch (sorttype) {
        default :
        case SORTBYDIR :
            dircmpfn = (unsigned long int) qsortdircmp;
            break;

        case SORTBYDUP :
            dircmpfn = (unsigned long int) qsortdupcmp;
            break;

        case SORTBYDUPNONM :
            dircmpfn = (unsigned long int) qsortnonamedupcmp;
            break;

    }

#ifdef __TURBOC__
    if ((dirptr [dbidx] * sizeof (struct dirlist)) < 65536u) {
        qsort  ((void *) dirbuf [dbidx], dirptr [dbidx], sizeof (struct dirlist),
            (int (*) (const void *a, const void *b)) dircmpfn);
    } else {
        mysort  ((void *) dirbuf [dbidx], dirptr [dbidx], sizeof (struct dirlist),
            (int (*) (const void *a, const void *b)) dircmpfn);
    }
#else
    qsort  ((void *) dirbuf [dbidx], dirptr [dbidx], sizeof (struct dirlist),
            (int (*) (const void *a, const void *b)) dircmpfn);
#endif


} /* dirsort */


int qsortdirsizecmp (const void *a, const void *b) {
/*****************************************************************************
 *
 *  Sort function by dir name then file name
 *
 *****************************************************************************/
    struct dirsize_ *aa, *bb;

    aa = (struct dirsize_ *) a;
    bb = (struct dirsize_ *) b;

    if (aa->dirthisgbytes != bb->dirthisgbytes) {
        if (aa->dirthisgbytes > bb->dirthisgbytes) {
            return (1);
        } else {
            return (-1);
        }
    }

    if (aa->dirthisbytes != bb->dirthisbytes) {
        if (aa->dirthisbytes > bb->dirthisbytes) {
            return (1);
        } else {
            return (-1);
        }
    }


    return (0);

} /* qsortdirsizecmp */


int qsortttldirsizecmp (const void *a, const void *b) {
/*****************************************************************************
 *
 *  Sort function by dir name then file name
 *
 *****************************************************************************/
    struct dirsize_ *aa, *bb;

    aa = (struct dirsize_ *) a;
    bb = (struct dirsize_ *) b;

    if (aa->dirsubgbytes != bb->dirsubgbytes) {
        if (aa->dirsubgbytes > bb->dirsubgbytes) {
            return (1);
        } else {
            return (-1);
        }
    }

    if (aa->dirsubbytes != bb->dirsubbytes) {
        if (aa->dirsubbytes > bb->dirsubbytes) {
            return (1);
        } else {
            return (-1);
        }
    }


    return (0);

} /* qsortttldirsizecmp */


int qsortpathdirsizecmp (const void *a, const void *b) {
/*****************************************************************************
 *
 *  Sort function by dir name then file name
 *
 *****************************************************************************/
    struct dirsize_ *aa, *bb;

    aa = (struct dirsize_ *) a;
    bb = (struct dirsize_ *) b;

    return (dirtstrcmp (aa->dirname, bb->dirname));

} /* qsortpathdirsizecmp */


void treesizesort (struct dirsize_ *dirsize, unsigned int nodir, int sortby) {
    /* int (*dircmpfn) (const void *a, const void *b);*/
    unsigned long int dircmpfn;

    switch (sortby) {
        default :
        case TREESIZESORTBYDIRSIZE :
            dircmpfn = (unsigned long int) qsortdirsizecmp;
            break;

        case TREESIZESORTBYSUBSIZE :
            dircmpfn = (unsigned long int) qsortttldirsizecmp;
            break;

        case TREESIZESORTBYPAHT :
            dircmpfn = (unsigned long int) qsortpathdirsizecmp;
            break;

    }

#ifdef __TURBOC__
    if ((nodir * sizeof (struct dirsize_)) < 65536u) {
        qsort  ((void *) dirsize, nodir, sizeof (struct dirsize_), (int (*) (const void *a, const void *b)) dircmpfn);
    } else {
        mysort ((void *) dirsize, nodir, sizeof (struct dirsize_), (int (*) (const void *a, const void *b)) dircmpfn);
    }
#else
    qsort      ((void *) dirsize, nodir, sizeof (struct dirsize_), (int (*) (const void *a, const void *b)) dircmpfn);
#endif


} /* treesizesort */


