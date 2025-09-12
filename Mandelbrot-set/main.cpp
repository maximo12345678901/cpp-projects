#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

inline sf::Vector2f complexPow(const sf::Vector2f &z, double power) {
    double r = std::sqrt(z.x * z.x + z.y * z.y);
    double theta = std::atan2(z.y, z.x);
    double rp = std::pow(r, power);
    return sf::Vector2f(
        static_cast<float>(rp * std::cos(power * theta)),
        static_cast<float>(rp * std::sin(power * theta))
    );
}


int main() {
    sf::Vector2i windowSize(1500, 1500);

    sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y), "Mandelbrot set");

    int maxIterations = 100;

    double xMin = -1.5;
    double xMax = -0.5;
    double yMin = -0.5;
    double yMax = 0.5;

    sf::Image image;
    image.create(windowSize.x, windowSize.y, sf::Color::Black);

    for (int x = 0; x < windowSize.x; x++) {
        for (int y = 0; y < windowSize.y; y++) {
            sf::Vector2f c(0.0, 0.0);
            sf::Vector2f z(0.0, 0.0);

            c.x = xMin + (xMax - xMin) * x / windowSize.x;
            c.y = yMin + (yMax - yMin) * y / windowSize.y;

            double exponent = 2.5;

            for (int i = 0; i < maxIterations; ++i) {
                sf::Vector2f power = complexPow(z, exponent);

                z.x = power.x + c.x;
                z.y = power.y + c.y;

                if (z.x * z.x + z.y * z.y > 4.0) {
                    float t = static_cast<float>(i) / maxIterations;
                    float colorValue = t * 255.0f;
                    sf::Uint8 gray = static_cast<sf::Uint8>(colorValue);
                    image.setPixel(x, y, sf::Color(gray, gray, gray));
                    break;
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