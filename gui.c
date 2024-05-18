/*
 *
 * Author1: Jan Faigl
 * Author2: Matyas Godula
 *
 */

#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>

#include "gui.h"
#include "xwin_sdl.h"
#include "computation.h"
#include "utils.h"
#include "event_queue.h"

#ifndef SDL_EVENT_POLL_WAIT_MS
#define SDL_EVENT_POLL_WAIT_MS 5
#endif

static struct {
    int w;
    int h;

    unsigned char *img;
} gui = { .img = NULL };

void gui_init(void) 
{
    get_grid_size(&gui.w, &gui.h);
    // since rgb is 3 bytes large
    gui.img = my_alloc(gui.w * gui.h * 3);
    my_assert(xwin_init(gui.w, gui.h) == 0, __func__, __LINE__, __FILE__);

}

void gui_cleanup(void)
{
    if (gui.img) {
        free(gui.img);
        gui.img = NULL;
    }
    xwin_close();
}

void gui_refresh(void)
{
    if (gui.img) {
        update_image(gui.w, gui.h, gui.img);
        xwin_redraw(gui.w, gui.h, gui.img);
    } else {
        info("image not yet initialized");
    }
}

void clean_image(void)
{
    if (gui.img) {
        clean_grid();
        for (int i = 0; i < gui.w * gui.h * 3; ++i) {
            gui.img[i] = 0;
        }
    }
    update_image(gui.w, gui.h, gui.img);
}

void* gui_win_thread(void* d) 
{
    info("GuiWin thread started");
    bool quit = false;
    SDL_Event sdl_event;
    event ev = {.type = EV_TYPE_NUM};
    while (!quit) {
        if (SDL_PollEvent(&sdl_event)) {
            if (sdl_event.type == SDL_KEYDOWN) {
                switch(sdl_event.key.keysym.sym) {
                        case SDLK_q:
                            ev.type = EV_QUIT;
                            break;
                        case SDLK_s:
                            ev.type = EV_SET_COMPUTE;
                            break;
                        case SDLK_1:
                            ev.type = EV_COMPUTE_KB;
                            break;
                        case SDLK_c:
                            ev.type = EV_COMPUTE_CPU_KB;
                            break;
                        case SDLK_a:
                            ev.type = EV_ABORT;
                            break;
                        case SDLK_g:
                            ev.type = EV_GET_VERSION;
                            break;
                        case SDLK_r:
                            ev.type = EV_READ;
                            break;
                        case SDLK_l:
                            ev.type = EV_RESET_CHUNK;
                            break; 
                        case SDLK_p:
                            ev.type = EV_SAVE_IMAGE;
                            break;
                        case SDLK_e:
                            ev.type = EV_ERASE_IMAGE;
                            break;
                        case SDLK_v:
                            ev.type = EV_VIDEO;
                            break; 
                        case SDLK_h:
                            ev.type = EV_HELP;
                            break;
// ------------------------ image control -------------------------
                        case SDLK_LEFT:
                            ev.type = EV_MOVE_LEFT;
                            break;
                        case SDLK_RIGHT:
                            ev.type = EV_MOVE_RIGHT;
                            break;
                        case SDLK_UP:
                            ev.type = EV_MOVE_UP;
                            break;
                        case SDLK_DOWN:
                            ev.type = EV_MOVE_DOWN;
                            break;
                        case SDLK_EQUALS:
                            ev.type = EV_ZOOM_IN;
                            break;
                        case SDLK_MINUS:
                            ev.type = EV_ZOOM_OUT;
                            break;
// ------------------------ image control -------------------------
                        default:
                            break;
                } // end of switch
                if (ev.type != EV_TYPE_NUM) {
                    queue_push(ev);
                }

            } else if (sdl_event.type == SDL_KEYUP) {
            } else {
                //debug("SDL_PollEvent initialized");
            }
        } else {
            //debug("SDL_PollEvent not initialized");
        }// end SDL_PollEvent 
        SDL_Delay(SDL_EVENT_POLL_WAIT_MS);
        quit = is_quit();
    }
    info("GuiWin thread ended");
    return NULL;
}