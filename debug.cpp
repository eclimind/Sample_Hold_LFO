//this is just to debug and view program outputs
#include <iostream>
#include <vector>

#include "PluginMath.h"
#include "SequenceProcessor.h"

int main()
{
    MainPRNG mainPRNG(123);

    std::vector<int> sequence(500);

    for(int i = 0; i < sequence.size(); i++)
    {
        sequence[i] = mainPRNG.next();
    }

    StartPRNG startPRNG(7325);

    std::vector<int> lfoValues;

    sequenceProcessor(sequence, startPRNG, lfoValues);

    std::cout << "LFO values:\n";

    for(int i = 0; i < lfoValues.size(); i++)
    {
        std::cout << i << ": " << lfoValues[i] << '\n';
    }

    return 0;
}