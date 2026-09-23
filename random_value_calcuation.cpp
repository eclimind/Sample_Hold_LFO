#include <stdio.h>
#include <iostream>
using namespace std;

int main(){
    printf("wassup boss\n");
    std::cout << "hello\n" ;

    //code from https://www.geeksforgeeks.org/dsa/pseudo-random-number-generator-prng/
    srand(time(NULL));
    int i;
    for(i = 0; i < 5; i++)
        std::cout << (rand()%10) << "\t";
    cout << endl;


    // X(n+1) = (a * X(n) + c) mod m
    int n = 3;

    int a=293482;
    int c=3;
    int m=2348;


    for(int i = 0;i < 256;i++){

        n = ((a * n) + c) % m;

        printf("%d",n);
        n++;

    }
    cout << endl;

    /*
    idea for the random lfo gui:
    one prng with a very long state/period, so long that it looks random. since its deterministic there will be a point that it loops back to the start.

    the second prng will choose a starting point along the first prng so that different instances of the lfo, if chosen, will have a different starting state in the lfo

    this is to make sure multiple instances of the lfo can use the same initial prng while starting at different points in the sequence, so that they dont automatically use the same lfo pattern
    */

    return 0;
}