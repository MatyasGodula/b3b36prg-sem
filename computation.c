/*
 *
 * Author1: Jan Faigl
 * Author2: Matyas Godula
 *
 */


#include "computation.h"
#include "utils.h"
#include "event_queue.h"

#define R_CALC(t) (9 * (1 - t) * t*t*t * 255)
#define G_CALC(t) (15 * (1 - t)*(1 - t) * t*t * 255)
#define B_CALC(t) (8.5 * (1 - t)*(1 - t)*(1 - t) * t * 255)

#define NUMBER_OF_PARAMETERS 8

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
    bool video;
    bool aborted;
    bool set_up;

    bool video_request;

    int video_target;

} comp = {
    .c_re = -0.4,
    .c_im = 0.6,

    .n = 400,

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
    .done = false,
    .aborted = false,
    .set_up = false
};

typedef struct {
    int type;
    void* pointer;
    char* name;
} pars;

enum types { INTEGER, DOUBLE, BOOL, UINT8 };

pars array[NUMBER_OF_PARAMETERS] = {
    { .type = DOUBLE, .pointer = &comp.c_re, .name = "real constant" },
    { .type = DOUBLE, .pointer = &comp.c_im, .name = "imaginary constant" },
    { .type = INTEGER, .pointer = &comp.n, .name = "number of iterations" },
    { .type = DOUBLE, .pointer = &comp.range_re_min, .name = "minimum real range" },
    { .type = DOUBLE, .pointer = &comp.range_re_max, .name = "maximum real range" },
    { .type = DOUBLE, .pointer = &comp.range_im_min, .name = "minimum imaginary range" },
    { .type = DOUBLE, .pointer = &comp.range_im_max, .name = "maximum imaginary range" },
    { .type = BOOL, .pointer = &comp.video, .name = "video"}
};

void print_check() 
{
    printf("c_re: %lf\n", comp.c_re);
    printf("c_im: %lf\n", comp.c_im);
    printf("n: %d\n", comp.n);
    printf("range_re_min: %lf\n", comp.range_re_min);
    printf("range_re_max: %lf\n", comp.range_re_max);
    printf("range_im_min: %lf\n", comp.range_im_min);
    printf("range_im_max: %lf\n", comp.range_im_max);
    printf("video: %d\n", comp.video);
}

bool read_input_file()
{
    char c;
    int idx = 0;
    double value_db;     
    int value_int;       
    char ch;             
    int ret_db, ret_ui;  
    FILE* file = fopen("input_parameters.txt", "r");

    while (((c = fgetc(file)) != EOF) && idx < NUMBER_OF_PARAMETERS) {
        if (c == '[') {
            switch(array[idx].type) {
                case DOUBLE:
                    ret_db = fscanf(file, "%lf", &value_db);
                    if (ret_db != 1) {
                        fprintf(stderr, "cannot read from file value: %s\n", array[idx].name);
                        fclose(file);
                        return false;
                    } else {
                        *(double *)(array[idx].pointer) = value_db;
                    }
                    idx++;
                    break;
                case INTEGER:
                    ret_ui = fscanf(file, "%d", &value_int);
                    if (ret_ui != 1 || value_int < 0) {
                        fprintf(stderr, "cannot read from file value: %s\n", array[idx].name);
                        fclose(file);
                        return false;
                    } else {
                        if (value_int > 255) {
                            fprintf(stderr, "the number of iterations inputted is larger than 255, the images are going to look strange");
                        }
                        *(int *)(array[idx].pointer) = value_int;
                    }
                    idx++;
                    break;
                case BOOL:
                    ch = fgetc(file);
                    if (ch != 'Y' && ch != 'n') {
                        fprintf(stderr, "cannot read from file value: %s\n", array[idx].name);
                        fclose(file);
                        return false;
                    } else {
                        if (ch == 'Y') {
                            *(bool *)(array[idx].pointer) = true;
                        } else if (ch == 'n') {
                            *(bool *)(array[idx].pointer) = false;
                        }
                    }
                default:
                    break;
            }
        }
    }
    fclose(file);
    return true;
}


void set_video() {
    comp.video_target = comp.n;
    comp.n = 0;
}

bool change_iters(int target) 
{
    comp.n += 1;
    if (comp.n > target) {
        return false;
    } 
    return true;
}

int video_target() { return comp.video_target; }

