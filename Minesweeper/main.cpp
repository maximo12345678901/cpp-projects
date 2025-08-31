#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>

void drawMineField(int width, int height, int cellSize, sf::RenderWindow& window) {
    int xGridSize = width * 2 * cellSize;
    int yGridSize = height * 2 * cellSize;
    for (int x = -width; x <= width; ++x) {
        int relativePixelCoordX = x * cellSize;
        int screenPixelCoordX = relativePixelCoordX + window.getSize().x / 2;
        sf::Vertex verticalLine[] = {
            sf::Vertex(sf::Vector2f(screenPixelCoordX, window.getSize().y / 2 + yGridSize / 2), sf::Color(255, 255, 255)),
            sf::Vertex(sf::Vector2f(screenPixelCoordX, window.getSize().y / 2 - yGridSize / 2), sf::Color(255, 255, 255)),
        };
        window.draw(verticalLine, 2, sf::Lines);
    }
    for (int y = -height; y <= height; ++y) {
        int relativePixelCoordY = y * cellSize;
        int screenPixelCoordY = relativePixelCoordY + window.getSize().y / 2;
        sf::Vertex horizontalLine[] = {
            sf::Vertex(sf::Vector2f(window.getSize().x / 2 + xGridSize / 2, screenPixelCoordY), sf::Color(255, 255, 255)),
            sf::Vertex(sf::Vector2f(window.getSize().x / 2 - xGridSize / 2, screenPixelCoordY), sf::Color(255, 255, 255)),
        };
        window.draw(horizontalLine, 2, sf::Lines);
    }
}
int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 1000), "Minesweper");

    int width = 5;
    int height = 5;

    int mineCount = 15;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            
        }

        window.clear(sf::Color(80, 80, 80));
        drawMineField(width, height, 80, window);
        window.display();
    }

    return 0;
}