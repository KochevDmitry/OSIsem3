#include <stdio.h>
#include <dlfcn.h>


int main(){
    int check;
    int key = 2;
    float result;
    float arg1, arg2;
    void *current_lib;
    current_lib = dlopen("./lib2/lib2.so", RTLD_LAZY); // Загрузка библиотеки

    if (!current_lib) {
        printf("Library loading error\n");
        return 1;
    }

    float (*square)(float, float) = dlsym(current_lib, "square"); // Получение указателя на функцию
    float (*derivative)(float, float) = dlsym(current_lib, "derivative");
    if (!square) {
        printf("Could not find function\n");
        return 1;
    }

    while(scanf("%d", &check) != EOF){
        if (check != 0 && check != 1 && check != 2){
            printf("You can only enter 1, 2 or 0\n");
            dlclose(current_lib); // Закрытие библиотеки
            return 1;
        }
        else if(check == 1){
            scanf("%f %f", &arg1, &arg2);
            result = derivative(arg1, arg2);
            printf("Result: %f\n", result);
        }
        else if(check == 2){
            scanf("%f %f", &arg1, &arg2);
            result = square(arg1, arg2);
            printf("Result: %f\n", result);
        }
        else if(check == 0){
            if (key == 2){
                current_lib = dlopen("./lib1/lib1.so", RTLD_LAZY); 
                key = 1;
            }
            else{
                current_lib = dlopen("./lib2/lib2.so", RTLD_LAZY); 
                key = 2;
            }

            if (!current_lib) {
                printf("Library loading error\n");
                return 1;
            }

            square = dlsym(current_lib, "square");
            derivative = dlsym(current_lib, "derivative");
            // if (!square) {
            //     printf("Could not find function\n");
            //     return 1;
            // }
        }   
    }
    dlclose(current_lib); 
    return 0;
}