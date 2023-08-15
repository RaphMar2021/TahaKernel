#include "lib.h"

// Thanks chatgpt for this file :sweat_smile:

void outb(unsigned short port, unsigned char value)
{
    asm volatile("outb %0, %1"
                 :
                 : "a"(value), "Nd"(port));
}

unsigned char inb(unsigned short port)
{
    unsigned char result;
    asm volatile("inb %1, %0"
                 : "=a"(result)
                 : "Nd"(port));
    return result;
}

void outl(uint16_t port, uint32_t val)
{
    asm volatile("outl %0, %1"
                 :
                 : "a"(val), "Nd"(port));
}

uint32_t inl(uint16_t port)
{
    uint32_t ret;
    asm volatile("inl %1, %0"
                 : "=a"(ret)
                 : "Nd"(port));
    return ret;
}

void insl(uint16_t port, void *addr, int cnt)
{
    asm volatile("cld; rep insl"
                 : "=D"(addr), "=c"(cnt)
                 : "d"(port), "0"(addr), "1"(cnt)
                 : "memory", "cc");
}

void outw(unsigned short port, unsigned short value)
{
    asm volatile("outw %0, %1"
                 :
                 : "a"(value), "Nd"(port));
}

void outsw(uint16_t port, const void *addr, uint32_t count)
{
    asm volatile("cld; rep outsw"
                 : "+S"(addr), "+c"(count)
                 : "d"(port));
}

unsigned short inw(unsigned short port)
{
    unsigned short result;
    asm volatile("inw %1, %0"
                 : "=a"(result)
                 : "Nd"(port));
    return result;
}

void insw(uint16_t port, void *addr, int cnt)
{
    asm volatile("cld; rep insw"
                 : "=D"(addr), "=c"(cnt)
                 : "d"(port), "0"(addr), "1"(cnt)
                 : "memory", "cc");
}

void io_wait()
{
    asm volatile("outb %%al, $0x80"
                 :
                 : "a"(0));
}

void *memcpy(void *dest, const void *src, size_t count)
{
    char *dest_c = (char *)dest;
    const char *src_c = (const char *)src;
    for (size_t i = 0; i < count; i++)
    {
        dest_c[i] = src_c[i];
    }
    return dest;
}

void *memset(void *ptr, int value, size_t count)
{
    char *ptr_c = (char *)ptr;
    for (size_t i = 0; i < count; i++)
    {
        ptr_c[i] = (char)value;
    }
    return ptr;
}

void *memmove(void *dest, const void *src, size_t n)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    if (s < d && (s + n) > d)
    {
        s += n;
        d += n;
        while (n-- > 0)
            *--d = *--s;
    }
    else
    {
        while (n-- > 0)
            *d++ = *s++;
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *p1 = s1, *p2 = s2;
    while (n--)
    {
        if (*p1 != *p2)
        {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

int strcmp(const char *str1, const char *str2)
{
    while (*str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
    }

    return *(unsigned char *)str1 - *(unsigned char *)str2;
}

char *strchr(const char *str, int character)
{
    while (*str != '\0')
    {
        if (*str == character)
            return (char *)str;
        str++;
    }
    return NULL;
}

size_t strlen(const char *str)
{
    const char *s = str;
    while (*s)
        s++;
    return s - str;
}

char *strcpy(char *dest, const char *src)
{
    char *originalDest = dest;

    while (*src != '\0')
    {
        *dest = *src;
        dest++;
        src++;
    }

    *dest = '\0';
    return originalDest;
}

char *strncpy(char *destination, const char *source, size_t num)
{
    char *ptr = destination;
    size_t i = 0;

    while (*source && i < num)
    {
        *ptr++ = *source++;
        i++;
    }

    while (i < num)
    {
        *ptr++ = '\0';
        i++;
    }

    return destination;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        if (s1[i] != s2[i])
        {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }
        else if (s1[i] == '\0')
        {
            return 0;
        }
    }
    return 0;
}

void strreverse(char *begin, char *end)
{
    char aux;
    while (end > begin)
    {
        aux = *end;
        *end-- = *begin;
        *begin++ = aux;
    }
}

int toupper(int c)
{
    if (c >= 'a' && c <= 'z')
    {
        return c - ('a' - 'A');
    }
    else
    {
        return c;
    }
}

int tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c + ('a' - 'A');
    }
    else
    {
        return c;
    }
}

