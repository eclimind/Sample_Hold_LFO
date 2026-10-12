
#include "PluginMath.h"
#include <cstdint>

MainPRNG::MainPRNG(int seed){
    state = static_cast<uint32_t>(seed);
}

int MainPRNG::next(){
    state = (22695477ULL * state + 1ULL) % 4294967296ULL;

    int raw_val = state % 200;
    return raw_val - 100;
}

StartPRNG::StartPRNG(int seed){
    state = static_cast<uint32_t>(seed);
}

int StartPRNG::next(){
    state = (22695477ULL * state + 1ULL) % 4294967296ULL;

    return state % 264;
}
