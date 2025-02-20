#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/procinfo.h"

char* get_error_message(int error_code) {
    switch(error_code) {
        case -1:
            return "Error: Failed to copy data";
        case -2:
            return "Error: Buffer too small";
        case -3:
            return "Error: Invalid memory address";
        default:
            return "Unknown error";
    }
}

int main(void) {
    int ret, proc_count;
    printf("=== Testing ps_listinfo system call ===\n\n");

    // Тест 1: Получение количества процессов
    printf("Test 1: Getting process count\n");
    proc_count = ps_listinfo(0, 0);
    printf("Number of processes: %d\n\n", proc_count);

    // Тест 2: Недостаточный размер буфера
    printf("Test 2: Buffer too small\n");
    struct procinfo small_buf[1];
    ret = ps_listinfo(small_buf, 1);
    printf("%s (code: %d)\n", get_error_message(ret), ret);
    printf("Process count: %d, buffer size: 1\n\n", proc_count);

    // Тест 3: Некорректный адрес
    printf("Test 3: Invalid address\n");
    ret = ps_listinfo((struct procinfo*)0xdeadbeef, 1);
    printf("%s (code: %d)\n\n", get_error_message(ret), ret);

    // Тест 4: Итеративное увеличение буфера
    printf("Test 4: Increasing buffer size\n");
    int size = 2;
    struct procinfo *buf = malloc(size * sizeof(struct procinfo));
    while ((ret = ps_listinfo(buf, size)) < 0) {
        if (ret == -2) {
            size *= 2;
            printf("Increasing buffer to %d entries...\n", size);
            free(buf);
            buf = malloc(size * sizeof(struct procinfo));
        } else {
            printf("Failed with error: %s (code: %d)\n", get_error_message(ret), ret);
            break;
        }
    }
    printf("Successfully retrieved %d processes with buffer size %d\n\n", ret, size);
    free(buf);

    // Тест 5: Проверка процесса после fork()
    printf("Test 5: Process count after fork\n");
    int before = ps_listinfo(0, 0);
    if (fork() == 0) {
        exit(0);
    }
    wait(0);
    int after = ps_listinfo(0, 0);
    printf("Processes before: %d, after: %d\n\n", before, after);

    exit(0);
}
