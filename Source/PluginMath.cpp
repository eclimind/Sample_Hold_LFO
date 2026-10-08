#include "PluginMath.h"

MainPRNG::MainPRNG(int seed){
    state = seed;
}

int MainPRNG::next(){
    state = (((2734566789528379345* state) + 23579) % 200) - 100;
    return state;

}
StartPRNG::StartPRNG(int seed){
    state = seed;
}
int StartPRNG::next(){
    state = (((352* state) + 46) % 264);
    return state;
    
}

