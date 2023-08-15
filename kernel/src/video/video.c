#include "video.h"
#include <device/serial/serial.h>

struct limine_framebuffer *current_framebuffer;
void *framebuffer_buffer;
int screen_width, screen_height;
uint32_t term_color = 0xffffff;
int term_x, term_y;

void init_video(struct limine_framebuffer *framebuffer)
{
    current_framebuffer = framebuffer;
    framebuffer_buffer = framebuffer->address;
    screen_width = framebuffer->width;
    screen_height = framebuffer->height;
}

void draw_pixel(int x, int y, uint32_t color)
{
    uint32_t *pixel_address = (uint32_t *)((uint64_t)framebuffer_buffer + y * current_framebuffer->pitch + x * (current_framebuffer->bpp / 8));

    *pixel_address = color;
}

void draw_rectangle(int x, int y, int width, int height, uint32_t color)
{
    for (int i = x; i < x + width; i++)
    {
        for (int j = y; j < y + height; j++)
        {
            draw_pixel(i, j, color);
        }
    }
}

uint32_t get_pixel(int x, int y)
{
    uint32_t *pixel_address = (uint32_t *)((uint64_t)framebuffer_buffer + y * current_framebuffer->pitch + x * (current_framebuffer->bpp / 8));

    return *pixel_address;
}

void clear_screen(uint32_t color)
{
    term_x = 0;
    term_y = 0;

    for (int y = 0; y < current_framebuffer->height; y++)
    {
        for (int x = 0; x < current_framebuffer->width; x++)
        {
            draw_pixel(x, y, color);
        }
    }
}

void set_color(uint32_t color)
{
    term_color = color;
}

void draw_char(char c, int x, int y)
{
    uint8_t *offset = font + sizeof(font_header) + 16 * c;

    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (offset[i] & (1 << j))
            {
                draw_pixel(x + 8 - j, y + i, term_color);
            }
        }
    }
}

void scroll_up()
{
    for (int y = 0; y < current_framebuffer->height - 16; y++)
    {
        for (int x = 0; x < current_framebuffer->width; x++)
        {
            uint32_t color = get_pixel(x, y + 16);
            draw_pixel(x, y, color);
        }
    }
    draw_rectangle(0, current_framebuffer->height - 16, current_framebuffer->width, 16, 0x000000);
}

void print_char(char c)
{
    if (c == '\n' || term_x >= current_framebuffer->width)
    {
        term_x = 0;
        term_y += 16;
    }

    if (term_y >= current_framebuffer->height)
    {
        scroll_up();
        term_y -= 16;
    }

    if (c != '\n')
    {
        draw_char(c, term_x, term_y);
        term_x += 8;
    }
}

void displayCharacter(char c, int *a)
{
    print_char(c);
    *a += 1;
}

void displayString(char *c, int *a)
{
    for (int i = 0; c[i]; ++i)
    {
        displayCharacter(c[i], a);
    }
}

void print_hex(uint32_t val)
{
    char buf[9];
    buf[8] = 0;
    int i;
    for (i = 7; i >= 0; i--, val >>= 4)
    {
        uint8_t digit = val & 0xF;
        buf[i] = digit + (digit < 10 ? '0' : 'A' - 10);
    }
    printf("%s", buf);
}

// PrintF from https://wiki.osdev.org/User:A22347/Printf

