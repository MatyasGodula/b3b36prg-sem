#include "computation.h"
#include "utils.h"

#define R_CALC(t) (9 * (1 - t) * t*t*t * 255)
#define G_CALC(t) (15 * (1 - t)*(1 - t) * t*t * 255)
#define B_CALC(t) (8.5 * (1 - t)*(1 - t)*(1 - t) * t * 255)

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
    comp.d_im = -(comp.range_im_max - comp.range_im_min) / (1. * comp.grid_h); // also a mistake in this line
    comp.nbr_chunks = (comp.grid_w * comp.grid_h) / (comp.chunk_n_re * comp.chunk_n_im);
}

void clean_grid(void)
{
    if (!comp.grid) {
        error("attempted grid cleanup when grid was not initialized yet");
        return;
    }
    for (int i = 0; i < comp.grid_w * comp.grid_h; ++i) {
        comp.grid[i] = 0;
    }
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
    if (!is_computing()) { // first chunk
        comp.cid = 0;
        comp.computing = true;
        comp.cur_x = comp.cur_y = 0; // start computation of the whole image
        comp.chunk_re = comp.range_re_min; // upper-"left" corner of the image
        comp.chunk_im = comp.range_im_max; // "upper"-left corner of the image
        msg->type = MSG_COMPUTE;
    } else { // next chunk
        comp.cid += 1;
        if (comp.cid < comp.nbr_chunks) {
            comp.cur_x += comp.chunk_n_re;
            comp.chunk_re += comp.chunk_n_re * comp.d_re;
            if (comp.cur_x >= comp.grid_w) {
                comp.cur_x = 0;
                comp.chunk_re = comp.range_re_min;
                comp.cur_y += comp.chunk_n_im;
                comp.chunk_im += comp.chunk_n_im * comp.d_im; // mistake on this line
            }
            msg->type = MSG_COMPUTE;
        } else { // all has been computed
        }
    }
    printf("computing chunk: %d\n", comp.cid);

    if (comp.computing && msg->type == MSG_COMPUTE) {
        msg->data.compute.cid = comp.cid;
        msg->data.compute.re = comp.chunk_re;
        msg->data.compute.im = comp.chunk_im; // the mistake was here probably
        msg->data.compute.n_re = comp.chunk_n_re;
        msg->data.compute.n_im = comp.chunk_n_im;
    }
    return is_computing();
}

void update_data(const msg_compute_data* compute_data)
{
    my_assert(compute_data != NULL, __func__, __LINE__, __FILE__);
    if (compute_data->cid == comp.cid) {
        const int idx = comp.cur_x + compute_data->i_re + (comp.cur_y + compute_data->i_im) * comp.grid_w;
        if (idx >= 0 && idx < (comp.grid_w * comp.grid_h)) {
            comp.grid[idx] = compute_data->iter;
        }
        if ((comp.cid + 1) >= comp.nbr_chunks && (compute_data->i_re + 1) == comp.chunk_n_re && (compute_data->i_im + 1) == comp.chunk_n_im) {
            comp.done = true;
            comp.computing = false;
        }
    } else {
        warning("Received chunk with unexpected chunk id");
    }
}

void get_grid_size(int *w, int *h)
{
    *w = comp.grid_w;
    *h = comp.grid_h;    
}

void update_image(int w, int h, unsigned char* img)
{
    my_assert(img && comp.grid && w == comp.grid_w && h == comp.grid_h, __func__, __LINE__, __FILE__);
    for (int i = 0; i < w * h; ++i) {
        const double t = 1. * comp.grid[i] / (comp.n + 1.0);
        *(img++) = R_CALC(t);
        *(img++) = G_CALC(t);
        *(img++) = B_CALC(t);

        //*(img++) = (9 * (1-t) * t*t*t * 255);
        //*(img++) = (15 * (1-t)*(1-t) * t*t * 255);
        //*(img++) = (8.5 * (1-t)*(1-t)*(1-t) * t * 255);
    }
    info("image is being updated");
}

