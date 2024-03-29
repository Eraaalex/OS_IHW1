#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define buf_size 5000

void read_file(int input_fd, int fd) {
    char str_buf[buf_size];
    ssize_t read_bytes;
    read_bytes = read(input_fd, str_buf, buf_size);
    if (read_bytes == -1) {
        exit(-1);
    }
    write(fd, str_buf, read_bytes);
    if(close(input_fd) < 0) {
        printf("Can\'t close file\n");
    }
    if(close(fd) < 0){
        printf("Can\'t close FIFO\n");
        exit(-1);
    }
}

void write_file(int fd, int output_fd) {
    char str_buf[buf_size];
    ssize_t read_bytes = read(fd, str_buf, buf_size);
    if (read_bytes > 0) {
        write(output_fd, str_buf, read_bytes);
    }

    if(close(output_fd) < 0) {
        printf("Can\'t close file\n");
    }
    if(close(fd) < 0){
        printf("Can\'t close FIFO\n");
        exit(-1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Incorrect number of arguments = %d.\n", argc);
        printf("Usage: %s input_file output_file\n", argv[0]);
        exit(-1);
    }
    const char *input_file = argv[1];
    const char *output_file = argv[2];
    const char *fifo1 = "fifo1";
    const char *fifo2 = "fifo2";

    int input_fd = open(input_file, O_RDONLY);
    if (input_fd == -1) {
        printf("Can\'t open input file\n");
        exit(-1);
    }
    mkfifo(fifo1, 0666); // Создание канала
    int fifo_fd = open(fifo1, O_WRONLY);
    if (fifo_fd == -1) {
        printf("Can\'t open my FIFO = %s for writing\n", fifo1);
        exit(-1);
    }
    read_file(input_fd, fifo_fd);
    int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd == -1) {
        printf("Can\'t open output file\n");
        exit(-1);
    }
    fifo_fd = open(fifo2, O_RDONLY);
    if (fifo_fd == -1) {
        printf("Can\'t open my FIFO = %s for reading\n", fifo2);
        exit(-1);
    }
    write_file(fifo_fd, output_fd);
    unlink(fifo1);
    unlink(fifo2);
    return 0;
}
