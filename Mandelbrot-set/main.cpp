#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>


int main() {
    sf::Vector2i windowSize(1000, 1000);

    sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y), "Mandelbrot set");

    int maxIterations = 20;

    double xMin = -2.0;
    double xMax = 2.0;
    double yMin = -2.0;
    double yMax = 2.0;

    sf::Image image;
    image.create(windowSize.x, windowSize.y, sf::Color::Black);

    for (int x = 0; x < windowSize.x; x++) {
        for (int y = 0; y < windowSize.y; y++) {
            sf::Vector2f c(0.0, 0.0);
            sf::Vector2f z(0.0, 0.0);

            c.x = xMin + (xMax - xMin) * x / windowSize.x;
            c.y = yMin + (yMax - yMin) * y / windowSize.y;



            for (int i = 0; i < maxIterations; ++i) {
                // z = sf::Vector2f(z.x*z.x*z.x*z.x - 6*z.x*z.x*z.y*z.y + z.y*z.y*z.y*z.y + c.x, 4*z.x*z.x*z.x*z.y - 4*z.x*z.y*z.y*z.y +c.y);
                z = sf::Vector2f(z.x*z.x - z.y*z.y + c.x, 2*z.x*z.y + c.y);
                if (i == maxIterations) {
                    image.setPixel(x, y, sf::Color::Black);
                    break;
                }
                if (z.x*z.x + z.y*z.y > 4) {
                    float t = static_cast<float>(i) / maxIterations;
                    float colorValue = t * 255.0f;
                    sf::Uint8 gray = static_cast<sf::Uint8>(colorValue);
                    image.setPixel(x, y, sf::Color(gray, gray, gray));
                }
            }
        }
    }

    sf::Texture texture;
    texture.loadFromImage(image);

    sf::Sprite sprite(texture);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed){
                window.close();
            }
        }
        window.clear();
        window.draw(sprite);
        window.display();
    }
}