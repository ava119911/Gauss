#ifndef _ARRAY_H_
#define _ARRAY_H_

#include "cmnhdr.h"

typedef struct _Array {
    int iLength;
    PBYTE pStorage;
    SIZE_T cbElementSize;
} *Array;


Array ArrayNew(int iLength, SIZE_T cbElementSize);

VOID ArrayFree(Array array);

int ArrayLinearSearch(Array array, LPCVOID pElement, 
                                        CompareFunction fnCompare); 

PVOID ArrayGet(Array array, int i);

VOID ArrayPut(Array array, int i, LPCVOID pElement);

BOOL ArrayResize(Array array, int iNewLength);

VOID ArrayShift(Array array, int iStart, int iEnd, int iDistance);


#endif  /* _ARRAY_H_ */