void zero_iters() { comp.n = 0; }

void cancel_done() { comp.done = false; }

bool is_video() { return comp.video; }

void set_iters(int n) { comp.n = n; }

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

void abort_comp(void) { comp.aborted = true; }

bool is_aborted(void) { return comp.aborted; }

void unabort(void) { comp.aborted = false; }

void cancel_computing(void) { comp.computing = false; }

bool is_set_up(void) { return comp.set_up; }

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
        comp.set_up = true;
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
}

// --------------------------------------------------------------------- local_computation -------------------------------------

static struct {

    double constant_real;
    double constant_imag;

    double real_increment; // the increment for each pixel on x axis
    double imag_increment; // the increment for each pixel on y axis

    uint8_t n; // number of iteration per pixel

    // current chunk computation
    uint8_t cid; // chunk id
    double start_real_chunk; // start of the real coordinates for this chunk
    double start_imag_chunk; // start of the imaginary coordinates for this chunk
    uint8_t n_real; // number of real cells
    uint8_t n_imag; // number of imaginary cells
    uint8_t iters;

    // current pixel to be computed, it is changed at the end of computation
    double real_coords;
    uint8_t x_calculated;
    double imag_coords;
    uint8_t y_calculated;

    bool computing;
    bool done;
    bool aborted;
    bool set_up;

} local_computation = {
    .computing = false,
    .done = false,
    .aborted = false,
    .set_up = false
};

void set_up_local_computation()
{
    local_computation.constant_imag = comp.c_im;
    local_computation.constant_real = comp.c_re;
    local_computation.real_increment = comp.d_re;
    local_computation.imag_increment = comp.d_im;
    local_computation.n = comp.n;
    local_computation.set_up = true;
}

void set_up_local_chunk_computation()
{
    if (local_computation.set_up) {
        // translates the message to the program's static struct
        local_computation.cid = comp.cid;
        local_computation.start_real_chunk = comp.chunk_re;
        local_computation.start_imag_chunk = comp.chunk_im;
        local_computation.n_real = comp.chunk_n_re;
        local_computation.n_imag = comp.chunk_n_im;
        // sets up the coord numbers for sending in compute_data
        local_computation.x_calculated = 0;
        local_computation.y_calculated = 0;
        local_computation.computing = true;
        local_computation.aborted = false;
        local_computation.done = false;
        // real coords in bool to keep track of which pixel is being computed
        local_computation.real_coords = local_computation.start_real_chunk;
        local_computation.imag_coords = local_computation.start_imag_chunk;
    }
}

uint8_t iteration_calculation()
{
    double z_real = local_computation.real_coords;
    double z_imag = local_computation.imag_coords;
    double const_real = local_computation.constant_real;
    double const_imag = local_computation.constant_imag;
    uint8_t max_iters = local_computation.n;
    double z_real2 = z_real * z_real;
    double z_imag2 = z_imag * z_imag;

    uint8_t iters = 0;

    while ((z_real2 + z_imag2) <= 4.0 && iters < max_iters) {
        double temp_real = z_real2 - z_imag2 + const_real;
        double temp_imag = 2.0 * z_real * z_imag + const_imag;

        z_real = temp_real;
        z_imag = temp_imag;

        z_real2 = z_real * z_real;
        z_imag2 = z_imag * z_imag;

        iters++;
    }
    return iters;
}

void compute_chunk_local() 
{
    while(!local_computation.done) {
        if (local_computation.set_up && !local_computation.aborted && local_computation.computing && !local_computation.done) {
            local_computation.iters = iteration_calculation();
            update_data_local();
            local_computation.real_coords += local_computation.real_increment;
            local_computation.x_calculated += 1;
            if (local_computation.x_calculated >= local_computation.n_real) {
                local_computation.real_coords = local_computation.start_real_chunk;
                local_computation.x_calculated = 0;
                local_computation.imag_coords += local_computation.imag_increment;
                local_computation.y_calculated += 1;
                if (local_computation.y_calculated >= local_computation.n_imag) {
                    local_computation.computing = false;
                    local_computation.done = true;
                }
            }
        }
    }
}

