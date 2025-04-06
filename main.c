#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 100

typedef struct {
    int line_number;
    char code[MAX_LINE_LENGTH];
} BasicLine;

BasicLine program[MAX_LINES];
int program_size = 0;

// Find a line by its line number
int find_line(int line_number) {
    for (int i = 0; i < program_size; i++) {
        if (program[i].line_number == line_number) {
            return i;
        }
    }
    return -1;
}

// Sort program lines by line_number
void sort_program() {
    for (int i = 0; i < program_size - 1; i++) {
        for (int j = i + 1; j < program_size; j++) {
            if (program[i].line_number > program[j].line_number) {
                BasicLine temp = program[i];
                program[i] = program[j];
                program[j] = temp;
            }
        }
    }
}

void run_program() {
    int pc = 0; // program counter (index in the program array)

    while (pc < program_size) {
        char command[MAX_LINE_LENGTH];
        strcpy(command, program[pc].code);

        if (strncmp(command, "PRINT ", 6) == 0) {
            printf("%s\n", command + 6);
            pc++;
        } else if (strncmp(command, "GOTO ", 5) == 0) {
            int target_line = atoi(command + 5);
            int target_index = find_line(target_line);
            if (target_index >= 0) {
                pc = target_index;
            } else {
                printf("LINE %d NOT FOUND\n", target_line);
                break;
            }
        } else {
            printf("UNKNOWN COMMAND: %s\n", command);
            pc++;
        }
    }
}

int main() {
    char input[MAX_LINE_LENGTH];
    printf("Tiny BASIC Interpreter. Type RUN to execute.\n");

    while (1) {
        printf("> ");
        if (!fgets(input, sizeof(input), stdin))
            break;

        // Remove newline
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "RUN") == 0) {
            sort_program();
            run_program();
            continue;
        } else if (strcmp(input, "EXIT") == 0) {
            break;
        }

        // Parse line number
        int line_number;
        char *rest = strchr(input, ' ');
        if (!rest) {
            printf("SYNTAX ERROR\n");
            continue;
        }

        *rest = 0; // split input into two strings
        line_number = atoi(input);
        rest++; // move to actual code

        if (program_size < MAX_LINES) {
            program[program_size].line_number = line_number;
            strncpy(program[program_size].code, rest, MAX_LINE_LENGTH);
            program_size++;
        } else {
            printf("PROGRAM TOO LARGE\n");
        }
    }

    return 0;
}

