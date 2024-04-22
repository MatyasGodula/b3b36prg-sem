#include "utils.h"

void my_assert(bool r, const char *func_name, int line, const char *file_name)
{
    if (!r) {
        fprintf(stderr, "\rError: %s() failed on line %d in file %s\n", func_name, line, file_name);
        exit(GENERIC_ERROR);
    }  
}

void* my_alloc(size_t size)
{
    void* ret = (void*)malloc(size);
    if (!ret) {
        fprintf(stderr, MALLOC_ERR_MESSAGE, __func__, __FILE__, __LINE__);
        exit(MALLOC_ERROR);
    }
    return ret;
}

void call_termios(int reset)
{
   static struct termios tio, tioOld;
   tcgetattr(STDIN_FILENO, &tio);
   if (reset) {
      tcsetattr(STDIN_FILENO, TCSANOW, &tioOld);
   } else {
      tioOld = tio; //backup 
      cfmakeraw(&tio);
      tcsetattr(STDIN_FILENO, TCSANOW, &tio);
   }
}