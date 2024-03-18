#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>

#define size 5000


int is_letter(char symbol) {
    return (symbol >= 'A' && symbol <= 'Z') ||
           (symbol >= 'a' && symbol <= 'z');
}

int is_digit(char symbol) {
    return (symbol >= '0' && symbol <= '9');
}

// Чтение из файла и запись в канал pipe_1 для передачи в процесс 2
void process_1(const char *input_file, int pipe_1[]) {
    close(pipe_1[0]);
    int fd = open(input_file, O_RDONLY);
    if (fd == -1) {
        printf("Error opening input file\n");
        exit(-1);
    }
    char str_buf[size];
    ssize_t read_bytes;
    read_bytes = read(fd, str_buf, size);
    if (read_bytes > 0) {
        write(pipe_1[1], str_buf, read_bytes);
    }

    close(pipe_1[1]);
    if(close(fd) < 0) {
        printf("Can\'t close file\n");
    }
}

// Чтение из канала pipe_1, подсчет чисел, букв, запись в канал pipe_2
void process_2(int pipe_1[], int pipe_2[]) {
    close(pipe_1[1]);
    close(pipe_2[0]);
    char str_buf[size];
    ssize_t read_bytes;
    int number_count = 0;
    int letter_count = 0;
    read_bytes = read(pipe_1[0], str_buf, size);
    for (int i = 0; i < read_bytes; i++) {
        number_count += is_digit(str_buf[i]);
        letter_count += is_letter(str_buf[i]);
    }

    // Запись в канал pipe_2 для передачи в процесс 3
    char buffer[20];
    sprintf(buffer, "Letter numbers = %d\n", letter_count);
    write(pipe_2[1], buffer, strlen(buffer));
    sprintf(buffer, "Number numbers = %d\n", number_count);
    write(pipe_2[1], buffer, strlen(buffer));

    close(pipe_1[0]);
    close(pipe_2[1]);
}

//  Чтение из pipe 2 и запись результата в файл
void process_3(const char *output_file, int pipe_1[], int pipe_2[]) {
    close(pipe_1[0]);
    close(pipe_1[1]);
    close(pipe_2[1]);
    int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (output_fd == -1) {
        printf("Error opening output file\n");
        exit(-1);
    }
    char str_buf[size];
    ssize_t read_bytes;
    while ((read_bytes = read(pipe_2[0], str_buf, size)) > 0) {
        write(output_fd, str_buf, read_bytes);
    }
    close(pipe_2[0]);

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
    int pipe_1[2]; // Канал между процессом 1 и процессом 2
    int pipe_2[2]; // Канал между процессом 2 и процессом 3

    if (pipe(pipe_1) == -1 || pipe(pipe_2) == -1) {
        printf("Can\'t create pipe\n");
        exit(-1);
    }
    pid_t pid;
    // Создание процесса 1
    pid = fork();
    if (pid == -1) {
        printf("Can\'t create process 1\n");
        exit(-1);
    } else if (pid == 0) {
        process_1(input_file, pipe_1);
        exit(0);
    }

    // Создание процесса 2
    pid = fork();
    if (pid == -1) {
        printf("Can't create process 2\n");
        exit(-1);
    } else if (pid == 0) {
        process_2(pipe_1, pipe_2);
        exit(0);
    }

    // Создание процесса 3
    pid = fork();
    if (pid == -1) {
        printf("Can't create  process 3\n");
        exit(-1);
    } else if (pid == 0) {
        process_3(output_file, pipe_1, pipe_2);
        exit(0);
    }
    close(pipe_1[0]);
    close(pipe_1[1]);
    close(pipe_2[0]);
    close(pipe_2[1]);
    // Ожидание завершения всех дочерних процессов
    while (wait(NULL) != -1);

    return 0;
}


