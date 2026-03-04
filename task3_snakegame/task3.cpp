#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <optional>
#include <cstdlib>
#include <ctime>
#include <sstream>

const int gridSize = 20;
const int width = 800;
const int height = 600;

struct Segment
{
    int x;
    int y;
};

enum Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

void resetGame(std::vector<Segment>& snake, Direction& dir, int& score)
{
    snake.clear();
    snake.push_back({10,10});
    snake.push_back({9,10});
    snake.push_back({8,10});
    dir = RIGHT;
    score = 0;
}

int main()
{
    sf::RenderWindow window(sf::VideoMode({width,height}), "Snake Game");
    window.setFramerateLimit(60);

    std::srand((unsigned)time(nullptr));

    // SOUND
    sf::SoundBuffer eatBuffer;
    if(!eatBuffer.loadFromFile("assets/eat.wav"))
        return -1;

    sf::Sound eatSound(eatBuffer);

    // FONT
    sf::Font font;
    if(!font.openFromFile("assets/Roboto-Regular.ttf"))
        return -1;

    sf::Text scoreText(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::Black);
    scoreText.setPosition({10,10});

    sf::Text pauseText(font);
    pauseText.setCharacterSize(40);
    pauseText.setFillColor(sf::Color::Black);
    pauseText.setPosition({320,260});
    pauseText.setString("PAUSED");

    sf::Text gameOverText(font);
    gameOverText.setCharacterSize(40);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setPosition({220,250});
    gameOverText.setString("GAME OVER\nPress R to Restart");

    std::vector<Segment> snake;
    Direction dir;
    int score = 0;

    resetGame(snake,dir,score);

    Segment food = {
        rand()%(width/gridSize),
        rand()%(height/gridSize)
    };

    bool paused=false;
    bool gameOver=false;

    sf::Clock moveClock;
    float moveDelay = 0.15f;

    while(window.isOpen())
    {
        while(const std::optional event = window.pollEvent())
        {
            if(event->is<sf::Event::Closed>())
                window.close();

            if(const auto* key = event->getIf<sf::Event::KeyPressed>())
            {
                if(key->code==sf::Keyboard::Key::Up && dir!=DOWN) dir=UP;
                if(key->code==sf::Keyboard::Key::Down && dir!=UP) dir=DOWN;
                if(key->code==sf::Keyboard::Key::Left && dir!=RIGHT) dir=LEFT;
                if(key->code==sf::Keyboard::Key::Right && dir!=LEFT) dir=RIGHT;

                if(key->code==sf::Keyboard::Key::F)
                    paused = !paused;

                if(key->code==sf::Keyboard::Key::R && gameOver)
                {
                    resetGame(snake,dir,score);
                    gameOver=false;
                    moveDelay=0.15f;
                }
            }
        }

        if(moveClock.getElapsedTime().asSeconds() > moveDelay && !paused && !gameOver)
        {
            moveClock.restart();

            Segment head = snake.front();

            if(dir==UP) head.y--;
            if(dir==DOWN) head.y++;
            if(dir==LEFT) head.x--;
            if(dir==RIGHT) head.x++;

            // wrap walls
            if(head.x < 0) head.x = width/gridSize -1;
            if(head.x >= width/gridSize) head.x = 0;
            if(head.y < 0) head.y = height/gridSize -1;
            if(head.y >= height/gridSize) head.y = 0;

            // self collision
            for(auto& s:snake)
            {
                if(s.x==head.x && s.y==head.y)
                    gameOver=true;
            }

            snake.insert(snake.begin(),head);

            if(head.x==food.x && head.y==food.y)
            {
                eatSound.play();
                score++;

                if(moveDelay > 0.05f)
                    moveDelay -= 0.004f;

                food = {
                    rand()%(width/gridSize),
                    rand()%(height/gridSize)
                };
            }
            else
            {
                snake.pop_back();
            }
        }

        std::stringstream ss;
        ss<<"Score: "<<score;
        scoreText.setString(ss.str());

        window.clear(sf::Color(245,245,245));

        // smooth snake body
        sf::CircleShape segment(gridSize / 2.f);
        segment.setFillColor(sf::Color::Black);

        for (size_t i = 0; i < snake.size(); i++)
        {
            segment.setPosition({
                static_cast<float>(snake[i].x * gridSize),
                static_cast<float>(snake[i].y * gridSize)
            });

            window.draw(segment);
        }
        
        // snake head
        sf::CircleShape headShape(gridSize/2);
        headShape.setFillColor(sf::Color::Black);
        headShape.setPosition({
            snake[0].x*gridSize,
            snake[0].y*gridSize
        });

        window.draw(headShape);

        // eyes
        sf::CircleShape eye(2);
        eye.setFillColor(sf::Color::White);

        float hx = snake[0].x*gridSize + gridSize/2;
        float hy = snake[0].y*gridSize + gridSize/2;

        eye.setPosition({hx-4,hy-3});
        window.draw(eye);

        eye.setPosition({hx+2,hy-3});
        window.draw(eye);

        // fruit
        sf::CircleShape fruit(gridSize/2 -2);
        fruit.setFillColor(sf::Color::Red);
        fruit.setPosition({
            food.x*gridSize+2,
            food.y*gridSize+2
        });

        window.draw(fruit);

        window.draw(scoreText);

        if(paused)
            window.draw(pauseText);

        if(gameOver)
            window.draw(gameOverText);

        window.display();
    }

    return 0;
}