#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>

/* This post explains the reason of why do { ... } while(0);
 * is used instead of a simple { ... } block;
 * https://groups.google.com/g/comp.lang.c/c/xGZxls194mI/m/dEIpTKz2okMJ
 */

#define INIT_CAPACITY 1
#define EXP_FACTOR 2

#define v_init(_v) \
    do { \
        (_v).size = 0; \
        (_v).capacity = INIT_CAPACITY; \
        (_v).items = malloc(INIT_CAPACITY * sizeof(*(_v).items)); \
    } while (0);

#define v_append(_v, _item) \
    do { \
        if ((_v).size == (_v).capacity) { \
            (_v).items = realloc((_v).items, ((_v).capacity * EXP_FACTOR) * sizeof(*(_v).items)); \
            (_v).capacity = (_v).capacity * EXP_FACTOR; \
        } \
        (_v).items[(_v).size++] = _item; \
    } while (0);

#endif
