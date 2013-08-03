#include "array.h"
#include "mem.h"
#include <assert.h>
#include <math.h>

Array ArrayNew(int iLength, SIZE_T cbElementSize)
{
    Array array = NULL;

    assert(iLength >= 0 && cbElementSize > 0);
    
    if (!(array = MALLOC(sizeof(*array))))
        return NULL;

    array->iLength = iLength;
    array->cbElementSize = cbElementSize;

    if (iLength > 0) {
        array->pStorage = CALLOC(iLength, cbElementSize);
        if (!array->pStorage) {
            FREE(array);
            return NULL;
		}
	} else {
        array->pStorage = NULL;
	}

    return array;
}

VOID ArrayFree(Array array)
{
    if (array) {
        FREE(array->pStorage);
        FREE(array);
	}
}

int ArrayLinearSearch(Array array,LPCVOID pElement, 
					                    CompareFunction fnCompare) 
{
    int i;

    assert(array);
    assert(pElement);
    assert(fnCompare);

    for (i = 0; i < array->iLength; i++) {
        if (fnCompare(array->pStorage + i * array->cbElementSize, pElement) == 0) {
            return i;
		}
	}

    return -1;
}


PVOID ArrayGet(Array array, int i)
{
    assert(array);
    assert(i >= 0 && i < array->iLength);

    return array->pStorage + i * array->cbElementSize;
}

VOID ArrayPut(Array array, int i, LPCVOID pElement)
{
	assert(array);
	assert(i >= 0 && i < array->iLength);
    assert(pElement);

    memcpy(array->pStorage + i * array->cbElementSize, 
		             pElement, array->cbElementSize);
}

BOOL ArrayResize(Array array, int iNewLength)
{
    assert(array);
    assert(iNewLength >= 0);

    if (iNewLength == 0) {
        if (array->pStorage) {
            FREE(array->pStorage);
            array->pStorage = NULL;
		} 
	} else if (array->iLength == 0) {
        array->pStorage = MALLOC(array->cbElementSize * iNewLength);
        if (!array->pStorage)
            return FALSE;
	} else {
        PBYTE tmpptr = REALLOC(array->pStorage, 
			                                            array->cbElementSize * iNewLength);
        if (tmpptr) 
            array->pStorage = tmpptr;
        else
            return FALSE;
	}

    array->iLength = iNewLength;
    return TRUE;
}

VOID ArrayShift(Array array, int iStart, int iEnd, int iDistance)
{
    assert(array);
    assert(iStart >= 0 && iStart <= iEnd && iEnd < array->iLength);
    assert(iDistance >= 0 ? iEnd + iDistance <= array->iLength : iStart + iDistance >=0);

    memmove(ArrayGet(array, iStart + iDistance), 
		                 ArrayGet(array, iStart), 
						 (iEnd - iStart) * array->cbElementSize);
}
