#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for usleep
#include <ctype.h>  // for isalpha and isdigit

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 100
#define MAX_VARIABLES 26  // A-Z

typedef struct {
    int line_number;
    char code[MAX_LINE_LENGTH];
} BasicLine;

BasicLine program[MAX_LINES];
int program_size = 0;
double clock_frequency = 0; // 0 = unlimited speed
int variables[MAX_VARIABLES] = {0}; // Variables A-Z

// Find a line by its line number
int find_line(int line_number) {
    for (int i = 0; i < program_size; i++) {
        if (program[i].line_number == line_number) {
            return i;
        }
    }
    return -1;
}

// Sort program lines by line_number!
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

// List all program lines
void list_program() {
    sort_program();
    for (int i = 0; i < program_size; i++) {
        printf("%d %s\n", program[i].line_number, program[i].code);
    }
}

// Sleep according to clock frequency
void sleep_for_clock() {
    if (clock_frequency > 0) {
        int microseconds = (int)(1000000.0 / clock_frequency);
        usleep(microseconds);
    }
}

// Evaluate a simple expression (only single variable or number)
int eval_expr(const char *expr) {
    while (isspace(*expr)) expr++; // skip spaces
    if (isalpha(*expr)) {
        char var = toupper(*expr);
        return variables[var - 'A'];
    } else {
        return atoi(expr);
    }
}

void run_program() {
    int pc = 0; // program counter

    while (pc < program_size) {
        char command[MAX_LINE_LENGTH];
        strcpy(command, program[pc].code);

        // Skip REM lines
        if (strncmp(command, "REM ", 4) == 0 || strcmp(command, "REM") == 0) {
            pc++;
            sleep_for_clock();
            continue;
        }

        if (strncmp(command, "PRINT ", 6) == 0) {
            const char *to_print = command + 6;
            while (*to_print == ' ') to_print++;

            if (isalpha(*to_print) && *(to_print + 1) == '\0') {
                // Single variable
                int value = variables[toupper(*to_print) - 'A'];
                printf("%d\n", value);
            } else {
                // Regular text
                printf("%s\n", to_print);
            }
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
        } else if (strncmp(command, "LET ", 4) == 0) {
            char var;
            int value;
            if (sscanf(command + 4, " %c = %d", &var, &value) == 2) {
                var = toupper(var);
                if (var >= 'A' && var <= 'Z') {
                    variables[var - 'A'] = value;
                } else {
                    printf("INVALID VARIABLE NAME: %c\n", var);
                }
            } else {
                printf("SYNTAX ERROR IN LET STATEMENT\n");
            }
            pc++;
        } else if (strncmp(command, "IF ", 3) == 0) {
            int left, right;
            char op[3], then_part[MAX_LINE_LENGTH];
            const char *cond = command + 3;

            if (sscanf(cond, "%d %2s %d THEN %[^\n]", &left, op, &right, then_part) == 4 ||
                sscanf(cond, "%c %2s %d THEN %[^\n]", (char*)&left, op, &right, then_part) == 4) {

                if (isalpha(left)) left = variables[toupper((char)left) - 'A'];

                int condition_met = 0;
                if (strcmp(op, "=") == 0) condition_met = (left == right);
                else if (strcmp(op, ">") == 0) condition_met = (left > right);
                else if (strcmp(op, "<") == 0) condition_met = (left < right);
                else {
                    printf("UNKNOWN OPERATOR: %s\n", op);
                    pc++;
                    continue;
                }

                if (condition_met) {
                    if (strncmp(then_part, "PRINT ", 6) == 0) {
                        printf("%s\n", then_part + 6);
                    } else if (strncmp(then_part, "GOTO ", 5) == 0) {
                        int target_line = atoi(then_part + 5);
                        int target_index = find_line(target_line);
                        if (target_index >= 0) {
                            pc = target_index;
                            sleep_for_clock();
                            continue;
                        } else {
                            printf("LINE %d NOT FOUND\n", target_line);
                            break;
                        }
                    } else {
                        printf("UNKNOWN COMMAND AFTER THEN: %s\n", then_part);
                    }
                }
            } else {
                printf("SYNTAX ERROR IN IF STATEMENT\n");
            }
            pc++;
        } else {
            printf("UNKNOWN COMMAND: %s\n", command);
            pc++;
        }

        sleep_for_clock();
    }
}

// Parse command-line arguments
void parse_args(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-cf") == 0 && i + 1 < argc) {
            clock_frequency = atof(argv[i + 1]);
            if (clock_frequency <= 0) {
                printf("Invalid clock frequency. Running at full speed.\n");
                clock_frequency = 0;
            } else {
                printf("Clock frequency set to %.2f Hz\n", clock_frequency);
            }
            i++; // Skip next argument since it’s the value
        } else {
            printf("Unknown option: %s\n", argv[i]);
        }
    }
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    char input[MAX_LINE_LENGTH];
    printf("Tiny BASIC Interpreter. Type RUN, LIST or EXIT.\n");

    while (1) {
        printf("> ");
        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = 0; // remove newline

        if (strcmp(input, "RUN") == 0) {
            sort_program();
            run_program();
            continue;
        } else if (strcmp(input, "LIST") == 0) {
            list_program();
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

        *rest = 0; // split input
        line_number = atoi(input);
        rest++; // move to code

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
