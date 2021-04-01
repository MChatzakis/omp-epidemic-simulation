#include <stdio.h>

unsigned int hash(unsigned int x)
{
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

int main()
{
    printf("hh: %u\n", hash(1198739817)%509);
    return 0;
}