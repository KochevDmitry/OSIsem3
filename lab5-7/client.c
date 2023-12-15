#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <zmq.h> // gcc server.c -o server -lzmq

int id;

int main(int argc, const char *argv[]){
    // id = atoi(argv[0]);
    sscanf(argv[1], "%d", &id);
    printf("Ok my id is: %d\n", id);

    void *context = zmq_ctx_new(); // Контекст
    void *subscriber = zmq_socket(context, ZMQ_SUB); // Сокет для принятия сообщений
    zmq_connect(subscriber, "tcp://127.0.0.1:5555"); // Подключаемся к адресу
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0); // Подписываемся на все сообщения (пустая строка)
    while(1){

        char buffer[1024]; // Буфер для принятого сообщения
        char command[20]; // хранение команды
        int arg1, arg2;
        memset(buffer, 0, sizeof(buffer)); // очищаем buffer
        memset(command, 0, sizeof(command)); // очищаем buffer

        zmq_recv(subscriber, buffer, sizeof(buffer), 0); // Принятие сообщения
        // printf("message: %s\n", buffer);

        sscanf(buffer, "%s", command); // Считываем начальное слово в command

        if (strcmp(command, "create") == 0){
            sscanf(buffer, "%*s %d %d", &arg1, &arg2);
            if (id == arg2){ // если команда предназначена для нас, то выполняем
                printf("Node %d: create child\n", id);
                pid_t id_child = fork();
                if (id_child == 0){
                    char str1[sizeof(int)];
                    sprintf(str1, "%d", arg1);
                    execl("./client", "./client", str1, NULL); 
                    perror("execl");
                }
                printf("Ok: %d\n", id_child);
            }
        }
        else if (strcmp(command, "exec") == 0){
            sscanf(buffer, "%*s %d %d", &arg1, &arg2);
            if (id == arg1){ // если команда предназначена для нас, то выполняем
                int* array;
                array = (int*)malloc(arg2 * sizeof(int));
                int i = 0;
                char *ptr = strchr(buffer, ' ');
                // читаем массив чисел из строки (+2, потому что мы в этот массив читаем еще первые два аргумента из buffer, то есть arg1 и arg2)
                while (ptr != NULL && sscanf(ptr, "%d", &array[i]) == 1 && i != arg2 + 2) { 
                    i++;
                    ptr = strchr(ptr + 1, ' ');
                }

                int s = 0;
                for (int i = 2; i < arg2 + 2; i++){ // складываем числа
                    s += array[i];
                }

                printf("Ok:%d: %d\n", id, s);
                free(array);
            }
        }

        if (strcmp(command, "kill") == 0){ // когда строка равна "kill"
            break;
        }
    }
}
