#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

typedef unsigned long  U32;
typedef long            S32;
typedef unsigned short U16;
typedef unsigned char  U8;
typedef char            S8;
typedef unsigned long  COLOR;

#define MAX(x, y)    ((x > y) ? x : y)
#define MIN(x, y)    ((x < y) ? x : y)
#define SWAP(x,y)    { (x) ^= (y); (y) ^= (x); (x) ^= (y); }
#define ABS(x)       ((x >= 0) ? x : (-x))

typedef enum {
   IMAGE_SCALE,
   IMAGE_CENTER,
   IMAGE_TILE,
} IMAGE_POSITION;


#endif
