#pragma once
/*header to display the sequence processor to a graph display
*/
#include <vector>
#include "PluginMath.h"

extern int sequenceDisplayCount;

int getDisplayCount();
void setDisplayCount(int count);

void sequenceProcessor(const std::vector<int> &sequence, StartPRNG& StartPRNG, std::vector<int>& lfoValues);