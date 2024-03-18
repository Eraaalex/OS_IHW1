#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
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

void counter(int fd1, int fd2) {
    char str_buf[buf_size];
    ssize_t read_bytes;
    int number_count = 0;
    int letter_count = 0;

    // Чтение из pipe1, подсчет
    while ((read_bytes = read(fd1, str_buf, buf_size)) > 0) {
        for (int i = 0; i < read_bytes; i++) {
            number_count += is_digit(str_buf[i]);
            letter_count += is_letter(str_buf[i]);
        }
    }

    // Записываем общее количество чисел в pipe2
    char count_buffer[20];
    sprintf(count_buffer, "Letter numbers = %d\n", letter_count);
    write(fd2, count_buffer, strlen(count_buffer));
    sprintf(count_buffer, "Number numbers = %d\n", number_count);
    write(fd2, count_buffer, strlen(count_buffer));

}

int main() {
    const char *fifo1 = "fifo1";
    const char *fifo2 = "fifo2";

    // Открываем именованный канал для чтения
    int fd1 = open(fifo1, O_RDONLY);
    if (fd1 == -1) {
        printf("Can\'t open my FIFO = %s for reading\n", fifo1);
        exit(-1);
    }

    // Создаем и открываем именованный канал для записи
    mkfifo(fifo2, 0666);
    int fd2 = open(fifo2, O_WRONLY);
    if (fd2 == -1) {
        printf("Can\'t open my FIFO = %s for writing\n", fifo2);
        exit(-1);
    }
    counter(fd1, fd2);

    if(close(fd1) < 0){
        printf("Can\'t close FIFO 1\n");
        exit(-1);
    }
    if(close(fd2) < 0){
        printf("Can\'t close FIFO 2\n");
        exit(-1);
    }

    return 0;
}