#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <vector>

void drawShape(std::vector<sf::Vector2f> points, sf::RenderWindow window) {

}

int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 1000), "Softbody simulation");
    std::vector<sf::Vector2f> points;
    points.push_back(sf::Vector2f(100.0, 100.0));
    points.push_back(sf::Vector2f(300.0, 400.0));

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }
    }
    return 0;
}