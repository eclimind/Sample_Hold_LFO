#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#include "Source/PluginMath.h"

const int gridColumns = 128;
const int gridRows = 128;
const int pixelsPerCell = 4;
const int imageWidth = gridColumns * pixelsPerCell;
const int imageHeight = gridRows * pixelsPerCell;

void writeU16(std::ofstream& output, std::uint16_t value)
{
    output.put(static_cast<char>(value & 0xff));
    output.put(static_cast<char>((value >> 8) & 0xff));
}

void writeU32(std::ofstream& output, std::uint32_t value)
{
    for (int shift = 0; shift < 32; shift += 8)
        output.put(static_cast<char>((value >> shift) & 0xff));
}

int toBrightness(int value, int minValue, int maxValue)
{
    if (maxValue == minValue)
        return 0;

    double normalized = (value - static_cast<double>(minValue))
        / (maxValue - static_cast<double>(minValue));

    int brightness = static_cast<int>(normalized * 255 + 0.5);

    return std::max(0, std::min(255, brightness));
}

bool writeBitmap(const std::vector<int>& values, const char* path)
{
    if (values.size() != gridColumns * gridRows)
        return false;

    int minValue = *std::min_element(values.begin(), values.end());
    int maxValue = *std::max_element(values.begin(), values.end());

    const std::uint32_t pixelBytes = imageWidth * imageHeight * 3;
    const std::uint32_t headerBytes = 14 + 40;

    std::ofstream output(path, std::ios::binary | std::ios::trunc);

    if (!output)
        return false;

    // write the bitmap file header
    output.put('B');
    output.put('M');
    writeU32(output, headerBytes + pixelBytes);
    writeU16(output, 0);
    writeU16(output, 0);
    writeU32(output, headerBytes);

    // write the bitmap information header
    writeU32(output, 40);
    writeU32(output, imageWidth);
    writeU32(output, imageHeight);
    writeU16(output, 1);
    writeU16(output, 24);
    writeU32(output, 0);
    writeU32(output, pixelBytes);
    writeU32(output, 2835);
    writeU32(output, 2835);
    writeU32(output, 0);
    writeU32(output, 0);

    // bitmap rows are written from bottom to top
    for (int y = imageHeight - 1; y >= 0; --y)
    {
        for (int x = 0; x < imageWidth; ++x)
        {
            int position = (y / pixelsPerCell) * gridColumns
                + x / pixelsPerCell;

            int brightness = toBrightness(
                values[position], minValue, maxValue
            );

            char gray = static_cast<char>(brightness);

            output.put(gray); // blue
            output.put(gray); // green
            output.put(gray); // red
        }
    }

    return static_cast<bool>(output);
}

int main()
{
    // generate the same raw sequence as debug.cpp
    MainPRNG mainPrng(4756);
    std::vector<int> sequence(gridColumns * gridRows);

    for (auto& value : sequence)
        value = mainPrng.next();

    const char* outputPath = "prng_visual_outputs.bmp";

    if (!writeBitmap(sequence, outputPath))
    {
        std::cerr << "Could not write the grayscale image to "
                  << outputPath << ".\n";
        return 1;
    }

    std::cout << "Wrote the 128x128 grayscale grid to "
              << outputPath << ".\n";

    return 0;
}