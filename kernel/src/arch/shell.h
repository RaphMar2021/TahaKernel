#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>
#include <disk/fat.h>
#include <video/video.h>

const int CHAR_WIDTH = 8;
const int CHAR_HEIGHT = 16;

void print_prompt();

#endif