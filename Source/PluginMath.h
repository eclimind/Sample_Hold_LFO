#pragma once

class MainPRNG{/*
    this class is responsible for creating a long prng sequence
    this will be the main source for the lfo to gain its pseudo randomized values
    */
    private:
        int state;
    public: MainPRNG(int seed){
        state = (seed);
    }
    int next();
};

class StartPRNG{
    /*
    this class is responsible for pseudorandomizing the start position for the mainprng
    */
    private:
        int state;
    public: StartPRNG(int seed){
        state = (seed);
    }
    int next();
};