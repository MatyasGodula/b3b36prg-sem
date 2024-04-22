#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#include "main.h"
#include "keyboard.h"
#include "event_queue.h"
#include "utils.h"
#include "prg_io_nonblock.h"

void* read_pipe_thread(void*); 

int main(int argc, char* argv[]) 
{
    int ret = EXIT_SUCCESS;
    const char* fname_pipe_in = argc > 1 ? argv[1] : "/tmp/computational_module.out";
    const char* fname_pipe_out = argc > 2 ? argv[2] : "/tmp/computational_module.in";

    int pipe_in = io_open_read(fname_pipe_in);
    int pipe_out = io_open_write(fname_pipe_out);

    my_assert(pipe_in != -1 && pipe_out != -1, __func__, __LINE__, __FILE__);

    enum {KEYBOARD_THREAD, READ_PIPE_THREAD, MAIN_THREAD, NUM_THREADS};
    const char* threads_names[] = {"Keyboard", "ReadPipe", "Main"};
    pthread_t threads[NUM_THREADS];
    void* (*thread_functions[])(void*) = {keyboard_thread, read_pipe_thread, main_thread};
    void* thread_data[NUM_THREADS] = {};
    thread_data[READ_PIPE_THREAD] = &pipe_in;
    thread_data[MAIN_THREAD] = &pipe_out;

    /*
    finished at part 4 15:33
    */

    for (int i = 0; i < NUM_THREADS; ++i) {
      int r = pthread_create(&threads[i], NULL, thread_functions[i], thread_data[i]);
      printf("Create thread '%s' %s\r\n", threads_names[i], ( r == 0 ? "OK" : "FAIL") );
    }

    int *ex;
    for (int i = 0; i < NUM_THREADS; ++i) {
      printf("Call join to the thread %s\r\n", threads_names[i]);
      int r = pthread_join(threads[i], (void*)&ex);
      printf("Joining the thread %s has been %s - exit value %i\r\n", threads_names[i], (r == 0 ? "OK" : "FAIL"), *ex);
    }

    io_close(pipe_in);
    io_close(pipe_out);

    return ret;
}

void* read_pipe_thread(void* data)
{

    return NULL;
}
