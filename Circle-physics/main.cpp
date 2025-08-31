#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <cmath>

struct Circle {
    float radius = 30;
    float gravityForce = 1;
    float bounciness = 1.0f;
    
    sf::Vector2f position;
    sf::Vector2f velocity;

    sf::CircleShape shape;

    Circle() : shape(radius) {
        shape.setFillColor(sf::Color::White);
        shape.setPosition(position);
    }

};

sf::Vector2f operator/(sf::Vector2f v, float scalar) {
    return sf::Vector2f(v.x / scalar, v.y / scalar);
}
sf::Vector2f operator*(sf::Vector2f v, float scalar) {
    return sf::Vector2f(v.x * scalar, v.y * scalar);
}

float magnitude(sf::Vector2f vector) {
    float mag = sqrt(pow(vector.x, 2) + pow(vector.y, 2));
    return mag;
}


int randomNumber(int min, int max);
void physics();
void onCollision();

int main() {
    sf::Vector2f screenSize(1600, 900);
    sf::RenderWindow window(sf::VideoMode(screenSize.x, screenSize.y), "Circles colliding");
    window.setFramerateLimit(60);

    std::vector<Circle> circles;
    for  (int i = 0; i < 40; i++)
    {
        Circle c;  // Create a Circle object
        c.position = {randomNumber(0, screenSize.x), randomNumber(0, screenSize.y)};
        c.velocity = {randomNumber(-2, 2), randomNumber(-2, 2)};
        c.shape.setPosition(c.position);

        circles.push_back(c);
    }
    circles[0].shape.setFillColor(sf::Color::Green);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
            {
                window.close();
            }
        }

        // Render
        window.clear();
        for (auto& circle : circles) {
            circle.shape.setPosition(circle.position);
            window.draw(circle.shape);
        }
        window.display();

        // Physics
        bool toSpawnCircle = false;
        for (auto& circle : circles) {
            sf::Vector2f newPosition = circle.position;
            // Calculate speed
            float speed = magnitude(circle.velocity);

            // Wall Collision
            if (circle.position.x < 0) // left wall 
            {
                newPosition.x = 0;
                circle.velocity.x = abs(circle.velocity.x) * circle.bounciness;
            }
            if (circle.position.x > screenSize.x - 2 * circle.radius) // right wall
            {
                newPosition.x = screenSize.x - 2 * circle.radius;
                circle.velocity.x = -abs(circle.velocity.x) * circle.bounciness;
            }

            if (circle.position.y < 0) // upper wall
            {
                newPosition.y = 0;
                circle.velocity.y = abs(circle.velocity.y) * circle.bounciness;
            }
            if (circle.position.y > screenSize.y - 2 * circle.radius) // lower wall
            {
                newPosition.y = screenSize.y - 2 * circle.radius;
                circle.velocity.y = -abs(circle.velocity.y) * circle.bounciness;
            }

            // Circle collision
            for (auto& otherCircle : circles) {
                
                float combinedRadius = circle.radius + otherCircle.radius;
                sf::Vector2f direction = otherCircle.position - circle.position;
                float distance = magnitude(direction);
                
                if (distance == 0)
                {
                    break;
                }

                if (combinedRadius > distance) {
                    // Spawn a new circle
                    toSpawnCircle = true;
                    
                    // Bounce back
                    circle.velocity = -direction * speed * circle.bounciness / distance;
                    otherCircle.velocity = direction * speed * otherCircle.bounciness / distance;           

                    // Avoid clipping into eachother
                    sf::Vector2f correction = (direction / distance) * (combinedRadius - distance) / 2.f;
                    newPosition -= correction;

                }

            }

            // Gravity
            circle.velocity.y += circle.gravityForce;

            // Manual movement
            float acceleration = 1.2;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
                circle.velocity.x -= acceleration;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                circle.velocity.x += acceleration;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
                circle.velocity.y -= acceleration;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                circle.velocity.y += acceleration;
            }
            // Movement
            newPosition += circle.velocity;
            circle.position = newPosition;

            // Finally update the position
            circle.shape.setPosition(circle.position);

        }

        // if (toSpawnCircle)
        // {

        //     Circle c;  // Create a Circle object
        //     c.position = {screenSize.x / 2, screenSize.y / 2};
        //     c.velocity = {randomNumber(-2, 2), randomNumber(-2, 2)};
        //     c.shape.setPosition(c.position);

        //     circles.push_back(c);
        // }
        toSpawnCircle = false;
    }

    return 0;
}

int randomNumber(int min, int max) {
    return (rand() % (max + min + 1)) - min; 
}