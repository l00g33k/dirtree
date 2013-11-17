/*
 * Copyright (c) 2008-2012 l00g33k@gmail.com All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef __TURBOC__
#include <alloc.h>
#else
#include <malloc.h>
#endif

unsigned long CalculateBufferCRCNoTbl (
    unsigned int count,
    unsigned long crc,
    unsigned char *p
);
static unsigned long CRCNoTable (int i);

#define CRC32_POLYNOMIAL     0xEDB88320L


/*  using CRC table did not speed up */
static unsigned long CRC_table [256];

#define CRCSZ 2048

unsigned long int crc32file (char *fname) {
    static char *buf = NULL;
    int i;
    unsigned long int crc;
    FILE *st;

    if (buf == NULL) {
        buf = malloc (CRCSZ);
        if (buf == NULL) {
            printf ("malloc failed %s %d\n", __FILE__, __LINE__);
            return (0);
        }
        for (i = 0; i < 256; i++) {
            CRC_table [i] = CRCNoTable (i);
        }
    }

    st = fopen (fname, "rb");
    if (st == NULL) {
        printf ("Unable to open '%s'\n", fname);
        return (0);
    }

    crc = 0xffffffff;
    while ((i = fread (buf, 1, CRCSZ, st)) > 0) {
        crc = CalculateBufferCRCNoTbl (i, crc, buf);
    }

    fclose (st);

    return (~crc);

} /* crc32file */

#ifndef LIBCALL

int main (int noarg, char *argv []) {
    int i, j;
    unsigned long int crc, length;
    FILE *st;

    if (noarg < 2) {
        puts ("This program computes the 32-bit CRC of the give file.");
        puts ("The computed CRC is the same as the one computed by PKZIP.");
        printf ("Usage: %s filename.ext\n", argv [0]);
        return (1);
    }

    crc = crc32file (argv [1]);

    st = fopen (argv [1], "rb");
    if (st == NULL) {
        return (0);
    }

    fseek (st, 0, 2);
    length = ftell (st);
    fclose (st);

    printf ("32-bit CRC 0x%08lx; ", crc);


    i = 0;
    j = length / 1000000;
    if (j > 0) {
        printf ("%3d,", j);
        length -= j * 1000000;
        i = 1;
    } else {
        printf ("    ");
    }
    j = length / 1000;
    if ((j > 0) || (i != 0)) {
        if (i != 0) {
            printf ("%03d,", j);
        } else {
            printf ("%3d,", j);
        }
        length -= j * 1000;
        i = 1;
    } else {
        printf ("    ");
    }

    j = length;
    if (i != 0) {
        printf ("%03d", j);
    } else {
        printf ("%3d", j);
    }
    printf (" bytes; %s\n", argv [1]);


    return (0);
}

#endif

static unsigned long CRCNoTable (int i) {
    int j;
    unsigned long crc;

    crc = i;
    for ( j = 8 ; j > 0; j-- ) {
        if ( crc & 1 )
            crc = ( crc >> 1 ) ^ CRC32_POLYNOMIAL;
        else
            crc >>= 1;
    }
    return (crc);

} /* CRCNoTable */


/* Routine calculates the CRC for a block of data using no table lookup.
 * It accepts an original value for the crc, and returns the updated value. */
unsigned long CalculateBufferCRCNoTbl (
    unsigned int count,
    unsigned long crc,
    unsigned char *p
) {
    unsigned long temp1;
    unsigned long temp2;

    while ( count-- != 0 ) {
        temp1 = ( crc >> 8 ) & 0x00FFFFFFL;
        /*  using no table */
        /*  using table */
        temp2 = CRC_table [(unsigned char) (( (unsigned char) crc ^ *p++ ) & 0xff)];
        crc = temp1 ^ temp2;
    }
    return( crc );
}
