#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "main.h"
#include "event_queue.h"
#include "utils.h"
#include "messages.h"
#include "computation.h"
#include "gui.h"

static void process_pipe_message(event* const ev);

/*

*/

void* main_thread(void* data) 
{
    my_assert(data != NULL, __func__, __LINE__, __FILE__);
    // main.c uses pipe_out to write into so it has the write end
    int pipe_out = *(int*)data;
    message msg;
    uint8_t msg_buffer[sizeof(message)];
    int msg_len;
    bool quit = false;

    computation_init();
    gui_init();
    // initialize computation & visualization
    do {
        event ev = queue_pop();
        msg.type = MSG_NBR;
        switch (ev.type) {
            case EV_QUIT:
                debug("Quit received");
                set_quit();
                break;
            case EV_GET_VERSION:
                msg.type = MSG_GET_VERSION;
                break;
            case EV_SET_COMPUTE:
                info(set_compute(&msg) ? "set_compute success" : "set_compute fail");
                break;
            case EV_COMPUTE:
                info(compute(&msg) ? "compute success" : "compute fail");
                break;
            case EV_ABORT:
                // TODO
                break;
            case EV_PIPE_IN_MESSAGE:
                process_pipe_message(&ev);
                break;
            default:
                break;
        } // switch end
        if (msg.type != MSG_NBR) {
            my_assert(fill_message_buf(&msg, msg_buffer, sizeof(message), &msg_len), __func__, __LINE__, __FILE__);
            // writes information back into the pipe_out
            if (write(pipe_out, msg_buffer, msg_len) == msg_len) {
                debug("send data to pipe out");
            } else {
                error("send message failed");
            }
        }
        quit = is_quit();
    } while (!quit);
    gui_cleanup();
    computation_cleanup();

    // cleanup computation, visualization
    return NULL;
}

void process_pipe_message(event* const ev)
{
    my_assert(ev != NULL && ev->type == EV_PIPE_IN_MESSAGE && ev->data.msg, __func__, __LINE__, __FILE__);
    ev->type = EV_TYPE_NUM;
    const message* msg =  ev->data.msg;
    switch (msg->type) {
        case MSG_OK:
            info("OK");
            break;
        case MSG_VERSION:
            fprintf(stderr, "INFO: Module version %d.%d-p%d\n", msg->data.version.major, msg->data.version.minor, msg->data.version.patch);
            break;
        case MSG_COMPUTE_DATA:
            update_data(&(msg->data.compute_data));
            break;
        case MSG_DONE:
            gui_refresh();
            if (is_done()) {
                info("Computation done");
            } else {
                event ev = { .type = EV_COMPUTE };
                queue_push(ev);
            }
            break;
        default:
            fprintf(stderr, "Unhandled pipe message type %d\n", msg->type);
            break;
        
    } // end of switch (msg->type)
    free(ev->data.msg);
    ev->data.msg = NULL;
}