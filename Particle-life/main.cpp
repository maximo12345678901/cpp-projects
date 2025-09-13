#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <random>
#include <omp.h>
#include <chrono>

class Particle {
    public:
        sf::Vector2f position;
        sf::Vector2f velocity;

        int radius;
        int color;
        bool nearEdges;

    Particle() {
        position = sf::Vector2f(0, 0);
        velocity = sf::Vector2f(0, 0);

        color = 0;
        radius = 3;
        nearEdges = false;
    }
};

void interact(Particle& particle1, const Particle& particle2, const std::vector<std::vector<float>>& attractionMatrix, int maxDistance = 100, int minDistance = 20) {

    float attraction = attractionMatrix.at(particle1.color).at(particle2.color);

    // direction
    sf::Vector2f diff = particle2.position - particle1.position;
    float distSq = diff.x * diff.x + diff.y * diff.y;
    float dist = std::sqrt(distSq);
        if (dist > maxDistance) {
        return;
    }

    if (distSq > 5) { // avoid division by zero
        sf::Vector2f dir = diff / dist;
        
        float force;

        float a = attraction/(maxDistance-minDistance);

        if (dist < minDistance) {
            force = dist/minDistance - 1;
        }
        else if (dist < maxDistance/2 + minDistance/2) {
            force = a*dist - a*minDistance;
        }
        else if (dist <= maxDistance) {
            force = -a*dist + a*maxDistance;
        }

        force *= 2;
        // Apply force to particle1’s velocity
        particle1.velocity += dir * force;
    }
}

int main() {

    int screenWidth = 2880;
    int screenHeight = 1620;

    int particleAmount = 2000;
    std::vector<Particle> particles;

    int maxDistance = 100;
    
    int colorsAmount = 5;

    // Create a random device (used to seed)
    std::random_device rd;
    
    // Use Mersenne Twister engine with the seed
    std::mt19937 gen(rd());
    
    // Define a distribution, e.g., uniform distribution between 1 and 100
    std::uniform_int_distribution<> rand_width(1, screenWidth);
    std::uniform_int_distribution<> rand_height(1, screenHeight);
    std::uniform_int_distribution<> rand_color(0, colorsAmount - 1);
    std::uniform_real_distribution<float> rand_attraction(-1.0f, 1.0f);

    std::vector<std::vector<float>> attractionMatrix(colorsAmount, std::vector<float>(colorsAmount, 1.0f));

    for (int i = 0; i < colorsAmount; ++i) {
        for (int j = 0; j < colorsAmount; ++j) {
            attractionMatrix.at(i).at(j) = rand_attraction(gen);
        }
    }
    for (int i = 0; i < particleAmount; ++i) {
        Particle particle;
        particle.position = sf::Vector2f(rand_width(gen), rand_height(gen));
        particle.color = rand_color(gen);
        particles.push_back(particle);
    }

    sf::RenderWindow window;
    window.create(sf::VideoMode(screenWidth, screenHeight), "particle life wooooo", sf::Style::Fullscreen);
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

        }

        window.clear(sf::Color(0, 0, 0));

        // auto start = std::chrono::high_resolution_clock::now(); //    CLOCK START  
        #pragma omp parallel for collapse(1)
        for (Particle &particle : particles) {

            // Wrap particles around the screen            
            particle.nearEdges = false;           
            if (particle.position.x < maxDistance) {
                particle.nearEdges = true;
                if (particle.position.x < 0) {
                    particle.position.x = screenWidth - 0.01f;
                }
            }
            if (particle.position.x > screenWidth - maxDistance) {
                particle.nearEdges = true;
                if (particle.position.x > screenWidth) {
                    particle.position.x = 0.01f;
                }
            }

            if (particle.position.y < maxDistance) {
                particle.nearEdges = true;
                if (particle.position.y < 0) {
                    particle.position.y = screenHeight - 0.01f;
                }
            }
            if (particle.position.y > screenHeight - maxDistance) {
                particle.nearEdges = true;
                if (particle.position.y > screenHeight) {
                    particle.position.y = 0.01f;
                }
            }

            // Calculate physics
            for (Particle &otherParticle : particles) {
                // Calculate force
                if (&otherParticle == &particle) {
                    continue;
                }
                interact(particle, otherParticle, attractionMatrix);
            }
            // Make forces wrap around the edges
            for (Particle otherParticleUp : particles) {
                if (otherParticleUp.nearEdges) {
                    otherParticleUp.position.y += screenHeight;
                    interact(particle, otherParticleUp, attractionMatrix, maxDistance);
                }
            }
            for (Particle otherParticleDown : particles) {
                if (otherParticleDown.nearEdges) {
                    otherParticleDown.position.y -= screenHeight;
                    interact(particle, otherParticleDown, attractionMatrix, maxDistance);
                }
            }
            for (Particle otherParticleLeft : particles) {
                if (otherParticleLeft.nearEdges) {
                    otherParticleLeft.position.x -= screenWidth;
                    interact(particle, otherParticleLeft, attractionMatrix, maxDistance);
                }
            }
            for (Particle otherParticleRight : particles) {
                if (otherParticleRight.nearEdges) {
                    otherParticleRight.position.x += screenWidth;
                    interact(particle, otherParticleRight, attractionMatrix, maxDistance);
                } 
            }
            particle.velocity *= 0.8f;
            particle.position += particle.velocity * 1.0f;
        }
        
        // auto end = std::chrono::high_resolution_clock::now();   //      CLOCK END
        // std::chrono::duration<double> elapsed = end - start;
        // std::cout << "Code took " << elapsed.count() << "s\n";

        for (Particle &particle : particles) {
            // Draw each particle
            sf::CircleShape particleShape(particle.radius);

            particleShape.setOrigin(sf::Vector2f(particle.radius, particle.radius));
            particleShape.setPosition(particle.position);
            switch (particle.color) {
                case 0:
                    particleShape.setFillColor(sf::Color(255, 0, 0));
                    break;
                case 1:
                    particleShape.setFillColor(sf::Color(255, 255, 0));
                    break;
                case 2:
                    particleShape.setFillColor(sf::Color(0, 255, 0));
                    break;
                case 3:
                    particleShape.setFillColor(sf::Color(0, 255, 255));
                    break;
                case 4:
                    particleShape.setFillColor(sf::Color(0, 0, 255));
                    break;
            }

            window.draw(particleShape);
        }
        window.display();

    }

    return 0;
}