# C++ Classic Snake Game

A fully playable classic Snake game built using standard C++ and the Simple and Fast Multimedia Library (SFML).

## Features
* Smooth 2D grid-based movement
* Collision detection (walls and self)
* Dynamic speed scaling (game gets faster as you eat)
* Audio sound effects and custom fonts
* Pause and Restart functionality

## Requirements
* C++17 compatible compiler (e.g., GCC/MinGW)
* **SFML 3.x** (This code uses modern SFML 3 `std::optional` event polling)
* The `assets` folder containing `eat.wav` and `Roboto-Regular.ttf` must be in the same directory as the executable.

## How to Build and Run
1. Ensure SFML 3 is installed and your compiler paths are correctly configured.
2. Run the included `build.bat` script to compile the game.
3. Execute `snake.exe` to play.

## Controls
* **Arrow Keys**: Move the snake
* **F**: Pause / Unpause the game
* **R**: Restart the game after Game Over