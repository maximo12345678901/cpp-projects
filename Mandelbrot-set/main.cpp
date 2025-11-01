#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <omp.h>
#include <iomanip>
#include <sstream>

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

    int maxIterations;
    std::cout << "Max iterations: ";
    std::cin >> maxIterations;

    float exponent;
    std::cout << "\nExponent: ";
    std::cin >> exponent;

    // double xMin = -0.752407411;
    // double xMax = 0.1152407411;
    // double yMin = 0.8568162204352258;
    // double yMax = 1.1168162204352258;

    
    double xMin = -2;
    double xMax = 1;
    double yMin = -1.5;
    double yMax = 1.5;

    int frameCount = 1;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed){
                window.close();
            }
        }

        sf::Image image;
        image.create(windowSize.x, windowSize.y, sf::Color::Black);
        #pragma omp parallel for collapse(2)
        for (int x = 0; x < windowSize.x; x++) {
            for (int y = 0; y < windowSize.y; y++) {
                sf::Vector2f c(0.0, 0.0);
                sf::Vector2f z(0.0, 0.0);

                c.x = xMin + (xMax - xMin) * x / windowSize.x;
                c.y = yMin + (yMax - yMin) * y / windowSize.y;


                for (int i = 0; i < maxIterations; ++i) {
                    sf::Vector2f power = complexPow(z, exponent);

                    z.x = power.x + c.x;
                    z.y = power.y + c.y;

                    if (z.x * z.x + z.y * z.y > 4.0) {
                        float t = static_cast<float>(i) / maxIterations;
                        float colorValue = std::sqrt(t) * 255.0f;
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

        std::ostringstream filenameStream;
        filenameStream << "frames/frame_" << std::setw(4) << std::setfill('0') << frameCount << ".png";
        std::string filename = filenameStream.str();

        image.saveToFile(filename);

        exponent += 0.01;
        frameCount++;
        window.clear();
        window.draw(sprite);
        window.display();
    }
}