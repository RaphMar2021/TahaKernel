#ifndef VIDEO_H
#define VIDEO_H

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine/limine.h>
#include "font.h"

#define LOG_ERROR 1
#define LOG_WARN 2
#define LOG_INFO 3
#define LOG_DEBUG 4

extern struct limine_framebuffer *current_framebuffer;
extern void *framebuffer_buffer;
extern int screen_width, screen_height;
extern int term_x, term_y;

void init_video(struct limine_framebuffer *framebuffer);
void draw_pixel(int x, int y, uint32_t color);
void draw_rectangle(int x, int y, int width, int height, uint32_t color);
uint32_t get_pixel(int x, int y);
void clear_screen(uint32_t color);
void set_color(uint32_t color);
void draw_char(char c, int x, int y);
void print_char(char c);
void printf(const char *format, ...);
void log(int level, const char *message, ...);
void print_animated(const char *message);

#endif