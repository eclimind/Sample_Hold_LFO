#include <stdio.h>
#include <iostream>
#include <array>
#include <vector>
using namespace std;

    /*
    idea for the random lfo gui:
    one prng with a very long state/period, so long that it looks random. since its deterministic there will be a point that it loops back to the start.

    the second prng will choose a starting point along the first prng so that different instances of the lfo, if chosen, will have a different starting state in the lfo

    this is to make sure multiple instances of the lfo can use the same initial prng while starting at different points in the sequence, so that they dont automatically use the same lfo pattern
    */

class mainprng{/*
    this class is responsible for creating a long prng sequence
    this will be the main source for the lfo to gain its pseudo randomized values
    */
    private:
        int state;
    public: mainprng(int seed){
        state = (seed);
    }
    int next(){
        state = (((2462346* state) + 332456) % 257274);
        return state;
    }
};

class startprng{
    /*
    this class is responsible for pseudorandomizing the start position for the mainprng
    */
    private:
        int state;
    public: startprng(int seed){
        state = (seed);
    }
    int next(){
        state = (((352* state) + 46) % 264);
        return state;
    }
};

int main(){
    /*
    main prng will generate multiple pseudo random values, with each of the values outputting into an array to save all of the states
    this array will have a fixed size. every random value will have its own index
    */
    auto random_val = 0;
    mainprng mainprng (2342);
    vector<int> sequence(500);

    for(int i = 0;i<sequence.size();i++){
        random_val = mainprng.next();
        sequence[i] = (random_val);
    }
    /*
    second prng is responsible for choosing where to start in the sequence
    */
    startprng startprng(7325);
    int startpos = startprng.next();
    startpos = startpos % sequence.size(); // for displaying a chunk of the first prng
    auto display = 100;
    for(int i=0;i<display;i++){
        int displayIndex = (startpos + i) % static_cast<int>(sequence.size());
        int lfo_val  = 0;
        lfo_val = sequence[displayIndex];

        int x = i;
        int y = lfo_val;
        cout << x << "  " << y << endl;
    }

    //keeps looping until the user decides to stop

    int j = 1;
    while(true){
        std::string userchoice = " ";
        cout<<"Want to start a new start position?\n";
        cin >> userchoice;
        if(userchoice == "yes" || userchoice ==  "Yes"){
            startpos = startprng.next();
            startpos = startpos % sequence.size();
            for(int i = 0;i< display;i++){
                int displayIndex = (startpos + i) % static_cast<int>(sequence.size());
                auto lfo_val = sequence[displayIndex];
                int x = i;
                int y = lfo_val;
                cout << x << " : " << y << endl;

        }
        }
        if(userchoice == "no" || userchoice == "No"){
            return 0;
        }

    }
    return 0;
}