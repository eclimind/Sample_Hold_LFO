#pragma once
/*header to display the sequence processor to a graph display
*/
#include <vector>
#include "PluginMath.h"

void sequenceProcessor(
    const std::vector<int> &sequence,
    StartPRNG& StartPRNG,
    std::vector<int>& lfoValues);