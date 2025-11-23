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
        bool nearLeftEdge;
        bool nearRightEdge;
        bool nearTopEdge;
        bool nearBottomEdge;

    Particle() {
        position = sf::Vector2f(0, 0);
        velocity = sf::Vector2f(0, 0);

        color = 0;
        radius = 3;
        nearLeftEdge = false;
        nearRightEdge = false;
        nearTopEdge = false;
        nearBottomEdge = false;
    }
};

sf::Vector2f normalized(sf::Vector2f vec) {
    return vec / (float) hypot(vec.x, vec.y);
}

void interact(Particle& particle1, const Particle& particle2, const std::vector<std::vector<float>>* attractionMatrix, int maxDistance = 100, int minDistance = 20, float manualForce = 5.0) {
    float attraction;

    if (attractionMatrix == nullptr) {
        attraction = manualForce;
    } else {
        attraction = (*attractionMatrix)[particle1.color][particle2.color];
    }
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
            force = (dist/minDistance)*2 - 2;
        }
        else if (dist < maxDistance/2 + minDistance/2) {
            force = a*dist - a*minDistance;
        }
        else if (dist <= maxDistance) {
            force = -a*dist + a*maxDistance;
        }

        force *= 1.0;
        // Apply force to particle1’s velocity
        particle1.velocity += dir * force;
    }
}

sf::Vector3f hueToRGB(float h)
{
    // Ensure hue wraps around
    h = fmodf(h, 1.0f);
    if (h < 0.0f) h += 1.0f;

    float r, g, b;

    float i = std::floor(h * 6.0f);
    float f = h * 6.0f - i;

    float p = 0.0f;
    float q = 1.0f - f;
    float t = f;

    switch (static_cast<int>(i) % 6) {
        case 0: r = 1.0f; g = t;     b = 0.0f; break;
        case 1: r = q;    g = 1.0f; b = 0.0f; break;
        case 2: r = 0.0f; g = 1.0f; b = t;     break;
        case 3: r = 0.0f; g = q;    b = 1.0f; break;
        case 4: r = t;    g = 0.0f; b = 1.0f; break;
        case 5: r = 1.0f; g = 0.0f; b = q;     break;
    }

    return sf::Vector3f(r, g, b);
}

