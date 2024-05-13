   #include "help.h"

#include <stdio.h>

#define RESET          "\033[0m"
#define RED            "\033[0;31m"
#define GREEN          "\033[0;32m"
#define YELLOW         "\033[0;33m"
#define BLUE           "\033[0;34m"
#define MAGENTA        "\033[0;35m"
#define CYAN           "\033[0;36m"
#define WHITE          "\033[0;37m"

#define BRIGHT_BLUE    "\033[0;94m"
#define BRIGHT_RED     "\033[0;91m"
#define BRIGHT_GREEN   "\033[0;92m"
#define BRIGHT_YELLOW  "\033[0;93m"
#define BRIGHT_BLUE    "\033[0;94m"
#define BRIGHT_MAGENTA "\033[0;95m"
#define BRIGHT_CYAN    "\033[0;96m"

#define BOLD_RED       "\033[1;31m"
#define BOLD_GREEN     "\033[1;32m"
#define BOLD_YELLOW    "\033[1;33m"
#define BOLD_BLUE      "\033[1;34m"
#define BOLD_MAGENTA   "\033[1;35m"
#define BOLD_CYAN      "\033[1;36m"


void print_help()
{
    printf("\n\n\n");
    printf("\r");
    printf("Help text for guidance with using this program\n\n");

    printf(BRIGHT_BLUE "s" RESET " ..... sets up the computation, program can not compute without this\n");
    printf(BRIGHT_BLUE "c" RESET " ..... starts the computation and drawing of the image\n");
    printf(BRIGHT_BLUE "1" RESET " ..... calculates the fractal in the control app\n");
    printf(BRIGHT_BLUE "e" RESET " ..... resets the whole image\n");
    printf(BRIGHT_BLUE "p" RESET " ..... saves the image that is currently displayed in the window\n");
    printf(BRIGHT_BLUE "a" RESET " ..... aborts the currently running program\n");
    printf(BRIGHT_BLUE "v" RESET " ..... plays the animation if animation was selected in input_parameters\n");
    printf(BOLD_RED "q" RESET " ..... quits the program\n\n");
    printf("\n\n");
    printf("\rIf you have not chosen to see the animation you can use\nthe arrow keys and - + in the window to move and zoom into the image\n(+ is actually binded to = to be used on US layouts so you might need to change your keyboard settings)\n\n");
    printf("\rIf you happened to change anything in the input_parameters file\npress " BRIGHT_BLUE "r" RESET " to read the file again and update the values\n\n");
    printf("\rFor further info down the line feel free to press " BRIGHT_GREEN "h" RESET " to display this help message again\n\n");

    printf("\n\n\n");
}