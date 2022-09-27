#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>

#if 1==0
    #include <stdio.h>
    #define DEBUG(...) printf(__VA_ARGS__)
#else
    #define DEBUG(...)
#endif

#define BUFFER_SIZE 300 //max buffer size

#define PRINT(text) write (STDOUT_FILENO, text, strlen(text));
#define PRINT_CHAR(c) write (STDOUT_FILENO, c, 1);

/**
* return the number of line to read
**/
int process_input_start(int fd, char* char_empty, char* char_full, char* char_obstacle) {
    char input;
    char input_buffer[10];
    int i = 0;
    int line_count = 0;
    int multiplier = 1;
    while(read(fd, &input, 1) > 0 && i < 10) {
        if(input == '\n') {
            //DEBUG("End of first line\n");
            if(i<4) {
                DEBUG("Missing element in first line\n");
                return -1;//Missing element in first line
            }
            *char_full = input_buffer[i-1];
            *char_obstacle = input_buffer[i-2];
            *char_empty = input_buffer[i-3];

            if(*char_full == *char_obstacle 
            || *char_obstacle == *char_empty 
            || *char_full == *char_empty) {
                DEBUG("Duplicated char\n");
                return -1;
            }

            i = i-4;
            while(i >= 0) {
                if(input_buffer[i] < '0' || input_buffer[i] > '9') {
                    DEBUG("Invalid line number\n");
                    return -1;
                }
                line_count += (multiplier*(input_buffer[i]-'0'));
                i--;
                multiplier = multiplier*10;
            }
            return line_count;
        }
        input_buffer[i] = input;
        i++;
        //DEBUG("read %c at %d\n",input, i);
    }
    return -1;
}

void print_table(int* table, int line_size, int col_size) {
    int i=0;
    int j=0;
    while(j < line_size) {
        i=0;
        while(i < col_size) {
            DEBUG("%d", *(table + j*col_size + i));
            i++;
        }
        DEBUG("\n");
        j++;
    }
   DEBUG("-------\n");
}


void print_final_table(int* table, int line_size, int col_size,
            char char_empty, char char_full, char char_obstacle,
            int result[3]) {
    int i=0;
    int j=0;

    int max_val = result[0] - 1;
    int max_line = result[1];
    int max_col = result[2];

    while(j < line_size) {
        i=0;
        while(i < col_size) {
            if(i >= (max_col - max_val) && i <= max_col
             && j >= (max_line - max_val) && j <= max_line) {
                PRINT_CHAR(&char_full);
             } else if(*(table + j * col_size + i) == 0) {
                PRINT_CHAR(&char_obstacle);
             } else {
                PRINT_CHAR(&char_empty);
             }
            i++;
        }
        PRINT("\n");
        j++;
    }
}

int get_col_size(char *input, int input_size) {
    int i = 0;
    while(i < input_size) {
        if(input[i] == '\n') {
            return i;
        }
        i++;
    }
    return -1;
}

int min(int a, int b, int c) {
    int r = a;
    if (r > b) 
        r = b;
    if (r > c) 
        r = c;
    return r;
}

int* process_input_table(   int fd, 
                            char char_empty, char char_full, char char_obstacle, 
                            int line_size, int *col_size_pt) {
    int input_line_c = 0;
    int input_col_c = 0;
    int buffer_c = 0;
    char input_buffer[BUFFER_SIZE]; //Buffer to store the input content (line by line)
    int input_size = BUFFER_SIZE; // size of element in the buffer, by default 
    int col_size = 0;

    int* input_table;

    while(input_size == BUFFER_SIZE) {
        buffer_c = 0;
        input_size = read(fd, input_buffer, BUFFER_SIZE);
        DEBUG("read %d\n", input_size);
        DEBUG("%s\n", input_buffer);

        if(col_size == 0) {
            col_size = get_col_size(input_buffer, input_size);
            *col_size_pt = col_size;
            if(col_size == 0) {
                DEBUG("Invalid col size\n");
                return NULL;
            }
            input_table = (int *)malloc((line_size * col_size) * sizeof(int));
        }

        buffer_c = 0;
        while(buffer_c < input_size && input_line_c < line_size) {
            if(input_buffer[buffer_c] == '\n') {
                //new line
                if(input_col_c != col_size) {
                    //error
                    DEBUG("Invalid input line %d col size doesn't match\n", input_line_c);
                    return NULL;
                }
                input_line_c++;
                input_col_c=0;
            } else {
                if(input_buffer[buffer_c] == char_empty) {
                    *(input_table + input_line_c * col_size + input_col_c) = 1;
                } else if(input_buffer[buffer_c] == char_obstacle) {
                    *(input_table + input_line_c * col_size + input_col_c) = 0;
                } else {
                    //invalid char
                    DEBUG("Invalid char at line %d col %d\n", input_line_c, input_col_c);
                    return NULL;
                }
                input_col_c++;
            }
            buffer_c++;
        }
    }
    return input_table;
}

