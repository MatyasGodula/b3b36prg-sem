#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>

#include "gui.h"
#include "xwin_sdl.h"
#include "computation.h"
#include "utils.h"
#include "event_queue.h"

#ifndef SDL_EVENT_POLL_WAIT_MS
#define SDL_EVENT_POLL_WAIT_MS 10
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
    event ev;
    while (!quit) {
        if (SDL_PollEvent(&sdl_event)) {
            if (sdl_event.type == SDL_KEYDOWN) {
                switch(sdl_event.key.keysym.sym) {
                        case SDLK_q:
                            ev.type = EV_QUIT;
                        queue_push(ev);// quits the app 
                            info("quit");
                            break;
                        case SDLK_s:
                            info("s received");
                            ev.type = EV_SET_COMPUTE;
                            break;
                        case SDLK_c:
                            info("c received");
                            ev.type = EV_COMPUTE;
                            break;
                        case SDLK_a:
                            info("a received");
                            ev.type = EV_ABORT;
                            break;
                        case SDLK_g:
                            info("g received");
                            ev.type = EV_GET_VERSION;
                            break;
                        default:
                            break;
                    }

            } else if (sdl_event.type == SDL_KEYUP) {
                    info("keyup");
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