//Program to calculate trapezoidal rule serially
#include <stdio.h>
main(){
    float integral; //Store results of integral
    float a,b; //Left and right end points
    int n; // Number of segments
    float h; // height 
    float x; 
    int i;

    float f(int x);
    printf("Enter a,b and n \n");
    scanf("%f %f %d", &a,&b,&n);
    h = (b-a)/n;
    integral = (f(a)+f(b))/2.0;
    x = a;
    for (int i=1;i<=n;i++){
        x= x + h;
        integral+=f(x);
    }
    integral = integral * h;
    printf("With n = %d trapezoids, our estimate\n", n);
    printf("of integrals from left to right = %f",integral);
}

float f(int x){
    //float return_value;
    // Calculate f(x) with f(x) = 2*x + 3
    switch(x){
        case (0):{
            return 3;
            break;
        }
        case (2):{
            return 7;
            break;
        }
        case (4):{
            return 11;
            break;
        case (6): {
            return 9;
            break;
        }
        case(8):{
            return 3;
            break;
        }
        default:{
            printf("Undefined condition in case statement!");
            return 0;
            break;
        }
        }
    }
}
    //return_value = 2*x+3;
    //return return_value;
