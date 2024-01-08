#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <zmq.h> // gcc client.c -o client -lzmq


#define SERVER_PORT "tcp://127.0.0.1:5555" // сервер отправляет сообщения клиентам
#define CLIENT_PORT "tcp://127.0.0.1:5556" // клиенты отправляет сообщения серверу

char my_name[100];
int can_write = 1;
int in_a_game = 0;
char name_my_game[100];

int main(){

    int id_message = 0;

    void *context = zmq_ctx_new(); // Контекст
    void *serverSubscriber = zmq_socket(context, ZMQ_SUB); // Сокет для принятия сообщений
    zmq_connect(serverSubscriber, SERVER_PORT); // Подключаемся к адресу
    zmq_setsockopt(serverSubscriber, ZMQ_SUBSCRIBE, "", 0); // Подписываемся на все сообщения (пустая строка)
    
    void *publisher = zmq_socket(context, ZMQ_PUSH); // Сокет для отправки сообщений
    zmq_connect(publisher, CLIENT_PORT); // Привязываем сокет к адресу

    char buffer[1024]; // Буфер для принятого сообщения
    char message[10000];
    char command[100];// хранение команды
    char command_serv[100];
    char possible_name[100];
    char possible_name_game[100];
    char result[100];
    char answer[100];
    char input[100000];


    printf("Write your name, please\n");
    if (fgets(input, sizeof(input), stdin) == NULL) { // Считываем вводную строку (NULL)
            printf("adios");
            exit(0);
    }
    sscanf(input, "%s", my_name);

    int check_name = 0;
    while (check_name == 0){
        memset(buffer, 0, sizeof(buffer)); // очищаем buffer
        memset(message, 0, sizeof(message));
        memset(command, 0, sizeof(command));
        memset(possible_name, 0, sizeof(possible_name));
        memset(result, 0, sizeof(result));
        memset(input, 0, sizeof(input));


        sprintf(message, "InitName %s %d", my_name, id_message); 
        zmq_send(publisher, message, strlen(message), 0); // отправили имя
        id_message++;

        // printf("DEBUG: waiting answer\n");
        zmq_recv(serverSubscriber, buffer, sizeof(buffer), 0); // ждем подтверждение
        // printf("DEBUG: we get answer\n");
        sscanf(buffer, "%s %s %s", command, possible_name, result); // Считываем начальное слово в command
        if ((strcmp(command, "AnswerName") == 0) && (strcmp(possible_name, my_name) == 0)){
            // printf("%s %s %s \n", command, possible_name, result);
            // printf("%s\n",possible_name);
            // printf("%s\n", my_name);
            if (strcmp(result, "okey") == 0){
                printf("Okey, lets play\n");
                check_name = 1;
            }
            else if (strcmp(result, "repeat") == 0){
                printf("Sorry, this name is already exist, try again:\n");
                memset(my_name, 0, sizeof(my_name));
                memset(input, 0, sizeof(input));
                if (fgets(input, sizeof(input), stdin) == NULL) { // Считываем вводную строку (NULL)
                        printf("adios\n");
                        exit(0);
                }
                sscanf(input, "%s", my_name);
            }
        }
        else if(strcmp(command, "ServerWasKilled") == 0){
            printf("Sorry, server doesnt work, goodbye\n");
            break;
        }
        else{
            // printf("something wrong:\n");
            // printf("DEBUG: command: '%s'; possible_name: '%s'; my_name: '%s'\n", command, possible_name, my_name);
        }

    }
    printf("You can write this:\n newgame [name of game] - create new game\n connect [name of game] - connect to another game\n leave - if you want to leave the game\n");

    while (1) {
        memset(buffer, 0, sizeof(buffer)); // очищаем buffer
        memset(message, 0, sizeof(message));
        memset(command, 0, sizeof(command));
        memset(command_serv, 0, sizeof(command_serv));
        memset(possible_name, 0, sizeof(possible_name));
        memset(possible_name_game, 0, sizeof(possible_name_game));
        memset(result, 0, sizeof(result));
        memset(input, 0, sizeof(input));
        memset(answer, 0, sizeof(answer));


        if(in_a_game == 0){ // еcли не в игре

            if (can_write){
                if (fgets(input, sizeof(input), stdin) == NULL) { // Считываем вводную строку (NULL)
                    printf("adios\n");
                    exit(0);
                }

                sscanf(input, "%s", command); //читаем команду

                if(strcmp(command, "newgame") == 0){
                    sscanf(input, "%*s %s", result);

                    memset(message, 0, sizeof(message));
                    sprintf(message, "create %s %s %d", result, my_name, id_message); // создаем строку message
                    zmq_send(publisher, message, strlen(message), 0);
                    id_message++;
                    can_write = 0;

                }
                else if(strcmp(command, "connect") == 0){
                    sscanf(input, "%*s %s", result);

                    memset(message, 0, sizeof(message));
                    sprintf(message, "connect %s %s %d", result, my_name, id_message); // создаем строку message
                    // printf("DEBUG CLIENT: massage for connect: %s;\n", message);
                    zmq_send(publisher, message, strlen(message), 0);
                    id_message++;
                    can_write = 0;
                    
                }
                else if(strcmp(command, "killserver") == 0){
                    int keyyy;
                    sscanf(input, "%*s %d", &keyyy);
                    memset(message, 0, sizeof(message));
                    sprintf(message, "KillServer %d", keyyy); // создаем строку message
                    // printf("DEBUG CLIENT: massage for connect: %s;\n", message);
                    zmq_send(publisher, message, strlen(message), 0);
                    id_message++;
                }
            }
            else{
                zmq_recv(serverSubscriber, buffer, sizeof(buffer), 0); 
                sscanf(buffer, "%s %s %s %s", command_serv, possible_name, possible_name_game, answer); //читаем команду
                // printf("DEBUG CLIENT: message from server NOT in game: %s;\n", buffer);
                if (strcmp(command_serv, "Private") == 0 && strcmp(possible_name, my_name) == 0){
                    if (strcmp(answer, "CreateSuccess") == 0){
                        printf("You are in the game!\n");
                        in_a_game = 1;
                        can_write = 1;
                        strcpy(name_my_game, possible_name_game);
                        // printf("DEBUG CLIENT: name of game: %s; \n", name_my_game);
                    }
                    else if (strcmp(answer, "CreateNotSuccess") == 0){ 
                        printf("This game already exist\n");
                        can_write = 1;
                    }
                    else if (strcmp(answer, "ConnectSuccess") == 0){
                        printf("You are in the game!\n");
                        in_a_game = 1;
                        can_write = 0;
                        printf("Wait your turn\n");
                        strcpy(name_my_game, possible_name_game);
                    }
                    else if (strcmp(answer, "ConnectNotSuccess") == 0){
                        printf("This game doesnt exist\n");
                        can_write = 1;
                    }
                }
                else if(strcmp(command, "ServerWasKilled") == 0){
                    printf("Sorry, server doesnt work, goodbye\n");
                    break;
                }
            }

        }
        else{
            if(can_write){
                printf("Please write your answer:\n");
                if (fgets(input, sizeof(input), stdin) == NULL) { // Считываем вводную строку (NULL)
                        printf("adios\n");
                        exit(0);
                    }
                

                sscanf(input, "%s", result);

                // если хочет покинуть игру
                if (strcmp(result, "leave") == 0){
                    memset(message, 0, sizeof(message));
                    sprintf(message, "LeaveGame %s %s %d", name_my_game, my_name, id_message); // создаем строку message
                    // printf("DEBUG CLIENT: message try answer: %s;\n", message);
                    zmq_send(publisher, message, strlen(message), 0);
                    id_message++;
                    in_a_game = 0;
                    can_write = 1;
                    printf("You leave the game\nPlease, create new game or connect\n");
                    continue;
                }

                memset(message, 0, sizeof(message));
                sprintf(message, "TryAnswer %s %s %s %d", name_my_game, my_name, result, id_message); // создаем строку message
                // printf("DEBUG CLIENT: message try answer: %s;\n", message);
                zmq_send(publisher, message, strlen(message), 0);
                id_message++;

                can_write = 0;
            }
            else{
                memset(buffer, 0, sizeof(buffer));
                zmq_recv(serverSubscriber, buffer, sizeof(buffer), 0); 
                // printf("DEBUG CLIENT: message from server in game: %s;\n", buffer);
                memset(command, 0, sizeof(command));
                sscanf(buffer, "%s", command);
                if (strcmp(command, "Checked") == 0){
                    // printf("DEBUG CLIENT: check answer good\n");
                    int cows;
                    int bulls;
                    sscanf(buffer, "%*s %s %s %s %d %d", possible_name_game, possible_name, result, &cows, &bulls);
                    // printf("User %s try: %s. Answer: cows: %d, bulls: %d\n", possible_name, result, cows, bulls);
                    // printf("%s||%s\n", name_my_game, possible_name_game);
                    if (strcmp(possible_name_game, name_my_game) == 0)
                        printf("User %s try: %s. Answer: cows: %d, bulls: %d\n", possible_name, result, cows, bulls);
                }
                else if (strcmp(command, "Win") == 0){
                    int cows;
                    int bulls;
                    sscanf(buffer, "%*s %s %s %s %d %d",possible_name_game, possible_name, result, &cows, &bulls);
                    if (strcmp(possible_name_game, name_my_game) == 0){
                        if (strcmp(possible_name, my_name) == 0)
                            printf("You are win!\n");
                        else
                            printf("User %s win with try: %s\n", possible_name, result);
                        
                        in_a_game = 0;
                        can_write = 1;
                        printf("Please, create new game or connect\n");
                    }
                }
                else if(strcmp(command, "YourTurn") == 0){
                    sscanf(buffer, "%*s %s %s",possible_name_game, possible_name);
                    if (strcmp(possible_name_game, name_my_game) == 0 && strcmp(possible_name, my_name) == 0){
                        can_write = 1;
                    }
                    else if( strcmp(possible_name_game, name_my_game) == 0 ){
                        printf("Waiting player: %s\n", possible_name);
                    }
                }
                else if(strcmp(command, "LeaveGAME") == 0){
                    sscanf(buffer, "%*s %s %s",possible_name_game, possible_name);
                    if(strcmp(possible_name_game, name_my_game) == 0 ){
                        printf("Player '%s' leave the game\n", possible_name);
                    }
                }
                else if(strcmp(command, "ServerWasKilled") == 0){
                    printf("Sorry, server doesnt work, goodbye\n");
                    break;
                }

            }

        }


    }

    zmq_close(publisher);
    zmq_close(serverSubscriber);
    zmq_ctx_destroy(context);

}
