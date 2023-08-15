#include "shell.h"

char command_line[256] = {0};
int command_index = 0;

const char keyboard_map[] = {
    0,
    27,
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',
    '=',
    '\b',
    '\t',
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    '\n',
    0,
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',
    '	\'',
    '`',
    0,
    '	\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    0,
    '*',
    0,
    ' ',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    '-',
    0,
    0,
    0,
    '+',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

void print_prompt()
{
        set_color(0x00ff00);
        printf("Taha$ ");
        set_color(0x0000ff);
        printf("%s ", current_path);
        set_color(0xfffffff);
}

void handle_backspace()
{
        if (term_x == 0 && term_y > 0)
        {
                term_x = screen_width - CHAR_WIDTH;
                term_y -= CHAR_HEIGHT;
        }
        else if (term_x > 0)
        {
                term_x -= CHAR_WIDTH;
        }

        uint32_t bgColor = get_pixel(term_x, term_y);
        draw_rectangle(term_x, term_y, CHAR_WIDTH, CHAR_HEIGHT, bgColor);
}

__attribute__((interrupt)) void kb_handler(struct interrupt_frame *frame)
{
        uint8_t scancode = inb(0x60);

        if (scancode < sizeof(keyboard_map))
        {
                char key = keyboard_map[scancode];

                if (key == '\b' && command_index > 0)
                {
                        command_index--;
                        command_line[command_index] = '\0';
                        handle_backspace();
                }
                else if (key == '\n')
                {
                        printf("\n");
                        execute_command(command_line);
                        command_index = 0;
                        memset(command_line, 0, sizeof(command_line));
                        print_prompt();
                }
                else if (key && command_index < sizeof(command_line) - 1)
                {
                        command_line[command_index++] = key;
                        print_char(key);
                }
        }

        pic_end_master();
}

void execute_command(const char *cmd)
{
        if (strcmp(cmd, "ls") == 0)
        {
                ls(current_directory_cluster, bs);
                printf("\n");
        }
        else if (strncmp(cmd, "echo ", 5) == 0)
        {
                printf("%s\n", cmd + 5);
        }
        else if (strncmp(cmd, "cat ", 4) == 0)
        {
                cat(cmd + 4, bs);
                printf("\n");
        }
        else if (strncmp(cmd, "cd ", 3) == 0)
        {
                cd(cmd + 3, bs);
        }
        else
        {
                printf("Command not found: %s\n", cmd);
        }
}
