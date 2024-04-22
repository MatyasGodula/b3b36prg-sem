#include <stdio.h>

int main(void)
{
   char n = -1;
   char i = 0;
   while (i != n) {
      printf("i: %i n: %i\n", i, n);
      i++;
   }
   return 0;
}
