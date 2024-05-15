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
#include "xwin_sdl.h"
#include "help.h"

static void process_pipe_message(event* const ev);

/*
manages different actions that are supposed to happen in the app, evaluates the situation and acts accordingly
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
    print_help();
    //print_check();
    bool success = read_input_file();
    if (!success) {
        set_quit();
    }
    //print_check();
    // initialize computation & visualization
    do {
        event ev = queue_pop();
        event ev1; // if i need to add more events
        msg.type = MSG_NBR;
        switch (ev.type) {
            case EV_QUIT:
                debug("Quit received");
                set_quit();
                break;
            case EV_GET_VERSION:
                msg.type = MSG_GET_VERSION;
                break;
            case EV_READ:
                bool success = read_input_file();
                if (!success) {
                    set_quit();
                    break;
                }
                ev.type = EV_SET_COMPUTE;
                queue_push(ev);
            case EV_SET_COMPUTE:
                info(set_compute(&msg) ? "set_compute success" : "set_compute fail");
                set_up_local_computation();
                break;
            case EV_VIDEO:
                if (is_video()) {
                    set_video();
                    ev1.type = EV_COMPUTE_CPU;
                    queue_push(ev1);
                }
                break;
            case EV_COMPUTE_KB:
                if (is_computing() && !is_aborted()) {
                    warning("Please do not press c twice in a row");
                } else {
                    unabort();
                    compute(&msg);
                }
                break;
            case EV_COMPUTE:
                if (!is_aborted()) {
                    compute(&msg);
                }
                break;
            case EV_RESET_CHUNK:
                cancel_computing();
                break;
            case EV_ABORT:
                abort_comp();
                //printf("abort sent to the comp_module\n");
                msg.type = MSG_ABORT;
                //abort_comp();
                break;
            case EV_PIPE_IN_MESSAGE:
                process_pipe_message(&ev);
                break;
            case EV_SAVE_IMAGE:
                save_surface_to_image("output.png");
                break;
            case EV_ERASE_IMAGE:
                clean_image();
                gui_refresh();
                break;
            case EV_REFRESH:
                gui_refresh();
                break;
            case EV_HELP:
                print_help();
                break;
            case EV_ZOOM_IN:
                if (!is_video()) {
                    zoom_in();
                    ev.type = EV_SET_COMPUTE;
                    ev1.type = EV_COMPUTE_CPU;
                    queue_push(ev);
                    queue_push(ev1);
                }
                break;
            case EV_ZOOM_OUT:
                if (!is_video()) {
                    zoom_out();
                    ev.type = EV_SET_COMPUTE;
                    ev1.type = EV_COMPUTE_CPU;
                    queue_push(ev);
                    queue_push(ev1);
                }
                break;
            case EV_MOVE_LEFT:
                if (!is_video()) {
                    move_left(0.2);
                    ev.type = EV_SET_COMPUTE;
                    ev1.type = EV_COMPUTE_CPU;
                    queue_push(ev);
                    queue_push(ev1);
                }
                break;
            case EV_MOVE_RIGHT:
                if (!is_video()) {
                    move_right(0.2);
                    ev.type = EV_SET_COMPUTE;
                    ev1.type = EV_COMPUTE_CPU;
                    queue_push(ev);
                    queue_push(ev1);
                }
                break;
            case EV_MOVE_UP:
                if (!is_video()) {
                    move_up(0.2);
                    ev.type = EV_SET_COMPUTE;
                    ev1.type = EV_COMPUTE_CPU;
                    queue_push(ev);
                    queue_push(ev1);
                }
                break;
            case EV_MOVE_DOWN:
                if (!is_video()) {
                    move_down(0.2);
                    ev.type = EV_SET_COMPUTE;
                    ev1.type = EV_COMPUTE_CPU;
                    queue_push(ev);
                    queue_push(ev1);
                }
                break;
            case EV_COMPUTE_CPU:
                if (!quit) {
                    if (is_done()) {
                        gui_refresh();
                        if (is_video()) {
                            if (change_iters(video_target())) {
                                cancel_done();
                                set_up_local_computation();
                                compute_local();
                            } else {
                                ev.type = EV_SET_COMPUTE;
                                queue_push(ev);
                            }
                        } else {
                            info("computation done");
                            ev.type = EV_SET_COMPUTE;
                            queue_push(ev);
                        }
                    } else {
                        compute_local();
                    }
                }
                break;
            default:
                break;
        } // switch end
        if (msg.type != MSG_NBR) {
            my_assert(fill_message_buf(&msg, msg_buffer, sizeof(message), &msg_len), __func__, __LINE__, __FILE__);
            // writes information back into the pipe_out
            if (write(pipe_out, msg_buffer, msg_len) == msg_len) {
            } else {
                error("send message failed");
            }
        }
        quit = is_quit();
    } while (!quit);
    gui_cleanup();
    computation_cleanup();
    queue_cleanup();

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
            info("message OK received");
            break;
        case MSG_VERSION:
            fprintf(stderr, "INFO: Module version %d.%d-p%d\n", msg->data.version.major, msg->data.version.minor, msg->data.version.patch);
            break;
        case MSG_COMPUTE_DATA:
            update_data(&(msg->data.compute_data));
            break;
        case MSG_DONE:
            printf("msg done received\n");
            gui_refresh();
            if (is_done()) {
                //gui_refresh();
                //info("Computation done");
                event ev1 = { .type = EV_SET_COMPUTE };
                queue_push(ev1);
            } else if (is_computing()) {
                printf("compute pushed into the queue\n");
                event ev = { .type = EV_COMPUTE };
                queue_push(ev);
            } else {}
            break;
        case MSG_ABORT:
            info("computation successfully aborted by the comp_module");
            break;
        default:
            fprintf(stderr, "Unhandled pipe message type %d\n", msg->type);
            break;
        
    } // end of switch (msg->type)
    free(ev->data.msg);
    ev->data.msg = NULL;
}