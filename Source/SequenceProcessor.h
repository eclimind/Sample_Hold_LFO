#pragma once
/*header to display the sequence processor to a graph display
*/
#include <vector>
#include "PluginProcessor.h"

void sequenceProcessor(const std::vector<int> sequence, StartPRNG& StartPRNG);

void graphDisplay(int x, int y);