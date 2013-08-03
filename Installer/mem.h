#ifndef _MEM_H_
#define _MEM_H_

#include <stdlib.h>

#define MALLOC(nbytes) malloc((nbytes))
#define CALLOC(num, size) calloc((num), (size))
#define REALLOC(ptr, size) realloc((ptr), (size))
#define FREE(ptr) free(ptr)

#endif  /* _MEM_H_ */