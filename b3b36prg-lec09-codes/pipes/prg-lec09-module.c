/*
 * Filename: prg-lec09-module.c
 * Date:     2019/12/25 21:37
 * Author:   Jan Faigl
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h>  // for usleep

#ifndef SLEEP_MS
#define SLEEP_MS 10
#endif 

#include "prg_io_nonblock.h"

typedef struct {
   bool quit;
   int fd; // file descriptor
   int period;
   bool enabled;
} data_t;

void draw_computing(void);

/// - main ---------------------------------------------------------------------
int main(int argc, char *argv[])
{
   data_t data = { .quit = false, .fd = -1, .enabled = false, .period = SLEEP_MS };
   int tiks = 0;
   unsigned char c;
   const char *out = argc > 1 ? argv[1] : "/tmp/prg-lec09.pipe";

   data.fd = io_open_read(out);
   if (data.fd == -1) {
      fprintf(stderr, "Cannot open named pipe port %s\n", out);
      exit(100);
   }
   int periods_ms[] = { 50, 100, 200, 500, 1000 };

   while(!data.quit) {
      int r = io_getc_timeout(data.fd, 0, &c);
      if (r == 1) {
	 switch(c) {
	    case 's':
	       data.enabled = false;
	       fprintf(stdout, "stop-s\n");
	       break;
	    case 'e':
	       data.enabled = false;
	       fprintf(stdout, "stop-e\n");
	       break;
	    case 'h':
	       fprintf(stdout, "hello\n");
	       break;
	    case 'b':
	       fprintf(stdout, "bye\n");
	       data.quit = true;
	       break;
	    default:
	       if (c >= '1' && c <= '5') { // (re)attach 
		  data.period = periods_ms[ c - '1' ]; 
		  data.enabled = true;
	       }
	       break;
	 } //end switch
      } // end char read
      usleep(SLEEP_MS * 1000); // smallest period
      tiks += SLEEP_MS;
      if (tiks >= data.period) {
	 tiks = 0;
	 if (data.enabled) {
	    draw_computing();
	 }
      }
   } // end while(!data.quit)
   fprintf(stderr, "quit\n");
   io_close(data.fd);
   return EXIT_SUCCESS;
}

// - function -----------------------------------------------------------------
void draw_computing(void)
{
   static char a[] = { '|', '/', '-', '\\', };
   static unsigned char i = 0;
   fprintf(stdout, "\r%c", a[i++]);
   fflush(stdout);
   i %= 4;
}

/* end of prg-lec09-module.c */