int main() {

    int screenWidth = 2880;
    int screenHeight = 1620;

    int particleAmount = 400;
    std::vector<Particle> particles;

    int maxDistance = 200;
    
    int colorsAmount = 10;

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
            float value = rand_attraction(gen);
            attractionMatrix.at(i).at(j) = value;
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
    window.setFramerateLimit(30);

    Particle cursorParticle;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::R) {
                    for (int i = 0; i < colorsAmount; ++i) {
                        for (int j = 0; j < colorsAmount; ++j) {
                            float value = rand_attraction(gen);
                            attractionMatrix.at(i).at(j) = value;
                        }
                    }
                }
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
            }

        }

        window.clear(sf::Color(0, 0, 0));
        
        // A few variables regarding mouse force
        sf::Vector2i mousePosInt = sf::Mouse::getPosition(window);
        cursorParticle.position = sf::Vector2f(mousePosInt.x, mousePosInt.y);
        float mouseForce = 5.0;
        float mouseInfluenceRangeMultiplier = 2.0;

        // auto start = std::chrono::high_resolution_clock::now(); //    CLOCK START  
        #pragma omp parallel for collapse(1)
        for (Particle &particle : particles) {
            
            // Wrap particles around the screen            
            particle.nearLeftEdge = false;
            particle.nearRightEdge = false;
            particle.nearTopEdge = false;
            particle.nearBottomEdge = false;

            if (particle.position.x < maxDistance) {
                particle.nearLeftEdge = true;
                if (particle.position.x < 0) {
                    particle.position.x = screenWidth - 0.01f;
                }
            }
            if (particle.position.x > screenWidth - maxDistance) {
                particle.nearRightEdge = true;
                if (particle.position.x > screenWidth) {
                    particle.position.x = 0.01f;
                }
            }

            if (particle.position.y < maxDistance) {
                particle.nearTopEdge = true;
                if (particle.position.y < 0) {
                    particle.position.y = screenHeight - 0.01f;
                }
            }
            if (particle.position.y > screenHeight - maxDistance) {
                particle.nearBottomEdge = true;
                if (particle.position.y > screenHeight) {
                    particle.position.y = 0.01f;
                }
            }

            // Calculate physics

            // Check if self
            for (Particle &otherParticle : particles) {
                // Calculate force
                if (&otherParticle == &particle) {
                    continue;
                }
                interact(particle, otherParticle, &attractionMatrix, maxDistance);
            }
            // Mouse controls

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                interact(particle, cursorParticle, nullptr, maxDistance * mouseInfluenceRangeMultiplier, 0, mouseForce);

                cursorParticle.position.x += screenWidth;
                interact(particle, cursorParticle, nullptr, maxDistance * mouseInfluenceRangeMultiplier, 0, mouseForce);
                cursorParticle.position.x -= 2*screenWidth;
                interact(particle, cursorParticle, nullptr, maxDistance * mouseInfluenceRangeMultiplier, 0, mouseForce);

                cursorParticle.position.x += screenWidth;

                cursorParticle.position.y += screenHeight;
                interact(particle, cursorParticle, nullptr, maxDistance * mouseInfluenceRangeMultiplier, 0, mouseForce);
                cursorParticle.position.y -= 2*screenHeight;
                interact(particle, cursorParticle, nullptr, maxDistance * mouseInfluenceRangeMultiplier, 0, mouseForce);
                
                cursorParticle.position.y += screenHeight;
            }
            if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
                interact(particle, cursorParticle, nullptr, maxDistance * mouseInfluenceRangeMultiplier, 0, -mouseForce);

                cursorParticle.position.x += screenWidth;
                interact(particle, cursorParticle, nullptr, maxDistance * mouseInfluenceRangeMultiplier, 0, -mouseForce);
                cursorParticle.position.x -= 2*screenWidth;
                interact(particle, cursorParticle, nullptr, maxDistance * mouseInfluenceRangeMultiplier, 0, -mouseForce);

                cursorParticle.position.x += screenWidth;

                cursorParticle.position.y += screenHeight;
                interact(particle, cursorParticle, nullptr, maxDistance * mouseInfluenceRangeMultiplier, 0, -mouseForce);
                cursorParticle.position.y -= 2*screenHeight;
                interact(particle, cursorParticle, nullptr, maxDistance * mouseInfluenceRangeMultiplier, 0, -mouseForce);
                
                cursorParticle.position.y += screenHeight;
            }
            // Make forces wrap around the edges
            for (Particle otherParticleUp : particles) {
                if (otherParticleUp.nearBottomEdge) {
                    otherParticleUp.position.y -= screenHeight;
                    interact(particle, otherParticleUp, &attractionMatrix, maxDistance);
                }
            }
            for (Particle otherParticleDown : particles) {
                if (otherParticleDown.nearTopEdge) {
                    otherParticleDown.position.y += screenHeight;
                    interact(particle, otherParticleDown, &attractionMatrix, maxDistance);
                }
            }
            for (Particle otherParticleLeft : particles) {
                if (otherParticleLeft.nearRightEdge) {
                    otherParticleLeft.position.x -= screenWidth;
                    interact(particle, otherParticleLeft, &attractionMatrix, maxDistance);
                }
            }
            for (Particle otherParticleRight : particles) {
                if (otherParticleRight.nearLeftEdge) {
                    otherParticleRight.position.x += screenWidth;
                    interact(particle, otherParticleRight, &attractionMatrix, maxDistance);
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
            particleShape.setFillColor(sf::Color(
                hueToRGB((float)particle.color / (float)colorsAmount).x * 255,
                hueToRGB((float)particle.color / (float)colorsAmount).y * 255,
                hueToRGB((float)particle.color / (float)colorsAmount).z * 255
            )); 

            window.draw(particleShape);
        }
        window.display();

    }

    return 0;
}