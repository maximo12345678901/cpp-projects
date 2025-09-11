#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <random>

class Particle {
    public:
        sf::Vector2f position;
        sf::Vector2f velocity;

        int radius;

        int color;

    Particle() {
        position = sf::Vector2f(0, 0);
        velocity = sf::Vector2f(0, 0);

        color = 0;
        radius = 5;
    }
};

int main() {
    int screenWidth = 1000;
    int screenHeight = 1000;

    // Create a random device (used to seed)
    std::random_device rd;
    
    // Use Mersenne Twister engine with the seed
    std::mt19937 gen(rd());
    
    // Define a distribution, e.g., uniform distribution between 1 and 100
    std::uniform_int_distribution<> rand_width(1, screenWidth);
    std::uniform_int_distribution<> rand_height(1, screenHeight);

    int particleAmount = 50;
    std::vector<Particle> particles;

    for (int i = 0; i < particleAmount; ++i) {
        Particle particle;
        particle.position = sf::Vector2f(rand_width(gen), rand_height(gen));
        particle.color = 0;
        particles.push_back(particle);
    }

    sf::RenderWindow window;
    window.create(sf::VideoMode(screenWidth, screenHeight), "particle life wooooo");
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            window.clear(sf::Color(0, 0, 0));

            for (Particle particle : particles) {
                sf::CircleShape particleShape(particle.radius);

                particleShape.setOrigin(sf::Vector2f(particle.radius, particle.radius));
                particleShape.setPosition(particle.position);
                particleShape.setFillColor(sf::Color(255, 255, 255));

                window.draw(particleShape);
            }
            window.display();
        }
    }

    return 0;
}