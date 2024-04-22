/*
 * Filename: prg-lec09-main.c
 * Date:     2019/12/25 21:38
 * Author:   Jan Faigl
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
 
#include <termios.h> 
#include <unistd.h>  // for STDIN_FILENO
 
#include "prg_io_nonblock.h"
 
typedef struct {
   bool quit;
   int fd; // file descriptor
} data_t;
 
void call_termios(int reset);

/// - main ---------------------------------------------------------------------
int main(int argc, char *argv[])
{
   data_t data = { .quit = false, .fd = -1 };
   int c;
   const char *out = argc > 1 ? argv[1] : "/tmp/prg-lec09.pipe";

   data.fd = io_open_write(out);
   if (data.fd == -1) {
      fprintf(stderr, "Cannot open named pipe port %s\n", out);
      exit(100);
   }
   call_termios(0); 

   while (!data.quit && (c = getchar()) != EOF) {
      switch(c) {
	 case 's':
	 case 'e':
	 case '1':
	 case '2':
	 case '3':
	 case '4':
	 case '5':
	 case 'b':
	 case 'h':
	    if (io_putc(data.fd, c) != 1) {
	       fprintf(stderr, "ERROR: Cannot send command to module, exit program\n");
	       data.quit = true;
	    }
	    fsync(data.fd);
	    fprintf(stderr, "Send command '%c'\n", c);
	    break;
	 case 'q':
	    fprintf(stderr, "quit\n");
	    data.quit = true;
	    break;
	 default: // discard all other keys
	    break;
      } //end switch
   } // end 
   fprintf(stderr, "quit\n");
   io_close(data.fd);
   call_termios(1); // restore terminal settings
   return EXIT_SUCCESS;
}

// - function -----------------------------------------------------------------
void call_termios(int reset)
{
   static struct termios tio, tioOld;
   tcgetattr(STDIN_FILENO, &tio);
   if (reset) {
      tcsetattr(STDIN_FILENO, TCSANOW, &tioOld);
   } else {
      tioOld = tio; //backup 
      cfmakeraw(&tio);
      tio.c_oflag |= OPOST;
      tcsetattr(STDIN_FILENO, TCSANOW, &tio);
   }
}

/* end of prg-lec09-main.c */
