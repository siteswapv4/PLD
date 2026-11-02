#pragma once
#include <SDL3/SDL.h>

/* Default arrayList size, modify it to fit your needs */
#define PLD_ARRAYLIST_SIZE 1000

/* Structure containing all arrayList information, access the fields directly to get the data */
typedef struct
{
    void** data;
    int size;
    int len;
}PLD_ArrayList;

/* Allocates a new arrayList of PLD_ARRAYLIST_SIZE on the heap
 * Always call PLD_FreeArrayList after use
 */
PLD_ArrayList* PLD_CreateArrayList();

/* Adds the data at the end of the data array, resizes if necessary */
int PLD_ArrayListAdd(PLD_ArrayList* array, void* data);

/* Adds the data at the specified index, resizes if necessary, index must be valid (> 0) */
int PLD_ArrayListAddAt(PLD_ArrayList* array, void* data, int index);

/* Finds and removes the data, shrinks the array */
int PLD_ArrayListRemove(PLD_ArrayList* array, void* data);

/* Removes the data at specified index, shrinks the array, returns the data */
void* PLD_ArrayListRemoveAt(PLD_ArrayList* array, int index);

/* Frees an arrayList, if freeFunc != NULL it'll be called on each element of the data array */
int PLD_DestroyArrayList(PLD_ArrayList* array, void (*freeFunc)(void*));