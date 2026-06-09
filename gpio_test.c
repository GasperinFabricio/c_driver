#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd;
    char buffer;

    fd = open("/dev/gpio_tcc_dev", O_RDONLY);

    if (fd < 0) {
        perror("Erro ao abrir o dispositivo");
        return -1;
    }

    if (read(fd, &buffer, 1) > 0) {
        printf("Estado atual do GPIO: %c\n", buffer);
    }

    close(fd);
    return 0;
}