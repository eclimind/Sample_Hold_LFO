#include <stdio.h>
#include <iostream>
using namespace std;

    /*
    idea for the random lfo gui:
    one prng with a very long state/period, so long that it looks random. since its deterministic there will be a point that it loops back to the start.

    the second prng will choose a starting point along the first prng so that different instances of the lfo, if chosen, will have a different starting state in the lfo

    this is to make sure multiple instances of the lfo can use the same initial prng while starting at different points in the sequence, so that they dont automatically use the same lfo pattern
    */
class mainprng{
    //oop translation of java into c++;
    /*
    private int state;
    public mainprng(int seed){
    
        state = seed;
    }
    public int next(){
        state = ((237 * state) + 248) % (256);
        return state;
    }
    
    */
    private:
        int state;
    public: 
        mainprng(int seed){
        state = (seed);
    }

}

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
    int state = 3;
    cout << "\n" << "\n" << "\n" << "\n" << "\n" << "\n";
    double value = static_cast<double>(state) / 256.0;
    cout << value;

    double phase = 0.0;

    return 0;
}