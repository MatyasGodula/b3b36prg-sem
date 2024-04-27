#ifndef __GUI_H__
#define __GUI_H__

void gui_init(void);
void gui_cleanup(void);
void gui_refresh(void);

void* gui_win_thread(void* d);

#endif