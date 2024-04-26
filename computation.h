#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "messages.h"
#include "utils.h"

#ifndef __COMPUTATION_H_
#define __COMPUTATION_H__


void computation_init(void);
void computation_cleanup(void);

bool is_computing(void);
bool is_done(void);

void abort_comp(void);

bool set_compute(message* msg);
bool compute(message* msg);
void update_image(int w, int h, unsigned char* img);

void update_data(const msg_compute_data* compute_data);

void get_grid_size(int *w, int *h);

#endif