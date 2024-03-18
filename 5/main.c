#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>

#define buf_size 5000
#define FIFO1 "fifo1"
#define FIFO2 "fifo2"

int is_letter(char symbol) {
    return (symbol >= 'A' && symbol <= 'Z') ||
           (symbol >= 'a' && symbol <= 'z');
}

int is_digit(char symbol) {
    return (symbol >= '0' && symbol <= '9');
}

void process_1(const char *input_file, int fd) {
    int input_fd = open(input_file, O_RDONLY);

    if (input_fd == -1) {
        printf("Can\'t open input file\n");
        exit(-1);
    }

    char str_buf[buf_size];
    ssize_t read_bytes;
    // Reading input file
    while ((read_bytes = read(input_fd, str_buf, buf_size)) > 0) {
        write(fd, str_buf, read_bytes);
    }

    close(input_fd);
    close(fd);
}

void process_2(int fd1, int fd2) {
    char str_buf[buf_size];
    ssize_t read_bytes;
    char numbers_buffer[buf_size];
    int number_count = 0;
    int letter_count = 0;

    read_bytes = read(fd1, str_buf, buf_size);
    for (int i = 0; i < read_bytes; i++) {
        number_count += is_digit(str_buf[i]);
        letter_count += is_letter(str_buf[i]);
    }
    char buffer[20];
    sprintf(buffer, "Letter numbers = %d\n", letter_count);
    write(fd2, buffer, strlen(buffer));
    sprintf(buffer, "Number numbers = %d\n", number_count);
    write(fd2, buffer, strlen(buffer));

    if(close(fd1) < 0) {
        printf("Can\'t close file\n");
    }
    if(close(fd2) < 0) {
        printf("Can\'t close file\n");
    }

}

void process_3(const char *output_file, int fd) {
    int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (output_fd == -1) {
        printf("Can't open output file\n");
        exit(-1);
    }

    char str_buf[buf_size];
    ssize_t read_bytes;
    while ((read_bytes = read(fd, str_buf, buf_size)) > 0) {
        write(output_fd, str_buf, read_bytes);
    }

    close(fd);
    close(output_fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s input_file output_file\n", argv[0]);
        exit(-1);
    }
    const char *input_file = argv[1];
    const char *output_file = argv[2];
    if (mkfifo(FIFO1, 0666) == -1 || mkfifo(FIFO2, 0666) == -1) {
        printf("Can't create FIFOs\n");
        exit(-1);
    }

    pid_t pid;
    pid = fork();
    if (pid == -1) {
        printf("Can't create process 1\n");
        exit(-1);
    } else if (pid == 0) {
        int fd1 = open(FIFO1, O_WRONLY);
        process_1(input_file, fd1);
        exit(0);
    }

    pid = fork();
    if (pid == -1) {
        printf("Can't create process 2\n");
        exit(-1);
    } else if (pid == 0) {
        int fd1 = open(FIFO1, O_RDONLY);
        int fd2 = open(FIFO2, O_WRONLY);
        process_2(fd1, fd2);
        exit(0);
    }
    pid = fork();
    if (pid == -1) {
        printf("Can't create process 3\n");
        exit(-1);
    } else if (pid == 0) {
        int fd2 = open(FIFO2, O_RDONLY);
        process_3(output_file, fd2);
        exit(0);
    }

    while (wait(NULL) != -1);
    unlink(FIFO1);
    unlink(FIFO2);

    return 0;
}