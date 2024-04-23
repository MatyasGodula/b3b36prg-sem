#include "keyboard.h"
#include "event_queue.h"
#include "utils.h"

void* keyboard_thread(void* data)
{
    fprintf(stderr, "Keyboard thread started\n");
    call_termios(0);
    int c;
    event ev;
    while ((c = getchar()) != 'q') {
        ev.type = EV_TYPE_NUM;
        printf("character: %c\n", c);
        switch(c) {
            case 'g': // request the current version of the module
                ev.type = EV_GET_VERSION;
                break;
            case 's': // set up the computation
                ev.type = EV_SET_COMPUTE;
                break;
            case 'c': // compute the fractal (test and control)
                ev.type = EV_COMPUTE;
                break;
            case 'a': // cancels the current calculation process
                ev.type = EV_ABORT;
                break;
            default:
                break;
        }
    } // end of while loop
    set_quit();
    ev.type = EV_QUIT;
    queue_push(ev);
    call_termios(1); // restore terminal
    fprintf(stderr, "Keyboard thread ended\n");
    return NULL;
}