#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "messages.h"

#ifndef __COMPUTATION_H_
#define __COMPUTATION_H__


void computation_init(void);
void computation_cleanup(void);

bool is_computing(void);
bool is_done(void);
void set_video();

// functionality for the animation
bool change_iters(int target);
void zero_iters();
void cancel_done();
int video_target();

bool is_video();

void abort_comp(void);

bool set_compute(message* msg);
bool compute(message* msg);
void update_image(int w, int h, unsigned char* img);

void update_data(const msg_compute_data* compute_data);
void clean_grid(void);

void get_grid_size(int *w, int *h);
bool read_input_file(FILE* file);

void print_check();

// --------------------------------------------------------------------- local_computation -------------------------------------
void set_up_local_chunk_computation();
uint8_t iteration_calculation();
void compute_chunk_local();
void update_data_local();
bool compute_local();
void set_up_local_computation();

// --------------------------------------------------------------------- image_interactions -------------------------------------

void zoom_in();
void zoom_out();
void move_left(double move_factor);
void move_right(double move_factor);
void move_up(double move_factor);
void move_down(double move_factor);

#endif