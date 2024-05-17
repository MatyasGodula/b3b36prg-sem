/*
 *
 * Author1: Jan Faigl
 * Author2: Matyas Godula
 *
 */


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

#define MOVEMENT_CONSTANT 0.2

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
                info("Input file read");
            case EV_SET_COMPUTE:
                info(set_compute(&msg) ? "set_compute success" : "set_compute fail");
                set_up_local_computation();
                break;
            case EV_VIDEO:
                if (is_local_set()) {
                    if (!is_computing() || (is_computing() && is_aborted())) {
                        if (is_video()) {
                            unabort();
                            set_video();
                            ev1.type = EV_COMPUTE_CPU;
                            queue_push(ev1);
                        }
                    } else {
                        warning("Please wait for the computation to finish, or press a to first abort");
                    }
                } else {
                    warning("Please first set up the computation using s");
                }
                break;
            case EV_COMPUTE_KB:
                if (is_computing() && !is_aborted()) {
                    warning("Please do not press c while computing");
                } else if (!is_set_up()) {
                    warning("Please first set up the computation using s");
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
                info("Chunk reset, press c or 1 to compute");
                break;
            case EV_ABORT:
                abort_comp();
                info("Abort sent to comp_module");
                msg.type = MSG_ABORT;
                break;
            case EV_PIPE_IN_MESSAGE:
                process_pipe_message(&ev);
                break;
            case EV_SAVE_IMAGE:
                save_surface_to_image("output.png");
                info("Image saved, the file is named output.png");
                break;
            case EV_ERASE_IMAGE:
                clean_image();
                gui_refresh();
                info("Image erased");
                break;
            case EV_REFRESH:
                gui_refresh();
                break;
            case EV_HELP:
                print_help();
                break;
            case EV_ZOOM_IN:
                if (!is_computing()) {
                    if (!is_video()) {
                        zoom_in();
                        ev.type = EV_SET_COMPUTE;
                        ev1.type = EV_COMPUTE_CPU;
                        queue_push(ev);
                        queue_push(ev1);
                    } else {
                        warning("Can not zoom while video is chosen, change input parameters and pres r");
                    }
                }
                break;
            case EV_ZOOM_OUT:
                if (!is_computing()) { 
                    if (!is_video()) {
                        zoom_out();
                        ev.type = EV_SET_COMPUTE;
                        ev1.type = EV_COMPUTE_CPU;
                        queue_push(ev);
                        queue_push(ev1);
                    } else {
                        warning("Can not zoom while video is chosen, change input parameters and pres r");
                    }
                }
                break;
            case EV_MOVE_LEFT:
                if (!is_computing()) {
                    if (!is_video()) {
                        move_left(MOVEMENT_CONSTANT);
                        ev.type = EV_SET_COMPUTE;
                        ev1.type = EV_COMPUTE_CPU;
                        queue_push(ev);
                        queue_push(ev1);
                    } else {
                        warning("Can not move while video is chosen, change input parameters and pres r");
                    }
                }
                break;
            case EV_MOVE_RIGHT:
                if (!is_computing()) {
                    if (!is_video()) {
                        move_right(MOVEMENT_CONSTANT);
                        ev.type = EV_SET_COMPUTE;
                        ev1.type = EV_COMPUTE_CPU;
                        queue_push(ev);
                        queue_push(ev1);
                    } else {
                        warning("Can not move while video is chosen, change input parameters and pres r");
                    }
                }
                break;
            case EV_MOVE_UP:
                if (!is_computing()) {
                    if (!is_video()) {
                        move_up(MOVEMENT_CONSTANT);
                        ev.type = EV_SET_COMPUTE;
                        ev1.type = EV_COMPUTE_CPU;
                        queue_push(ev);
                        queue_push(ev1);
                    } else {
                        warning("Can not move while video is chosen, change input parameters and pres r");
                    }
                } 
                break;
            case EV_MOVE_DOWN:
                if (!is_computing()) {
                    if (!is_video()) {
                        move_down(MOVEMENT_CONSTANT);
                        ev.type = EV_SET_COMPUTE;
                        ev1.type = EV_COMPUTE_CPU;
                        queue_push(ev);
                        queue_push(ev1);
                    } else {
                        warning("Can not move while video is chosen, change input parameters and pres r");
                    }
                }
                break;
            case EV_COMPUTE_CPU_KB:
                if (is_local_set()) { // prevents start without set_up
                    if (!is_computing() || is_aborted()) {
                        ev.type = EV_COMPUTE_CPU;
                        queue_push(ev);
                    }
                } else {
                    warning("Please first set up the computation using s");
                }
                break;
            case EV_COMPUTE_CPU:
                if (!quit) {
                    if (is_done()) {
                        gui_refresh();
                        if (is_video()) { // if video was activated the animation cycles through the iterations
                            if (change_iters(video_target()) && !is_aborted()) {
                                cancel_done();
                                set_up_local_computation();
                                compute_local();
                            } else if (is_aborted()) { // detects the abort 
                                unabort();
                                if (video_target() != 0) { // makes sure the iterations were set up in the first place
                                    set_iters(video_target()); // sets the iterations to the previous target for the animation
                                }
                                ev.type = EV_SET_COMPUTE;
                                queue_push(ev);
                            } 
                            else {
                                ev.type = EV_SET_COMPUTE;
                                queue_push(ev);
                            }
                        } else {
                            info("Computation done");
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
                error("Send message failed");
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
            info("Message OK received");
            break;
        case MSG_VERSION:
            display_module_ver(msg->data.version.major, msg->data.version.minor, msg->data.version.patch);
            break;
        case MSG_COMPUTE_DATA:
            update_data(&(msg->data.compute_data));
            break;
        case MSG_DONE:
            gui_refresh();
            if (is_done()) { // sets up the next computation 
                event ev1 = { .type = EV_SET_COMPUTE };
                queue_push(ev1);
            } else if (is_computing()) { // if not finished pushes a new computation
                event ev = { .type = EV_COMPUTE };
                queue_push(ev);
            } else {}
            break;
        case MSG_ABORT:
            info("Computation successfully aborted by the comp_module");
            break;
        default:
            pipe_message_report(msg->type);
            break;
        
    } // end of switch (msg->type)
    free(ev->data.msg);
    ev->data.msg = NULL;
}