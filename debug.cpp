//this is just to debug and view program outputs
#include <fstream>
#include <iostream>
#include <vector>

#include "Source/PluginMath.h"

int main()
{
    MainPRNG mainPrng(2348);
    std::vector<int> sequence(1000);

    for (auto& value : sequence)
        value = mainPrng.next();

    std::ofstream output("prng_outputs.txt", std::ios::trunc);
    if (!output)
    {
        std::cerr << "Could not open prng_outputs.txt for writing.\n";
        return 1;
    }

    for (std::size_t i = 0; i < sequence.size(); ++i)
    {
        std::cout << i << ": " << sequence[i] << '\n';
        output << i << ": " << sequence[i] << '\n';
    }

    if (!output)
    {
        std::cerr << "Failed while writing prng_outputs.txt.\n";
        return 1;
    }

    std::cout << "Wrote " << sequence.size()
              << " PRNG outputs to prng_outputs.txt.\n";

    return 0;
}