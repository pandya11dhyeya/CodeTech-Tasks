@echo off
echo Building Snake Game...

g++ task3.cpp -o snake.exe -std=c++17 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

if %ERRORLEVEL% NEQ 0 (
echo.
echo Build failed.
pause
exit /b
)

echo.
echo Build successful!
echo Run snake.exe to play the game.
pause