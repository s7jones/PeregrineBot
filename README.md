# PeregrineBot
A C++ AI bot that plays the game Starcraft Broodwar using a "Zerg 5-pool" strategy.

<img src="bot_score.png">

*Caption: The scores that PeregrineBot has got against other bots in an online 24/7 league hosted at [SSCAI Tournament](http://sscaitournament.com/index.php?action=scores).*

## How to build:
* Make sure `BWAPI_DIR` AND `BWTA_DIR` environment variables are set to the relevant folders.
* Build the solution using `Release`.
* Make sure the BWTA dlls `libgmp-10.dll` & `libgmp-10.dll` from `BWTAlib_2.2\windows` are in the root Starcraft folder.

## Dependencies:
* BWAPI v4.1.2 (VS2013).
* BWTA2 v2.2 (has it own version of Boost).
* Boost v1.63.
