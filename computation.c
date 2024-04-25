#include "computation.h"

static struct {
    double c_re;
    double c_im;
    int n;

    double range_re_min;
    double range_re_max;
    double range_im_min;
    double range_im_max;

    int grid_w;
    int grid_h;

    int cur_x;
    int cur_y;

    double d_re;
    double d_im; 

    int nbr_chunks; // total number of chunks
    int cid; // id of a chunk
    double chunk_re; // real coordinate of a chunk
    double chunk_im; // imaginary coordinate of a chunk

    uint8_t chunk_n_re; // number of real elements in a chunk
    uint8_t chunk_n_im; // number of imaginary elements in a chunk 

    uint8_t* grid;
    bool computing;
    bool done;

} comp = {
    .c_re = -0.4,
    .c_im = 0.6,

    .n = 60,

    .range_re_min = -1.6,
    .range_re_max = 1.6,
    .range_im_min = -1.1,
    .range_im_max = 1.1,

    .grid = NULL,
    .grid_w = 640,
    .grid_h = 480,


    .chunk_n_re = 64,
    .chunk_n_im = 48,
    
    .computing = false,
    .done = false
};


void computation_init(void) 
{
    comp.grid = my_alloc(comp.grid_w * comp.grid_h);
    comp.d_re = (comp.range_re_max - comp.range_re_min) / (1. * comp.grid_w);
    comp.d_re = -(comp.range_im_max - comp.range_im_min) / (1. * comp.grid_h);
    comp.nbr_chunks = (comp.grid_w * comp.grid_h) / (comp.chunk_n_re * comp.chunk_n_im);
}

void computation_cleanup(void)
{
    if (comp.grid) {
        free(comp.grid);
    }
    comp.grid = NULL;
}

bool is_computing(void) { return comp.computing; }

bool is_done(void) { return comp.done; }

void abort_comp(void) { comp.computing = false; }

bool set_compute(message* msg)
{
    bool ret = !is_computing();
    my_assert(msg != NULL,__func__, __LINE__, __FILE__);
    if (ret) {
        msg->type = MSG_SET_COMPUTE;
        msg->data.set_compute.c_re = comp.c_re;
        msg->data.set_compute.c_im = comp.c_im;
        msg->data.set_compute.d_re = comp.d_re;
        msg->data.set_compute.d_im = comp.d_im;
        msg->data.set_compute.n = comp.n;
        comp.done = false;
    }
    return ret;
}

bool compute(message* msg)
{
    my_assert(msg != NULL, __func__, __LINE__, __FILE__);
    if (!is_computing()) {
        comp.cid = 0;
        comp.computing = true;
        comp.cur_x = comp.cur_y = 0; // start computation of the whole image
        comp.chunk_re = comp.range_re_min; // upper-"left" corner of the image
        comp.chunk_im = comp.range_im_max; // "upper"-left corner of the image
        msg->type = MSG_COMPUTE;
    }

    if (is_computing() && msg->type == MSG_COMPUTE) {
        msg->data.compute.cid = comp.cid;
        msg->data.compute.re = comp.chunk_re;
        msg->data.compute.im = comp.chunk_re;
        msg->data.compute.n_re = comp.chunk_n_re;
        msg->data.compute.n_im = comp.chunk_n_im;
    }
    return is_computing();
}

void update_data(const msg_compute_data* compute_data)
{
    my_assert(compute_data != NULL, __func__, __LINE__, __FILE__);
    if (compute_data->cid == comp.cid) {
        const int idx = (comp.cur_x + compute_data->i_re + (comp.cur_y + compute_data->i_im) * comp.grid_w);
        if (idx >= 0 && idx < (comp.grid_w * comp.grid_h)) {
            comp.grid[idx] = compute_data->iter;
        }
    } else {
        warning("Received chunk with unexpected chunk id");
    }
}