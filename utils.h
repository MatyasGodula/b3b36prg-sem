#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h> // for STDIN_FILENO

#define MALLOC_ERR_MESSAGE "\rError: could not allocate memory: %s, in %s at line: %d\n"

enum {
    MALLOC_ERROR = 101,
    GENERIC_ERROR = 105
};

void my_assert(bool r, const char *func_name, int line, const char *file_name);
void* my_alloc(size_t size);
void call_termios(int reset);

void info(const char *str);
void debug(const char *str);
void error(const char *str);
void warning(const char* str);
void display_module_ver(int major, int minor, int patch);
void pipe_message_report(int msg_name);
void report_read_error(char* str);

void print_help(void);

#endif