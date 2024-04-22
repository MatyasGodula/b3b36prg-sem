#include <stdio.h>

int main(void)
{
   for (int i = 2147483640; i >= 0; ++i) {
      printf("%i %x\n", i, i);
   }
   return 0;
}
