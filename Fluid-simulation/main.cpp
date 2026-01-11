#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <random>

class Particle {
    public:
    sf::Vector2f position;
    sf::Color color;
    sf::Vector2f velocity;
    int radius;

    Particle(sf::Vector2f position_, float radius_, sf::Color color_ = sf::Color::White) {
        position = position_;
        color = color_;
        velocity = sf::Vector2f(0.0f, 0.0f);
        radius = radius_;
    }
};

float attractionForce(float distance, float maxRepulsion, float targetDistance, float attractionCoefficient, float maxRange) {
    return std::min(maxRepulsion/targetDistance * distance - maxRepulsion, std::max(attractionCoefficient/distance - attractionCoefficient/maxRange, 0.0f));
}

int main() {
    int screenSize = 1000;
    sf::RenderWindow window(
        sf::VideoMode(screenSize, screenSize),
        "likwid simulion"
    );
    window.setFramerateLimit(60);

    std::vector<Particle> particles;
    int particleAmount = 100;
    float viscosity = 1.0;
    float gravity = 0.5;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (int i = 0; i < particleAmount; ++i) {
        Particle particle = Particle(sf::Vector2f(dis(gen) * screenSize, dis(gen) * screenSize), 5.0);
        particles.push_back(particle);

    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }


        window.clear();
        for (Particle &particle : particles) {

            // Drag
            particle.velocity *= 0.99f;
            
            // Wall repulsion

            float leftDist   = particle.position.x;          // distance from left wall
            float rightDist  = screenSize - particle.position.x; // distance from right wall
            float floorDist  = screenSize - particle.position.y; // distance from floor (bottom)
            float ceilingDist = particle.position.y;             // distance from ceiling (top)


            float leftWall   = 0.0f;
            float rightWall  = screenSize;
            float floorWall  = screenSize;

            // Floor → push upward
            if (floorDist > 0.01f) {
                particle.velocity.y -= attractionForce(floorDist, 50.0f, 100.0f, 0.0f, 100.0f);
            }

            // Ceiling → push downward
            if (ceilingDist > 0.01f) {
                particle.velocity.y += attractionForce(ceilingDist, 50.0f, 100.0f, 0.0f, 100.0f);
            }

            // Left wall → push right
            if (leftDist > 0.01f) {
                particle.velocity.x += attractionForce(leftDist, 50.0f, 100.0f, 0.0f, 100.0f);
            }

            // Right wall → push left
            if (rightDist > 0.01f) {
                particle.velocity.x -= attractionForce(rightDist, 50.0f, 100.0f, 0.0f, 100.0f);
            }

            // Floor
            if (particle.position.y > floorWall) {
                particle.position.y = floorWall;
                particle.velocity.y *= -0.5f; // bounce + damping
            }

            // Left wall
            if (particle.position.x < leftWall) {
                particle.position.x = leftWall;
                particle.velocity.x *= -0.5f;
            }

            // Right wall
            if (particle.position.x > rightWall) {
                particle.position.x = rightWall;
                particle.velocity.x *= -0.5f;
            }


            // Interaction
            for (Particle &otherParticle : particles) {
                sf::Vector2f direction = otherParticle.position - particle.position;
                float distance = hypotf(direction.x, direction.y);
                if (distance > 0.01f) {
                    particle.velocity += direction/distance * attractionForce(distance, 10.0f, 10.0f, 1.0f, 40.0f);
                }
            }

            // Simulate
            particle.velocity.y += gravity;

            particle.position += particle.velocity;

            // Draw
            sf::CircleShape circleShape(particle.radius);
            circleShape.setPosition(particle.position);
            circleShape.setOrigin(sf::Vector2f(particle.radius, particle.radius));
            circleShape.setFillColor(particle.color);
            window.draw(circleShape);
        }
        window.display();
    }

    return 0;
}