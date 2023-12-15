#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <zmq.h> // gcc server.c -o server -lzmq
#include "tree.h"

char message[1000000];

void killWithChildren(TNode *node, void * publisher) { // удаляем процессы
    if (node == NULL) {
        return;
    }

    // Отключаем текущий узел
    sprintf(message, "kill %d", node->data); 
    zmq_send(publisher, message, strlen(message), 0);
    memset(message, 0, sizeof(message));

    // Рекурсивно отключаем всех потомков
    killWithChildren(node->firstChild, publisher);
    killWithChildren(node->nextBrother, publisher);
}

int pingallCommand(TNode *node, int i){
    if (node == NULL) {
        return i;
    }

    if (node->exist == false){
        printf("%d;", node->data);
        i += 1;
    }

    // Рекурсивно отключаем всех потомков
    i = pingallCommand(node->firstChild, i);
    i = pingallCommand(node->nextBrother, i);
    return i;
}


int main(){ 
    void *context = zmq_ctx_new(); // Контекст

    void *publisher = zmq_socket(context, ZMQ_PUB); // Сокет для отправки сообщений
    zmq_bind(publisher, "tcp://127.0.0.1:5555"); // Привязываем сокет к адресу

    char input[256];
    char command[60]; // массив для ввода команды
    pid_t * my_forks = (pid_t*)malloc(100 * sizeof(pid_t)); // 100 дочерних процессов
    int count_of_forks = 0;
    int param1, param2;
    TNode *root = createNode(-1);
    while(1){
        memset(message, 0, sizeof(message)); // очищаем строку сообщения
        memset(command, 0, sizeof(command)); // очищаем command
        // memset(input, 0, sizeof(input)); // очищаем input

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
                    sprintf(message, "create %d %d", param1, param2); // создаем строку message
                    zmq_send(publisher, message, strlen(message), 0);
                }
            }
        }
        else if (strcmp(command, "exec") == 0){ // --------------------exec--------------------------------
            sscanf(input, "%*s %d", &param1);
            TNode *node3 = find_node(root, param1);
            if (param1 == -1)
                printf("The controlling process only controls\n");
            else if(node3 == NULL){
                printf("This node is dead or doesn`t exist ever\n");
            }
            else{
                zmq_send(publisher, input, strlen(input), 0);
            }
        }
        else if (strcmp(command, "kill") == 0){
            sscanf(input, "%*s %d", &param1);
            TNode *node4 = find_node(root, param1);
            if (param1 == -1)
                printf("Nope\n");
            else if(node4 == NULL){
                printf("This node is dead or doesn`t exist ever\n");
            }
            else{
                int *childrenArray;
                int size;

                zmq_send(publisher, input, strlen(input), 0);
                
                killWithChildren(node4->firstChild, publisher);

                disableNode(node4->firstChild);
                disableOneNode(node4);

                free(childrenArray);
             
            }
        }
        else if(strcmp(command, "pingall") == 0){
            printf("Ok:");
            int i = pingallCommand(root, 0);
            if (i == 0)
                printf("-1");
            printf("\n");
        }

    }
    const char *message2 = "killall 1234";
    zmq_send(publisher, message2, strlen(message2), 0);
    printTree(root, 0);
    freeTree(root);
    free(my_forks);
    zmq_close(publisher);
    zmq_ctx_destroy(context);

}
