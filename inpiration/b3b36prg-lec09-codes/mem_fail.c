#include <stdio.h>
#include <stdlib.h>

int main(void)
{
   const size_t size = 20 * 1024 * 1024; // 20 MB
   size_t *a = malloc(size * sizeof *a);  // 20 MB * sizeof(long)
   if (!a) {
      fprintf(stderr, "ERROR: malloc failed!\n");
      return -1;
   }
   for (size_t i = 0; i < size; ++i) {
      a[i] = i;
   }
   fprintf(stderr, "INFO: array of %lu size_t values initialized.\n", size);
   free(a);
   return 0;
}
