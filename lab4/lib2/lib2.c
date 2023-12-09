#include "lib2.h"

float derivative(float a, float deltax){
    float result;
    result = sinf(a + deltax) - sinf(a - deltax);
    result /= (2 * deltax);
    return result;
}

float square(float a, float b){
    float result = a * b;
    result /= 2;
    return result;
}