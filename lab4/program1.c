#include <stdio.h>
#include "lib1/lib1.h"

int main(){
        int check;
    while(scanf("%d", &check) != EOF){
        if (check != 0 && check != 1 && check != 2){
            printf("You can only enter 1, 2 or 0\n");
            return 1;
        }
        else if(check == 1){
            float arg1, arg2;
            scanf("%f %f", &arg1, &arg2);
            float result = derivative(arg1, arg2);
            printf("Result: %f\n", result);
        }
        else if(check == 2){
            float arg1, arg2;
            scanf("%f %f", &arg1, &arg2);
            float result = square(arg1, arg2);
            printf("Result: %f\n", result);
        }
        else{
            printf("Sorry, adiós\n");
            return 0;
        }   
    }
    return 0;
}
//gcc -o prog1 program1.c lib1/lib1.c -lm