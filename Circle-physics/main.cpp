#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <random>

struct Circle {
    sf::Vector2f position;
    sf::Vector2f velocity;

    float mass;
    int radius;

    sf::CircleShape circleshape;

    Circle(sf::Vector2f position_ = sf::Vector2f(0.0f, 0.0f), float mass_ = 1.0, int radius_ = 10, sf::Color color_ = sf::Color::White) {
        position = position_;
        velocity = sf::Vector2f(10.0f, 0.0f);

        mass = mass_;
        radius = radius_;
        
        circleshape.setFillColor(color_);
        circleshape.setOrigin(sf::Vector2f(radius, radius));
        circleshape.setPosition(position);
        circleshape.setRadius(radius);
    }
};
float dot(const sf::Vector2f& a, const sf::Vector2f& b) {
    return a.x * b.x + a.y * b.y;
}

float hypot (sf::Vector2f vec) {
    return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

sf::Vector2f normalized (sf::Vector2f vec) {
    return vec / hypot(vec);
}

sf::Vector2f reflect(const sf::Vector2f& v, const sf::Vector2f& axis) {
    sf::Vector2f n = normalized(axis); // ensure the axis is normalized
    return v - 2.f * dot(v, n) * n;
}
int main() {
    float gravity = 1.0;
    std::vector<Circle> circles;

    int width = 1600;
    int height = 900;

    // Create a random device and seed a Mersenne Twister engine
    std::random_device rd;
    std::mt19937 gen(rd());

    // Define a uniform distribution between 0 and 1
    std::uniform_real_distribution<> rand(0.0, 1.0);


    for (int i = 0; i < 100; i++) {
        Circle circle = Circle(sf::Vector2f(width / 2.0, height / 4.0));
        circle.velocity = sf::Vector2f((rand(gen) - 0.5f) * 20.0f, (rand(gen) - 0.5f) * 20.0f);
        circles.push_back(circle);
    }


    sf::RenderWindow window(sf::VideoMode(width, height), "cirkel simultion");
    window.setFramerateLimit(60);
    while (window.isOpen()) {
        // Event loop
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed){
                window.close();
            }
        }

        window.clear();
        for (Circle &circle : circles) {
            // Set the position
            circle.velocity += sf::Vector2f(0.0f, gravity * circle.mass);
            circle.position += circle.velocity;

            for (int i = 0; i < 1; i++) {
                // Collision detection (edges)
                if (circle.position.x <= 0 + circle.radius) {
                    circle.velocity.x *= -1.0;
                    circle.position.x = circle.radius;
                }
                if (circle.position.x >= width - circle.radius) {
                    circle.velocity.x *= -1.0;
                    circle.position.x = width - circle.radius;
                }
                if (circle.position.y <= 0 + circle.radius) {
                    circle.velocity.y *= -1.0;
                    circle.position.y = circle.radius;
                }
                if (circle.position.y >= height - circle.radius) {
                    circle.velocity.y *= -1.0;
                    circle.position.y = height - circle.radius;
                }

                // Collision detection (other circles)
                for (Circle &otherCircle : circles) {
                    if (&otherCircle == &circle) {
                        continue;
                    }
                    float distance = hypot(otherCircle.position - circle.position);
                    sf::Vector2f direction = normalized(otherCircle.position - circle.position);
                    
                    float radiiSum = circle.radius + otherCircle.radius;
                    if (distance < radiiSum){
                        circle.velocity = reflect(-circle.velocity, direction);
                        float goBackDistance = (radiiSum - distance) / 2.0f;
                        otherCircle.position += direction * goBackDistance;
                        circle.position -= direction * goBackDistance;
                    }
                }   
            }

            // Draw the circle
            circle.circleshape.setPosition(circle.position);
            window.draw(circle.circleshape);
        }
        window.display();
    }

    return 0;
}