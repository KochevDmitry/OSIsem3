#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <zmq.h> // gcc server.c -o server -lzmq
#include "tree.h"


int main(){ 
    void *context = zmq_ctx_new(); // Контекст

    void *publisher = zmq_socket(context, ZMQ_PUB); // Сокет для отправки сообщений
    zmq_bind(publisher, "tcp://127.0.0.1:5555"); // Привязываем сокет к адресу

    char input[256];
    char command[60]; // массив для ввода команды
    char message[1000000];
    pid_t * my_forks = (pid_t*)malloc(100 * sizeof(pid_t)); // 100 дочерних процессов
    int count_of_forks = 0;
    int param1, param2;
    TNode *root = createNode(-1);
    while(1){
        memset(message, 0, sizeof(message)); // очищаем строку сообщения
        if (fgets(input, sizeof(input), stdin) == NULL) { // Считываем вводную строку (NULL)
            // printf("adios\n");
            // Если встречен конец файла, завершаем цикл
            break;
        }

        sscanf(input, "%s", command); //читаем команду

        if (strcmp(command, "create") == 0){ // ------------------------------------create----------------------------------------
            sscanf(input, "%*s %d %d", &param1, &param2);
            TNode *node = find_node(root, param2); //ищем родителя
            TNode *node2 = find_node(root, param1); //ищем id, вдруг он уже есть
            if (node == NULL){
                printf("Error: Parent not found \n");
                // return 1;
            }
            else if(node2 != NULL){
                printf("Error: Already exist \n");
                // return 1;
            }
            else{
                addChild(node, param1); //добавляем дочерний процесс в дерево
                if (param2 == -1){ // если родительский процесс - родитель
                    pid_t id = fork();
                    if (id == 0){
                        char str1[sizeof(int)];
                        sprintf(str1, "%d", param1);
                        execl("./client", "./client", str1, NULL); 
                        perror("execl");
                    }
                    printf("Ok: %d\n", id);
                    // my_forks[count_of_forks] = id;
                    // count_of_forks ++;
                }
                else{ // если родительский процесс - кто-то в дереве
                    sprintf(message, "create %d %d", param1, param2);
                    zmq_send(publisher, message, strlen(message), 0);
                }
            }
        }
        else if (strcmp(command, "exec") == 0){
            sscanf(input, "%*s %d", &param1);
            if (param1 == -1)
                printf("The controlling process only controls\n");
            else{
                zmq_send(publisher, input, strlen(input), 0);
            }
        }
    }
    const char *message2 = "kill";
    zmq_send(publisher, message2, strlen(message2), 0);
    printTree(root, 0);
    freeTree(root);
    free(my_forks);
    zmq_close(publisher);
    zmq_ctx_destroy(context);

}