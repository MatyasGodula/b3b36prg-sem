/*
 * File name: threads.c
 * Date:      2016/11/03 07:24
 * Author:    Jan Faigl
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <termios.h>
#include <unistd.h> // for STDIN_FILENO

#include <threads.h>

#define PERIOD_STEP 10
#define PERIOD_MAX 2000
#define PERIOD_MIN 10

typedef struct {
   int alarm_period;
   int alarm_counter;
   bool quit;
   mtx_t *mtx;
   cnd_t *cond;
} data_t;


void call_termios(int reset);

int input_thread(void*);
int output_thread(void*);
int alarm_thread(void*);

// - main function -----------------------------------------------------------
int main(int argc, char *argv[])
{
   data_t data = { .alarm_period = 100, .alarm_counter = 0, .quit = false };

   enum { INPUT, OUTPUT, ALARM, NUM_THREADS };
   const char *threads_names[] = { "Input", "Output", "Alarm" };

   int (*thr_functions[])(void*) = { input_thread, output_thread, alarm_thread };

   thrd_t threads[NUM_THREADS];
   mtx_t mtx;
   cnd_t cond;
   mtx_init(&mtx, mtx_plain); // initialize mutex 
   cnd_init(&cond); // initialize condition variable 
   data.mtx = &mtx;
   data.cond = &cond;

   call_termios(0);

   for (int i = 0; i < NUM_THREADS; ++i) {
      int r = thrd_create(&threads[i], thr_functions[i], &data);
      printf("Create thread '%s' %s\r\n", threads_names[i], ( r == thrd_success ? "OK" : "FAIL") );
   }

   int ex;
   for (int i = 0; i < NUM_THREADS; ++i) {
      printf("Call join to the thread %s\r\n", threads_names[i]);
      int r = thrd_join(threads[i], (void*)&ex);
      printf("Joining the thread %s has been %s - exit value %i\r\n", threads_names[i], (r == thrd_success ? "OK" : "FAIL"), ex);
   }

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
      tcsetattr(STDIN_FILENO, TCSANOW, &tio);
   }
}

// - function -----------------------------------------------------------------
int input_thread(void* d)
{
   data_t *data = (data_t*)d;
   static int r = 0;
   int c;
   while (( c = getchar()) != 'q') {
      mtx_lock(data->mtx);
      int period = data->alarm_period;
      switch(c) {
         case 'r':
            period -= PERIOD_STEP;
            if (period < PERIOD_MIN) {
               period = PERIOD_MIN;
            }
            break;
         case 'p':
            period += PERIOD_STEP;
            if (period > PERIOD_MAX) {
               period = PERIOD_MAX;
            }
            break;
      }
      if (data->alarm_period != period) {
         cnd_signal(data->cond);
      }
      data->alarm_period = period;
      mtx_unlock(data->mtx);
   }
   r = 1;
   mtx_lock(data->mtx);
   data->quit = true;
   cnd_broadcast(data->cond);
   mtx_unlock(data->mtx);
   fprintf(stderr, "Exit input thread\r\n");
   return r;
}

// - function -----------------------------------------------------------------
int output_thread(void* d)
{
   data_t *data = (data_t*)d;
   static int r = 0;
   bool q = false;
   mtx_lock(data->mtx);
   while (!q) {
      cnd_wait(data->cond, data->mtx); // wait for next event
      q = data->quit;
      printf("\rAlarm time: %10i   Alarm counter: %10i", data->alarm_period, data->alarm_counter);
      fflush(stdout);
   }
   mtx_unlock(data->mtx);
   fprintf(stderr, "Exit output thread\r\n");
   return r;
}

// - function -----------------------------------------------------------------
int alarm_thread(void* d) 
{
   data_t *data = (data_t*)d;
   static int r = 0;
   mtx_lock(data->mtx);
   bool q = data->quit;
   useconds_t period = data->alarm_period * 1000; // alarm_period is in ms
   mtx_unlock(data->mtx);

   while (!q) {
      usleep(period);
      mtx_lock(data->mtx);
      q = data->quit;
      data->alarm_counter += 1;
      period = data->alarm_period * 1000; // update the period is it has been changed
      cnd_broadcast(data->cond);
      mtx_unlock(data->mtx);
   }
   fprintf(stderr, "Exit alarm thread\r\n");
   return r;
}

/* end of threads.c */
