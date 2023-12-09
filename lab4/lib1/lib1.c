#include "lib1.h"

float derivative(float a, float deltax){
    float result;

    result = sinf(a + deltax) - sinf(a);
    result /= deltax;
    return result;
}

float square(float a, float b){
    float result = a * b;
    return result;
}

//gcc -c -fPIC lib1.c
//gcc -shared lib1.o -o lib1.so -lm