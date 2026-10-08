#include <stdio.h>
#include <iostream>
#include <array>
#include <vector>
using namespace std;
#include <math.h>

#include "PluginMath.h"
#include "SequenceProcessor.h"

void sequenceProcessor(const vector<int>& sequence, StartPRNG& StartPRNG,std::vector<int>& lfoValues){
    /*
    this method is responsible for sequencing the mainprng's values pseudorandomly
    */ 
    int startpos = StartPRNG.next();
    startpos = startpos % sequence.size(); // for displaying a chunk of the first prng
    auto display = 32;

    lfoValues.clear();
    for(int i=0;i<display;i++){
        int displayIndex = (startpos + i) % static_cast<int>(sequence.size());
        int lfo_val = sequence[displayIndex];

        lfoValues.push_back(lfo_val);

        std::cout << lfo_val << " ";
    }

    std::cout << "start position: " << startpos << std::endl;
}

