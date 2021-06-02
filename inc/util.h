#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "types.h"

#define RANDOM(min, max) ((min) + ((float)rand() / ((float)RAND_MAX / ((max) - (min)))))

#define BLOCK_TIMER(name, func) do\
    {\
        clock_t clock_start = clock();\
		func\
		clock_t clock_diff = clock() - clock_start;\
		fprintf(stderr, "[%s: %d] [TIMER] %s took %f ms\n", __FILE__, __LINE__, #name, (double)(clock_diff / (double)CLOCKS_PER_SEC) * 1000.0);\
	} while(0)

#define BLACK 0x00000000
#define WHITE 0xFFFFFFFF

// Vector operations
//================================================================================================
static inline V2 v2(float x, float y)     { return (V2){x, y}; }

static inline V2 v2_add(V2 a, V2 b)       { return v2(a.x + b.x, a.y + b.y); }
static inline V2 v2_sub(V2 a, V2 b)       { return v2(a.x - b.x, a.y - b.y); }
static inline V2 v2_mul(V2 a, V2 b)       { return v2(a.x * b.x, a.y * b.y); }

static inline V2 v2_add_s(V2 a, float b)  { return v2(a.x + b, a.y + b); }
static inline V2 v2_sub_s(V2 a, float b)  { return v2(a.x - b, a.y - b); }
static inline V2 v2_mul_s(V2 a, float b)  { return v2(a.x * b, a.y * b); }
//================================================================================================

#endif /* UTIL_H */
