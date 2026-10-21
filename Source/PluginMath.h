#pragma once
#include <cstdint>

class MainPRNG{/*
    this class is responsible for creating a long prng sequence
    this will be the main source for the lfo to gain its pseudo randomized values
    */
    private: std::uint32_t state;
    public: MainPRNG(std::uint32_t seed);
    int next();
};

class StartPRNG{
    /*
    this class is responsible for pseudorandomizing the start position for the mainprng
    */
    private: std::uint32_t state;
    public: StartPRNG(std::uint32_t seed);
    int next();
};