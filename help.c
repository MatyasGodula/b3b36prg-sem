#include "help.h"

#include <stdio.h>

#define RESET   "\033[0m"
#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[0;33m"
#define BLUE    "\033[0;34m"
#define MAGENTA "\033[0;35m"
#define CYAN    "\033[0;36m"
#define WHITE   "\033[0;37m"
#define BRIGHT_BLUE "\033[0;94m"

void print_help()
{
    printf("\r");
    printf("Help text for guidance with using this program\n\n");

    printf(YELLOW, "s", RESET, " ..... sets up the computation, program can not compute without this\n");
    printf(GREEN, "c", RESET, " ..... starts the computation and drawing of the image\n");
    printf(BLUE, "e", RESET, " ..... resets the whole image\n");
    printf(MAGENTA, "x", RESET, " ..... saves the image that is currently displayed in the window\n");
    printf(CYAN, "a", RESET, " ..... aborts the currently running program\n");
    printf(RED, "q", RESET, " ..... quits the program\n\n");

    printf("\rfor further info down the line feel free to press ", BRIGHT_BLUE "h", RESET, " to display this help message again\n");

    printf("\n\n\n");
}