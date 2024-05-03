#include "utils.h"
#include "messages.h"

void my_assert(bool r, const char *func_name, int line, const char *file_name)
{
    if (!r) {
        fprintf(stderr, "\rError: %s() failed on line %d in file %s\n", func_name, line, file_name);
        exit(GENERIC_ERROR);
    }  
}

void* my_alloc(size_t size)
{
    void* ret = (void*)malloc(size);
    if (!ret) {
        fprintf(stderr, MALLOC_ERR_MESSAGE, __func__, __FILE__, __LINE__);
        exit(MALLOC_ERROR);
    }
    return ret;
}

void call_termios(int reset)
{
    static struct termios tio, tioOld;
    tcgetattr(STDIN_FILENO, &tio);
    if (reset) {
        tcsetattr(STDIN_FILENO, TCSANOW, &tioOld);
    } else {
        tioOld = tio; //backup 
        cfmakeraw(&tio);
        tio.c_oflag |= OPOST;
        tcsetattr(STDIN_FILENO, TCSANOW, &tio);
    }
}

void info(const char *str) 
{
    fprintf(stderr, "INFO: %s\n", str);
}

void debug(const char *str)
{
    fprintf(stdout, "DEBUG: %s\n", str);
}

void error(const char *str)
{
    fprintf(stderr, "ERROR: %s\n", str);
}

void warning(const char* str) 
{
    fprintf(stderr, "WARNING: %s\n", str);
}

// just for the case of readability
void pipe_message_report(int msg_name)
{
    printf("ERROR: unhandled message type ");
    switch(msg_name) {
        case MSG_OK:
            printf("MSG_OK\n");
            break;
        case MSG_ERROR:
            printf("MSG_ERROR\n");
            break;
        case MSG_STARTUP:
            printf("MSG_STARTUP\n");
            break;
        case MSG_COMPUTE:
            printf("MSG_COMPUTE\n");
            break;
        case MSG_DONE:
            printf("MSG_DONE\n");
            break;
        case MSG_COMPUTE_DATA:
            printf("MSG_COMPUTE_DATA\n");
            break;
        case MSG_VERSION:
            printf("MSG_VERSION\n");
            break;
        case MSG_ABORT:
            printf("MSG_ABORT\n");
            break;
        case MSG_SET_COMPUTE:
            printf("MSG_SET_COMPUTE\n");
            break;
        case MSG_NBR:
            printf("MSG_NBR\n");
            break;
        default:
            printf("otherwise unknown message\n");
            break;
    }
}