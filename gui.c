#include <SDL.h>

#include "gui.h"
#include "xwin_sdl.h"
#include "computation.h"
#include "utils.h"
#include "event_queue.h"

#ifndef SDL_EVENT_POLL_WAIT_MS
#define SDL_EVENT_POLL_WAIT_MS 100
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
    }
}

void* gui_win_thread(void* d)
{
    info("GuiWin thread started");
    bool quit = false;
    SDL_Event event;
    while (!quit) {
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                info("keydown");
            } else if (event.type == SDL_KEYUP) {
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