void process_table(int *table, int line_size, int col_size, int result[3]) {
    int *table_copy;
    int i, j;

    int max_val, max_col, max_line;
    
    table_copy = (int *)malloc((line_size * col_size) * sizeof(int));

    //First we copy col
    i = 0;
    while(i < col_size) {
        *(table_copy + 0 * col_size + i) = *(table + 0 * col_size + i);
        i++;
    }
    //Then first line
    j = 0;
    while(j < line_size) {
        *(table_copy + j * col_size + 0) = *(table + j * col_size + 0);
        j++;
    }

    //Sum of adjacent in table copy
    j = 1;
    while(j < line_size) {
        i=1;
        while(i < col_size) {
            if(*(table + j * col_size + i) == 1) {
                *(table_copy + j * col_size + i) = min(
                                                    *(table_copy + (j-1) * col_size + i), 
                                                    *(table_copy + j * col_size + (i-1)), 
                                                    *(table_copy + (j-1) * col_size + (i-1))
                                                    ) + 1;
            } else {
                *(table_copy + j * col_size + i) = 0;
            }
            i++;
        }
        j++;
    }
    print_table(table_copy, line_size, col_size);

    //We need to find the max value in the table
    j = 0;
    max_val = *(table_copy);//Max value init from 0 0
    max_col = 0;
    max_line = 0;
    while(j < line_size) {
        i=0;
        while(i < col_size) {
            if(*(table_copy + j * col_size + i) > max_val) {//stric so we will always get the top left corner
                max_val = *(table_copy + j * col_size + i);
                max_col = i;
                max_line = j;
            }
            i++;
        }
        j++;
    }

    DEBUG("Found max in line: %d col: %d at %d\n", max_line, max_col, max_val);
    result[0] = max_val;
    result[1] = max_line;
    result[2] = max_col;

    free(table_copy);
}

int process_stream(int fd) {
    char char_empty; //Char used for empty
    char char_full; //Char used for full
    char char_obstacle; //Char used for obstacle

    int line_size = 0;
    int col_size = 0;

    int* input_table;

    int result[3] = {0,0,0};

    //We need to look if there is an input in STDIN
    line_size = process_input_start(fd, &char_empty, &char_full, &char_obstacle);
    if(line_size < 0) {
        return -1;
    }
    DEBUG("Char empty: %c\n", char_empty);
    DEBUG("Char full: %c\n", char_full);
    DEBUG("Char obstacle: %c\n", char_obstacle);
    DEBUG("Number of line: %d\n", line_size);

    input_table = process_input_table(fd, char_empty, char_full, char_obstacle, line_size, &col_size);
    if(col_size <= 0) {
        return -1;
    }

    print_table(input_table, line_size, col_size);
    process_table(input_table, line_size, col_size, result);
    if(result[0] > 1) {
        print_final_table(input_table, line_size, col_size, char_empty, char_full, char_obstacle, result);
    } else {
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int fd; //File descriptor for the input

    int i = 1;

    if(argc > 1) {//We are oppening a file
        while(i < argc) {
            DEBUG("file name is: %s\n", argv[i]);
            fd = open(argv[i], O_RDONLY);
            if(fd < 0) {
                PRINT("map error\n");
                exit(-1);
            }
            if(process_stream(fd) < 0) {
                PRINT("map error\n");
            }
            close(fd);
            i++;
        } 
    } else {
        //We need to look if there is an input in STDIN
        if(process_stream(STDIN_FILENO) < 0) {
            PRINT("map error\n");
        }
    }
    return 0;
}
