# runningDS

runningDS is a small little DS game made with libnds and C!

I've made another DS game before, see [here](https://github.com/JuanR4140/AeroGuy), although more simpler and with 
NightFoxLib, a wrapper for libnds. 

This project though, I decided to get a little more involved and use libnds directly.
Here, you interact with the OAM and hardware initialization directly. Spooky stuff!

## What is runningDS?

For lack of a better name, runningDS is a DS platformer game inspired by Mario NES and Zelda 2.

You run across grass fields, collecting coins and slashing monsters along the way. With enough coins,
you can buy more hearts and.. get this.. **a bow**! (in Alpha 2) Isn't that exciting?!

## Sweet, how do I play?!

You have two options to get the game up and running! 

1. If you just want to play the game, you can check out both releases under `./releases` where both `runningA1.nds` and `runningA2.nds` are at. You can use an emulator to play them or play them on real hardware! (Only tested on a Nintendo DSi.)

2. If you'd like to compile from source, you can go into a release directory and run `make` to compile the source code into an `.nds`
You can then run `make clean` to clean up everything that isn't the source code. Keep in mind you may need to have libnds and nflib to compile the code (although I think the nflib folder suffices, libnds is mandatory though!)

***NOTE:*** runningA2 has a save and load functionality, however it only works on hardware and not on emulators! (Only tested on a Nintendo DSi with an SD card.)

### Happy gaming!