TEMPORARY READ ME:
Sample Hold LFO
A JUCE based vst3 plugin that will create randomized sample and hold standalone lfo to route parameters by having a psuedo random number generator to create randomized values with the option of smoothly interpolate between values

It will be similar to https://kilohearts.com/docs/modulation#sample_and_hold and https://kilohearts.com/docs/modulation#random with some differences

It will have the option to be synced with host tempo or have a hz controller.
On Time Sync: the lfo will restart into a new position everytime transport starts
Note On: when a midi note gets inputed a new position to start
Free: the lfo will free run and will not restart into a new position regardless of tempo or midi information

It will also have a button to manually make a new seed

Baseline for randomization will be 2 prng's with the first one to generate a very long 'random' sequence, and the second one will determine a starting position on the first prng to prevent multiple instances of the lfo to have the same "seed"