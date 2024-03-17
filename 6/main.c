#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>

#define buf_size 5000

int is_letter(char symbol) {
    return (symbol >= 'A' && symbol <= 'Z') ||
           (symbol >= 'a' && symbol <= 'z');
}

int is_digit(char symbol) {
    return (symbol >= '0' && symbol <= '9');
}

void process_1(const char *input_file, int pipe1[]) {
    close(pipe1[0]);
    int input_fd = open(input_file, O_RDONLY);
    if (input_fd == -1) {
        printf("Can\'t open input file\n");
        exit(-1);
    }
    char str_buf[buf_size];
    ssize_t read_bytes = read(input_fd, str_buf, buf_size);
    if (read_bytes > 0) {
        write(pipe1[1], str_buf, read_bytes);
    }
    close(pipe1[1]);
    if (close(input_fd) < 0) {
        printf("Can\'t close file\n");
    }
}

void process_2(int pipe1[], int pipe2[]) {
    close(pipe1[1]);
    close(pipe2[0]);
    char str_buf[buf_size];
    ssize_t read_bytes;
    char numbers_buffer[buf_size];
    int number_count = 0;
    int letter_count = 0;
    while ((read_bytes = read(pipe1[0], str_buf, buf_size)) > 0) {
        for (int i = 0; i < read_bytes; i++) {
            number_count += is_digit(str_buf[i]);
            letter_count += is_letter(str_buf[i]);
        }
    }

    char count_buffer[20];
    sprintf(count_buffer, "Letter numbers = %d\n", letter_count);
    write(pipe2[1], count_buffer, strlen(count_buffer));
    sprintf(count_buffer, "Number numbers = %d\n", number_count);
    write(pipe2[1], count_buffer, strlen(count_buffer));

    close(pipe1[0]);
    close(pipe2[1]);
}

void process_3(const char *output_file, int pipe1[], int pipe2[]) {
    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[1]);

    int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (output_fd == -1) {
        printf("Can\'t open output file\n");
        exit(-1);
    }
    char str_buf[buf_size];
    ssize_t read_bytes = read(pipe2[0], str_buf, buf_size);
    if (read_bytes > 0) {
        write(output_fd, str_buf, read_bytes);
    }
    close(pipe2[0]);
    if (close(output_fd) < 0) {
        printf("Can\'t close file\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s input_file output_file\n", argv[0]);
        exit(-1);
    }

    const char *input_file = argv[1];
    const char *output_file = argv[2];
    int pipe1[2];
    int pipe2[2];

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        printf("Can\'t create a pipe\n");
        exit(-1);
    }
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        printf("Can\'t create a process 1\n");
        exit(-1);
    } else if (pid == 0) {
        process_1(input_file, pipe1);
    } else {
        pid = fork();
        if (pid == -1) {
            printf("Can\'t create a process 2\n");
            exit(-1);
        } else if (pid == 0) {
            process_2(pipe1, pipe2);
            exit(0);
        } else {
            process_3(output_file, pipe1, pipe2);
            exit(0);
        }
    }
    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);
    // Ожидание завершения всех дочерних процессов
    while (wait(NULL) != -1);

    return 0;
}