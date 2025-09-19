#include <PLD_array_list.h>

int PLD_IncreaseArrayList(PLD_ArrayList* array)
{
    array->size += PLD_ARRAYLIST_SIZE;
    array->data = SDL_realloc(array->data, array->size * sizeof(void*));

    return 0;
}

PLD_ArrayList* PLD_CreateArrayList()
{
    PLD_ArrayList* array = SDL_malloc(sizeof(PLD_ArrayList));

    array->data = SDL_malloc(PLD_ARRAYLIST_SIZE * sizeof(void*));
    array->size = PLD_ARRAYLIST_SIZE;
    array->len = 0;

    return array;
}

int PLD_ArrayListAdd(PLD_ArrayList* array, void* data)
{
    if (array->size == array->len)
    {
        PLD_IncreaseArrayList(array);
    }

    array->data[array->len] = data;
    array->len++;

    return 0;
}

int PLD_ArrayListAddAt(PLD_ArrayList* array, void* data, int index)
{
    if (array->size == array->len)
    {
        PLD_IncreaseArrayList(array);
    }

    for (int i = array->len; i > index; i--)
    {
        array->data[i] = array->data[i-1];
    }

    array->data[index] = data;
    array->len++;

    return 0;
}

int PLD_ArrayListRemove(PLD_ArrayList* array, void* data)
{
    for (int i = 0; i < array->len; i++)
    {
        if (array->data[i] == data)
        {
            for (int j = i + 1; j < array->len; j++)
            {
                array->data[j - 1] = array->data[j];
            }

            array->len--;

            return 0;
        }
    }

    return 1;
}

void* PLD_ArrayListRemoveAt(PLD_ArrayList* array, int index)
{
    void* data = array->data[index];
    
    for (int i = index + 1; i < array->len; i++)
    {
        array->data[i - 1] = array->data[i];
    }

    array->len--;

    return data;
}

int PLD_DestroyArrayList(PLD_ArrayList* array, void (*freeFunc)(void*))
{
    if (array != NULL)
    {
        if (freeFunc != NULL)
        {
            for (int i = 0; i < array->len; i++)
            {
                freeFunc(array->data[i]);
            }
        }

        SDL_free(array->data);
        SDL_free(array);
    }

    return 0;
}