#include <stdio.h>
#include <iostream>
#include <array>
#include <vector>
using namespace std;
#include <math.h>

#include <PluginMath.h>

void sequenceProcessor(const vector<int> sequence, StartPRNG& StartPRNG){
    /*
    this method is responsible for sequencing the mainprng's values pseudorandomly
    */
    int startpos = StartPRNG.next();
    startpos = startpos % sequence.size(); // for displaying a chunk of the first prng
    auto display = 256;
    for(int i=0;i<display;i++){
        int displayIndex = (startpos + i) % static_cast<int>(sequence.size());
        int lfo_val  = 0;
        lfo_val = sequence[displayIndex];

        int x = i;
        int y = lfo_val;
    }
}