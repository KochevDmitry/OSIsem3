#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <zmq.h> // gcc game.c -o game -lzmq

char word[] = "abcd";
char my_name[100];

#define GAME_PORT "tcp://127.0.0.1:5557" // клиенты отправляет сообщения серверу
#define SERVER_PORT "tcp://127.0.0.1:5555" // сервер отправляет сообщения клиентам

void generateRandomString() {
    // Символы, которые могут быть использованы в строке
    const char charset[] = "abcdefghijklmnopqrstuvwxyz";

    // Инициализация генератора случайных чисел
    srand((unsigned int)time(NULL));

    // Генерация случайных символов
    for (int i = 0; i < 4; ++i) {
        int index = rand() % (sizeof(charset) - 1);
        word[i] = charset[index];
    }

    // Добавляем завершающий символ '\0'
    // result[4] = '\0';
}   

int main(int argc, const char *argv[]){

    int id_message = 0;

    generateRandomString();
    printf("DEBUG GAME: answer is %s\n", word);

    if (argc > 1) {
        // Копируем переданное имя в глобальную строку
        strcpy(my_name, argv[1]);
    }

    void *context = zmq_ctx_new(); // Контекст
    void *serverSubscriber = zmq_socket(context, ZMQ_SUB); // Сокет для принятия сообщений
    zmq_connect(serverSubscriber, SERVER_PORT); // Подключаемся к адресу
    zmq_setsockopt(serverSubscriber, ZMQ_SUBSCRIBE, "", 0); // Подписываемся на все сообщения (пустая строка)

    void *publisher = zmq_socket(context, ZMQ_PUSH); // Сокет для отправки сообщений
    zmq_connect(publisher, GAME_PORT); // Привязываем сокет к адресу


    char buffer[100000];
    char command[50];
    char result[10];
    char name_man[101];
    char possible_name[100];
    // char lastMessage[100000];
    char message[10000];

    while(1){
        memset(buffer, 0, sizeof(buffer)); // очищаем buffer
        memset(command, 0, sizeof(command)); // очищаем command
        memset(result, 0, sizeof(result)); 
        memset(name_man, 0, sizeof(name_man));
        memset(possible_name, 0, sizeof(possible_name));
        memset(message, 0, sizeof(message));
        // printf("DEBUG GAME: wait massege \n");
        zmq_recv(serverSubscriber, buffer, sizeof(buffer), 0);
        // printf("DEBUG GAME: get massege \n");

        // if (strcmp(buffer, lastMessage) == 0){
        //     continue;
        // }
        

        sscanf(buffer, "%s", command); // Считываем начальное слово в command

        printf("DEBUG GAME: buffer:%s;\n", buffer);
        if (strcmp(command, "TryAnswer") == 0){ // проверяем к нам ли обращаются
        sscanf(buffer, "%*s %s", possible_name); // Считываем начальное слово в command
        // printf("DEBUG GAME: command: %s; possible name: %s; my_name: %s;\n", command, possible_name, my_name);
            // printf("DEBUG GAME: checked name \n");
            if (strcmp(possible_name, my_name) == 0){
                // printf("DEBUG GAME: lastMessage: %s;\n", lastMessage);
                // strcpy(lastMessage, buffer);
                // printf("DEBUG GAME: lastMessage: %s;\n", lastMessage);
                printf("DEBUG GAME: try answer \n");
                sscanf(buffer, "%*s %*s %s %s", name_man, result);
                if (strcmp(result, word) == 0) {
                    printf("DEBUG GAME: send to winner \n");
                    sprintf(message, "Win %s %s %s %d", my_name, name_man, result, id_message);
                    zmq_send(publisher, message, strlen(message), 0);
                    id_message++;

                    break;
                }
                int cows = 0;
                int bulls = 0;
                // Подсчет быков
                for (int i = 0; i < strlen(word); ++i) {
                    if (word[i] == result[i]) {
                        bulls ++;
                    }
                }
                // Подсчет коров
                for (int i = 0; i < strlen(word); ++i) {
                    for (int j = 0; j < strlen(result); ++j) {
                        if (i != j && word[i] == result[j]) {
                            cows ++;
                            break;
                        }
                    }
                }
                sprintf(message, "Checked %s %s %s %d %d %d", my_name, name_man, result, cows, bulls, id_message);
                zmq_send(publisher, message, strlen(message), 0);
                id_message++;
            }
        }
        else if(strcmp(command, "ServerWasKilled") == 0){
            break;
        }

    }

    zmq_close(publisher);
    zmq_close(serverSubscriber);
    zmq_ctx_destroy(context);
}
