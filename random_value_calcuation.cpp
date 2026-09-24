#include <stdio.h>
#include <iostream>
#include <array>
#include <vector>
using namespace std;
#include <math.h>

    /*
    idea for the random lfo gui:
    one prng with a very long state/period, so long that it looks random. since its deterministic there will be a point that it loops back to the start.

    the second prng will choose a starting point along the first prng so that different instances of the lfo, if chosen, will have a different starting state in the lfo

    this is to make sure multiple instances of the lfo can use the same initial prng while starting at different points in the sequence, so that they dont automatically use the same lfo pattern
    */

class MainPRNG{/*
    this class is responsible for creating a long prng sequence
    this will be the main source for the lfo to gain its pseudo randomized values
    */
    private:
        int state;
    public: MainPRNG(int seed){
        state = (seed);
    }
    int next(){
        state = (((38839* state) + 11186023687) % 200) - 100;
        return state;
    }
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
    int next(){
        state = (((352* state) + 46) % 264);
        return state;
    }
};

void sequenceProcessor(vector<int>& sequence, StartPRNG& startprng){
    /*
    this method is responsible for sequencing the mainprng's values pseudorandomly
    */
    int startpos = startprng.next();
    startpos = startpos % sequence.size(); // for displaying a chunk of the first prng
    auto display = 256;
    for(int i=0;i<display;i++){
        int displayIndex = (startpos + i) % static_cast<int>(sequence.size());
        int lfo_val  = 0;
        lfo_val = sequence[displayIndex];

        int x = i;
        int y = lfo_val;
        graphDisplay(x,y);
    }
}

void graphDisplay(int x, int y){
    cout<< x << ":" << y << endl;

}


int main(){
    /*
    main prng will generate multiple pseudo random values, with each of the values outputting into an array to save all of the states
    this array will have a fixed size. every random value will have its own index
    */
    auto random_val = 0; //initializes variable to assign the random values of mainprng
    MainPRNG mainprng (2342);//generates mainprng seed
    vector<int> sequence(500);

    for(int i = 0;i<sequence.size();i++){
        random_val = mainprng.next();
        sequence[i] = (random_val);
    }
    StartPRNG startprng(7325);
    sequenceProcessor(sequence, startprng);
    graphDisplay();

    //keeps looping until the user decides to stop
    int j = 1;
    while(true){
        std::string userchoice = " ";
        cout<<"Want to start a new start position?\n";
        cin >> userchoice;
        if(userchoice == "yes" || userchoice ==  "Yes"){
            sequenceProcessor(sequence, startprng);
            graphDisplay();
        }
        if(userchoice == "no" || userchoice == "No"){
            return 0;
        }

    }
    return 0;
}
