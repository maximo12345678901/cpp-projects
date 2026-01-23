#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <omp.h>
#include <iomanip>
#include <sstream>

struct ComplexNumber {
    double x;
    double y;
    ComplexNumber (double x_, double y_) {
        x = x_;
        y = y_;
    }
}; 

inline ComplexNumber complexPow(const ComplexNumber &z, double power) {
    double r = std::sqrt(z.x * z.x + z.y * z.y);
    double theta = std::atan2(z.y, z.x);
    double rp = std::pow(r, power);
    return ComplexNumber(rp * std::cos(power * theta), rp * std::sin(power * theta));

}


int main() {
    sf::Vector2i windowSize(1500, 1500);

    sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y), "Mandelbrot set");

    int maxIterations = 1500;
    // std::cout << "Max iterations: ";
    // std::cin >> maxIterations;

    float exponent = 2.0f;
    // std::cout << "\nExponent: ";
    // std::cin >> exponent;

    double xCenter = -0.0452407411;
    double yCenter = 0.9868162204352258;

    // double xCenter = -0.5;
    // double yCenter = 0.0;

    double zoom = 500000000.0;
    double screenRadius = 1.5/zoom;
    double xMin = xCenter - screenRadius;
    double xMax = xCenter + screenRadius;
    double yMin = -yCenter - screenRadius;
    double yMax = -yCenter + screenRadius;

    int frameCount = 1;


    sf::Image image;
    image.create(windowSize.x, windowSize.y, sf::Color::Black);
    #pragma omp parallel for collapse(2)
    for (int x = 0; x < windowSize.x; x++) {
        for (int y = 0; y < windowSize.y; y++) {
            ComplexNumber c(0.0, 0.0);
            ComplexNumber z(0.0, 0.0);

            c.x = xMin + (xMax - xMin) * x / windowSize.x;
            c.y = yMin + (yMax - yMin) * y / windowSize.y;


            for (int i = 0; i < maxIterations; ++i) {
                ComplexNumber power = complexPow(z, exponent);

                z.x = power.x + c.x;
                z.y = power.y + c.y;

                if (z.x * z.x + z.y * z.y > 4.0) {
                    float t = static_cast<float>(i) / maxIterations;
                    float threshold = 0.25f;

                    sf::Color color;
                    if (t <= threshold) {
                        float u = t / threshold;
                        color = sf::Color(
                            255,
                            0,
                            static_cast<sf::Uint8>(255 * u)
                        );
                    } else {
                        float u = (t - threshold) / (1-threshold);
                        color = sf::Color(
                            static_cast<sf::Uint8>(255 * (1.0f - u)),
                            0,
                            255
                        );
                    }

                    image.setPixel(x, y, color);
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

        // std::ostringstream filenameStream;
        // filenameStream << "frames/frame_" << std::setw(4) << std::setfill('0') << frameCount << ".png";
        // std::string filename = filenameStream.str();

        // image.saveToFile(filename);

        // exponent += 0.01;
        // frameCount++;
        
        window.clear();
        window.draw(sprite);
        window.display();
    }
}