void itoa(int value, char *str, int base)
{
    char *ptr = str, *ptr1 = str, tmp_char;
    int tmp_value;

    if (value < 0 && base == 10)
    {
        *ptr++ = '-';
        value *= -1;
    }

    do
    {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdef"[tmp_value - value * base];
    } while (value);

    *ptr-- = '\0';
    while (ptr1 < ptr)
    {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

void utoa(unsigned int value, char *str, int base)
{
    char *ptr = str, *ptr1 = str, tmp_char;
    unsigned int tmp_value;

    do
    {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdef"[tmp_value - value * base];
    } while (value);

    *ptr-- = '\0';
    while (ptr1 < ptr)
    {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

void litoa(long int value, char *str, int base)
{
    static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char *wstr = str;
    long int sign;

    if (base < 2 || base > 35)
    {
        *wstr = '\0';
        return;
    }

    if ((sign = value) < 0)
    {
        value = -value;
    }

    do
        *wstr++ = num[value % base];
    while (value /= base);

    if (sign < 0)
    {
        *wstr++ = '-';
    }
    *wstr = '\0';

    strreverse(str, wstr - 1);
}

void ullitoa(unsigned long long int value, char *str, int base)
{
    static char num[] = "0123456789abcdef"; // hexademical uses 'a' to 'f'
    char *wstr = str;

    if (base < 2 || base > 16) // hexademical is base 16
    {
        *wstr = '\0';
        return;
    }

    do
        *wstr++ = num[value % base];
    while (value /= base);
    *wstr = '\0';

    // Reverse the string
    strreverse(str, wstr - 1);
}

void ulltoa(unsigned long long value, char *str, int base)
{
    char *ptr = str, *ptr1 = str, tmp_char;
    unsigned long long tmp_value;

    do
    {
        tmp_value = value;
        value /= base;
        *ptr++ = "fedcba9876543210123456789abcdef"[15 + (tmp_value - value * base)];
    } while (value);

    // Apply negative sign
    *ptr-- = '\0';
    while (ptr1 < ptr)
    {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

void ltoa(long value, char *str, int base)
{
    static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char *wstr = str;
    long sign;

    if (base < 2 || base > 35)
    {
        *wstr = '\0';
        return;
    }

    if ((sign = value) < 0)
        value = -value;

    do
        *wstr++ = num[value % base];
    while (value /= base);

    if (sign < 0)
        *wstr++ = '-';

    *wstr = '\0';

    strreverse(str, wstr - 1);
}

void ultoa(unsigned long value, char *str, int base)
{
    static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char *wstr = str;

    if (base < 2 || base > 35)
    {
        *wstr = '\0';
        return;
    }

    do
        *wstr++ = num[value % base];
    while (value /= base);

    *wstr = '\0';

    strreverse(str, wstr - 1);
}

void delay(uint16_t ms)
{
    for (long long int i = 0; i < 5000 * (uint16_t)ms / 2; i++)
        io_wait();
}

char *__int_str(intmax_t i, char b[], int base, bool plusSignIfNeeded, bool spaceSignIfNeeded,
                int paddingNo, bool justify, bool zeroPad)
{

    char digit[32] = {0};
    memset(digit, 0, 32);
    strcpy(digit, "0123456789");

    if (base == 16)
    {
        strcat(digit, "ABCDEF");
    }
    else if (base == 17)
    {
        strcat(digit, "abcdef");
        base = 16;
    }

    char *p = b;
    if (i < 0)
    {
        *p++ = '-';
        i *= -1;
    }
    else if (plusSignIfNeeded)
    {
        *p++ = '+';
    }
    else if (!plusSignIfNeeded && spaceSignIfNeeded)
    {
        *p++ = ' ';
    }

    intmax_t shifter = i;
    do
    {
        ++p;
        shifter = shifter / base;
    } while (shifter);

    *p = '\0';
    do
    {
        *--p = digit[i % base];
        i = i / base;
    } while (i);

    int padding = paddingNo - (int)strlen(b);
    if (padding < 0)
        padding = 0;

    if (justify)
    {
        while (padding--)
        {
            if (zeroPad)
            {
                b[strlen(b)] = '0';
            }
            else
            {
                b[strlen(b)] = ' ';
            }
        }
    }
    else
    {
        char a[256] = {0};
        while (padding--)
        {
            if (zeroPad)
            {
                a[strlen(a)] = '0';
            }
            else
            {
                a[strlen(a)] = ' ';
            }
        }
        strcat(a, b);
        strcpy(b, a);
    }

    return b;
}

int isdigit(int c)
{
    return (c >= '0' && c <= '9');
}

char *strcat(char *dest, const char *src)
{
    char *original_dest = dest;

    while (*dest)
    {
        dest++;
    }

    while ((*dest++ = *src++))
        ;

    return original_dest;
}

char *strrchr(const char *s, int c)
{
    char *last_occurrence = NULL;
    while (*s)
    {
        if (*s == c)
        {
            last_occurrence = (char *)s;
        }
        s++;
    }
    return last_occurrence;
}