void update_data_local()
{
    if (local_computation.cid == comp.cid) {
        const int idx = comp.cur_x + local_computation.x_calculated + (comp.cur_y + local_computation.y_calculated) * comp.grid_w;
        if (idx >= 0 && idx < (comp.grid_w * comp.grid_h)) {
            comp.grid[idx] = local_computation.iters;
        }
        if ((comp.cid + 1) >= comp.nbr_chunks && (local_computation.x_calculated + 1) == comp.chunk_n_re && (local_computation.y_calculated + 1) == comp.chunk_n_im) {
            comp.done = true;
            comp.computing = false;
        }
    } else {
        warning("Received chunk with unexpected chunk id");
    }
}

bool compute_local()
{
    if (!is_computing()) { // first chunk
        comp.cid = 0;
        comp.computing = true;
        comp.cur_x = comp.cur_y = 0; // start computation of the whole image
        comp.chunk_re = comp.range_re_min; // upper-"left" corner of the image
        comp.chunk_im = comp.range_im_max; // "upper"-left corner of the image
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
        } else { // all has been computed
        }
    }

    if (comp.computing) {
        local_computation.cid = comp.cid;
        local_computation.start_real_chunk = comp.chunk_re;
        local_computation.start_imag_chunk = comp.chunk_im;
        local_computation.n_real = comp.chunk_n_re;
        local_computation.n_imag = comp.chunk_n_im;
        set_up_local_chunk_computation();
        compute_chunk_local();
    }

    event ev;
    ev.type = EV_COMPUTE_CPU;
    queue_push(ev);
    return is_computing();
}

bool is_local_set(void) { return local_computation.set_up; }

// --------------------------------------------------------------------- image_interactions -------------------------------------

void zoom_in() 
{
    // Calculate the center of the current range
    double center_re = (comp.range_re_min + comp.range_re_max) / 2.0;
    double center_im = (comp.range_im_min + comp.range_im_max) / 2.0;
    
    double zoom_factor = 0.2;
    
    double range_re_half = (comp.range_re_max - comp.range_re_min) / 2.0;
    double range_im_half = (comp.range_im_max - comp.range_im_min) / 2.0;

    comp.range_re_min = center_re - (range_re_half * (1 - zoom_factor));
    comp.range_re_max = center_re + (range_re_half * (1 - zoom_factor));
    comp.range_im_min = center_im - (range_im_half * (1 - zoom_factor));
    comp.range_im_max = center_im + (range_im_half * (1 - zoom_factor));
    
    comp.d_re = (comp.range_re_max - comp.range_re_min) / (1.0 * comp.grid_w);
    comp.d_im = -(comp.range_im_max - comp.range_im_min) / (1.0 * comp.grid_h);
}


void zoom_out() 
{
    double center_re = (comp.range_re_min + comp.range_re_max) / 2.0;
    double center_im = (comp.range_im_min + comp.range_im_max) / 2.0;
    
    double zoom_factor = 0.2;
    
    double range_re_half = (comp.range_re_max - comp.range_re_min) / 2.0;
    double range_im_half = (comp.range_im_max - comp.range_im_min) / 2.0;

    comp.range_re_min = center_re - (range_re_half * (1 + zoom_factor));
    comp.range_re_max = center_re + (range_re_half * (1 + zoom_factor));
    comp.range_im_min = center_im - (range_im_half * (1 + zoom_factor));
    comp.range_im_max = center_im + (range_im_half * (1 + zoom_factor));
    
    comp.d_re = (comp.range_re_max - comp.range_re_min) / (1.0 * comp.grid_w);
    comp.d_im = -(comp.range_im_max - comp.range_im_min) / (1.0 * comp.grid_h);
}

void move_left(double move_factor)
{
    double range_width = comp.range_re_max - comp.range_re_min;
    double shift = range_width * move_factor;
    comp.range_re_min -= shift;
    comp.range_re_max -= shift;
}

void move_right(double move_factor)
{
    double range_width = comp.range_re_max - comp.range_re_min;
    double shift = range_width * move_factor;
    comp.range_re_min += shift;
    comp.range_re_max += shift;
}

void move_up(double move_factor)
{
    double range_height = comp.range_im_max - comp.range_im_min;
    double shift = range_height * move_factor;
    comp.range_im_min += shift;
    comp.range_im_max += shift;
}

void move_down(double move_factor)
{
    double range_height = comp.range_im_max - comp.range_im_min;
    double shift = range_height * move_factor;
    comp.range_im_min -= shift;
    comp.range_im_max -= shift;
}
