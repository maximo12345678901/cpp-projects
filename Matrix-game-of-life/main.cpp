#include <SFML/Graphics.hpp>
#include <iostream>
#include <random>
#include <cmath>


sf::Color hueToColor(int hue)
{
    float H = std::fmod(hue, 360.f);
    if (H < 0) H += 360.f;

    float C = 1.0f; // V * S = 1 * 1
    float X = C * (1 - std::fabs(std::fmod(H / 60.f, 2) - 1));
    float m = 0.0f;

    float r = 0, g = 0, b = 0;

    if (H < 60)       { r = C; g = X; }
    else if (H < 120) { r = X; g = C; }
    else if (H < 180) { g = C; b = X; }
    else if (H < 240) { g = X; b = C; }
    else if (H < 300) { r = X; b = C; }
    else              { r = C; b = X; }

    return sf::Color(
        static_cast<sf::Uint8>((r + m) * 255),
        static_cast<sf::Uint8>((g + m) * 255),
        static_cast<sf::Uint8>((b + m) * 255)
    );
}


int main() {
    int screenSize = 1000;
    int simulationSize = 50;

    int oldCellStates[simulationSize][simulationSize];
    int cellStates[simulationSize][simulationSize];

    bool paused = false;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 1);

    sf::RenderWindow window(
        sf::VideoMode(screenSize, screenSize),
        "Matrex leif"
    );
    window.setFramerateLimit(1);


    sf::Image image;
    image.create(simulationSize, simulationSize);
    sf::Texture texture;
    texture.loadFromImage(image);
    sf::Sprite sprite;
    float scale = (float) screenSize / (float) simulationSize;
    sprite.setScale(scale, scale);

    for (int x = 0; x < simulationSize; x++) {
        for (int y = 0; y < simulationSize; y++) {
            int random = dist(gen);
            cellStates[x][y] = random;
            oldCellStates[x][y] = random;
        }
    }


    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        if (!paused) {
            for (int x = 0; x < simulationSize; x++) {
                for (int y = 0; y < simulationSize; y++) {
                    cellStates[x][y] = 0;
                    for (int i = 0; i < simulationSize; i++) {
                        cellStates[x][y] += oldCellStates[x][i] * oldCellStates[i][y];
                    }

                    int cap = 360;
                    cellStates[x][y] %= cap;
                    sf::Color color = (cellStates[x][y] < cap/2.0) ? sf::Color::White : sf::Color::Black;
                    image.setPixel(x, y, color);
                }
            }

            for (int x = 0; x < simulationSize; x++)
            {
                for (int y = 0; y < simulationSize; y++) {
                    oldCellStates[x][y] = cellStates[x][y];
                }
            }
            
        }

        texture.update(image);
        sprite.setTexture(texture);

        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}