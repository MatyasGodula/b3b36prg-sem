/*
 *
 * Author1: Jan Faigl
 * Author2: Matyas Godula
 *
 */

#include "keyboard.h"
#include "event_queue.h"
#include "utils.h"

#include <sys/select.h>

void* keyboard_thread(void* data)
{
    fprintf(stderr, "Keyboard thread started\n");
    call_termios(0);
    int c;
    event ev;
    while (!is_quit() && (c = getchar()) != 'q') {
        ev.type = EV_TYPE_NUM;
        switch(c) {
            case 'g': // request the current version of the module
                ev.type = EV_GET_VERSION;
                break;
            case 's': // set up the computation
                ev.type = EV_SET_COMPUTE;
                break;
            case '1': // compute the fractal (test and control)
                ev.type = EV_COMPUTE_KB;
                break;
            case 'a': // cancels the current calculation process
                ev.type = EV_ABORT;
                break;
            case 'c':
                ev.type = EV_COMPUTE_CPU_KB;
                break;
            case 'r':
                ev.type = EV_READ;
                break;
            case 'p':
                ev.type = EV_SAVE_IMAGE;
                break;
            case 'e':
                ev.type = EV_ERASE_IMAGE;
                break;
            case 'f':
                ev.type = EV_REFRESH;
                break;
            case 'h':
                ev.type = EV_HELP;
                break;
            case 'v':
                ev.type = EV_VIDEO;
                break;
            case '+':
                ev.type = EV_ZOOM_IN;
                break;
            case '-':
                ev.type = EV_ZOOM_OUT;
                break;
            case 'l':
                ev.type = EV_RESET_CHUNK;
                break;
            default:
                break;
        } // end of switch
        if (ev.type != EV_TYPE_NUM) {
            queue_push(ev);
        }
    } // end of while loop
    set_quit();
    ev.type = EV_QUIT;
    queue_push(ev);
    call_termios(1); // restore terminal
    fprintf(stderr, "Keyboard thread ended\n");
    return NULL;
}