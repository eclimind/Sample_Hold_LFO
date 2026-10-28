#include <stdio.h>
#include <iostream>
#include <array>
#include <vector>
using namespace std;
#include <math.h>

#include "PluginMath.h"
#include "SequenceProcessor.h"

int sequenceDisplayCount = 64;

int getDisplayCount() {
    return sequenceDisplayCount;
}

void setDisplayCount(int count) {
    if (count == 4 || count == 8 || count == 16 || count == 32 || count == 64 || count == 128)
        sequenceDisplayCount = count;
}

void sequenceProcessor(const vector<int>& sequence, StartPRNG& StartPRNG, std::vector<int>& lfoValues){
    /*
    this method is responsible for sequencing the mainprng's values pseudorandomly
    */
    int startpos = StartPRNG.next();

    lfoValues.clear();

    if(sequence.empty())
        return;

    startpos = startpos % static_cast<int>(sequence.size()); // for displaying a chunk of the first prng

    for(int i=0;i<sequenceDisplayCount;i++){
        const int displayIndex = (startpos + i) % static_cast<int>(sequence.size());
        const int lfo_val = sequence[displayIndex];

        lfoValues.push_back(lfo_val);
    }
}
