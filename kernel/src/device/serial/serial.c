#include <lib/lib.h>
#include <stdint.h>
#include <stdarg.h>

void serial_putc(char c)
{
    outb(0xE9, c);
}

void serial_putsf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char buffer[256];

    for (const char *p = format; *p != '\0'; p++)
    {
        if (*p != '%')
        {
            serial_putc(*p);
            continue;
        }

        switch (*++p)
        {
        case 'c':
            serial_putc(va_arg(args, int));
            break;
        case 'd':
            itoa(va_arg(args, int), buffer, 10);
            for (char *p = buffer; *p != '\0'; p++)
                serial_putc(*p);
            break;
        case 'u':
            utoa(va_arg(args, unsigned int), buffer, 10);
            for (char *p = buffer; *p != '\0'; p++)
                serial_putc(*p);
            break;
        case 'x':
            utoa(va_arg(args, unsigned int), buffer, 16);
            for (char *p = buffer; *p != '\0'; p++)
                serial_putc(*p);
            break;
        case 's':
            for (char *p = va_arg(args, char *); *p != '\0'; p++)
                serial_putc(*p);
            break;
        case 'l':
            switch (*++p)
            {
            case 'l':
                switch (*++p)
                {
                case 'x':
                    ullitoa(va_arg(args, unsigned long long int), buffer, 16);
                    for (char *p = buffer; *p != '\0'; p++)
                        serial_putc(*p);
                    break;
                }
                break;
            case 'd':
                itoa(va_arg(args, long int), buffer, 10);
                for (char *p = buffer; *p != '\0'; p++)
                    serial_putc(*p);
                break;
            case 'u':
                utoa(va_arg(args, unsigned long int), buffer, 10);
                for (char *p = buffer; *p != '\0'; p++)
                    serial_putc(*p);
                break;
            case 'x':
                utoa(va_arg(args, unsigned long int), buffer, 16);
                for (char *p = buffer; *p != '\0'; p++)
                    serial_putc(*p);
                break;
            }
            break;

        case 'p':
            utoa(va_arg(args, uintptr_t), buffer, 16);
            for (char *p = buffer; *p != '\0'; p++)
                serial_putc(*p);
            break;
        }
    }

    va_end(args);
}

void serial_puts(const char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        serial_putc(str[i]);
    }
}