#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#include "main.h"
#include "keyboard.h"
#include "event_queue.h"
#include "utils.h"
#include "prg_io_nonblock.h"
#include "gui.h"

#ifndef IO_READ_TIMEOUT_MS
#define IO_READ_TIMEOUT_MS 1
#endif

void* read_pipe_thread(void*); 

/*
takes information from the comp module and interprets it, also opens pipes for reading and writing and also starts 
different threads that each do something else
*/

int main(int argc, char* argv[]) 
{
    int ret = EXIT_SUCCESS;
	// changed these two lines
    const char* fname_pipe_in = argc > 1 ? argv[1] : "/tmp/computational_module.out";
    const char* fname_pipe_out = argc > 2 ? argv[2] : "/tmp/computational_module.in";

    int pipe_in = io_open_read(fname_pipe_in);
    int pipe_out = io_open_write(fname_pipe_out);

	printf("finished opening\n");
    my_assert(pipe_in != -1 && pipe_out != -1, __func__, __LINE__, __FILE__);

    enum {KEYBOARD_THREAD, READ_PIPE_THREAD, MAIN_THREAD, WIN_THREAD, NUM_THREADS};
    const char* thread_names[] = {"Keyboard", "ReadPipe", "Main", "GuiWin"};
    pthread_t threads[NUM_THREADS];
    void* (*thread_functions[])(void*) = {keyboard_thread, read_pipe_thread, main_thread, gui_win_thread};
    void* thread_data[NUM_THREADS];
    thread_data[READ_PIPE_THREAD] = &pipe_in;
    thread_data[MAIN_THREAD] = &pipe_out;

	for (int i = 0; i < NUM_THREADS; ++i) {
		int r = pthread_create(&threads[i], NULL, thread_functions[i], thread_data[i]);
		printf("Create thread '%s' %s\r\n", thread_names[i], ( r == 0 ? "OK" : "FAIL") );
	}

	int *ex;
	for (int i = 0; i < NUM_THREADS; ++i) {
		printf("Call join to the thread %s\r\n", thread_names[i]);
		int r = pthread_join(threads[i], (void*)&ex);
		printf("Joining the thread %s has been %s\r\n", thread_names[i], (r == 0 ? "OK" : "FAIL"));
	}

    io_close(pipe_in);
    io_close(pipe_out);

    return ret;
}

void* read_pipe_thread(void* data)
{
	my_assert(data != NULL, __func__, __LINE__, __FILE__);
	int pipe_in = *(int*)data;
	fprintf(stderr, "read_pipe thread started\n");
	bool end = false;
	uint8_t msg_buf[sizeof(message)];
	int i = 0;
	int len = 0;

	unsigned char c;
	while (io_getc_timeout(pipe_in, IO_READ_TIMEOUT_MS, &c) > 0) {}; // discard garbage

	while (!end) {
		int ret = io_getc_timeout(pipe_in, IO_READ_TIMEOUT_MS, &c);
		if (ret > 0) { // char has been read
			if (i == 0) {
				if (get_message_size(c, &len)) {
					msg_buf[i++] = c;
				} else {
					fprintf(stderr, "Unknown message type detected 0x%x\n", c);
				}
			} else { // read remaining bytes of message
				msg_buf[i++] = c;
			}
			if (len  > 0 && i == len) {
				message* msg = my_alloc(sizeof(message));
				if (parse_message_buf(msg_buf, len, msg)) {
					event ev = { .type = EV_PIPE_IN_MESSAGE };
					ev.data.msg = msg;
					queue_push(ev);
				} else {
					fprintf(stderr, "Error: Cannot parse message type %d\n", msg_buf[0]);
					free(msg);
				}
				i = len = 0;
			}
		} else if (ret == 0) { // timeout happened

		} else { // error occurred
			fprintf(stderr, "Error: problem reading from a file\n");
			set_quit();
			event ev = { .type = EV_QUIT }; // deactivated for now
			/*
			 * added queue_push to push the quit into the queue 
			 */
			queue_push(ev);
		}
		end = is_quit();
	} // end of while cycle
	
	fprintf(stderr, "read_pipe thread ended\n");
    return NULL;
}
