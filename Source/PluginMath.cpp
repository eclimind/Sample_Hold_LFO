#include "PluginMath.h"

MainPRNG::MainPRNG(int seed){
    state = ((seed % 200) + 200) % 200;
}

int MainPRNG::next(){
    state = ((145 * state) + 179) % 200;

    return -100 + (state * 200 / 199);
}

StartPRNG::StartPRNG(int seed){
    state = ((seed % 264) + 264) % 264;
}

int StartPRNG::next(){
    state = ((352 * state) + 46) % 264;
    return state;
}
