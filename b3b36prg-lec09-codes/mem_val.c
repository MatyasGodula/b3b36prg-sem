#include <stdio.h>
#include <stdlib.h>

int main(void)
{
   int *a = malloc(2 * sizeof *a);

   for (int i = 0; i < 3; ++i) {
      a[i] = i;
   }
   for (int i = 0; i < 3; ++i) {
      printf("%d\n", a[i]);
   }
   //free(a);
   return 0;
}
