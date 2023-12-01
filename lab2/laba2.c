#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>


typedef struct {
    int parami;
    int param_numbers;
    int number_of_thread;
} ThreadParams;

int** arr;
int n;
int* determinant;

void freeMemory() {
    for (int i = 0; i < n; ++i) {
        free(arr[i]);
    }
    free(arr);
}

void findMinor(int minor[n][n], int row, int col, int size) {
    int minor_plus[size][size];
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            minor_plus[i][j] = minor[i][j];
    
    int minorRow = 0, minorCol = 0;

    for (int i = 0; i < size; ++i) {
        if (i != row) {
            for (int j = 0; j < size; ++j) {
                if (j != col) {
                    minor[minorRow][minorCol] = minor_plus[i][j];
                    minorCol++;
                }
            }
            minorCol = 0;
            minorRow++;
        }
    }
}

// Функция для нахождения определителя матрицы
int findDeterminant(int matrix[n][n], int size) {
    if (size == 1) {
        return matrix[0][0];
    } else if (size == 2) {
        return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];
    } else {
        int det = 0;
        int sign = 1;

        for (int i = 0; i < size; i++) {
            int minor[n][n]; // ОБЯЗАТЕЛЬНО РАЗМЕРНОСТЬ ТАКАЯ ЖЕ, КАК В ОБЪЯВЛЕНИИ ФУНКЦИИ

             for (int u = 0; u < size; u++){
                for (int j = 0; j < size; j++)
                    minor[u][j] = matrix[u][j];
            }

            // for (int u = 0; u < size; u++){
            //     for (int j = 0; j < size; j++)
            //         printf("%d ", minor[u][j]);
            //     printf("\n");
            // }

            findMinor(minor, 0, i, size);

            // for (int u = 0; u < size - 1; u ++){
            //     for (int j = 0; j < size - 1; j++)
            //         printf("%d ", minor[u][j]);
            //     printf("\n");
            // }
            // printf("\n");

            det += sign * matrix[0][i] * findDeterminant(minor, size - 1);
            sign = -sign;
        }

        return det;
    }
}

int help_algСomplement(int row, int column){
    int arr_copy[n][n];
    for (int i = 0; i < n; i ++)
        for (int j = 0; j < n; j++)
            arr_copy[i][j] = arr[i][j];
    findMinor(arr_copy, row, column, n);
    return arr[row][column] * findDeterminant(arr_copy, n - 1);
}


void thread_create(pthread_t* thread, const pthread_attr_t* attr, void *(*start)(void *), void* arg) {
    if (pthread_create(thread, attr, start, arg) != 0) {
        perror("create thread\n");
        exit(-1);
    }
}

void* threadDistributor(void* arg){
    ThreadParams* paramsss = (ThreadParams*)arg;
    int i = paramsss->parami;
    int numbers = paramsss->param_numbers;
    // printf("i in Distributor: %d \n", paramsss->parami);
    // printf("numbers: %d \n", paramsss->param_numbers);
    printf("номер потока: %d \n", paramsss->number_of_thread);
    int s = 0;
    for (int j = i; j < i + numbers; j++){
        // printf("determinant before: %d \n", determinant[j]);
        determinant[j] = help_algСomplement(0, j);
        // printf("determinant after: %d \n", determinant[j]);
        // s += algСomplement(0, j);
    }

    pthread_exit((void*)&s);
    // pthread_exit(NULL);
}


int main(int argc, char* argv[]){
    if (argc != 2){
        printf("Incorrect input of arguments\n");
        return 1;
    }
    int max_threads = atoi(argv[1]);
    if (max_threads <= 0){
        printf("Number of threads must be more then 0\n");
        return 1;
    }
    printf("Please enter the matrix dimension: ");
    scanf("%d", &n);

    arr = (int**)malloc(n * sizeof(int*));
    determinant = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++){
        arr[i] = (int*)malloc(n * sizeof(int));
        for (int j = 0; j < n; j++)
            scanf("%d", &arr[i][j]);
    }

    // for (int i = 0; i < n; i++){
    //     for (int j = 0; j < n; j++)
    //         printf("%d ", arr[i][j]);
    // printf("\n");
    // }
    
    double numbers_in_threads_doub = (double)n / max_threads;
    int numbers_in_threads = n / max_threads;
    if (numbers_in_threads_doub != (double)numbers_in_threads)
        numbers_in_threads += 1;
    // printf("numbers_in_threads: %d \n", numbers_in_threads);
    

    pthread_t* threads = (pthread_t*)malloc(max_threads * sizeof(pthread_t));
    // int* thread_args = (int*)malloc(max_threads * sizeof(int));

    ThreadParams params[n];
    int k = 0;
    for (int i = 0; i < n; i += numbers_in_threads){
        params[k].param_numbers = numbers_in_threads;
        params[k].parami = i;
        params[k].number_of_thread = k + 1;
        // printf("i in main: %d \n", i);
        thread_create(&threads[k], NULL, threadDistributor, (void*)&params[k]);
        k++;
    }

    for (int i = 0; i < k; i += 1){
        // printf("я здесь кккк \n");
        // void* result;
        // // // printf("я здесь tttt \n");
        // pthread_join(threads[k], &result);
        // // // printf("я здесь ooooo \n");
        // determinant += *((int*)result);
        // printf("я здесь hhhhh \n");
        pthread_join(threads[i], NULL);
    }

    int det_final = 0;
    int check = 1;
    for (int i = 0; i< n; i++){
        det_final += determinant[i] * check;
        // printf("%d ", determinant[i]);
        check *= -1;
    }
    
    printf("Ответ: %d \n", det_final);

    freeMemory();
    free(determinant);
    free(threads);

        
}