int vprintf(const char *format, va_list list)
{
    int chars = 0;
    char intStrBuffer[256] = {0};

    for (int i = 0; format[i]; ++i)
    {

        char specifier = '\0';
        char length = '\0';

        int lengthSpec = 0;
        int precSpec = 0;
        bool leftJustify = false;
        bool zeroPad = false;
        bool spaceNoSign = false;
        bool altForm = false;
        bool plusSign = false;
        bool emode = false;
        int expo = 0;

        if (format[i] == '%')
        {
            ++i;

            bool extBreak = false;
            while (1)
            {

                switch (format[i])
                {
                case '-':
                    leftJustify = true;
                    ++i;
                    break;

                case '+':
                    plusSign = true;
                    ++i;
                    break;

                case '#':
                    altForm = true;
                    ++i;
                    break;

                case ' ':
                    spaceNoSign = true;
                    ++i;
                    break;

                case '0':
                    zeroPad = true;
                    ++i;
                    break;

                default:
                    extBreak = true;
                    break;
                }

                if (extBreak)
                    break;
            }

            while (isdigit(format[i]))
            {
                lengthSpec *= 10;
                lengthSpec += format[i] - 48;
                ++i;
            }

            if (format[i] == '*')
            {
                lengthSpec = va_arg(list, int);
                ++i;
            }

            if (format[i] == '.')
            {
                ++i;
                while (isdigit(format[i]))
                {
                    precSpec *= 10;
                    precSpec += format[i] - 48;
                    ++i;
                }

                if (format[i] == '*')
                {
                    precSpec = va_arg(list, int);
                    ++i;
                }
            }
            else
            {
                precSpec = 6;
            }

            if (format[i] == 'h' || format[i] == 'l' || format[i] == 'j' ||
                format[i] == 'z' || format[i] == 't' || format[i] == 'L')
            {
                length = format[i];
                ++i;
                if (format[i] == 'h')
                {
                    length = 'H';
                }
                else if (format[i] == 'l')
                {
                    length = 'q';
                    ++i;
                }
            }
            specifier = format[i];

            memset(intStrBuffer, 0, 256);

            int base = 10;
            if (specifier == 'o')
            {
                base = 8;
                specifier = 'u';
                if (altForm)
                {
                    displayString("0", &chars);
                }
            }
            if (specifier == 'p')
            {
                base = 16;
                length = 'z';
                specifier = 'u';
            }
            switch (specifier)
            {
            case 'X':
                base = 16;
            case 'x':
                base = base == 10 ? 17 : base;
                if (altForm)
                {
                    displayString("0x", &chars);
                }

            case 'u':
            {
                switch (length)
                {
                case 0:
                {
                    unsigned int integer = va_arg(list, unsigned int);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    displayString(intStrBuffer, &chars);
                    break;
                }
                case 'H':
                {
                    unsigned char integer = (unsigned char)va_arg(list, unsigned int);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    displayString(intStrBuffer, &chars);
                    break;
                }
                case 'h':
                {
                    unsigned short int integer = va_arg(list, unsigned int);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    displayString(intStrBuffer, &chars);
                    break;
                }
                case 'l':
                {
                    unsigned long integer = va_arg(list, unsigned long);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    displayString(intStrBuffer, &chars);
                    break;
                }
                case 'q':
                {
                    unsigned long long integer = va_arg(list, unsigned long long);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    displayString(intStrBuffer, &chars);
                    break;
                }
                case 'j':
                {
                    uintmax_t integer = va_arg(list, uintmax_t);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    displayString(intStrBuffer, &chars);
                    break;
                }
                case 'z':
                {
                    size_t integer = va_arg(list, size_t);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    displayString(intStrBuffer, &chars);
                    break;
                }
                case 't':
                {
                    ptrdiff_t integer = va_arg(list, ptrdiff_t);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    displayString(intStrBuffer, &chars);
                    break;
                }
                default:
                    break;
                }
                break;
            }

            case 'd':
            case 'i':
            {
                switch (length)
                {
                case 0:
                {
                    int integer = va_arg(list, int);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    displayString(intStrBuffer, &chars);
                    break;
                }
                case 'H':
                {
                    signed char integer = (signed char)va_arg(list, int);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    displayString(intStrBuffer, &chars);
                    break;
                }
                case 'h':
                {
                    short int integer = va_arg(list, int);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    displayString(intStrBuffer, &chars);
                    break;
                }
                case 'l':
                {
                    long integer = va_arg(list, long);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    displayString(intStrBuffer, &chars);
                    break;
                }
                case 'q':
                {
                    long long integer = va_arg(list, long long);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    displayString(intStrBuffer, &chars);
                    break;
                }
                case 'j':
                {
                    intmax_t integer = va_arg(list, intmax_t);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    displayString(intStrBuffer, &chars);
                    break;
                }
                case 'z':
                {
                    size_t integer = va_arg(list, size_t);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    displayString(intStrBuffer, &chars);
                    break;
                }
                case 't':
                {
                    ptrdiff_t integer = va_arg(list, ptrdiff_t);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    displayString(intStrBuffer, &chars);
                    break;
                }
                default:
                    break;
                }
                break;
            }

            case 'c':
            {
                if (length == 'l')
                {
                    displayCharacter(va_arg(list, int), &chars);
                }
                else
                {
                    displayCharacter(va_arg(list, int), &chars);
                }

                break;
            }

            case 's':
            {
                displayString(va_arg(list, char *), &chars);
                break;
            }

            case 'n':
            {
                switch (length)
                {
                case 'H':
                    *(va_arg(list, signed char *)) = chars;
                    break;
                case 'h':
                    *(va_arg(list, short int *)) = chars;
                    break;

                case 0:
                {
                    int *a = va_arg(list, int *);
                    *a = chars;
                    break;
                }

                case 'l':
                    *(va_arg(list, long *)) = chars;
                    break;
                case 'q':
                    *(va_arg(list, long long *)) = chars;
                    break;
                case 'j':
                    *(va_arg(list, intmax_t *)) = chars;
                    break;
                case 'z':
                    *(va_arg(list, size_t *)) = chars;
                    break;
                case 't':
                    *(va_arg(list, ptrdiff_t *)) = chars;
                    break;
                default:
                    break;
                }
                break;
            }

            case 'e':
            case 'E':
                emode = true;

            case 'f':
            case 'F':
            case 'g':

            case 'a':
            case 'A':
                // ACK! Hexadecimal floating points...
                break;

            default:
                break;
            }

            if (specifier == 'e')
            {
                displayString("e+", &chars);
            }
            else if (specifier == 'E')
            {
                displayString("E+", &chars);
            }

            if (specifier == 'e' || specifier == 'E')
            {
                __int_str(expo, intStrBuffer, 10, false, false, 2, false, true);
                displayString(intStrBuffer, &chars);
            }
        }
        else
        {
            displayCharacter(format[i], &chars);
        }
    }

    return chars;
}

void printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char buffer[256];

    for (const char *p = format; *p != '\0'; p++)
    {
        if (*p != '%')
        {
            print_char(*p);
            continue;
        }

        switch (*++p)
        {
        case 'c':
            print_char(va_arg(args, int));
            break;
        case 'd':
            itoa(va_arg(args, int), buffer, 10);
            for (char *p = buffer; *p != '\0'; p++)
                print_char(*p);
            break;
        case 'u':
            utoa(va_arg(args, unsigned int), buffer, 10);
            for (char *p = buffer; *p != '\0'; p++)
                print_char(*p);
            break;
        case 'x':
            utoa(va_arg(args, unsigned int), buffer, 16);
            for (char *p = buffer; *p != '\0'; p++)
                print_char(*p);
            break;
        case 's':
            for (char *p = va_arg(args, char *); *p != '\0'; p++)
                print_char(*p);
            break;
        case 'l':
            switch (*++p)
            {
            case 'l':
                switch (*++p)
                {
                case 'u':
                    ullitoa(va_arg(args, unsigned long long int), buffer, 10);
                    for (char *p = buffer; *p != '\0'; p++)
                        print_char(*p);
                    break;
                case 'x':
                    ullitoa(va_arg(args, unsigned long long int), buffer, 16);
                    for (char *p = buffer; *p != '\0'; p++)
                        print_char(*p);
                    break;
                }
                break;
            case 'd':
                litoa(va_arg(args, long int), buffer, 10);
                for (char *p = buffer; *p != '\0'; p++)
                    print_char(*p);
                break;
            case 'u':
                ultoa(va_arg(args, unsigned long int), buffer, 10);
                for (char *p = buffer; *p != '\0'; p++)
                    print_char(*p);
                break;
            case 'x':
            {
                uint32_t hex = va_arg(args, uint32_t);
                print_hex(hex);
                break;
            }
            break;

            case 'p':
                utoa(va_arg(args, uintptr_t), buffer, 16);
                for (char *p = buffer; *p != '\0'; p++)
                    print_char(*p);
                break;
            }
        }

        va_end(args);
    }
}

void log(int level, const char *message, ...)
{
    switch (level)
    {
    case LOG_ERROR:
        set_color(0xC0392B);
        printf("[ERROR] ");
        serial_puts("\033[0;31m[ERROR] \033[0;37m");
        break;
    case LOG_WARN:
        set_color(0xF39C12);
        printf("[WARN] ");
        serial_puts("\033[1;33m[WARN] \033[0;37m");
        break;
    case LOG_INFO:
        set_color(0x27AE60);
        printf("[INFO] ");
        serial_puts("\033[1;32m[INFO] \033[0;37m");
        break;
    case LOG_DEBUG:
        set_color(0x2980B9);
        printf("[DEBUG] ");
        serial_puts("\033[1;34m[DEBUG] \033[0;37m");
        break;
    }

    set_color(0xFFFFFF);

    va_list args;
    va_start(args, message);
    printf(message, args);
    serial_putsf(message, args);
    va_end(args);
}

void print_animated(const char *message)
{
    uint32_t color = 0x0000FF;

    while (*message)
    {
        set_color(color);

        print_char(*message++);
        delay(4);

        color += 0x001100;
        if (color > 0x00FF00)
            color = 0x0000FF;
    }

    set_color(0xFFFFFF);
}
