#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <zmq.h> // gcc server.c -o server -lzmq
#include "my_dict.h"

#define SERVER_PORT "tcp://127.0.0.1:5555" // сервер отправляет сообщения клиентам
#define CLIENT_PORT "tcp://127.0.0.1:5556" // клиенты отправляет сообщения серверу
#define GAME_PORT "tcp://127.0.0.1:5557" // клиенты отправляет сообщения серверу

#define MAX_COUNT_OF_GAMES 100
#define MAX_MAN_IN_GAME 100
#define MAX_NAME_OF_PLAYER 30
#define MAX_NAME_OF_GAME 40

char message[1000000];

int main(){
    struct Dictionary my_dict = createDictionary();

    char *stringsClients[100000];
    int count_client = 0;

    void *context = zmq_ctx_new(); // Контекст

    void *publisher = zmq_socket(context, ZMQ_PUB); // Сокет для отправки сообщений
    zmq_bind(publisher, SERVER_PORT); // Привязываем сокет к адресу

    // Создаем сокет для получения сообщений от клиента
    void *clientSubscriber = zmq_socket(context, ZMQ_PULL);
    zmq_bind(clientSubscriber, CLIENT_PORT); // Привязываем сокет к адресу

    // Создаем сокет для получения сообщений от игры
    // Я решил сделать через pull и push, потому что не мог делать много публикаторов и один подписчик
    // По итогу можно сделать так, только делать zmq_bind от ZMQ_SUB именно здесь, а в клиенте делать zmq_connect от ZMQ_PUB
    void *gameSubscriber = zmq_socket(context, ZMQ_PULL);
    zmq_bind(gameSubscriber, GAME_PORT); // Привязываем сокет к адресу
    
    // стуктура для сообщений от client и game
    // нужно для того, чтобы в цикле не ждать прихода сообщений, а проверять без блокировки, если что-то или нет
    // эту проблему можно избежать просто используя один порт, но такое плохо масшатибурется 
    zmq_pollitem_t items_for_sockets[] = {
        { clientSubscriber, 0, ZMQ_POLLIN, 0 },
        { gameSubscriber, 0, ZMQ_POLLIN, 0 }
    };


    char buffer_client[100000];
    char buffer_game[100000];
    char command[50]; // хранение команды
    char lastMessage[100000];
    char nextValue[1000];

    char name_of_game[50];
    char name_of_client[50];
    while (1) { // работает пока не будет введен eof и не ждет, пока что-то введут!
        int rc = zmq_poll(items_for_sockets, 2, 0); // Неблокирующий вызов, возвращает, на скольких сокетах произошли изменения

        // if (rc > 0){}
        // memset(buffer_client, 0, sizeof(buffer_client)); // очищаем buffer_client

        if (items_for_sockets[0].revents & ZMQ_POLLIN){ // проверяем были ли изменения в этом сокете
            printf("DEBUG SERVER: get client message\n");
            memset(buffer_client, 0, sizeof(buffer_client)); // очищаем buffer_client

            zmq_recv(clientSubscriber, buffer_client, sizeof(buffer_client), 0); // Принятие сообщения 

            // if (strcmp(buffer_client, lastMessage) == 0){
            //     printf("DEBUG SERVER: %s \n", buffer_client);
            //     continue;
            // }
            


            memset(command, 0, sizeof(command));
            sscanf(buffer_client, "%s", command);
    
            if (strcmp(command, "create") == 0){

                memset(name_of_game, 0, sizeof(name_of_game));
                memset(name_of_client, 0, sizeof(name_of_client));
                sscanf(buffer_client, "%*s %s %s", name_of_game, name_of_client);

                int found = keyExists(&my_dict, name_of_game);

                if(found){
                    memset(message, 0, sizeof(message));
                    sprintf(message, "Private %s %s CreateNotSuccess", name_of_client, name_of_game); // создаем строку message
                    zmq_send(publisher, message, strlen(message), 0);
                }
                else{
                    pid_t id = fork();
                    if (id == 0){
                        execl("./game", "./game", name_of_game, NULL); 
                        perror("execl");
                    }
                    addToDictionary(&my_dict, name_of_game, name_of_client);

                    memset(message, 0, sizeof(message));
                    sprintf(message, "Private %s %s CreateSuccess", name_of_client, name_of_game); // создаем строку message
                    zmq_send(publisher, message, strlen(message), 0);

                }

            }

            else if (strcmp(command, "connect") == 0){

                memset(name_of_game, 0, sizeof(name_of_game));
                memset(name_of_client, 0, sizeof(name_of_client));
                sscanf(buffer_client, "%*s %s %s", name_of_game, name_of_client);

                int found = keyExists(&my_dict, name_of_game);
                printf("DEBUG SERVER: try to connect %s, game: %s; found: %d;\n", name_of_client, name_of_game, found);

                if(found){
                    memset(message, 0, sizeof(message));
                    sprintf(message, "Private %s %s ConnectSuccess", name_of_client, name_of_game); // создаем строку message
                    zmq_send(publisher, message, strlen(message), 0);
                    addToDictionary(&my_dict, name_of_game, name_of_client);
                }
                else{
                    memset(message, 0, sizeof(message));
                    sprintf(message, "Private %s %s ConnectNotSuccess", name_of_client, name_of_game); // создаем строку message
                    zmq_send(publisher, message, strlen(message), 0);
                }
            }

            else if (strcmp(command, "TryAnswer") == 0){ //проверяем предположеие игрока
                zmq_send(publisher, buffer_client, strlen(buffer_client), 0); // отправляем его игре
                printf("DEBUG SERVER: try answer massege: %s;\n", buffer_client);
            }

            else if (strcmp(command, "InitName") == 0){ // проверяем, существует ли такое имя у игрока
                memset(name_of_client, 0, sizeof(name_of_client));
                sscanf(buffer_client, "%*s %s", name_of_client);

                int found = 0; // Флаг для указания наличия строки
                for (int i = 0; i < count_client + 1; ++i) {
                    if (stringsClients[i] != NULL && strcmp(stringsClients[i], name_of_client) == 0) {
                        found = 1;
                        break; // Нашли совпадение, выходим из цикла
                    }
                }
                if (found){
                    memset(message, 0, sizeof(message));
                    sprintf(message, "AnswerName %s repeat", name_of_client); // создаем строку message
                    zmq_send(publisher, message, strlen(message), 0);
                }
                else{
                    memset(message, 0, sizeof(message));
                    sprintf(message, "AnswerName %s okey", name_of_client); // создаем строку message
                    zmq_send(publisher, message, strlen(message), 0);
                    stringsClients[count_client] = strdup(name_of_client);
                    count_client ++;
                }
            }
            else if(strcmp(command, "KillServer") == 0){
                int keyyy;
                sscanf(buffer_client, "%*s %d", &keyyy);
                if (keyyy == 123456){
                    memset(message, 0, sizeof(message));
                    sprintf(message, "ServerWasKilled"); // создаем строку message
                    zmq_send(publisher, message, strlen(message), 0);
                    break;
                }

            }
            else if(strcmp(command, "LeaveGame") == 0){
                memset(name_of_game, 0, sizeof(name_of_game));
                memset(name_of_client, 0, sizeof(name_of_client));
                sscanf(buffer_client, "%*s %s %s", name_of_game, name_of_client);
                //дальше отправляем клиенту его ход
                memset(nextValue, 0, sizeof(nextValue));
                // nextValue = getNextValue(&my_dict, name_of_game, name_of_client);
                strcpy(nextValue,  getNextValue(&my_dict, name_of_game, name_of_client));
                if (nextValue != NULL){
                    memset(message, 0, sizeof(message));
                    sprintf(message, "LeaveGAME %s %s", name_of_game, name_of_client); // создаем строку message
                    zmq_send(publisher, message, strlen(message), 0);

                    printf("DEBUG SERVER: nextValue: %s;\n", nextValue);

                    memset(message, 0, sizeof(message));
                    sprintf(message, "YourTurn %s %s", name_of_game, nextValue); // создаем строку message
                    zmq_send(publisher, message, strlen(message), 0);
                }

                removePersonFromGameDictionary(&my_dict, name_of_game, name_of_client);
            }

            strcpy(lastMessage, buffer_client);
            printf("DEBUG SERVER: lastMessage: %s; buffer_client: %s;\n", lastMessage, buffer_client);

        }

        if (items_for_sockets[1].revents & ZMQ_POLLIN){
            printf("DEBUG SERVER: get game message\n");
            memset(buffer_game, 0, sizeof(buffer_game)); // очищаем buffer_game

            zmq_recv(gameSubscriber, buffer_game, sizeof(buffer_game), 0);

            if (strcmp(buffer_game, lastMessage) == 0)
                continue; 

            memset(command, 0, sizeof(command));
            sscanf(buffer_game, "%s", command);

            if (strcmp(command, "Checked") == 0){
                zmq_send(publisher, buffer_game, strlen(buffer_game), 0);
                sscanf(buffer_game, "%*s %s %s", name_of_game, name_of_client);

                //дальше отправляем клиенту его ход
                memset(nextValue, 0, sizeof(nextValue));
                // nextValue = getNextValue(&my_dict, name_of_game, name_of_client);
                strcpy(nextValue,  getNextValue(&my_dict, name_of_game, name_of_client));
                if (nextValue != NULL){
                    memset(message, 0, sizeof(message));
                    sprintf(message, "YourTurn %s %s", name_of_game, nextValue); // создаем строку message
                    zmq_send(publisher, message, strlen(message), 0);
                }
            }
            else if(strcmp(command, "Win") == 0){
                zmq_send(publisher, buffer_game, strlen(buffer_game), 0);
                sscanf(buffer_game, "%*s %s", name_of_game);
                removeFromDictionary(&my_dict, name_of_game);
            }

            strcpy(lastMessage, buffer_game);
            // printf("DEBUG SERVER: lastMessage: %s; buffer_game: %s;\n", lastMessage, buffer_game);

        }

    }


    zmq_close(publisher);
    zmq_close(clientSubscriber);
    zmq_close(gameSubscriber);
    zmq_ctx_destroy(context);

    // free(stringsClients);

}
