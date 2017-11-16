Evolubot
========
Evolubot is a StarCraft:BroodWar 1.6 playing bot written in C++ with [BWAPI: The BroodWar API](https://bwapi.github.io).
The goal of this bot is to learn tactics and strategies by evolving over multiple games and within the same game.

Currently it only support evolving low-scale tactics for individual units by NeuroEvolution using NEAT.

## Requirements

- A compiler supporting c++17 standard is a compulsory to build.
- The [last stable NEToolKit library](https://github.com/CBenoit/NEToolKit).
- [BWAPI](https://bwapi.github.io) 4.2.0 or greater.

## Building on Windows

You can use the Visual Studio C++ 2017 project files.
You'll need to tell Visual Studio where are located BWAPI and NEToolKit by adding the following environement variables:

- `BWAPI_DIR`: the absolute path to the root folder containing BWAPI.
- `NETOOLKIT_DIR`: the absolute path to the root folder containing NEToolKit.

## Testing maps

You can find Starcraft map designed to test the bot in the `maps` folder.
It currently contains:

- `marines_battle`, a marines versus marines scenario.
- `vultures_vs_zealots`, a vultures versus zealots scenario.

To use these maps, just copy them in the `maps` folder of your Starcraft installation path.
