#include "arm_book_lib.h"

// Implementación personalizada de strlen
unsigned int len(const char *str) 
{
    unsigned int length = 0;
    while (str[length] != '\0') {
        length++;
    }
    return length;
}